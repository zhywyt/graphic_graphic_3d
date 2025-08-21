/**
 * Wireframe Bounding Box Example
 * 
 * This example demonstrates how to create and render a wireframe bounding box
 * for the current scene using the 3D graphics engine.
 */

// Import the Scene API (assuming it's available in the environment)
// const { Scene, Custom, PrimitiveTopology } = require('@ohos/graphics3d');

/**
 * Creates wireframe geometry for a bounding box
 * @param {Object} bounds - Bounding box with min and max coordinates
 * @param {Object} bounds.min - Minimum coordinates {x, y, z}
 * @param {Object} bounds.max - Maximum coordinates {x, y, z}
 * @returns {Object} Custom geometry definition for wireframe box
 */
function createWireframeBoundingBoxGeometry(bounds) {
    const min = bounds.min;
    const max = bounds.max;
    
    // Define the 8 vertices of the bounding box
    const vertices = [
        // Bottom face (z = min.z)
        [min.x, min.y, min.z], // 0: bottom-front-left
        [max.x, min.y, min.z], // 1: bottom-front-right
        [max.x, max.y, min.z], // 2: bottom-back-right
        [min.x, max.y, min.z], // 3: bottom-back-left
        
        // Top face (z = max.z)
        [min.x, min.y, max.z], // 4: top-front-left
        [max.x, min.y, max.z], // 5: top-front-right
        [max.x, max.y, max.z], // 6: top-back-right
        [min.x, max.y, max.z], // 7: top-back-left
    ];
    
    // Define the line indices for the wireframe (each pair forms a line)
    const indices = [
        // Bottom face edges
        0, 1,  // front edge
        1, 2,  // right edge
        2, 3,  // back edge
        3, 0,  // left edge
        
        // Top face edges
        4, 5,  // front edge
        5, 6,  // right edge
        6, 7,  // back edge
        7, 4,  // left edge
        
        // Vertical edges connecting bottom and top
        0, 4,  // front-left vertical
        1, 5,  // front-right vertical
        2, 6,  // back-right vertical
        3, 7,  // back-left vertical
    ];
    
    // Create the custom geometry object
    return {
        topology: PrimitiveTopology.LINE_LIST,
        vertices: vertices,
        indices: indices,
        normals: [],   // Not needed for wireframe
        uvs: [],       // Not needed for wireframe
        colors: []     // Can be empty, will use material color
    };
}

/**
 * Creates a wireframe material for the bounding box
 * @param {Scene} scene - The scene object
 * @returns {Promise<Material>} Material for wireframe rendering
 */
async function createWireframeMaterial(scene) {
    // Create a basic material with wireframe properties
    const material = await scene.createMaterial({
        name: "WireframeBoundingBox",
        // Set material properties for wireframe rendering
        baseColor: [1.0, 1.0, 0.0, 1.0], // Yellow color
        metallic: 0.0,
        roughness: 1.0,
        // Enable wireframe mode if supported
        wireframe: true
    });
    
    return material;
}

/**
 * Creates and adds a wireframe bounding box to the scene
 * @param {Scene} scene - The scene object
 * @returns {Promise<Node>} The created wireframe bounding box node
 */
async function createSceneWireframeBoundingBox(scene) {
    try {
        // Get the scene bounds
        const bounds = scene.getSceneBounds();
        console.log("Scene bounds:", bounds);
        
        // Create wireframe geometry
        const wireframeGeometry = createWireframeBoundingBoxGeometry(bounds);
        
        // Create custom geometry definition
        const customGeometry = new Custom();
        customGeometry.topology = wireframeGeometry.topology;
        customGeometry.vertices = wireframeGeometry.vertices;
        customGeometry.indices = wireframeGeometry.indices;
        customGeometry.normals = wireframeGeometry.normals;
        customGeometry.uvs = wireframeGeometry.uvs;
        customGeometry.colors = wireframeGeometry.colors;
        
        // Create mesh resource from geometry
        const meshResource = await scene.createMesh({
            name: "WireframeBoundingBoxMesh"
        }, customGeometry);
        
        // Create material for wireframe
        const material = await createWireframeMaterial(scene);
        
        // Create geometry node in the scene
        const wireframeNode = await scene.createGeometry({
            name: "SceneWireframeBoundingBox",
            material: material
        }, meshResource);
        
        console.log("Wireframe bounding box created successfully");
        return wireframeNode;
        
    } catch (error) {
        console.error("Failed to create wireframe bounding box:", error);
        throw error;
    }
}

/**
 * Main function to demonstrate wireframe bounding box creation
 */
async function demonstrateWireframeBoundingBox() {
    try {
        // Create or load a scene
        const scene = await Scene.load("scene://empty");
        
        // Add some content to the scene first (optional)
        // This would make the bounding box more interesting
        /*
        const cubeGeometry = new Cube();
        cubeGeometry.width = 2.0;
        cubeGeometry.height = 2.0;
        cubeGeometry.depth = 2.0;
        
        const cubeMesh = await scene.createMesh({
            name: "TestCube"
        }, cubeGeometry);
        
        const testCube = await scene.createGeometry({
            name: "TestCubeNode"
        }, cubeMesh);
        */
        
        // Create wireframe bounding box
        const wireframeBox = await createSceneWireframeBoundingBox(scene);
        
        // Set up camera to view the bounding box
        const camera = await scene.createCamera({
            name: "WireframeCamera",
            position: [10, 10, 10],
            target: [0, 0, 0]
        });
        
        // Render the scene
        scene.renderFrame();
        
        console.log("Wireframe bounding box demo completed successfully");
        
    } catch (error) {
        console.error("Demo failed:", error);
    }
}

/**
 * Alternative function for creating a simple wireframe box with hardcoded bounds
 */
async function createSimpleWireframeBox(scene, size = 5.0) {
    const bounds = {
        min: { x: -size, y: -size, z: -size },
        max: { x: size, y: size, z: size }
    };
    
    const wireframeGeometry = createWireframeBoundingBoxGeometry(bounds);
    
    // Create the wireframe box
    const customGeometry = new Custom();
    Object.assign(customGeometry, wireframeGeometry);
    
    const meshResource = await scene.createMesh({
        name: "SimpleWireframeBox"
    }, customGeometry);
    
    const material = await createWireframeMaterial(scene);
    
    const wireframeNode = await scene.createGeometry({
        name: "SimpleWireframeBox"
    }, meshResource);
    
    return wireframeNode;
}

// Export functions for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        createWireframeBoundingBoxGeometry,
        createWireframeMaterial,
        createSceneWireframeBoundingBox,
        demonstrateWireframeBoundingBox,
        createSimpleWireframeBox
    };
}

// If running directly, execute the demo
if (typeof window === 'undefined' && require.main === module) {
    demonstrateWireframeBoundingBox().catch(console.error);
}