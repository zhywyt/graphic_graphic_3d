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

#include "render_post_process_sr.h"

#include <core/property_tools/property_api_impl.inl>

#include "util/log.h"

using namespace BASE_NS;
using namespace CORE_NS;
using namespace RENDER_NS;

CORE_BEGIN_NAMESPACE()
DATA_TYPE_METADATA(RenderPostProcessSrNode::EffectProperties,
    MEMBER_PROPERTY(enabled, "Enabled", true),
    MEMBER_PROPERTY(upscaleRatio, "Upscaling ratio", 2.0f),
    MEMBER_PROPERTY(edgeSensitivity, "Edge detection sensitivity", 0.5f),
    MEMBER_PROPERTY(sharpeningStrength, "Sharpening strength", 0.3f),
    MEMBER_PROPERTY(qualityLevel, "Quality level (0=Performance, 1=Quality, 2=Ultra)", 1U),
    MEMBER_PROPERTY(useDepthEdges, "Use depth buffer for edge detection", true),
    MEMBER_PROPERTY(enableSharpening, "Enable post-process sharpening", true),
    MEMBER_PROPERTY(adaptiveQuality, "Adaptive quality based on performance", false))
CORE_END_NAMESPACE()

RENDER_BEGIN_NAMESPACE()

namespace {
constexpr size_t PROPERTY_BYTE_SIZE { sizeof(RenderPostProcessSrNode::EffectProperties) };
}

RenderPostProcessSr::RenderPostProcessSr()
    : properties_(&propertiesData,
          array_view(PropertyType::DataType<RenderPostProcessSrNode::EffectProperties>::properties))
{}

CORE_NS::IPropertyHandle* RenderPostProcessSr::GetProperties()
{
    return properties_.GetData();
}

void RenderPostProcessSr::SetData(BASE_NS::array_view<const uint8_t> data)
{
    if (data.size_bytes() == PROPERTY_BYTE_SIZE) {
        BASE_NS::CloneData(&propertiesData, PROPERTY_BYTE_SIZE, data.data(), data.size_bytes());
        
        // Validate and clamp property values
        propertiesData.upscaleRatio = Math::clamp(propertiesData.upscaleRatio, 1.0f, 4.0f);
        propertiesData.edgeSensitivity = Math::clamp(propertiesData.edgeSensitivity, 0.0f, 1.0f);
        propertiesData.sharpeningStrength = Math::clamp(propertiesData.sharpeningStrength, 0.0f, 1.0f);
        propertiesData.qualityLevel = Math::min(propertiesData.qualityLevel, 2U);
    } else {
        PLUGIN_LOG_W("RenderPostProcessSr: trying to set data with invalid size (%zu vs %zu)",
            data.size_bytes(), PROPERTY_BYTE_SIZE);
    }
}

BASE_NS::array_view<const uint8_t> RenderPostProcessSr::GetData() const
{
    return array_view(reinterpret_cast<const uint8_t*>(&propertiesData), PROPERTY_BYTE_SIZE);
}

RENDER_END_NAMESPACE()