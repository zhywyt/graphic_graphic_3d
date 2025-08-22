{
    "compatibility_info" : {
        "version" : "22.00",
        "type" : "shader"
    },
    "category": "3D",
    "displayName": "Environment Alpha",
    "shaders": [
        {
            "displayName": "Environment with Alpha",
            "variantName": "ENV_ALPHA",
            "vert": "3dshaders://shader/core3d_dm_env.vert.spv",
            "frag": "3dshaders://shader/core3d_dm_env_alpha.frag.spv",
            "pipelineLayout": "3dpipelinelayouts://core3d_dm_env.shaderpl",
            "renderSlot": "CORE3D_RS_DM_ENV_ALPHA",
            "renderSlotDefaultShader": true,
            "state": {
                "rasterizationState": {
                    "enableDepthClamp": false,
                    "enableDepthBias": false,
                    "enableRasterizerDiscard": false,
                    "polygonMode": "fill",
                    "cullModeFlags": "none",
                    "frontFace": "counter_clockwise"
                },
                "depthStencilState": {
                    "enableDepthTest": true,
                    "enableDepthWrite": false,
                    "enableDepthBoundsTest": false,
                    "enableStencilTest": false,
                    "depthCompareOp": "less_or_equal"
                },
                "colorBlendState": {
                    "colorAttachments": [
                        {
                            "enableBlend": true,
                            "srcColorBlendFactor": "src_alpha",
                            "dstColorBlendFactor": "one_minus_src_alpha",
                            "colorBlendOp": "add",
                            "srcAlphaBlendFactor": "one",
                            "dstAlphaBlendFactor": "one_minus_src_alpha",
                            "alphaBlendOp": "add",
                            "colorWriteMask": "r_bit|g_bit|b_bit|a_bit"
                        },
                        {
                            "colorWriteMask": "r_bit|g_bit|b_bit|a_bit"
                        }
                    ]
                }
            }
        }
    ]
}