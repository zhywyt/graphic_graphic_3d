# Spatial GPU Upscaling Render Node Design Document

## Overview

This document describes the design and implementation of a spatial GPU upscaling render node (`render_node_sr`) for the Lume rendering framework. The implementation is based on HarmonyOS XEngine Kit GPU spatial upscaling capabilities and supports both GLES and Vulkan backends.

## Architecture

### Component Overview

The spatial upscaling implementation follows the existing Lume post-processing architecture with the following components:

1. **RenderNodeSr** - Main render node interface
2. **RenderPostProcessSr** - Post-process effect interface  
3. **RenderPostProcessSrNode** - Actual implementation of spatial upscaling logic

### Class Hierarchy

```
IRenderNode
    └── RenderNodeSr

IRenderPostProcess  
    └── RenderPostProcessSr

IRenderPostProcessNode
    └── RenderPostProcessSrNode
```

## Design Principles

### 1. Spatial-Only Upscaling
Unlike temporal super-resolution techniques (FSR2, DLSS), this implementation focuses purely on spatial domain upscaling techniques:
- Edge-aware interpolation
- Gradient-based filtering  
- Multi-pass sharpening
- Contrast-aware sampling

### 2. Backend Agnostic
The implementation supports both GLES and Vulkan backends through:
- Unified shader interface
- Backend-agnostic resource management
- Cross-platform compute shader support

### 3. Configurable Quality
Multiple quality levels and parameters:
- Upscaling ratio (1.5x, 2.0x, 2.5x, 3.0x, 4.0x)
- Edge detection sensitivity
- Sharpening strength
- Filter kernel size

## Implementation Details

### Input/Output Structure

```cpp
struct NodeInputs {
    RENDER_NS::BindableImage input;      // Source low-resolution image
    RENDER_NS::BindableImage depth;      // Optional depth buffer for edge detection
};

struct NodeOutputs {
    RENDER_NS::BindableImage output;     // Upscaled high-resolution image
};
```

### Spatial Upscaling Algorithm

The implementation uses a multi-pass approach:

#### Pass 1: Edge Detection
- Sobel edge detection on luminance channel
- Depth-aware edge enhancement (if depth available)
- Generate edge mask for selective processing

#### Pass 2: Initial Upscaling  
- Bicubic interpolation as baseline
- Edge-guided interpolation for edge pixels
- Preserve sharp edges while smoothing gradients

#### Pass 3: Sharpening & Enhancement
- Unsharp masking for detail enhancement
- Contrast-adaptive local sharpening
- Noise reduction in flat areas

#### Pass 4: Output Processing
- Tone mapping adjustments
- Optional gamma correction
- Final output formatting

### Compute Shader Pipeline

```
Input Image (Low-Res) 
    ↓
Edge Detection Pass (CS)
    ↓  
Edge Mask + Gradient Info
    ↓
Spatial Upscaling Pass (CS)
    ↓
Intermediate High-Res
    ↓
Sharpening Pass (CS)
    ↓
Final Output (High-Res)
```

### Resource Management

#### Intermediate Textures
- Edge mask texture (R8_UNORM)
- Gradient texture (RG16F)
- Intermediate upscaled texture (same format as input)

#### Uniform Buffers
- Upscaling parameters (ratio, thresholds)
- Edge detection configuration
- Sharpening parameters

### Quality Configurations

#### Performance Mode
- 2-pass implementation
- Simple bicubic upscaling
- Basic edge preservation
- Minimal compute requirements

#### Quality Mode  
- 4-pass implementation
- Advanced edge detection
- Gradient-aware interpolation
- Enhanced sharpening

#### Ultra Mode
- 6-pass implementation
- Multi-scale edge analysis
- Adaptive interpolation
- Advanced noise reduction

## API Interface

### Configuration Parameters

```cpp
struct SpatialUpscalingConfig {
    float upscaleRatio = 2.0f;           // Target upscaling factor
    float edgeSensitivity = 0.5f;        // Edge detection threshold
    float sharpeningStrength = 0.3f;     // Post-upscaling sharpening
    uint32_t qualityLevel = 1;           // 0=Performance, 1=Quality, 2=Ultra
    bool useDepthEdges = true;           // Use depth buffer for edge detection
    bool enableSharpening = true;        // Enable post-process sharpening
};
```

### Usage in Render Graph

```json
{
  "renderNodeType": "RenderNodeSr",
  "nodeName": "SPATIAL_UPSCALING",
  "resources": {
    "customInputImages": [
      {
        "resourceName": "LOW_RES_COLOR",
        "binding": 0
      },
      {
        "resourceName": "DEPTH_BUFFER", 
        "binding": 1
      }
    ],
    "customOutputImages": [
      {
        "resourceName": "HIGH_RES_COLOR",
        "binding": 0
      }
    ]
  },
  "renderDataStore": {
    "dataStoreName": "PostProcessDataStore",
    "configurationName": "SpatialUpscalingConfig"
  }
}
```

## Performance Considerations

### GPU Utilization
- Optimized for modern GPU architectures
- Compute shader-based implementation
- Efficient memory access patterns
- Minimal CPU-GPU synchronization

### Memory Usage
- Intermediate texture count minimized
- Reuse textures between passes where possible
- Support for different precision formats (FP16/FP32)

### Scalability
- Works with various input resolutions
- Adaptive LOD selection based on GPU capabilities
- Graceful quality degradation on low-end hardware

## Backend Compatibility

### GLES Support
- OpenGL ES 3.2+ compute shaders
- EXT_texture_filter_anisotropic for quality enhancement
- Fallback vertex/fragment shader path for older hardware

### Vulkan Support  
- Full compute pipeline utilization
- Efficient descriptor set management
- Timeline semaphores for optimal GPU scheduling

## Testing Strategy

### Functional Testing
- Various input resolutions (720p, 1080p, 1440p)
- Different upscaling ratios (1.5x - 4.0x)
- Edge cases (solid colors, high-frequency patterns)
- Backend switching validation

### Performance Testing
- Frame time analysis across quality levels
- Memory usage profiling
- GPU utilization monitoring
- Power consumption measurement

### Quality Assessment
- PSNR/SSIM metrics vs reference implementations
- Visual quality assessment
- Edge preservation analysis
- Artifact detection

## Future Enhancements

### Machine Learning Integration
- Neural network-based upscaling kernels
- Runtime model selection based on content
- Adaptive quality prediction

### Advanced Algorithms
- Multi-frame temporal information (hybrid approach)
- Content-aware parameter tuning
- Perceptual quality optimization

### Platform-Specific Optimizations
- Mali GPU optimizations
- Adreno GPU specific tuning
- Power efficiency improvements

## Dependencies

### Core Dependencies
- LumeBase: Math utilities, containers
- LumeEngine: Core engine interfaces
- LumeRender: Rendering framework

### Optional Dependencies  
- Depth buffer for enhanced edge detection
- Motion vectors for future temporal hybrid mode
- HDR metadata for tone mapping integration

## File Structure

```
lume/LumeRender/src/node/
├── render_node_sr.h                    # Main render node interface
└── render_node_sr.cpp                  # Main render node implementation

lume/LumeRender/src/postprocesses/
├── render_post_process_sr.h            # Post-process interface
├── render_post_process_sr.cpp          # Post-process implementation  
├── render_post_process_sr_node.h       # Post-process node interface
└── render_post_process_sr_node.cpp     # Post-process node implementation

lume/LumeRender/assets/render/shaders/computeshader/
├── sr_edge_detection.comp              # Edge detection compute shader
├── sr_spatial_upscale.comp             # Main upscaling compute shader
├── sr_sharpening.comp                  # Post-processing sharpening
└── sr_common.h                         # Common definitions and utilities
```

This design provides a solid foundation for spatial GPU upscaling while maintaining compatibility with the existing Lume rendering architecture and supporting both GLES and Vulkan backends.