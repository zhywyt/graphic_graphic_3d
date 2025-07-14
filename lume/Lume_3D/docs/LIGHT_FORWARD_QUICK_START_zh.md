# LIGHT_FORWARD 后处理快速入门指南

## 快速设置

```cpp
#include <3d/ecs/components/camera_component.h>
#include <render/datastore/render_data_store_render_pods.h>

// 1. 配置相机使用 LIGHT_FORWARD 管线
RenderCamera camera;
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;
camera.postProcessName = "CORE3D_POST_PROCESS_CAM";

// 2. 配置兼容的后处理效果
PostProcessConfiguration ppConfig = {};

// 启用泛光 + 色调映射 + FXAA（均与 LIGHT_FORWARD 兼容）
ppConfig.flags.x = PostProcessConfiguration::ENABLE_BLOOM_BIT | 
                   PostProcessConfiguration::ENABLE_TONEMAP_BIT |
                   PostProcessConfiguration::ENABLE_FXAA_BIT;

// 设置效果参数
ppConfig.factors[PostProcessConfiguration::INDEX_BLOOM] = 1.0f;     // 泛光强度
ppConfig.factors[PostProcessConfiguration::INDEX_TONEMAP] = 1.0f;   // 曝光度
ppConfig.factors[PostProcessConfiguration::INDEX_FXAA] = 0.75f;     // FXAA 质量

// 3. 管线会自动处理：
//    - HDR 渲染目标创建
//    - 正确的 MSAA 解析
//    - 兼容效果验证
```

## 兼容效果清单

✅ **支持 LIGHT_FORWARD：**
- 色调映射、暗角、抖动
- 颜色转换、色彩边缘  
- 模糊、泛光、FXAA

❌ **需要 FORWARD/DEFERRED：**
- TAA（时域抗锯齿）
- 运动模糊、景深
- 镜头光晕

## 验证输出

**成功：**
```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline using current-frame-only post-processing (flags: 0x300)
```

**警告（不兼容效果）：**
```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline has incompatible post-processing effects (enabled: 0x1800, incompatible: 0x1000)
```

完整文档请参见 [LIGHT_FORWARD_POST_PROCESSING_zh.md](LIGHT_FORWARD_POST_PROCESSING_zh.md)