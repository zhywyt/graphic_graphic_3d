# LIGHT_FORWARD 管线后处理指南

## 概述

LIGHT_FORWARD 渲染管线现在支持仅依赖当前帧颜色附件的后处理效果。这使得轻量级管线能够使用常见效果如泛光、色调映射和 FXAA，同时在不需要 FORWARD 或 DEFERRED 管线完整功能集的场景中保持优异性能。

## 什么是 LIGHT_FORWARD 管线？

LIGHT_FORWARD 是一个轻量级渲染管线，专为以下场景优化：
- **性能**：简单渲染场景的最小开销
- **内存效率**：不使用后处理时直接渲染到后缓冲区
- **简洁性**：相比完整的前向或延迟管线降低复杂度

## 支持的后处理效果

以下后处理效果与 LIGHT_FORWARD 管线**兼容**，因为它们只需要当前帧颜色数据：

### ✅ **颜色处理效果**
- **色调映射** (`ENABLE_TONEMAP_BIT`) - HDR 到 LDR 色调映射
- **暗角** (`ENABLE_VIGNETTE_BIT`) - 图像边缘变暗效果
- **抖动** (`ENABLE_DITHER_BIT`) - 基于噪声的抖动减少色带
- **颜色转换** (`ENABLE_COLOR_CONVERSION_BIT`) - 色彩空间转换
- **色彩边缘** (`ENABLE_COLOR_FRINGE_BIT`) - 色差效果

### ✅ **图像增强效果**
- **模糊** (`ENABLE_BLUR_BIT`) - 高斯模糊效果
- **泛光** (`ENABLE_BLOOM_BIT`) - HDR 泛光/发光效果
- **FXAA** (`ENABLE_FXAA_BIT`) - 快速近似抗锯齿

## 不支持的后处理效果

以下效果与 LIGHT_FORWARD **不兼容**，因为它们需要额外的渲染数据：

### ❌ **时域效果** 
- **TAA** (`ENABLE_TAA_BIT`) - 需要前一帧数据进行时域累积
- **运动模糊** (`ENABLE_MOTION_BLUR_BIT`) - 需要物体运动的速度向量

### ❌ **深度相关效果**
- **景深** (`ENABLE_DOF_BIT`) - 需要深度缓冲区信息进行焦点计算

### ❌ **复杂光照效果**
- **镜头光晕** (`ENABLE_LENS_FLARE_BIT`) - 需要额外光照数据

## 如何使用带后处理的 LIGHT_FORWARD

### 基本设置

```cpp
// 配置相机使用带后处理的 LIGHT_FORWARD
RenderCamera camera;
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;
camera.postProcessName = "CORE3D_POST_PROCESS_CAM"; // 您的后处理配置名称

// 管线会自动检测兼容的后处理效果
// 并在需要时启用合适的渲染目标
```

### 后处理配置

使用标准后处理配置来配置您的后处理效果：

```cpp
// 示例：启用泛光和色调映射（均与 LIGHT_FORWARD 兼容）
PostProcessConfiguration config = {};
config.flags.x = PostProcessConfiguration::ENABLE_BLOOM_BIT | 
                 PostProcessConfiguration::ENABLE_TONEMAP_BIT |
                 PostProcessConfiguration::ENABLE_FXAA_BIT;

// 设置泛光参数
config.factors[PostProcessConfiguration::INDEX_BLOOM] = bloomStrength;

// 设置色调映射参数
config.factors[PostProcessConfiguration::INDEX_TONEMAP] = tonemapExposure;
```

### 使用 LIGHT_FORWARD 进行 HDR 渲染

当检测到兼容的后处理时，LIGHT_FORWARD 会自动：

1. **创建 HDR 渲染目标** 而不是直接渲染到后缓冲区
2. **启用自定义渲染分辨率** 用于后处理链
3. **正确处理 MSAA** 在需要时创建颜色解析目标

```cpp
// HDR 泛光示例 - 与 LIGHT_FORWARD 自动兼容
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;
camera.postProcessName = "bloom_postprocess";

// 管线检测泛光效果并自动：
// - 创建 HDR 颜色目标
// - 启用渲染分辨率支持
// - 在后处理链中处理泛光
```

## 验证和调试

实现包含全面验证以帮助开发者：

### 成功消息

当 LIGHT_FORWARD 后处理正常工作时，您会看到：

```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline using current-frame-only post-processing (flags: 0x300)
```

### 警告消息

如果您尝试在 LIGHT_FORWARD 中使用不兼容的效果：

```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline has incompatible post-processing effects (enabled: 0x1800, incompatible: 0x1000)
```

这告诉您：
- `enabled: 0x1800` - 所有启用的后处理标志
- `incompatible: 0x1000` - 特定不兼容的标志（例如 TAA）

### 故障排除

**问题**：后处理在 LIGHT_FORWARD 中不工作
- **检查**：确保您只使用兼容的效果（见上面支持列表）
- **检查**：验证后处理配置名称是否有效
- **检查**：查看调试输出中的验证警告

**问题**：性能比预期差
- **解决方案**：验证您没有意外使用导致管线回退的不兼容效果
- **解决方案**：检查是否不必要地创建了 HDR 目标

**问题**：MSAA 工作不正常
- **解决方案**：带后处理的 LIGHT_FORWARD 自动处理 MSAA 解析 - 无需额外配置

## 性能考虑

### 当后处理被禁用时
- **直接后缓冲区渲染** - 最大性能，无中间目标
- **最小开销** - 保持轻量级管线优势

### 当启用兼容后处理时
- **HDR 中间目标** - 仅在后处理需要时创建
- **自定义渲染分辨率** - 支持后处理的分辨率缩放
- **高效内存使用** - 目标根据启用的效果合理调整大小

### 从其他管线迁移

如果您当前仅为了后处理而使用 FORWARD 或 DEFERRED：

```cpp
// 之前：仅为后处理使用 FORWARD 管线
camera.renderPipelineType = RenderCamera::RenderPipelineType::FORWARD;
camera.postProcessName = "bloom_tonemap"; 

// 之后：使用 LIGHT_FORWARD 获得更好性能
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;  
camera.postProcessName = "bloom_tonemap"; // 相同效果，更好性能
```

## 实现细节

### 自动目标创建

LIGHT_FORWARD 管线自动检测后处理何时需要中间目标：

1. **分析启用的后处理标志** 使用 `IsCurrentFrameOnlyPostProcessing()`
2. **在需要时创建 HDR 目标** 用于需要高动态范围的效果如泛光
3. **启用渲染分辨率支持** 用于需要自定义缩放的后处理链
4. **保持向后兼容性** - 现有不带后处理的 LIGHT_FORWARD 使用保持不变

### 效果兼容性检测

```cpp
// 只需要当前帧颜色的效果（兼容）
constexpr uint32_t CURRENT_FRAME_ONLY_PP_FLAGS = 
    PostProcessConfiguration::ENABLE_TONEMAP_BIT |
    PostProcessConfiguration::ENABLE_VIGNETTE_BIT |
    PostProcessConfiguration::ENABLE_DITHER_BIT |
    PostProcessConfiguration::ENABLE_COLOR_CONVERSION_BIT |
    PostProcessConfiguration::ENABLE_COLOR_FRINGE_BIT |
    PostProcessConfiguration::ENABLE_BLUR_BIT |
    PostProcessConfiguration::ENABLE_BLOOM_BIT |
    PostProcessConfiguration::ENABLE_FXAA_BIT;
```

系统检查启用的效果是否是这些兼容标志的子集。任何不兼容的效果都会触发警告，可能导致后处理被跳过或需要不同的管线。

## 最佳实践

1. **新项目从 LIGHT_FORWARD 开始** 如果需要后处理
2. **优先使用兼容效果** - 在考虑 TAA 或景深之前先尝试泛光、色调映射、FXAA
3. **监控验证输出** 在开发过程中及早发现不兼容配置
4. **性能分析** 验证您获得了预期的优势
5. **考虑效果组合** - 多个兼容效果可以高效地一起使用

## 示例配置

### 电影渲染
```cpp
// 泛光 + 色调映射 + 暗角实现电影效果
config.flags.x = PostProcessConfiguration::ENABLE_BLOOM_BIT | 
                 PostProcessConfiguration::ENABLE_TONEMAP_BIT |
                 PostProcessConfiguration::ENABLE_VIGNETTE_BIT;
```

### 清晰图像质量
```cpp  
// FXAA + 抖动实现清晰抗锯齿输出
config.flags.x = PostProcessConfiguration::ENABLE_FXAA_BIT |
                 PostProcessConfiguration::ENABLE_DITHER_BIT;
```

### 风格化效果
```cpp
// 颜色处理实现风格化视觉效果
config.flags.x = PostProcessConfiguration::ENABLE_COLOR_CONVERSION_BIT |
                 PostProcessConfiguration::ENABLE_COLOR_FRINGE_BIT |
                 PostProcessConfiguration::ENABLE_BLUR_BIT;
```

这个 LIGHT_FORWARD 后处理支持为许多常见渲染场景提供了性能和视觉质量的绝佳平衡。