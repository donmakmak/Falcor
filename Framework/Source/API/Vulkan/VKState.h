/***************************************************************************
# Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
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
#pragma once
#include "API/GraphicsStateObject.h"

namespace Falcor
{
    struct BlendStateCreateInfo
    {
        std::vector<VkPipelineColorBlendAttachmentState> attachmentStates;
        VkPipelineColorBlendStateCreateInfo info;
    };

    struct VertexInputStateCreateInfo
    {
        std::vector<VkVertexInputBindingDescription> bindingDescs;
        std::vector<VkVertexInputAttributeDescription> attribDescs;
        VkPipelineVertexInputStateCreateInfo info;
    };

    struct RenderPassCreateInfo
    {
        std::vector<VkAttachmentDescription> attachmentDescs;
        std::vector<VkAttachmentReference> attachmentRefs;
        std::vector<VkSubpassDescription> subpassDescs;
        std::vector<VkSubpassDependency> subpassDependencies;
        VkRenderPassCreateInfo info;
    };

    void initVkBlendInfo(const BlendState* pState, BlendStateCreateInfo& infoOut);
    void initVkRasterizerInfo(const RasterizerState* pState, VkPipelineRasterizationStateCreateInfo& infoOut);
    void initVkDepthStencilInfo(const DepthStencilState* pState, VkPipelineDepthStencilStateCreateInfo& infoOut);
    void initVkVertexLayoutInfo(const VertexLayout* pLayout, VertexInputStateCreateInfo& infoOut);
    void initVkSamplerInfo(const Sampler* pSampler, VkSamplerCreateInfo& infoOut);
    void initVkMultiSampleInfo(const BlendState* pState, const Fbo::Desc& fboDesc, const uint32_t& sampleMask, VkPipelineMultisampleStateCreateInfo& infoOut);
    void initVkInputAssemblyInfo(const Vao* pVao, VkPipelineInputAssemblyStateCreateInfo& infoOut);
    void initVkRenderPassInfo(const Fbo::Desc& fboDesc, const DepthStencilState* pDsState, RenderPassCreateInfo& infoOut);
}
