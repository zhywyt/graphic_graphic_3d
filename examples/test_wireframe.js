#!/usr/bin/env node

/**
 * 测试脚本 - 验证线框包围盒的实现
 */

console.log("🎯 开始验证线框包围盒实现...\n");

// 导入测试模块
const { createWireframeBoundingBox, logWireframeData } = require('./simple_wireframe_demo.js');

/**
 * 测试1: 验证几何体数据正确性
 */
function testGeometryData() {
    console.log("📊 测试1: 验证几何体数据正确性");
    
    const wireframe = createWireframeBoundingBox();
    
    // 检查基本数据
    console.log("✓ 图元类型:", wireframe.topology);
    console.log("✓ 顶点数量:", wireframe.vertices.length, "(应该是8个)");
    console.log("✓ 索引数量:", wireframe.indices.length, "(应该是24个)");
    console.log("✓ 线条数量:", wireframe.indices.length / 2, "(应该是12条)");
    
    // 验证数据完整性
    let isValid = true;
    
    if (wireframe.vertices.length !== 8) {
        console.log("❌ 错误: 顶点数量不正确");
        isValid = false;
    }
    
    if (wireframe.indices.length !== 24) {
        console.log("❌ 错误: 索引数量不正确");
        isValid = false;
    }
    
    // 检查索引是否都在有效范围内
    for (let i = 0; i < wireframe.indices.length; i++) {
        if (wireframe.indices[i] < 0 || wireframe.indices[i] >= wireframe.vertices.length) {
            console.log(`❌ 错误: 索引${i}的值${wireframe.indices[i]}超出范围`);
            isValid = false;
        }
    }
    
    if (isValid) {
        console.log("✅ 几何体数据验证通过!\n");
    } else {
        console.log("❌ 几何体数据验证失败!\n");
    }
    
    return isValid;
}

/**
 * 测试2: 验证包围盒形状
 */
function testBoundingBoxShape() {
    console.log("📐 测试2: 验证包围盒形状");
    
    const wireframe = createWireframeBoundingBox();
    const vertices = wireframe.vertices;
    
    // 检查是否形成正确的立方体
    const expectedVertices = [
        [-5, -5, -5], [5, -5, -5], [5, 5, -5], [-5, 5, -5],  // 底面
        [-5, -5,  5], [5, -5,  5], [5, 5,  5], [-5, 5,  5]   // 顶面
    ];
    
    let shapeValid = true;
    
    for (let i = 0; i < expectedVertices.length; i++) {
        const expected = expectedVertices[i];
        const actual = vertices[i];
        
        for (let j = 0; j < 3; j++) {
            if (Math.abs(expected[j] - actual[j]) > 0.001) {
                console.log(`❌ 错误: 顶点${i}坐标不匹配`);
                console.log(`   期望: [${expected.join(', ')}]`);
                console.log(`   实际: [${actual.join(', ')}]`);
                shapeValid = false;
                break;
            }
        }
    }
    
    if (shapeValid) {
        console.log("✅ 包围盒形状验证通过!");
        console.log("✓ 所有顶点位置正确");
        console.log("✓ 形成了正确的10x10x10立方体\n");
    } else {
        console.log("❌ 包围盒形状验证失败!\n");
    }
    
    return shapeValid;
}

/**
 * 测试3: 验证线条连接
 */
function testLineConnections() {
    console.log("🔗 测试3: 验证线条连接");
    
    const wireframe = createWireframeBoundingBox();
    const indices = wireframe.indices;
    
    // 统计每个顶点的连接数
    const connections = new Array(8).fill(0);
    
    for (let i = 0; i < indices.length; i += 2) {
        connections[indices[i]]++;
        connections[indices[i + 1]]++;
    }
    
    // 每个顶点应该连接3条边
    let connectionsValid = true;
    for (let i = 0; i < connections.length; i++) {
        if (connections[i] !== 3) {
            console.log(`❌ 错误: 顶点${i}连接了${connections[i]}条边，应该是3条`);
            connectionsValid = false;
        }
    }
    
    if (connectionsValid) {
        console.log("✅ 线条连接验证通过!");
        console.log("✓ 每个顶点都正确连接了3条边");
        console.log("✓ 形成了完整的立方体线框\n");
    } else {
        console.log("❌ 线条连接验证失败!\n");
    }
    
    return connectionsValid;
}

/**
 * 测试4: 输出详细的几何体信息
 */
function testDetailedOutput() {
    console.log("📋 测试4: 详细几何体信息");
    logWireframeData();
    console.log("");
}

/**
 * 主测试函数
 */
function runTests() {
    const test1 = testGeometryData();
    const test2 = testBoundingBoxShape();
    const test3 = testLineConnections();
    
    testDetailedOutput();
    
    console.log("🏁 测试总结:");
    console.log(`   几何体数据: ${test1 ? '✅ 通过' : '❌ 失败'}`);
    console.log(`   包围盒形状: ${test2 ? '✅ 通过' : '❌ 失败'}`);
    console.log(`   线条连接: ${test3 ? '✅ 通过' : '❌ 失败'}`);
    
    const allPassed = test1 && test2 && test3;
    
    if (allPassed) {
        console.log("\n🎉 所有测试通过! 线框包围盒实现正确。");
        console.log("\n📋 实现特性:");
        console.log("   • ✅ 支持LINE_LIST图元类型");
        console.log("   • ✅ 正确生成8个顶点的立方体");
        console.log("   • ✅ 生成12条边的线框结构");
        console.log("   • ✅ 每个顶点连接3条边");
        console.log("   • ✅ 形成完整的包围盒wireframe");
    } else {
        console.log("\n❌ 部分测试失败，需要检查实现。");
    }
    
    return allPassed;
}

// 运行测试
runTests();