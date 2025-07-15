/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SR_COMMON_H
#define SR_COMMON_H

// Common definitions for spatial GPU upscaling shaders

// Quality levels
#define SR_QUALITY_PERFORMANCE  0
#define SR_QUALITY_BALANCED     1  
#define SR_QUALITY_ULTRA        2

// Edge detection parameters
#define SR_EDGE_SOBEL_THRESHOLD     0.1
#define SR_EDGE_DEPTH_THRESHOLD     0.01
#define SR_EDGE_LUMA_WEIGHT         0.299, 0.587, 0.114

// Spatial upscaling parameters
#define SR_UPSCALE_BICUBIC_A        -0.5    // Bicubic interpolation parameter
#define SR_UPSCALE_EDGE_PRESERVE    0.8     // Edge preservation strength
#define SR_UPSCALE_DETAIL_ENHANCE   1.2     // Detail enhancement factor

// Sharpening parameters  
#define SR_SHARPEN_KERNEL_SIZE      3       // Unsharp mask kernel size
#define SR_SHARPEN_CONTRAST_LIMIT   2.0     // Maximum contrast enhancement
#define SR_SHARPEN_NOISE_THRESHOLD  0.05    // Noise suppression threshold

// Thread group sizes (must match C++ code)
#define SR_THREAD_GROUP_SIZE_X      8
#define SR_THREAD_GROUP_SIZE_Y      8
#define SR_THREAD_GROUP_SIZE_Z      1

// Utility functions
float sr_luminance(vec3 color) {
    return dot(color, vec3(SR_EDGE_LUMA_WEIGHT));
}

// Bicubic interpolation weight function
float sr_bicubic_weight(float x) {
    float a = SR_UPSCALE_BICUBIC_A;
    float absx = abs(x);
    if (absx <= 1.0) {
        return (a + 2.0) * absx * absx * absx - (a + 3.0) * absx * absx + 1.0;
    } else if (absx <= 2.0) {
        return a * absx * absx * absx - 5.0 * a * absx * absx + 8.0 * a * absx - 4.0 * a;
    }
    return 0.0;
}

// Sobel edge detection
vec2 sr_sobel_edge(sampler2D tex, vec2 uv, vec2 texelSize) {
    // Sample 3x3 neighborhood
    float tl = sr_luminance(texture(tex, uv + vec2(-texelSize.x, -texelSize.y)).rgb);
    float tm = sr_luminance(texture(tex, uv + vec2(0.0, -texelSize.y)).rgb);
    float tr = sr_luminance(texture(tex, uv + vec2(texelSize.x, -texelSize.y)).rgb);
    float ml = sr_luminance(texture(tex, uv + vec2(-texelSize.x, 0.0)).rgb);
    float mr = sr_luminance(texture(tex, uv + vec2(texelSize.x, 0.0)).rgb);
    float bl = sr_luminance(texture(tex, uv + vec2(-texelSize.x, texelSize.y)).rgb);
    float bm = sr_luminance(texture(tex, uv + vec2(0.0, texelSize.y)).rgb);
    float br = sr_luminance(texture(tex, uv + vec2(texelSize.x, texelSize.y)).rgb);
    
    // Sobel operators
    float gx = (tr + 2.0 * mr + br) - (tl + 2.0 * ml + bl);
    float gy = (bl + 2.0 * bm + br) - (tl + 2.0 * tm + tr);
    
    return vec2(gx, gy);
}

#endif // SR_COMMON_H