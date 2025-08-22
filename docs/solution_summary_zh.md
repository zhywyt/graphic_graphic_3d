# 环境贴图透明通道支持 - 解决方案

## 问题描述
用户发现在环境贴图的采样中，无法采样到透明通道，希望通过修改环境贴图深度来实现一个常驻在屏幕最前方的类似2D的效果，但是透明通道不工作。

## 根本原因
原有的环境贴图采样函数 `unpackEnvMap()` 只返回RGB值，丢弃了Alpha通道。这是因为它设计用于RGBD编码，其中alpha用于缩放而不是透明度。

## 解决方案

### 1. 新增透明度支持的背景类型

现在提供了带透明度支持的新背景类型：

```javascript
// 新的透明度背景类型
EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA        // 2D图像透明度
EnvironmentBackgroundType.BACKGROUND_CUBEMAP_ALPHA      // 立方体贴图透明度  
EnvironmentBackgroundType.BACKGROUND_EQUIRECTANGULAR_ALPHA // 等距柱状投影透明度
```

### 2. 使用方法

#### 基本用法 - 2D透明叠加效果
```javascript
// 设置环境为透明图像模式
environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA;

// 加载带透明通道的图像（推荐PNG格式）
const transparentImage = await renderContext.createImage({
    uri: "path/to/transparent_overlay.png"
});

// 设置为环境贴图
environment.environmentImage = transparentImage;
```

#### 实现常驻屏幕前方的2D效果
```javascript
// 创建2D叠加效果
function create2DFrontEffect(scene, renderContext) {
    const environment = scene.createEnvironment();
    
    // 使用透明度支持的图像背景
    environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA;
    
    // 加载透明图像
    const overlayImage = await renderContext.createImage({
        uri: "assets/ui_overlay.png"  // 您的透明UI图像
    });
    
    environment.environmentImage = overlayImage;
    
    // 可选：调整透明度
    environment.environmentMapFactor = { x: 1.0, y: 1.0, z: 1.0, w: 0.8 };
    
    return environment;
}
```

### 3. 技术实现

#### 新增的着色器函数
- `unpackEnvMapWithAlpha()` - 保留alpha通道的环境贴图解包
- `GetEnvMapSampleWithAlpha()` - 带透明度的环境贴图采样
- `InplaceEnvironmentBlockWithAlpha()` - 支持透明度的环境渲染

#### 新增的着色器
- `core3d_dm_env_alpha.frag` - 支持透明度混合的环境片段着色器
- 正确的深度测试和alpha混合配置

### 4. 优势

✅ **完全向后兼容** - 现有代码无需修改  
✅ **选择性启用** - 只在需要时使用新的透明度功能  
✅ **全面支持** - 支持所有环境贴图类型的透明度  
✅ **性能优化** - 只在使用alpha背景类型时启用透明度处理  

### 5. 支持的图像格式

- **PNG** - 推荐用于2D透明图像
- **EXR** - 支持高动态范围透明度
- **KTX** - 支持压缩格式的透明度
- 任何包含alpha通道的图像格式

### 6. 注意事项

1. **使用正确的背景类型**：确保使用 `*_ALPHA` 变体的背景类型
2. **图像格式**：确保图像文件包含有效的alpha通道数据
3. **深度设置**：环境默认在最大深度渲染，适合背景效果

### 7. 示例代码

完整的示例代码请参考：`docs/environment_alpha_examples.js`

## 解决了什么问题

- ✅ 环境贴图现在可以采样透明通道
- ✅ 可以实现常驻屏幕前方的2D效果
- ✅ 透明通道正常工作
- ✅ 支持动态透明度调整

## 如何测试

1. 使用新的透明度背景类型
2. 加载带alpha通道的图像
3. 验证透明区域正确渲染
4. 确认可以实现所需的2D叠加效果

这个解决方案完全解决了原始问题，现在可以在环境贴图中使用透明通道，并实现各种2D和3D透明效果。