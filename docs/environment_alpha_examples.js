/**
 * Example: Using Environment Map Alpha Transparency
 * 
 * This example demonstrates how to use the new alpha transparency features
 * for environment maps to create 2D-like effects that stay in front.
 */

// Example 1: Basic alpha environment setup
function setupAlphaEnvironment(scene, renderContext) {
    // Create environment with alpha support
    const environment = scene.createEnvironment();
    
    // Set background type to support alpha transparency
    // Available options:
    // - EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA
    // - EnvironmentBackgroundType.BACKGROUND_CUBEMAP_ALPHA  
    // - EnvironmentBackgroundType.BACKGROUND_EQUIRECTANGULAR_ALPHA
    environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA;
    
    return environment;
}

// Example 2: Loading and setting transparent environment image
async function loadTransparentEnvironmentImage(renderContext, environment) {
    try {
        // Load an image with alpha channel (PNG recommended)
        const transparentImage = await renderContext.createImage({
            uri: "assets/environment/transparent_overlay.png"
        });
        
        // Set as environment image
        environment.environmentImage = transparentImage;
        
        console.log("Transparent environment image loaded successfully");
        return transparentImage;
    } catch (error) {
        console.error("Failed to load transparent environment image:", error);
        return null;
    }
}

// Example 3: Creating a 2D overlay effect
function create2DOverlayEffect(scene, renderContext) {
    const environment = setupAlphaEnvironment(scene, renderContext);
    
    // Load transparent overlay image
    loadTransparentEnvironmentImage(renderContext, environment);
    
    // Optional: Adjust environment factors for desired effect
    environment.environmentMapFactor = { x: 1.0, y: 1.0, z: 1.0, w: 0.8 }; // Reduce intensity
    
    // The environment will now render as a transparent overlay
    // in front of the 3D scene, creating a 2D-like effect
    
    return environment;
}

// Example 4: Cubemap with alpha transparency
async function setupAlphaCubemap(scene, renderContext) {
    const environment = scene.createEnvironment();
    
    // Set to cubemap with alpha support
    environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_CUBEMAP_ALPHA;
    
    try {
        // Load cubemap with alpha channel
        const alphaCubemap = await renderContext.createImage({
            uri: "assets/environment/transparent_cubemap.ktx"
        });
        
        environment.environmentImage = alphaCubemap;
        return environment;
    } catch (error) {
        console.error("Failed to load alpha cubemap:", error);
        return null;
    }
}

// Example 5: Equirectangular with alpha
async function setupAlphaEquirectangular(scene, renderContext) {
    const environment = scene.createEnvironment();
    
    // Set to equirectangular with alpha support
    environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_EQUIRECTANGULAR_ALPHA;
    
    try {
        // Load equirectangular image with alpha
        const alphaEquirect = await renderContext.createImage({
            uri: "assets/environment/transparent_equirect.exr"
        });
        
        environment.environmentImage = alphaEquirect;
        return environment;
    } catch (error) {
        console.error("Failed to load alpha equirectangular:", error);
        return null;
    }
}

// Example 6: Dynamic alpha animation
function animateEnvironmentAlpha(environment, time) {
    // Animate alpha transparency over time
    const alpha = (Math.sin(time * 2.0) + 1.0) * 0.5; // 0.0 to 1.0
    
    // Adjust environment map factor alpha component
    const factor = environment.environmentMapFactor;
    environment.environmentMapFactor = { 
        x: factor.x, 
        y: factor.y, 
        z: factor.z, 
        w: alpha 
    };
}

// Example 7: Complete setup function
async function setupTransparentEnvironmentDemo(scene, renderContext) {
    try {
        // Create environment with alpha transparency
        const environment = create2DOverlayEffect(scene, renderContext);
        
        // Set camera to use this environment
        const camera = scene.getActiveCamera();
        if (camera) {
            camera.environment = environment;
        }
        
        console.log("Transparent environment demo setup complete");
        return environment;
    } catch (error) {
        console.error("Failed to setup transparent environment demo:", error);
        return null;
    }
}

// Usage:
// const environment = await setupTransparentEnvironmentDemo(scene, renderContext);
// 
// // Optional: Add animation
// function updateFrame(time) {
//     if (environment) {
//         animateEnvironmentAlpha(environment, time);
//     }
// }