/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "render_post_process_sr_node.h"

#include <base/containers/string.h>
#include <base/math/vector.h>
#include <core/property_tools/property_api_impl.inl>
#include <render/datastore/intf_render_data_store_manager.h>
#include <render/device/intf_gpu_resource_manager.h>
#include <render/device/intf_shader_manager.h>
#include <render/nodecontext/intf_node_context_descriptor_set_manager.h>
#include <render/nodecontext/intf_pipeline_descriptor_set_binder.h>
#include <render/nodecontext/intf_render_command_list.h>
#include <render/nodecontext/intf_render_node_context_manager.h>

#include "util/log.h"

using namespace BASE_NS;
using namespace CORE_NS;
using namespace RENDER_NS;

CORE_BEGIN_NAMESPACE()
DATA_TYPE_METADATA(RenderPostProcessSrNode::NodeInputs,
    MEMBER_PROPERTY(input, "Input", BindableImage {}),
    MEMBER_PROPERTY(depth, "Depth", BindableImage {}))

DATA_TYPE_METADATA(RenderPostProcessSrNode::NodeOutputs,
    MEMBER_PROPERTY(output, "Output", BindableImage {}))
CORE_END_NAMESPACE()

RENDER_BEGIN_NAMESPACE()

namespace {
// Shader resource names for spatial upscaling
constexpr string_view SR_EDGE_DETECTION_SHADER = "sr_edge_detection.comp";
constexpr string_view SR_SPATIAL_UPSCALE_SHADER = "sr_spatial_upscale.comp";
constexpr string_view SR_SHARPENING_SHADER = "sr_sharpening.comp";
constexpr string_view SR_OUTPUT_SHADER = "sr_output.comp";

// Fallback vertex/fragment shaders for older GLES
constexpr string_view SR_SPATIAL_UPSCALE_VERT = "sr_spatial_upscale.vert";
constexpr string_view SR_SPATIAL_UPSCALE_FRAG = "sr_spatial_upscale.frag";

constexpr string_view SAMPLER_LINEAR_CLAMP = "RENDER_CORE_SAMPLER_LINEAR_CLAMP";
constexpr string_view SAMPLER_NEAREST_CLAMP = "RENDER_CORE_SAMPLER_NEAREST_CLAMP";

// Thread group sizes optimized for different GPU architectures
constexpr uint32_t THREAD_GROUP_SIZE_X = 8U;
constexpr uint32_t THREAD_GROUP_SIZE_Y = 8U;
} // namespace

RenderPostProcessSrNode::RenderPostProcessSrNode()
    : inputProperties_(&nodeInputsData,
          array_view(PropertyType::DataType<NodeInputs>::properties)),
      outputProperties_(&nodeOutputsData,
          array_view(PropertyType::DataType<NodeOutputs>::properties))
{}

CORE_NS::IPropertyHandle* RenderPostProcessSrNode::GetRenderInputProperties()
{
    return inputProperties_.GetData();
}

CORE_NS::IPropertyHandle* RenderPostProcessSrNode::GetRenderOutputProperties()
{
    return outputProperties_.GetData();
}

DescriptorCounts RenderPostProcessSrNode::GetRenderDescriptorCounts() const
{
    // Each pass typically needs:
    // - Combined image samplers for inputs
    // - Storage images for outputs
    // - Uniform buffers for parameters
    return DescriptorCounts {
        .samplerCount = 4U,              // Linear and nearest samplers
        .combinedImageSamplerCount = 8U, // Input textures for each pass
        .sampledImageCount = 0U,
        .storageImageCount = 8U,         // Output textures for each pass
        .uniformBufferCount = 4U,        // Parameter buffers for each pass
        .storageBufferCount = 0U,
    };
}

void RenderPostProcessSrNode::SetRenderAreaRequest(const RenderAreaRequest& renderAreaRequest)
{
    renderAreaRequest_ = renderAreaRequest;
    useRequestedRenderArea_ = true;
}

IRenderNode::ExecuteFlags RenderPostProcessSrNode::GetExecuteFlags() const
{
    if (!effectProperties_.enabled) {
        return IRenderNode::ExecuteFlagBits::EXECUTE_FLAG_BITS_DO_NOT_EXECUTE;
    }
    return IRenderNode::ExecuteFlagBits::EXECUTE_FLAG_BITS_DEFAULT;
}

void RenderPostProcessSrNode::Init(const IRenderPostProcess::Ptr& postProcess,
    IRenderNodeContextManager& renderNodeContextMgr)
{
    renderNodeContextMgr_ = &renderNodeContextMgr;
    postProcess_ = postProcess;

    const auto& gpuResourceMgr = renderNodeContextMgr_->GetGpuResourceManager();
    
    // Create persistent sampler for texture sampling
    {
        GpuSamplerDesc samplerDesc {};
        samplerDesc.magFilter = Filter::CORE_FILTER_LINEAR;
        samplerDesc.minFilter = Filter::CORE_FILTER_LINEAR;
        samplerDesc.mipMapMode = SamplerMipmapMode::CORE_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerDesc.addressModeU = SamplerAddressMode::CORE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerDesc.addressModeV = SamplerAddressMode::CORE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerDesc.addressModeW = SamplerAddressMode::CORE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        
        samplerHandle_ = gpuResourceMgr.Create(samplerDesc);
    }

    CreatePsos();
    valid_ = true;
}

void RenderPostProcessSrNode::PreExecute()
{
    if (!valid_ || !effectProperties_.enabled) {
        return;
    }

    // Calculate output size based on upscaling ratio
    if (RenderHandleUtil::IsValid(nodeInputsData.input.handle)) {
        const auto& gpuResourceMgr = renderNodeContextMgr_->GetGpuResourceManager();
        const GpuImageDesc inputDesc = gpuResourceMgr.GetImageDescriptor(nodeInputsData.input.handle);
        
        inputSize_ = { inputDesc.width, inputDesc.height };
        outputSize_ = {
            static_cast<uint32_t>(inputSize_.x * effectProperties_.upscaleRatio),
            static_cast<uint32_t>(inputSize_.y * effectProperties_.upscaleRatio)
        };

        CreateTargets(inputSize_, outputSize_);
    }

    EvaluateOutput();
}

void RenderPostProcessSrNode::Execute(IRenderCommandList& cmdList)
{
    if (!valid_ || !effectProperties_.enabled) {
        return;
    }

    const PushConstant pc {
        .firstSet = 0U,
        .byteSize = sizeof(EdgeDetectionPushConstant),
    };

    // Multi-pass spatial upscaling pipeline
    
    // Pass 1: Edge Detection (if depth available and enabled)
    if (RenderHandleUtil::IsValid(nodeInputsData.depth.handle) && effectProperties_.useDepthEdges) {
        ComputeEdgeDetection(pc, cmdList);
    }

    // Pass 2: Main Spatial Upscaling
    ComputeSpatialUpscale(pc, cmdList);

    // Pass 3: Sharpening (if enabled)
    if (effectProperties_.enableSharpening) {
        ComputeSharpening(pc, cmdList);
    }

    // Pass 4: Final Output
    ComputeOutput(pc, cmdList);
}

void RenderPostProcessSrNode::ComputeEdgeDetection(const PushConstant& pc, IRenderCommandList& cmdList)
{
    EdgeDetectionPushConstant pushConstant {};
    pushConstant.inputSizeInvSize = {
        static_cast<float>(inputSize_.x), static_cast<float>(inputSize_.y),
        1.0f / static_cast<float>(inputSize_.x), 1.0f / static_cast<float>(inputSize_.y)
    };
    pushConstant.edgeSensitivity = effectProperties_.edgeSensitivity;
    pushConstant.useDepthEdges = effectProperties_.useDepthEdges ? 1U : 0U;
    pushConstant.depthSensitivity = 0.1f; // Fixed depth sensitivity

    const auto groupCountX = (inputSize_.x + psos_.edgeDetectionTGS.x - 1U) / psos_.edgeDetectionTGS.x;
    const auto groupCountY = (inputSize_.y + psos_.edgeDetectionTGS.y - 1U) / psos_.edgeDetectionTGS.y;

    cmdList.BeginRenderPass(RenderPassBeginInfo {}, RenderPassSubpassBeginInfo {});
    cmdList.BindPipeline(psos_.edgeDetection);
    
    // Update push constant data
    cmdList.PushConstant(pc, array_view(reinterpret_cast<const uint8_t*>(&pushConstant), sizeof(pushConstant)));
    
    if (binders_.edgeDetection) {
        binders_.edgeDetection->Bind(cmdList);
    }
    
    cmdList.Dispatch(groupCountX, groupCountY, 1U);
    cmdList.EndRenderPass();
}

void RenderPostProcessSrNode::ComputeSpatialUpscale(const PushConstant& pc, IRenderCommandList& cmdList)
{
    SpatialUpscalePushConstant pushConstant {};
    pushConstant.inputSizeInvSize = {
        static_cast<float>(inputSize_.x), static_cast<float>(inputSize_.y),
        1.0f / static_cast<float>(inputSize_.x), 1.0f / static_cast<float>(inputSize_.y)
    };
    pushConstant.outputSizeInvSize = {
        static_cast<float>(outputSize_.x), static_cast<float>(outputSize_.y),
        1.0f / static_cast<float>(outputSize_.x), 1.0f / static_cast<float>(outputSize_.y)
    };
    pushConstant.upscaleRatio = effectProperties_.upscaleRatio;
    pushConstant.qualityLevel = effectProperties_.qualityLevel;
    pushConstant.edgeThreshold = 0.1f;

    const auto groupCountX = (outputSize_.x + psos_.spatialUpscaleTGS.x - 1U) / psos_.spatialUpscaleTGS.x;
    const auto groupCountY = (outputSize_.y + psos_.spatialUpscaleTGS.y - 1U) / psos_.spatialUpscaleTGS.y;

    cmdList.BeginRenderPass(RenderPassBeginInfo {}, RenderPassSubpassBeginInfo {});
    cmdList.BindPipeline(psos_.spatialUpscale);
    
    cmdList.PushConstant(pc, array_view(reinterpret_cast<const uint8_t*>(&pushConstant), sizeof(pushConstant)));
    
    if (binders_.spatialUpscale) {
        binders_.spatialUpscale->Bind(cmdList);
    }
    
    cmdList.Dispatch(groupCountX, groupCountY, 1U);
    cmdList.EndRenderPass();
}

void RenderPostProcessSrNode::ComputeSharpening(const PushConstant& pc, IRenderCommandList& cmdList)
{
    SharpeningPushConstant pushConstant {};
    pushConstant.outputSizeInvSize = {
        static_cast<float>(outputSize_.x), static_cast<float>(outputSize_.y),
        1.0f / static_cast<float>(outputSize_.x), 1.0f / static_cast<float>(outputSize_.y)
    };
    pushConstant.sharpeningStrength = effectProperties_.sharpeningStrength;
    pushConstant.enableAdaptiveSharpening = 1U;
    pushConstant.contrastThreshold = 0.2f;

    const auto groupCountX = (outputSize_.x + psos_.sharpeningTGS.x - 1U) / psos_.sharpeningTGS.x;
    const auto groupCountY = (outputSize_.y + psos_.sharpeningTGS.y - 1U) / psos_.sharpeningTGS.y;

    cmdList.BeginRenderPass(RenderPassBeginInfo {}, RenderPassSubpassBeginInfo {});
    cmdList.BindPipeline(psos_.sharpening);
    
    cmdList.PushConstant(pc, array_view(reinterpret_cast<const uint8_t*>(&pushConstant), sizeof(pushConstant)));
    
    if (binders_.sharpening) {
        binders_.sharpening->Bind(cmdList);
    }
    
    cmdList.Dispatch(groupCountX, groupCountY, 1U);
    cmdList.EndRenderPass();
}

void RenderPostProcessSrNode::ComputeOutput(const PushConstant& pc, IRenderCommandList& cmdList)
{
    // Final output pass - copy processed result to output target
    renderCopyOutput_.Execute(cmdList, targets_.finalColor, nodeOutputsData.output);
}

void RenderPostProcessSrNode::CreateTargets(const BASE_NS::Math::UVec2 inputSize, const BASE_NS::Math::UVec2 outputSize)
{
    const auto& gpuResourceMgr = renderNodeContextMgr_->GetGpuResourceManager();
    
    targets_.inputResolution = inputSize;
    targets_.outputResolution = outputSize;

    // Get input format for intermediate textures
    Format intermediateFormat = Format::BASE_FORMAT_R8G8B8A8_UNORM;
    if (RenderHandleUtil::IsValid(nodeInputsData.input.handle)) {
        const GpuImageDesc inputDesc = gpuResourceMgr.GetImageDescriptor(nodeInputsData.input.handle);
        intermediateFormat = inputDesc.format;
    }

    // Edge detection outputs
    {
        const GpuImageDesc desc {
            .imageType = ImageType::CORE_IMAGE_TYPE_2D,
            .format = Format::BASE_FORMAT_R8_UNORM,
            .imageTiling = ImageTiling::CORE_IMAGE_TILING_OPTIMAL,
            .usageFlags = ImageUsageFlagBits::CORE_IMAGE_USAGE_STORAGE_BIT |
                         ImageUsageFlagBits::CORE_IMAGE_USAGE_SAMPLED_BIT,
            .memoryPropertyFlags = MemoryPropertyFlagBits::CORE_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .width = inputSize.x,
            .height = inputSize.y,
            .depth = 1U,
            .mipCount = 1U,
            .layerCount = 1U,
        };
        targets_.edgeMask = gpuResourceMgr.Create(desc);
    }

    {
        const GpuImageDesc desc {
            .imageType = ImageType::CORE_IMAGE_TYPE_2D,
            .format = Format::BASE_FORMAT_R16G16_SFLOAT,
            .imageTiling = ImageTiling::CORE_IMAGE_TILING_OPTIMAL,
            .usageFlags = ImageUsageFlagBits::CORE_IMAGE_USAGE_STORAGE_BIT |
                         ImageUsageFlagBits::CORE_IMAGE_USAGE_SAMPLED_BIT,
            .memoryPropertyFlags = MemoryPropertyFlagBits::CORE_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .width = inputSize.x,
            .height = inputSize.y,
            .depth = 1U,
            .mipCount = 1U,
            .layerCount = 1U,
        };
        targets_.gradientInfo = gpuResourceMgr.Create(desc);
    }

    // Intermediate upscaling buffers
    {
        const GpuImageDesc desc {
            .imageType = ImageType::CORE_IMAGE_TYPE_2D,
            .format = intermediateFormat,
            .imageTiling = ImageTiling::CORE_IMAGE_TILING_OPTIMAL,
            .usageFlags = ImageUsageFlagBits::CORE_IMAGE_USAGE_STORAGE_BIT |
                         ImageUsageFlagBits::CORE_IMAGE_USAGE_SAMPLED_BIT,
            .memoryPropertyFlags = MemoryPropertyFlagBits::CORE_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .width = outputSize.x,
            .height = outputSize.y,
            .depth = 1U,
            .mipCount = 1U,
            .layerCount = 1U,
        };
        targets_.intermediateColor = gpuResourceMgr.Create(desc);
        targets_.tempBuffer = gpuResourceMgr.Create(desc);
        targets_.sharpenedColor = gpuResourceMgr.Create(desc);
    }

    // Parameter uniform buffer
    {
        const GpuBufferDesc desc {
            .bufferUsageFlags = BufferUsageFlagBits::CORE_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .memoryPropertyFlags = MemoryPropertyFlagBits::CORE_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  MemoryPropertyFlagBits::CORE_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .byteSize = sizeof(EffectProperties),
        };
        targets_.parameterUbo = gpuResourceMgr.Create(desc);
    }

    // Set final output target
    targets_.finalColor = effectProperties_.enableSharpening ? targets_.sharpenedColor : targets_.intermediateColor;
}

void RenderPostProcessSrNode::CreatePsos()
{
    const auto& shaderMgr = renderNodeContextMgr_->GetShaderManager();
    
    // Create compute pipeline state objects
    {
        const ShaderSpecilizationConstant specialization {};
        psos_.edgeDetection = shaderMgr.GetComputePipelineHandle(SR_EDGE_DETECTION_SHADER, specialization);
        psos_.spatialUpscale = shaderMgr.GetComputePipelineHandle(SR_SPATIAL_UPSCALE_SHADER, specialization);
        psos_.sharpening = shaderMgr.GetComputePipelineHandle(SR_SHARPENING_SHADER, specialization);
        psos_.output = shaderMgr.GetComputePipelineHandle(SR_OUTPUT_SHADER, specialization);
    }

    // Set thread group sizes
    psos_.edgeDetectionTGS = { THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1U };
    psos_.spatialUpscaleTGS = { THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1U };
    psos_.sharpeningTGS = { THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1U };
    psos_.outputTGS = { THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1U };
}

void RenderPostProcessSrNode::EvaluateOutput()
{
    // Update parameters and prepare for rendering
    if (RenderHandleUtil::IsValid(targets_.parameterUbo)) {
        const auto& gpuResourceMgr = renderNodeContextMgr_->GetGpuResourceManager();
        auto* mappedData = static_cast<EffectProperties*>(gpuResourceMgr.MapBuffer(targets_.parameterUbo));
        if (mappedData) {
            *mappedData = effectProperties_;
            gpuResourceMgr.UnmapBuffer(targets_.parameterUbo);
        }
    }
}

RENDER_END_NAMESPACE()