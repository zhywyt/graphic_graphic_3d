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

#ifndef RENDER_POSTPROCESS_RENDER_POSTPROCESS_SR_NODE_H
#define RENDER_POSTPROCESS_RENDER_POSTPROCESS_SR_NODE_H

#include <array>

#include <base/containers/unique_ptr.h>
#include <base/math/matrix_util.h>
#include <base/util/uid.h>
#include <core/plugin/intf_interface.h>
#include <core/plugin/intf_interface_helper.h>
#include <core/property/intf_property_api.h>
#include <core/property/intf_property_handle.h>
#include <core/property/property_types.h>
#include <core/property_tools/property_api_impl.h>
#include <core/property_tools/property_macros.h>
#include <render/device/intf_shader_manager.h>
#include <render/namespace.h>
#include <render/nodecontext/intf_pipeline_descriptor_set_binder.h>
#include <render/nodecontext/intf_render_node.h>
#include <render/nodecontext/intf_render_post_process.h>
#include <render/nodecontext/intf_render_post_process_node.h>
#include <render/property/property_types.h>
#include <render/render_data_structures.h>

#include "nodecontext/render_node_copy_util.h"

RENDER_BEGIN_NAMESPACE()

/**
 * @brief Spatial GPU upscaling post-process node implementation
 * 
 * Implements the core spatial upscaling algorithms using compute shaders.
 * Supports multiple quality levels and configurable parameters.
 */
class RenderPostProcessSrNode : public CORE_NS::IInterfaceHelper<RENDER_NS::IRenderPostProcessNode> {
public:
    static constexpr auto UID = BASE_NS::Uid("b2c3d4e5-f6a7-8901-bcde-f23456789012");

    using Ptr = BASE_NS::refcnt_ptr<RenderPostProcessSrNode>;

    /**
     * @brief Spatial upscaling configuration parameters
     */
    struct EffectProperties {
        /** Enable/disable spatial upscaling effect */
        bool enabled { true };
        
        /** Target upscaling ratio (1.5x - 4.0x) */
        float upscaleRatio { 2.0f };
        
        /** Edge detection sensitivity (0.0 - 1.0) */
        float edgeSensitivity { 0.5f };
        
        /** Post-upscaling sharpening strength (0.0 - 1.0) */
        float sharpeningStrength { 0.3f };
        
        /** Quality level: 0=Performance, 1=Quality, 2=Ultra */
        uint32_t qualityLevel { 1U };
        
        /** Use depth buffer for enhanced edge detection */
        bool useDepthEdges { true };
        
        /** Enable post-process sharpening pass */
        bool enableSharpening { true };
        
        /** Adaptive quality based on GPU performance */
        bool adaptiveQuality { false };
    };

    RenderPostProcessSrNode();
    ~RenderPostProcessSrNode() = default;

    CORE_NS::IPropertyHandle* GetRenderInputProperties() override;
    CORE_NS::IPropertyHandle* GetRenderOutputProperties() override;
    RENDER_NS::DescriptorCounts GetRenderDescriptorCounts() const override;
    void SetRenderAreaRequest(const RenderAreaRequest& renderAreaRequest) override;

    RENDER_NS::IRenderNode::ExecuteFlags GetExecuteFlags() const override;
    void Init(const RENDER_NS::IRenderPostProcess::Ptr& postProcess,
        RENDER_NS::IRenderNodeContextManager& renderNodeContextMgr) override;
    void PreExecute() override;
    void Execute(RENDER_NS::IRenderCommandList& cmdList) override;

    /**
     * @brief Input image bindings for spatial upscaling
     */
    struct NodeInputs {
        /** Source low-resolution color image */
        RENDER_NS::BindableImage input;
        /** Optional depth buffer for edge-aware processing */
        RENDER_NS::BindableImage depth;
    };

    /**
     * @brief Output image bindings for spatial upscaling
     */
    struct NodeOutputs {
        /** Target high-resolution upscaled image */
        RENDER_NS::BindableImage output;
    };

    NodeInputs nodeInputsData;
    NodeOutputs nodeOutputsData;

private:
    RENDER_NS::IRenderNodeContextManager* renderNodeContextMgr_ { nullptr };
    RENDER_NS::IRenderPostProcess::Ptr postProcess_;

    // Spatial upscaling compute passes
    void ComputeEdgeDetection(const PushConstant& pc, RENDER_NS::IRenderCommandList& cmdList);
    void ComputeSpatialUpscale(const PushConstant& pc, RENDER_NS::IRenderCommandList& cmdList);
    void ComputeSharpening(const PushConstant& pc, RENDER_NS::IRenderCommandList& cmdList);
    void ComputeOutput(const PushConstant& pc, RENDER_NS::IRenderCommandList& cmdList);

    void CreateTargets(const BASE_NS::Math::UVec2 inputSize, const BASE_NS::Math::UVec2 outputSize);
    void CreatePsos();

    BASE_NS::Math::UVec2 inputSize_ { 0U, 0U };
    BASE_NS::Math::UVec2 outputSize_ { 0U, 0U };

    static constexpr uint32_t INTERMEDIATE_TARGET_COUNT { 4U };

    /**
     * @brief Intermediate render targets for multi-pass processing
     */
    struct Targets {
        /** Input resolution */
        BASE_NS::Math::UVec2 inputResolution { 0U, 0U };
        /** Output resolution */
        BASE_NS::Math::UVec2 outputResolution { 0U, 0U };

        // Edge detection pass textures
        RenderHandleReference edgeMask;          // R8_UNORM edge mask
        RenderHandleReference gradientInfo;      // RG16F gradient information

        // Spatial upscaling intermediate textures
        RenderHandleReference intermediateColor; // Same format as input
        RenderHandleReference tempBuffer;        // Temporary processing buffer

        // Sharpening pass targets
        RenderHandleReference sharpenedColor;    // Final sharpened output

        // Uniform buffer for parameters
        RenderHandle parameterUbo;
    };
    Targets targets_;

    /**
     * @brief Pipeline state objects for each compute pass
     */
    struct PSOs {
        // Edge detection pass
        RenderHandle edgeDetection;
        
        // Main spatial upscaling pass  
        RenderHandle spatialUpscale;
        
        // Post-process sharpening pass
        RenderHandle sharpening;
        
        // Final output pass
        RenderHandle output;

        // Compute shader thread group sizes
        ShaderThreadGroup edgeDetectionTGS { 8, 8, 1 };
        ShaderThreadGroup spatialUpscaleTGS { 8, 8, 1 };
        ShaderThreadGroup sharpeningTGS { 8, 8, 1 };
        ShaderThreadGroup outputTGS { 8, 8, 1 };
    };
    PSOs psos_;

    /**
     * @brief Descriptor set binders for each pass
     */
    struct Binders {
        IDescriptorSetBinder::Ptr edgeDetection;
        IDescriptorSetBinder::Ptr spatialUpscale;
        IDescriptorSetBinder::Ptr sharpening;
        IDescriptorSetBinder::Ptr output;
    };
    Binders binders_;

    RenderHandleReference samplerHandle_;
    RenderNodeCopyUtil renderCopyOutput_;

    void EvaluateOutput();

    /**
     * @brief Push constants for edge detection pass
     */
    struct EdgeDetectionPushConstant {
        BASE_NS::Math::Vec4 inputSizeInvSize { 0.0f, 0.0f, 0.0f, 0.0f };
        float edgeSensitivity { 0.5f };
        uint32_t useDepthEdges { 1U };
        float depthSensitivity { 0.1f };
        uint32_t padding { 0U };
    };

    /**
     * @brief Push constants for spatial upscaling pass
     */
    struct SpatialUpscalePushConstant {
        BASE_NS::Math::Vec4 inputSizeInvSize { 0.0f, 0.0f, 0.0f, 0.0f };
        BASE_NS::Math::Vec4 outputSizeInvSize { 0.0f, 0.0f, 0.0f, 0.0f };
        float upscaleRatio { 2.0f };
        uint32_t qualityLevel { 1U };
        float edgeThreshold { 0.1f };
        uint32_t padding { 0U };
    };

    /**
     * @brief Push constants for sharpening pass
     */
    struct SharpeningPushConstant {
        BASE_NS::Math::Vec4 outputSizeInvSize { 0.0f, 0.0f, 0.0f, 0.0f };
        float sharpeningStrength { 0.3f };
        uint32_t enableAdaptiveSharpening { 1U };
        float contrastThreshold { 0.2f };
        uint32_t padding { 0U };
    };

    RenderAreaRequest renderAreaRequest_;
    bool useRequestedRenderArea_ { false };

    CORE_NS::PropertyApiImpl<NodeInputs> inputProperties_;
    CORE_NS::PropertyApiImpl<NodeOutputs> outputProperties_;

    bool valid_ { false };

    EffectProperties effectProperties_;
};
RENDER_END_NAMESPACE()

#endif // RENDER_POSTPROCESS_RENDER_POSTPROCESS_SR_NODE_H