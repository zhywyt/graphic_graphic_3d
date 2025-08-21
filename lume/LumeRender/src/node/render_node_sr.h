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

#ifndef RENDER_NODE_RENDER_NODE_SR_H
#define RENDER_NODE_RENDER_NODE_SR_H

#include <postprocesses/render_post_process_sr_node.h>

#include <base/util/uid.h>
#include <render/datastore/render_data_store_render_pods.h>
#include <render/namespace.h>
#include <render/nodecontext/intf_render_node.h>
#include <render/nodecontext/intf_render_post_process.h>
#include <render/nodecontext/intf_render_post_process_node.h>
#include <render/render_data_structures.h>

RENDER_BEGIN_NAMESPACE()
class IRenderCommandList;
class IRenderNodeContextManager;
class IPipelineDescriptorSetBinder;
class IRenderNodeRenderDataStoreManager;
struct RenderNodeGraphInputs;

/**
 * @brief Spatial GPU upscaling render node
 * 
 * Implements spatial domain super-resolution upscaling techniques
 * for real-time rendering. Supports both GLES and Vulkan backends.
 * 
 * Features:
 * - Edge-aware spatial interpolation
 * - Multi-pass sharpening and enhancement
 * - Configurable quality levels (Performance/Quality/Ultra)
 * - Optional depth-based edge detection
 * - Adaptive upscaling ratios (1.5x - 4.0x)
 */
class RenderNodeSr final : public IRenderNode {
public:
    RenderNodeSr() = default;
    ~RenderNodeSr() override = default;

    void InitNode(IRenderNodeContextManager& renderNodeContextMgr) override;
    void PreExecuteFrame() override;
    void ExecuteFrame(IRenderCommandList& cmdList) override;

    ExecuteFlags GetExecuteFlags() const override;

    // for plugin / factory interface
    static constexpr BASE_NS::Uid UID { "a1b2c3d4-e5f6-7890-abcd-ef1234567890" };
    static constexpr const char* TYPE_NAME = "RenderNodeSr";
    static constexpr IRenderNode::BackendFlags BACKEND_FLAGS = IRenderNode::BackendFlagBits::BACKEND_FLAG_BITS_DEFAULT;
    static constexpr IRenderNode::ClassType CLASS_TYPE = IRenderNode::ClassType::CLASS_TYPE_NODE;
    static IRenderNode* Create();
    static void Destroy(IRenderNode* instance);

private:
    IRenderNodeContextManager* renderNodeContextMgr_ { nullptr };

    void ParseRenderNodeInputs();
    void CreatePostProcessInterface();
    void ProcessPostProcessConfiguration(const IRenderNodeRenderDataStoreManager& dataStoreMgr);

    // Json resources which might need re-fetching
    struct JsonInputs {
        RenderNodeGraphInputs::InputResources resources;
        RenderNodeGraphInputs::RenderDataStore renderDataStore;

        bool hasChangeableResourceHandles { false };
    };
    JsonInputs jsonInputs_;
    RenderNodeHandles::InputRenderPass inputRenderPass_;
    RenderNodeHandles::InputResources inputResources_;

    struct PostProcessInterfaces {
        IRenderPostProcess::Ptr postProcess;
        IRenderPostProcessNode::Ptr postProcessNode;
    };
    PostProcessInterfaces ppRenderSrInterface_;

    PostProcessConfiguration ppConfig_;

    bool valid_ { false };
};
RENDER_END_NAMESPACE()

#endif // RENDER_NODE_RENDER_NODE_SR_H