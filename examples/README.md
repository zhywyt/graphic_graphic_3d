# Wireframe Bounding Box Example

这个示例演示了如何在3D图形引擎中绘制当前场景的包围盒，使用线框模式。

## 功能说明

本示例提供了以下功能：

1. **获取场景边界** - 通过 `scene.getSceneBounds()` 方法获取当前场景的包围盒信息
2. **创建线框几何体** - 生成包围盒的线框几何数据
3. **LINE_LIST 图元支持** - 扩展了自定义几何体以支持线条渲染
4. **线框材质** - 创建适合线框渲染的材质

## 新增API

### SceneJS.getSceneBounds()

返回当前场景的包围盒信息：

```javascript
const bounds = scene.getSceneBounds();
console.log(bounds);
// 输出：
// {
//   center: { x: 0, y: 0, z: 0 },    // 包围球中心
//   radius: 8.66,                     // 包围球半径
//   min: { x: -5, y: -5, z: -5 },    // AABB最小坐标
//   max: { x: 5, y: 5, z: 5 }        // AABB最大坐标
// }
```

### 扩展的 PrimitiveTopology

在自定义几何体中新增了 `LINE_LIST` 图元类型：

```javascript
const customGeometry = new Custom();
customGeometry.topology = PrimitiveTopology.LINE_LIST;
customGeometry.vertices = [...];  // 顶点数组
customGeometry.indices = [...];   // 线条索引（每两个索引定义一条线）
```

## 使用方法

### 基本用法

```javascript
// 创建场景
const scene = await Scene.load("scene://empty");

// 创建并添加线框包围盒
const wireframeBox = await createSceneWireframeBoundingBox(scene);

// 渲染场景
scene.renderFrame();
```

### 创建简单线框盒子

```javascript
// 创建固定大小的线框盒子
const simpleBox = await createSimpleWireframeBox(scene, 5.0);
```

### 自定义线框几何体

```javascript
function createWireframeBoundingBoxGeometry(bounds) {
    const min = bounds.min;
    const max = bounds.max;
    
    // 定义包围盒的8个顶点
    const vertices = [
        [min.x, min.y, min.z], // 0: 底面-前-左
        [max.x, min.y, min.z], // 1: 底面-前-右
        [max.x, max.y, min.z], // 2: 底面-后-右
        [min.x, max.y, min.z], // 3: 底面-后-左
        [min.x, min.y, max.z], // 4: 顶面-前-左
        [max.x, min.y, max.z], // 5: 顶面-前-右
        [max.x, max.y, max.z], // 6: 顶面-后-右
        [min.x, max.y, max.z], // 7: 顶面-后-左
    ];
    
    // 定义线条索引（每两个索引定义一条线）
    const indices = [
        // 底面边
        0, 1, 1, 2, 2, 3, 3, 0,
        // 顶面边  
        4, 5, 5, 6, 6, 7, 7, 4,
        // 垂直边
        0, 4, 1, 5, 2, 6, 3, 7
    ];
    
    return {
        topology: PrimitiveTopology.LINE_LIST,
        vertices: vertices,
        indices: indices,
        normals: [],
        uvs: [],
        colors: []
    };
}
```

## 技术实现

### LINE_LIST 到 TRIANGLE_LIST 转换

由于底层场景API只支持三角形图元，本实现将LINE_LIST自动转换为细三角形：

1. 每条线段转换为一个四边形（由两个三角形组成）
2. 四边形具有很小的宽度（0.01单位）以模拟线条效果
3. 自动计算垂直于线段方向的向量来创建线条宽度

### 包围盒计算

当前实现返回一个默认的包围盒（-5到+5的立方体）。在完整实现中，这会访问渲染预处理系统来获取实际的场景边界。

## 文件说明

- `wireframe_bounding_box.js` - 主要的示例代码
- `SceneJS.cpp` - 添加了 `getSceneBounds()` 方法
- `CustomJS.cpp` - 扩展了 `LINE_LIST` 图元支持

## 运行示例

```javascript
// 在支持的环境中运行
const { Scene, Custom, PrimitiveTopology } = require('@ohos/graphics3d');

demonstrateWireframeBoundingBox().catch(console.error);
```

## 注意事项

1. 当前的包围盒获取返回默认值，实际使用时需要连接到场景的实际边界计算
2. LINE_LIST通过转换为细三角形实现，性能可能不如原生线条渲染
3. 线框材质的具体属性可能需要根据渲染器支持进行调整

## 扩展建议

1. 实现实际的场景边界获取（连接到RenderPreprocessorSystem）
2. 添加动态更新包围盒的能力
3. 支持不同的线条样式（虚线、粗细等）
4. 添加包围球的线框渲染