# LIGHT_FORWARD Pipeline Post-Processing Guide

## Overview

The LIGHT_FORWARD rendering pipeline now supports post-processing effects that only depend on current frame color attachments. This enables the use of common effects like bloom, tonemap, and FXAA with the lightweight pipeline, while maintaining excellent performance for scenarios that don't require the full feature set of FORWARD or DEFERRED pipelines.

## What is LIGHT_FORWARD Pipeline?

LIGHT_FORWARD is a lightweight rendering pipeline optimized for:
- **Performance**: Minimal overhead for simple rendering scenarios
- **Memory efficiency**: Direct-to-backbuffer rendering when no post-processing is used
- **Simplicity**: Reduced complexity compared to full forward or deferred pipelines

## Supported Post-Processing Effects

The following post-processing effects are **compatible** with LIGHT_FORWARD pipeline because they only require current frame color data:

### ✅ **Color Processing Effects**
- **Tonemap** (`ENABLE_TONEMAP_BIT`) - HDR to LDR tone mapping
- **Vignette** (`ENABLE_VIGNETTE_BIT`) - Darkening of image periphery  
- **Dither** (`ENABLE_DITHER_BIT`) - Noise-based dithering for banding reduction
- **Color Conversion** (`ENABLE_COLOR_CONVERSION_BIT`) - Color space conversions
- **Color Fringe** (`ENABLE_COLOR_FRINGE_BIT`) - Chromatic aberration effects

### ✅ **Image Enhancement Effects**
- **Blur** (`ENABLE_BLUR_BIT`) - Gaussian blur effects
- **Bloom** (`ENABLE_BLOOM_BIT`) - HDR bloom/glow effects
- **FXAA** (`ENABLE_FXAA_BIT`) - Fast approximate anti-aliasing

## Unsupported Post-Processing Effects

The following effects are **NOT compatible** with LIGHT_FORWARD because they require additional render data:

### ❌ **Temporal Effects** 
- **TAA** (`ENABLE_TAA_BIT`) - Requires previous frame data for temporal accumulation
- **Motion Blur** (`ENABLE_MOTION_BLUR_BIT`) - Requires velocity vectors from object movement

### ❌ **Depth-Dependent Effects**
- **Depth of Field** (`ENABLE_DOF_BIT`) - Requires depth buffer information for focus calculations

### ❌ **Complex Lighting Effects**
- **Lens Flare** (`ENABLE_LENS_FLARE_BIT`) - Requires additional lighting data

## How to Use LIGHT_FORWARD with Post-Processing

### Basic Setup

```cpp
// Configure camera for LIGHT_FORWARD with post-processing
RenderCamera camera;
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;
camera.postProcessName = "CORE3D_POST_PROCESS_CAM"; // Your post-process configuration name

// The pipeline will automatically detect compatible post-processing effects
// and enable proper render targets when needed
```

### Post-Processing Configuration

Configure your post-processing effects using the standard post-processing configuration:

```cpp
// Example: Enable bloom and tonemap (both compatible with LIGHT_FORWARD)
PostProcessConfiguration config = {};
config.flags.x = PostProcessConfiguration::ENABLE_BLOOM_BIT | 
                 PostProcessConfiguration::ENABLE_TONEMAP_BIT |
                 PostProcessConfiguration::ENABLE_FXAA_BIT;

// Set bloom parameters
config.factors[PostProcessConfiguration::INDEX_BLOOM] = bloomStrength;

// Set tonemap parameters  
config.factors[PostProcessConfiguration::INDEX_TONEMAP] = tonemapExposure;
```

### HDR Rendering with LIGHT_FORWARD

When compatible post-processing is detected, LIGHT_FORWARD automatically:

1. **Creates HDR render targets** instead of rendering directly to backbuffer
2. **Enables custom render resolution** for post-processing chains
3. **Handles MSAA properly** by creating color resolve targets when needed

```cpp
// HDR bloom example - works automatically with LIGHT_FORWARD
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;
camera.postProcessName = "bloom_postprocess";

// Pipeline detects bloom effect and automatically:
// - Creates HDR color targets
// - Enables render resolution support  
// - Processes bloom in post-processing chain
```

## Validation and Debugging

The implementation includes comprehensive validation to help developers:

### Success Messages

When LIGHT_FORWARD post-processing is working correctly, you'll see:

```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline using current-frame-only post-processing (flags: 0x300)
```

### Warning Messages

If you try to use incompatible effects with LIGHT_FORWARD:

```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline has incompatible post-processing effects (enabled: 0x1800, incompatible: 0x1000)
```

This tells you:
- `enabled: 0x1800` - All enabled post-processing flags
- `incompatible: 0x1000` - The specific incompatible flags (e.g., TAA)

### Troubleshooting

**Problem**: Post-processing not working with LIGHT_FORWARD
- **Check**: Ensure you're only using compatible effects (see supported list above)
- **Check**: Verify post-processing configuration name is valid
- **Check**: Look for validation warnings in debug output

**Problem**: Performance is worse than expected
- **Solution**: Verify you're not accidentally using incompatible effects that force pipeline fallback
- **Solution**: Check if HDR targets are being created unnecessarily

**Problem**: MSAA not working properly
- **Solution**: LIGHT_FORWARD with post-processing automatically handles MSAA resolve - no additional configuration needed

## Performance Considerations

### When Post-Processing is Disabled
- **Direct backbuffer rendering** - Maximum performance, no intermediate targets
- **Minimal overhead** - Maintains lightweight pipeline benefits

### When Compatible Post-Processing is Enabled  
- **HDR intermediate targets** - Created only when needed for post-processing
- **Custom render resolution** - Supports resolution scaling for post-processing
- **Efficient memory usage** - Targets sized appropriately for enabled effects

### Migration from Other Pipelines

If you're currently using FORWARD or DEFERRED only for post-processing:

```cpp
// Before: Using FORWARD pipeline just for post-processing
camera.renderPipelineType = RenderCamera::RenderPipelineType::FORWARD;
camera.postProcessName = "bloom_tonemap"; 

// After: Use LIGHT_FORWARD for better performance
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;  
camera.postProcessName = "bloom_tonemap"; // Same effects, better performance
```

## Implementation Details

### Automatic Target Creation

The LIGHT_FORWARD pipeline automatically detects when post-processing requires intermediate targets:

1. **Analyzes enabled post-processing flags** using `IsCurrentFrameOnlyPostProcessing()`
2. **Creates HDR targets when needed** for effects like bloom that require high dynamic range
3. **Enables render resolution support** for post-processing chains that need custom scaling
4. **Maintains backward compatibility** - existing LIGHT_FORWARD usage without post-processing is unchanged

### Effect Compatibility Detection

```cpp
// Effects that only need current frame color (compatible)
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

The system checks if enabled effects are a subset of these compatible flags. Any incompatible effects will trigger warnings and may cause the post-processing to be skipped or require a different pipeline.

## Best Practices

1. **Start with LIGHT_FORWARD** for new projects that need post-processing
2. **Use compatible effects first** - try bloom, tonemap, FXAA before considering TAA or DOF
3. **Monitor validation output** during development to catch incompatible configurations early
4. **Profile performance** to verify you're getting the expected benefits
5. **Consider effect combinations** - multiple compatible effects can be used together efficiently

## Example Configurations

### Cinematic Rendering
```cpp
// Bloom + Tonemap + Vignette for cinematic look
config.flags.x = PostProcessConfiguration::ENABLE_BLOOM_BIT | 
                 PostProcessConfiguration::ENABLE_TONEMAP_BIT |
                 PostProcessConfiguration::ENABLE_VIGNETTE_BIT;
```

### Clean Image Quality
```cpp  
// FXAA + Dither for clean anti-aliased output
config.flags.x = PostProcessConfiguration::ENABLE_FXAA_BIT |
                 PostProcessConfiguration::ENABLE_DITHER_BIT;
```

### Stylized Effects
```cpp
// Color processing for stylized visuals
config.flags.x = PostProcessConfiguration::ENABLE_COLOR_CONVERSION_BIT |
                 PostProcessConfiguration::ENABLE_COLOR_FRINGE_BIT |
                 PostProcessConfiguration::ENABLE_BLUR_BIT;
```

This LIGHT_FORWARD post-processing support provides an excellent balance of performance and visual quality for many common rendering scenarios.