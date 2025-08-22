# Environment Map Alpha Transparency Support

## Problem
The original issue was that environment maps could not sample the transparent channel, preventing 2D-like effects with transparency that stay in front of the screen.

## Root Cause
The `unpackEnvMap()` function in the shader pipeline only returned RGB values and discarded the alpha channel. This was because it was designed for RGBD encoding where alpha is used for scaling rather than transparency.

## Solution
Added comprehensive alpha transparency support to the environment mapping system:

### 1. New Alpha-Preserving Functions

**Shader Functions**:
- `unpackEnvMapWithAlpha()` - Preserves alpha channel from environment textures
- `GetEnvMapSampleWithAlpha()` - Environment map sampling with alpha support  
- `InplaceEnvironmentBlockWithAlpha()` - Environment rendering with transparency

### 2. New Background Types

**JavaScript API**:
```javascript
EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA        // For 2D images with alpha
EnvironmentBackgroundType.BACKGROUND_CUBEMAP_ALPHA      // For cubemaps with alpha
EnvironmentBackgroundType.BACKGROUND_EQUIRECTANGULAR_ALPHA // For equirectangular with alpha
```

### 3. New Alpha Environment Shader

**Shader**: `core3d_dm_env_alpha.frag`
- Handles alpha blending for transparent environment maps
- Proper depth testing for 2D-like effects
- Supports all environment map types with transparency

## Usage

### Basic Usage
```javascript
// Set environment to use alpha transparency
environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA;

// Load an image with alpha channel
const imageWithAlpha = await renderContext.createImage({
    uri: "path/to/image_with_alpha.png"
});

// Set as environment map
environment.environmentImage = imageWithAlpha;
```

### Advanced Usage for 2D-like Effects
```javascript
// For 2D overlay-like environment that stays in front
environment.backgroundType = EnvironmentBackgroundType.BACKGROUND_IMAGE_ALPHA;
environment.environmentImage = transparentImage;

// The environment will now render with transparency and can be used
// for 2D-like effects that stay at the front of the screen
```

## Technical Details

### Shader Pipeline Changes
1. **Alpha Preservation**: Modified environment sampling to preserve alpha throughout the pipeline
2. **Conditional Sampling**: Different sampling paths for alpha vs non-alpha background types
3. **Blending Support**: Added proper alpha blending in the environment shader

### Backward Compatibility
- All existing environment mapping functionality remains unchanged
- New alpha features are opt-in via new background types
- Original shaders continue to work as before

### File Changes
- `3d_dm_indirect_lighting_common.h` - Added `unpackEnvMapWithAlpha()`
- `3d_dm_inplace_env_common.h` - Added alpha-preserving sampling functions
- `3d_dm_structures_common.h` - Added new background type constants
- `EnvironmentJS.h/cpp` - Extended JavaScript interface
- `intf_environment.h` - Added scene-level alpha background types
- `core3d_dm_env_alpha.frag` - New alpha environment fragment shader
- `core3d_dm_env_alpha.shader` - Shader configuration with alpha blending

## Testing
To test the alpha transparency:

1. Create an environment with alpha background type
2. Load a texture with alpha channel (PNG with transparency)
3. Verify that transparent areas are properly rendered
4. Check that the environment can achieve the desired 2D-like effect

## Possible Issues and Solutions

### Issue: Alpha not working
**Solution**: Ensure you're using the new alpha background types (`*_ALPHA` variants)

### Issue: Environment not visible
**Solution**: Check that the environment image has valid alpha channel data

### Issue: Depth issues
**Solution**: The environment renders at maximum depth by default. For front-rendering 2D effects, you may need to adjust the depth values in the shader or use a custom shader.

## Future Enhancements
- Add depth control for environment maps
- Support for alpha animation in environment maps  
- Per-pixel depth writing for complex transparency effects