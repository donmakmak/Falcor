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
#include "Framework.h"
#include "VKState.h"
#include "API/FBO.h"

namespace Falcor
{
    VkBlendFactor getVkBlendFactor(BlendState::BlendFunc func)
    {
        switch (func)
        {
        case BlendState::BlendFunc::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendState::BlendFunc::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendState::BlendFunc::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendState::BlendFunc::OneMinusSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendState::BlendFunc::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendState::BlendFunc::OneMinusDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendState::BlendFunc::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendState::BlendFunc::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendState::BlendFunc::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendState::BlendFunc::OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendState::BlendFunc::RgbaFactor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendState::BlendFunc::OneMinusRgbaFactor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendState::BlendFunc::SrcAlphaSaturate:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendState::BlendFunc::Src1Color:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendState::BlendFunc::OneMinusSrc1Color:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendState::BlendFunc::Src1Alpha:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendState::BlendFunc::OneMinusSrc1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            should_not_get_here();
            return VK_BLEND_FACTOR_ZERO;
        }
    }

    VkBlendOp getVkBlendOp(BlendState::BlendOp op)
    {
        switch (op)
        {
        case BlendState::BlendOp::Add:
            return VK_BLEND_OP_ADD;
        case BlendState::BlendOp::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case BlendState::BlendOp::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendState::BlendOp::Min:
            return VK_BLEND_OP_MIN;
        case BlendState::BlendOp::Max:
            return VK_BLEND_OP_MAX;
        default:
            should_not_get_here();
            return VK_BLEND_OP_ADD;
        }
    }

    void initVkBlendInfo(const BlendState* pState, std::vector<VkPipelineColorBlendAttachmentState>& attachmentStateOut, VkPipelineColorBlendStateCreateInfo& infoOut)
    {
        infoOut = {};

        infoOut.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        infoOut.logicOpEnable = VK_FALSE;
        infoOut.attachmentCount = (uint32_t)pState->getRtCount();

        attachmentStateOut.resize(infoOut.attachmentCount);
        for (uint32_t i = 0; i < infoOut.attachmentCount; i++)
        {
            const BlendState::Desc::RenderTargetDesc& rtDesc = pState->getRtDesc(i);
            VkPipelineColorBlendAttachmentState& state = attachmentStateOut[i];
            state.blendEnable = vkBool(rtDesc.blendEnabled);
            state.srcColorBlendFactor = getVkBlendFactor(rtDesc.srcRgbFunc);
            state.dstColorBlendFactor = getVkBlendFactor(rtDesc.dstRgbFunc);
            state.colorBlendOp = getVkBlendOp(rtDesc.rgbBlendOp);
            state.srcAlphaBlendFactor = getVkBlendFactor(rtDesc.srcAlphaFunc);
            state.dstAlphaBlendFactor = getVkBlendFactor(rtDesc.dstAlphaFunc);
            state.alphaBlendOp = getVkBlendOp(rtDesc.alphaBlendOp);

            state.colorWriteMask = rtDesc.writeMask.writeRed ? VK_COLOR_COMPONENT_R_BIT : 0;
            state.colorWriteMask |= rtDesc.writeMask.writeGreen ? VK_COLOR_COMPONENT_G_BIT : 0;
            state.colorWriteMask |= rtDesc.writeMask.writeBlue ? VK_COLOR_COMPONENT_B_BIT : 0;
            state.colorWriteMask |= rtDesc.writeMask.writeAlpha ? VK_COLOR_COMPONENT_A_BIT : 0;
        }
    }

    VkPolygonMode getVkPolygonMode(RasterizerState::FillMode fill)
    {
        switch (fill)
        {
        case RasterizerState::FillMode::Solid:
            return VK_POLYGON_MODE_FILL;
        case RasterizerState::FillMode::Wireframe:
            return VK_POLYGON_MODE_LINE;
        default:
            should_not_get_here();
            return VK_POLYGON_MODE_FILL;
        }
    }

    VkCullModeFlags getVkCullMode(RasterizerState::CullMode cull)
    {
        switch (cull)
        {
        case Falcor::RasterizerState::CullMode::None:
            return VK_CULL_MODE_NONE;
        case Falcor::RasterizerState::CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case Falcor::RasterizerState::CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        default:
            should_not_get_here();
            return VK_CULL_MODE_NONE;
        }
    }

    void initVkRasterizerInfo(const RasterizerState* pState, VkPipelineRasterizationStateCreateInfo& infoOut)
    {
        infoOut = {};

        infoOut.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        infoOut.depthClampEnable = VK_FALSE;
        infoOut.rasterizerDiscardEnable = VK_TRUE;
        infoOut.polygonMode = getVkPolygonMode(pState->getFillMode());
        infoOut.cullMode = getVkCullMode(pState->getCullMode());
        infoOut.frontFace = pState->isFrontCounterCW() ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        infoOut.depthBiasEnable = vkBool(pState->getDepthBias() > 0);
        infoOut.depthBiasConstantFactor = (float)pState->getDepthBias();
        infoOut.depthBiasClamp = 0.0f;
        infoOut.depthBiasSlopeFactor = pState->getSlopeScaledDepthBias();
        infoOut.lineWidth = 1.0f;
    }

    template<typename FalcorOp>
    VkCompareOp getVkCompareOp(FalcorOp op)
    {
        switch (op)
        {
        case FalcorOp::Disabled:
            return VK_COMPARE_OP_ALWAYS;
        case FalcorOp::Never:
            return VK_COMPARE_OP_NEVER;
        case FalcorOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        case FalcorOp::Less:
            return VK_COMPARE_OP_LESS;
        case FalcorOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case FalcorOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case FalcorOp::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case FalcorOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case FalcorOp::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:
            should_not_get_here();
            return VK_COMPARE_OP_ALWAYS;
        }
    }

    VkStencilOp getVkStencilOp(DepthStencilState::StencilOp op)
    {
        switch (op)
        {
        case DepthStencilState::StencilOp::Keep:
            return VK_STENCIL_OP_KEEP;
        case DepthStencilState::StencilOp::Zero:
            return VK_STENCIL_OP_ZERO;
        case DepthStencilState::StencilOp::Replace:
            return VK_STENCIL_OP_REPLACE;
        case DepthStencilState::StencilOp::Increase:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case DepthStencilState::StencilOp::IncreaseSaturate:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case DepthStencilState::StencilOp::Decrease:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case DepthStencilState::StencilOp::DecreaseSaturate:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case DepthStencilState::StencilOp::Invert:
            return VK_STENCIL_OP_INVERT;
        default:
            should_not_get_here();
            return VK_STENCIL_OP_KEEP;
        }
    }

    VkStencilOpState getVkStencilOpState(const DepthStencilState::StencilDesc& desc, uint8_t readMask, uint8_t writeMask, uint8_t stencilRef)
    {
        VkStencilOpState opState = {};
        opState.failOp = getVkStencilOp(desc.stencilFailOp);
        opState.passOp = getVkStencilOp(desc.depthStencilPassOp);
        opState.depthFailOp = getVkStencilOp(desc.depthFailOp);
        opState.compareOp = getVkCompareOp(desc.func);
        opState.compareMask = readMask;
        opState.writeMask = writeMask;
        opState.reference = stencilRef;

        return opState;
    }

    void initVkDepthStencilInfo(const DepthStencilState* pState, VkPipelineDepthStencilStateCreateInfo& infoOut)
    {
        infoOut = {};

        infoOut.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        infoOut.depthTestEnable = vkBool(pState->isDepthTestEnabled());
        infoOut.depthWriteEnable = vkBool(pState->isDepthWriteEnabled());
        infoOut.depthCompareOp = getVkCompareOp(pState->getDepthFunc());
        infoOut.stencilTestEnable = vkBool(pState->isStencilTestEnabled());
        infoOut.front = getVkStencilOpState(pState->getStencilDesc(DepthStencilState::Face::Front), pState->getStencilReadMask(), pState->getStencilWriteMask(), pState->getStencilRef());
        infoOut.back = getVkStencilOpState(pState->getStencilDesc(DepthStencilState::Face::Back), pState->getStencilReadMask(), pState->getStencilWriteMask(), pState->getStencilRef());

        // #VKTODO do we need this?
        infoOut.depthBoundsTestEnable = VK_FALSE;
    }

    VkVertexInputRate getVkInputRate(VertexBufferLayout::InputClass falcorClass)
    {
        switch (falcorClass)
        {
        case VertexBufferLayout::InputClass::PerVertexData:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case VertexBufferLayout::InputClass::PerInstanceData:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
        default:
            should_not_get_here();
            return VK_VERTEX_INPUT_RATE_VERTEX;
        }
    }

    void initVkVertexLayoutInfo(const VertexLayout* pLayout, std::vector<VkVertexInputBindingDescription>& bindingDescs, std::vector<VkVertexInputAttributeDescription>& attribDescs, VkPipelineVertexInputStateCreateInfo& infoOut)
    {
        //
        // Build Vertex input and binding info
        //

        bindingDescs.clear();
        attribDescs.clear();

        for (size_t vb = 0; vb < pLayout->getBufferCount(); vb++)
        {
            const auto& pVB = pLayout->getBufferLayout(vb);
            if (pVB)
            {
                // Per buffer binding
                VkVertexInputBindingDescription& bindingDesc = bindingDescs[vb];
                bindingDesc.binding = (uint32_t)vb;
                bindingDesc.stride = pVB->getStride();
                bindingDesc.inputRate = getVkInputRate(pVB->getInputClass());
                bindingDescs.push_back(bindingDesc);

                for (uint32_t elemID = 0; elemID < pVB->getElementCount(); elemID++)
                {
                    // Per shader location specified
                    VkVertexInputAttributeDescription attribDesc = {};
                    attribDesc.location = pVB->getElementShaderLocation(elemID);
                    attribDesc.binding = (uint32_t)vb;
                    attribDesc.format = getVkFormat(pVB->getElementFormat(elemID));
                    attribDesc.offset = pVB->getElementOffset(elemID);

                    for (uint32_t i = 0; i < pVB->getElementArraySize(elemID); i++)
                    {
                        attribDescs.push_back(attribDesc);
                        attribDesc.offset += getFormatBytesPerBlock(pVB->getElementFormat(elemID));
                    }
                }
            }
        }

        //
        // Now put together the actual layout create info
        //

        infoOut = {};

        infoOut.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        infoOut.vertexBindingDescriptionCount = (uint32_t)bindingDescs.size();
        infoOut.pVertexBindingDescriptions = bindingDescs.data();
        infoOut.vertexAttributeDescriptionCount = (uint32_t)attribDescs.size();
        infoOut.pVertexAttributeDescriptions = attribDescs.data();
    }

    VkFilter getVkFilter(Sampler::Filter filter)
    {
        switch (filter)
        {
        case Sampler::Filter::Point:
            return VK_FILTER_NEAREST;
        case Sampler::Filter::Linear:
            return VK_FILTER_LINEAR;
        default:
            should_not_get_here();
            return VK_FILTER_NEAREST;
        }
    }

    VkSamplerMipmapMode getVkMipMapFilterMode(Sampler::Filter filter)
    {
        switch (filter)
        {
        case Sampler::Filter::Point:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case Sampler::Filter::Linear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            should_not_get_here();
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    VkSamplerAddressMode getVkAddressMode(Sampler::AddressMode mode)
    {
        switch (mode)
        {
        case Sampler::AddressMode::Wrap:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case Sampler::AddressMode::Mirror:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case Sampler::AddressMode::Clamp:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case Sampler::AddressMode::Border:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case Sampler::AddressMode::MirrorOnce:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default:
            should_not_get_here();
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    void initVkSamplerInfo(const Sampler* pSampler, VkSamplerCreateInfo& infoOut)
    {
        infoOut = {};

        infoOut.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        infoOut.magFilter = getVkFilter(pSampler->getMagFilter());
        infoOut.minFilter = getVkFilter(pSampler->getMinFilter());
        infoOut.mipmapMode = getVkMipMapFilterMode(pSampler->getMipFilter());
        infoOut.addressModeU = getVkAddressMode(pSampler->getAddressModeU());
        infoOut.addressModeV = getVkAddressMode(pSampler->getAddressModeV());
        infoOut.addressModeW = getVkAddressMode(pSampler->getAddressModeW());
        infoOut.mipLodBias = pSampler->getLodBias();
        infoOut.anisotropyEnable = vkBool(pSampler->getMaxAnisotropy() > 1);
        infoOut.maxAnisotropy = (float)pSampler->getMaxAnisotropy();
        infoOut.compareEnable = vkBool(pSampler->getComparisonMode() != Sampler::ComparisonMode::Disabled);
        infoOut.compareOp = getVkCompareOp(pSampler->getComparisonMode());
        infoOut.minLod = pSampler->getMinLod();
        infoOut.maxLod = pSampler->getMaxLod();
        infoOut.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        infoOut.unnormalizedCoordinates = VK_FALSE;
    }

    void convertVkViewport(const GraphicsStateObject::Viewport& falcorViewport, VkViewport& vkViewportOut)
    {
        vkViewportOut.x = falcorViewport.originX;
        vkViewportOut.y = falcorViewport.originY;
        vkViewportOut.width = falcorViewport.width;
        vkViewportOut.height = falcorViewport.height;
        vkViewportOut.minDepth = falcorViewport.minDepth;
        vkViewportOut.maxDepth = falcorViewport.maxDepth;
    }

    void convertVkScissor(const GraphicsStateObject::Scissor& falcorScissor, VkRect2D& vkScissorOut)
    {
        vkScissorOut.offset.x = falcorScissor.left;
        vkScissorOut.offset.y = falcorScissor.top;
        vkScissorOut.extent.width = falcorScissor.right - falcorScissor.left;
        vkScissorOut.extent.height = falcorScissor.bottom - falcorScissor.top;
    }

    void initVkViewportInfo(
        const std::vector<GraphicsStateObject::Viewport>& viewports, const std::vector<GraphicsStateObject::Scissor>& scissors,
        std::vector<VkViewport>& vkViewportsOut, std::vector<VkRect2D>& vkScissorsOut,
        VkPipelineViewportStateCreateInfo& infoOut)
    {
        vkViewportsOut.resize(viewports.size());
        vkScissorsOut.resize(scissors.size());

        infoOut = {};

        infoOut.viewportCount = (uint32_t)viewports.size();
        for (uint32_t i = 0; i < infoOut.viewportCount; i++)
        {
            convertVkViewport(viewports[i], vkViewportsOut[i]);
        }
        
        infoOut.scissorCount = (uint32_t)scissors.size();
        for (uint32_t i = 0; i < infoOut.scissorCount; i++)
        {
            convertVkScissor(scissors[i], vkScissorsOut[i]);
        }
    }

    void initVkMultiSampleInfo(const BlendState* pState, const Fbo::Desc& fboDesc, const uint32_t& sampleMask, VkPipelineMultisampleStateCreateInfo& infoOut)
    {
        infoOut = {};

        infoOut.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        infoOut.rasterizationSamples = (VkSampleCountFlagBits)fboDesc.getSampleCount();
        infoOut.sampleShadingEnable = VK_FALSE;
        infoOut.minSampleShading = 0.0f;
        infoOut.pSampleMask = &sampleMask;
        infoOut.alphaToCoverageEnable = vkBool(pState->isAlphaToCoverageEnabled());
        infoOut.alphaToOneEnable = VK_FALSE;
    }

    VkPrimitiveTopology getVkPrimitiveTopology(Vao::Topology topology)
    {
        switch (topology)
        {
        case Vao::Topology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case Vao::Topology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case Vao::Topology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case Vao::Topology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default:
            should_not_get_here();
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }
    }

    void initVkInputAssemblyInfo(const Vao* pVao, VkPipelineInputAssemblyStateCreateInfo& infoOut)
    {
        infoOut = {};

        infoOut.topology = getVkPrimitiveTopology(pVao->getPrimitiveTopology());
        infoOut.primitiveRestartEnable = VK_FALSE;
    }

}
