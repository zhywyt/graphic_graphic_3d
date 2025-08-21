/**
 * 简化的线框包围盒示例 - 专门用于演示
 * 
 * 这个文件提供了一个更简化的例子，专注于核心功能的演示
 */

/**
 * 创建线框包围盒的核心函数
 */
function createWireframeBoundingBox() {
    // 模拟场景边界数据（10x10x10的立方体）
    const sceneBounds = {
        min: { x: -5, y: -5, z: -5 },
        max: { x: 5, y: 5, z: 5 },
        center: { x: 0, y: 0, z: 0 },
        radius: 8.66
    };

    // 定义包围盒的8个顶点
    const vertices = [
        // 底面 (z = min.z = -5)
        [-5, -5, -5], // 0: 底面-前-左
        [ 5, -5, -5], // 1: 底面-前-右  
        [ 5,  5, -5], // 2: 底面-后-右
        [-5,  5, -5], // 3: 底面-后-左
        
        // 顶面 (z = max.z = 5)
        [-5, -5,  5], // 4: 顶面-前-左
        [ 5, -5,  5], // 5: 顶面-前-右
        [ 5,  5,  5], // 6: 顶面-后-右
        [-5,  5,  5], // 7: 顶面-后-左
    ];

    // 定义线条索引（每两个索引定义一条线，总共12条边）
    const indices = [
        // 底面的4条边
        0, 1,  // 前边
        1, 2,  // 右边
        2, 3,  // 后边  
        3, 0,  // 左边
        
        // 顶面的4条边
        4, 5,  // 前边
        5, 6,  // 右边
        6, 7,  // 后边
        7, 4,  // 左边
        
        // 连接底面和顶面的4条垂直边
        0, 4,  // 前左垂直边
        1, 5,  // 前右垂直边
        2, 6,  // 后右垂直边
        3, 7,  // 后左垂直边
    ];

    // 返回线框几何体定义
    return {
        topology: "LINE_LIST", // 使用LINE_LIST图元类型
        vertices: vertices,
        indices: indices,
        normals: [],   // 线框不需要法线
        uvs: [],       // 线框不需要UV坐标
        colors: []     // 使用材质颜色
    };
}

/**
 * 使用示例（伪代码，展示API用法）
 */
async function demonstrateWireframeBoundingBox() {
    // 1. 创建或加载场景
    const scene = await Scene.load("scene://empty");
    
    // 2. 获取场景边界（使用新添加的API）
    const bounds = scene.getSceneBounds();
    console.log("场景边界:", bounds);
    // 输出: { center: {x: 0, y: 0, z: 0}, radius: 8.66, min: {x: -5, y: -5, z: -5}, max: {x: 5, y: 5, z: 5} }
    
    // 3. 创建线框几何体数据
    const wireframeGeometry = createWireframeBoundingBox();
    
    // 4. 创建自定义几何体对象
    const customGeometry = new Custom();
    customGeometry.topology = PrimitiveTopology.LINE_LIST;  // 使用新添加的LINE_LIST
    customGeometry.vertices = wireframeGeometry.vertices;
    customGeometry.indices = wireframeGeometry.indices;
    customGeometry.normals = wireframeGeometry.normals;
    customGeometry.uvs = wireframeGeometry.uvs;
    customGeometry.colors = wireframeGeometry.colors;
    
    // 5. 创建网格资源
    const meshResource = await scene.createMesh({
        name: "WireframeBoundingBoxMesh"
    }, customGeometry);
    
    // 6. 创建线框材质
    const material = await scene.createMaterial({
        name: "WireframeMaterial",
        baseColor: [1.0, 1.0, 0.0, 1.0], // 黄色
        metallic: 0.0,
        roughness: 1.0
    });
    
    // 7. 创建几何体节点并添加到场景
    const wireframeNode = await scene.createGeometry({
        name: "SceneWireframeBoundingBox",
        material: material
    }, meshResource);
    
    // 8. 设置相机位置以便观察
    const camera = await scene.createCamera({
        name: "WireframeCamera",
        position: [10, 10, 10],
        target: [0, 0, 0]
    });
    
    // 9. 渲染场景
    scene.renderFrame();
    
    console.log("线框包围盒创建完成！");
    
    return wireframeNode;
}

/**
 * 输出几何体数据以便验证
 */
function logWireframeData() {
    const wireframe = createWireframeBoundingBox();
    
    console.log("=== 线框包围盒几何体数据 ===");
    console.log("图元类型:", wireframe.topology);
    console.log("顶点数量:", wireframe.vertices.length);
    console.log("索引数量:", wireframe.indices.length);
    console.log("线条数量:", wireframe.indices.length / 2);
    
    console.log("\n顶点坐标:");
    wireframe.vertices.forEach((vertex, i) => {
        console.log(`  顶点${i}: [${vertex.join(', ')}]`);
    });
    
    console.log("\n线条索引（每两个索引定义一条线）:");
    for (let i = 0; i < wireframe.indices.length; i += 2) {
        const v1 = wireframe.indices[i];
        const v2 = wireframe.indices[i + 1];
        console.log(`  线条${i/2}: 顶点${v1} -> 顶点${v2}`);
    }
}

// 运行数据验证
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        createWireframeBoundingBox,
        demonstrateWireframeBoundingBox,
        logWireframeData
    };
} else {
    // 在浏览器环境中运行验证
    logWireframeData();
}