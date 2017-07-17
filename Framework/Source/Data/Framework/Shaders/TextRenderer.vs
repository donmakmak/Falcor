/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#version 450
#extension GL_ARB_separate_shader_objects : enable
#ifdef FALCOR_HLSL
cbuffer PerFrameCB : register(b0)
{
	float4x4 gvpTransform;
	float3 gFontColor;
};

void main(float2 posS : POSITION, inout float2 texC : TEXCOORD, out float4 posSV : SV_POSITION)
{
	posSV = mul(float4(posS, 0.5f, 1), gvpTransform);
}
#endif
#ifdef FALCOR_GLSL
layout(set = 0, binding = 0) uniform PerFrameCB
{
	mat4 gvpTransform;
	vec3 gFontColor;
};

layout (location = 0) in vec2 posS;
layout (location = 1) in vec2 texCIn;
layout (location = 0) out vec2 texC;

void main()
{
	gl_Position = gvpTransform * vec4(posS, 0.5f, 1);
    gl_Position.y = -gl_Position.y;
	texC = texCIn;
}
#endif