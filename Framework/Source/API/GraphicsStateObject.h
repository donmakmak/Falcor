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
#pragma once
#include "API/VertexLayout.h"
#include "API/FBO.h"
#include "API/ProgramVersion.h"
#include "API/RasterizerState.h"
#include "API/DepthStencilState.h"
#include "API/BlendState.h"
#include "API/LowLevel/RootSignature.h"
#include "API/VAO.h"

namespace Falcor
{
    class RenderContext;
    class GraphicsState;

    class GraphicsStateObject
    {
    public:
        using SharedPtr = std::shared_ptr<GraphicsStateObject>;
        using SharedConstPtr = std::shared_ptr<const GraphicsStateObject>;
        using ApiHandle = PsoHandle;

        ~GraphicsStateObject();

        static const uint32_t kSampleMaskAll = -1;

        /** Primitive topology
        */
        enum class PrimitiveType
        {
            Undefined,
            Point,
            Line,
            Triangle,
            Patch,
        };

        struct Viewport
        {
            Viewport() = default;
            Viewport(float x, float y, float w, float h, float minZ, float maxZ) : originX(x), originY(y), width(w), height(h), minDepth(minZ), maxDepth(maxZ) {}
            float originX = 0;
            float originY = 0;
            float width = 0;
            float height = 0;
            float minDepth = 0;
            float maxDepth = 1;
        };

        struct Scissor
        {
            Scissor() = default;
            Scissor(int32_t l, int32_t t, int32_t r, int32_t b) : left(l), top(t), right(r), bottom(b) {}
            int32_t left = 0;
            int32_t top = 0;
            int32_t right = 0;
            int32_t bottom = 0;
        };

        class Desc
        {
        public:
#ifdef FALCOR_VK
            Desc();
#endif

            Desc& setRootSignature(RootSignature::SharedPtr pSignature) { mpRootSignature = pSignature; return *this; }
            Desc& setVertexLayout(VertexLayout::SharedConstPtr pLayout) { mpLayout = pLayout; return *this; }
            Desc& setFboFormats(const Fbo::Desc& fboFormats) { mFboDesc = fboFormats; return *this; }
            Desc& setProgramVersion(ProgramVersion::SharedConstPtr pProgram) { mpProgram = pProgram; return *this; }
            Desc& setBlendState(BlendState::SharedPtr pBlendState) { mpBlendState = pBlendState; return *this; }
            Desc& setRasterizerState(RasterizerState::SharedPtr pRasterizerState) { mpRasterizerState = pRasterizerState; return *this; }
            Desc& setDepthStencilState(DepthStencilState::SharedPtr pDepthStencilState) { mpDepthStencilState = pDepthStencilState; return *this; }
            Desc& setSampleMask(uint32_t sampleMask) { mSampleMask = sampleMask; return *this; }
            Desc& setPrimitiveType(PrimitiveType type) { mPrimType = type; return *this; }
            Desc& setSinglePassStereoEnable(bool sps) { mSinglePassStereoEnabled = sps; return *this; }

            BlendState::SharedPtr getBlendState() const { return mpBlendState; }
            RasterizerState::SharedPtr getRasterizerState() const { return mpRasterizerState; }
            DepthStencilState::SharedPtr getDepthStencilState() const { return mpDepthStencilState; }
            uint32_t getSampleMask() const { return mSampleMask; }
            GraphicsStateObject::PrimitiveType getPrimitiveType() const { return mPrimType; }
            VertexLayout::SharedConstPtr getVertexLayout() const { return mpLayout; }
            const Fbo::Desc& getFboDesc() const { return mFboDesc; }
            ProgramVersion::SharedConstPtr getProgramVersion() const { return mpProgram; }

            bool getSinglePassStereoEnabled() const { return mSinglePassStereoEnabled; }

            bool operator==(const Desc& other) const;

#ifdef FALCOR_VK
            Desc& setViewport(uint32_t index, const Viewport& vp) { mViewports[index] = vp; return *this; }
            Desc& setScissor(uint32_t index, const Scissor& sc) { mScissors[index] = sc; return *this; }
            Desc& setVao(const Vao::SharedConstPtr& pVao) { mpVao = pVao; return *this; }

            const Viewport& getViewport(uint32_t index) const { return mViewports[index]; }
            const Scissor& getScissor(uint32_t index) const { return mScissors[index]; }
            const Vao::SharedConstPtr& getVao() const { return mpVao; }

            const std::vector<Viewport>& getViewports() const { return mViewports; }
            const std::vector<Scissor>& getScissors() const { return mScissors; }
#endif
        private:
            friend class GraphicsStateObject;
            VertexLayout::SharedConstPtr mpLayout;
            Fbo::Desc mFboDesc;
            ProgramVersion::SharedConstPtr mpProgram;
            RasterizerState::SharedPtr mpRasterizerState;
            DepthStencilState::SharedPtr mpDepthStencilState;
            BlendState::SharedPtr mpBlendState;
            uint32_t mSampleMask = kSampleMaskAll;
            RootSignature::SharedPtr mpRootSignature;
            PrimitiveType mPrimType = PrimitiveType::Undefined;

#ifdef FALCOR_VK
            std::vector<Viewport> mViewports;
            std::vector<Scissor> mScissors;

            Vao::SharedConstPtr mpVao;
#endif

            bool mSinglePassStereoEnabled = false;
        };

        static SharedPtr create(const Desc& desc);

        ApiHandle getApiHandle() { return mApiHandle; }

        const Desc& getDesc() const { return mDesc; }
    private:
        GraphicsStateObject(const Desc& desc) : mDesc(desc) {}
        Desc mDesc;
        ApiHandle mApiHandle;

        // Default state objects
        static BlendState::SharedPtr spDefaultBlendState;
        static RasterizerState::SharedPtr spDefaultRasterizerState;
        static DepthStencilState::SharedPtr spDefaultDepthStencilState;

        bool apiInit();
    };
}