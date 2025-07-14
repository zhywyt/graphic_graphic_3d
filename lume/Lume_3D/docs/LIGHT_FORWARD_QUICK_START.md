# LIGHT_FORWARD Post-Processing Quick Start

## Quick Setup

```cpp
#include <3d/ecs/components/camera_component.h>
#include <render/datastore/render_data_store_render_pods.h>

// 1. Configure camera for LIGHT_FORWARD pipeline
RenderCamera camera;
camera.renderPipelineType = RenderCamera::RenderPipelineType::LIGHT_FORWARD;
camera.postProcessName = "CORE3D_POST_PROCESS_CAM";

// 2. Configure compatible post-processing effects
PostProcessConfiguration ppConfig = {};

// Enable bloom + tonemap + FXAA (all compatible with LIGHT_FORWARD)
ppConfig.flags.x = PostProcessConfiguration::ENABLE_BLOOM_BIT | 
                   PostProcessConfiguration::ENABLE_TONEMAP_BIT |
                   PostProcessConfiguration::ENABLE_FXAA_BIT;

// Set effect parameters
ppConfig.factors[PostProcessConfiguration::INDEX_BLOOM] = 1.0f;     // Bloom strength
ppConfig.factors[PostProcessConfiguration::INDEX_TONEMAP] = 1.0f;   // Exposure
ppConfig.factors[PostProcessConfiguration::INDEX_FXAA] = 0.75f;     // FXAA quality

// 3. The pipeline automatically handles:
//    - HDR render target creation
//    - Compatible effect validation
//    
// Note: MSAA is not supported with LIGHT_FORWARD post-processing
// Use non-MSAA rendering or switch to FORWARD/DEFERRED pipeline
```

## Compatible Effects Checklist

✅ **Works with LIGHT_FORWARD:**
- Tonemap, Vignette, Dither
- Color Conversion, Color Fringe  
- Blur, Bloom, FXAA

❌ **Requires FORWARD/DEFERRED:**
- TAA (temporal anti-aliasing)
- Motion Blur, Depth of Field
- Lens Flare

## Validation Output

**Success:**
```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline using current-frame-only post-processing (flags: 0x300)
```

**Warning (incompatible effects):**
```
CORE3D_VALIDATION: LIGHT_FORWARD pipeline has incompatible post-processing effects (enabled: 0x1800, incompatible: 0x1000)
```

For complete documentation, see [LIGHT_FORWARD_POST_PROCESSING.md](LIGHT_FORWARD_POST_PROCESSING.md)