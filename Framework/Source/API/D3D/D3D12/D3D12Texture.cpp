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
#include "Framework.h"
#include "API/Texture.h"
#include "API/Device.h"
#include "API/D3D/D3DViews.h"
#include <vector>
#include "API/Device.h"
#include "API/ProgramVars.h"
#include "Graphics/FullScreenPass.h"
#include "Graphics/PipelineState.h"

namespace Falcor
{
    RtvHandle Texture::sNullRTV;
    DsvHandle Texture::sNullDSV;

    struct
    {
        FullScreenPass::UniquePtr pFullScreenPass;
        ProgramVars::SharedPtr pVars;
        PipelineState::SharedPtr pState;
    } static gGenMips;

    template<>
    D3D12_SRV_DIMENSION getViewDimension<D3D12_SRV_DIMENSION>(Texture::Type type, bool isTextureArray)
    {
        switch(type)
        {
        case Texture::Type::Texture1D:
            return (isTextureArray) ? D3D12_SRV_DIMENSION_TEXTURE1DARRAY : D3D12_SRV_DIMENSION_TEXTURE1D;
        case Texture::Type::Texture2D:
            return (isTextureArray) ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D;
        case Texture::Type::Texture3D:
            assert(isTextureArray == false);
            return D3D12_SRV_DIMENSION_TEXTURE3D;
        case Texture::Type::Texture2DMultisample:
            return (isTextureArray) ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D12_SRV_DIMENSION_TEXTURE2DMS;
        case Texture::Type::TextureCube:
            return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        default:
            should_not_get_here();
            return D3D12_SRV_DIMENSION_UNKNOWN;
        }
    }

    static const D3D12_HEAP_PROPERTIES kDefaultHeapProps =
    {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        0,
        0
    };

    Texture::~Texture()
    {
        if (gGenMips.pFullScreenPass)
        {
            gGenMips.pFullScreenPass = nullptr;
            gGenMips.pVars = nullptr;
            gGenMips.pState = nullptr;
        }
    }

    uint64_t Texture::makeResident(const Sampler* pSampler) const
    {
        UNSUPPORTED_IN_D3D12("Texture::makeResident()");
        return 0;
    }

    void Texture::evict(const Sampler* pSampler) const
    {
        UNSUPPORTED_IN_D3D12("Texture::evict()");
    }

    void createTextureCommon(const Texture* pTexture, Texture::ApiHandle& apiHandle, const void* pData, D3D12_RESOURCE_DIMENSION dim, bool autoGenMips)
    {
        ResourceFormat texFormat = pTexture->getFormat();

        D3D12_RESOURCE_DESC desc = {};

        desc.MipLevels = pTexture->getMipCount();
        desc.Format = getDxgiFormat(texFormat);
        desc.Width = align_to(getFormatWidthCompressionRatio(texFormat), pTexture->getWidth());
        desc.Height = align_to(getFormatHeightCompressionRatio(texFormat), pTexture->getHeight());
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        desc.DepthOrArraySize = (pTexture->getType() == Texture::Type::TextureCube) ? pTexture->getArraySize() * 6 : pTexture->getArraySize();
        desc.SampleDesc.Count = pTexture->getSampleCount();
        desc.SampleDesc.Quality = 0;
        desc.Dimension = dim;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Alignment = 0;

        D3D12_CLEAR_VALUE clearValue = {};
        D3D12_CLEAR_VALUE* pClearVal = nullptr;
        if (isCompressedFormat(texFormat) == false)
        {
            desc.Flags = isDepthStencilFormat(texFormat) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            clearValue.Format = desc.Format;
            if (isDepthStencilFormat(texFormat))
            {
                clearValue.DepthStencil.Depth = 1.0f;
            }
            pClearVal = &clearValue;
        }

        d3d_call(gpDevice->getApiHandle()->CreateCommittedResource(&kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, pClearVal, IID_PPV_ARGS(&apiHandle)));

        if (pData)
        {
            auto& pCopyCtx = gpDevice->getCopyContext();
            if (autoGenMips)
            {
                size_t arraySliceSize = pTexture->getWidth() * pTexture->getHeight() * getFormatBytesPerBlock(pTexture->getFormat());
                const uint8_t* pSrc = (uint8_t*)pData;
                for (uint32_t i = 0; i < pTexture->getArraySize(); i++)
                {
                    uint32_t subresource = pTexture->getSubresourceIndex(i, 0);
                    pCopyCtx->updateTextureSubresource(pTexture, subresource, pSrc, false);
                    pSrc += arraySliceSize;
                }
            }
            else
            {
                pCopyCtx->updateTexture(pTexture, pData, false);
            }
            pCopyCtx->submit(true);

            if (autoGenMips)
            {
                pTexture->generateMips();
            }
        }
    }

    Texture::SharedPtr Texture::create1D(uint32_t width, ResourceFormat format, uint32_t arraySize, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, 1, 1, 1, mipLevels, arraySize, format, Type::Texture1D));
        createTextureCommon(pTexture.get(), pTexture->mApiHandle, pData, D3D12_RESOURCE_DIMENSION_TEXTURE1D, (mipLevels == kEntireMipChain));
        return pTexture->mApiHandle ? pTexture : nullptr;
    }
    
    Texture::SharedPtr Texture::create2D(uint32_t width, uint32_t height, ResourceFormat format, uint32_t arraySize, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, 1, mipLevels, arraySize, format, Type::Texture2D));
        createTextureCommon(pTexture.get(), pTexture->mApiHandle, pData, D3D12_RESOURCE_DIMENSION_TEXTURE2D, (mipLevels == kEntireMipChain));
        return pTexture->mApiHandle ? pTexture : nullptr;
    }

    DsvHandle Texture::getDSV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize) const
    {
        // FIXME D3D12 this code is almost identical to getRTV and getSRV
        assert(mipLevel < mMipLevels);
        assert(firstArraySlice < mArraySize);
        if (arraySize == kEntireArraySlice)
        {
            arraySize = mArraySize - firstArraySlice;
        }
        assert(firstArraySlice + arraySize <= mArraySize);

        ViewInfo view{ firstArraySlice, arraySize, mipLevel, 1 };

        if (mDsvs.find(view) == mDsvs.end())
        {
            DescriptorHeap::SharedPtr& pHeap = gpDevice->getDsvDescriptorHeap();

            // Create the render-target view
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            initializeDsvDesc<D3D12_DEPTH_STENCIL_VIEW_DESC>(this, mipLevel, firstArraySlice, arraySize, dsvDesc);
            uint32_t dsvIndex = pHeap->allocateHandle();
            DescriptorHeap::CpuHandle dsv = pHeap->getCpuHandle(dsvIndex);
            gpDevice->getApiHandle()->CreateDepthStencilView(mApiHandle, &dsvDesc, dsv);
            mDsvs[view] = dsv;
        }

        return mDsvs[view];
    }

    RtvHandle Texture::getRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize) const
    {
        assert(mipLevel < mMipLevels);
        assert(firstArraySlice < mArraySize);
        if (arraySize == kEntireArraySlice)
        {
            arraySize = mArraySize - firstArraySlice;
        }
        assert(firstArraySlice + arraySize <= mArraySize);

        ViewInfo view{ firstArraySlice, arraySize, mipLevel, 1};

        if (mRtvs.find(view) == mRtvs.end())
        {
            DescriptorHeap::SharedPtr& pHeap = gpDevice->getRtvDescriptorHeap();

            // Create the render-target view
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            initializeRtvDesc<D3D12_RENDER_TARGET_VIEW_DESC>(this, mipLevel, firstArraySlice, arraySize, rtvDesc);
            uint32_t rtvIndex = pHeap->allocateHandle();
            DescriptorHeap::CpuHandle rtv = pHeap->getCpuHandle(rtvIndex);
            gpDevice->getApiHandle()->CreateRenderTargetView(mApiHandle, &rtvDesc, rtv);
            mRtvs[view] = rtv;
        }

        return mRtvs[view];
    }


    SrvHandle Texture::getSRV(uint32_t firstArraySlice, uint32_t arraySize, uint32_t mostDetailedMip, uint32_t mipCount) const
    {
        assert(firstArraySlice < mArraySize);
        assert(mostDetailedMip < mMipLevels);
        if (mipCount == kEntireMipChain)
        {
            mipCount = mMipLevels - mostDetailedMip;
        }
        if (arraySize == kEntireArraySlice)
        {
            arraySize = mArraySize - firstArraySlice;
        }
        assert(mipCount + mostDetailedMip <= mMipLevels);
        assert(arraySize + firstArraySlice <= mArraySize);

        ViewInfo view{ firstArraySlice, arraySize, mostDetailedMip, mipCount };

        if (mSrvs.find(view) == mSrvs.end())
        {
            DescriptorHeap::SharedPtr& pHeap = gpDevice->getSrvDescriptorHeap();

            // Create the shader-resource view
            D3D12_SHADER_RESOURCE_VIEW_DESC desc;
            initializeSrvDesc(mFormat, mType, firstArraySlice, arraySize, mostDetailedMip, mipCount, mArraySize > 1, desc);
            uint32_t srvHandle = pHeap->allocateHandle();
            DescriptorHeap::CpuHandle srv = pHeap->getCpuHandle(srvHandle);
            gpDevice->getApiHandle()->CreateShaderResourceView(mApiHandle, &desc, srv);
            mSrvs[view] = pHeap->getGpuHandle(srvHandle);
        }

        return mSrvs[view];
    }

    Texture::SharedPtr Texture::create3D(uint32_t width, uint32_t height, uint32_t depth, ResourceFormat format, uint32_t mipLevels, const void* pData, bool isSparse)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, depth, 1, mipLevels, 1, format, Type::Texture3D));
        createTextureCommon(pTexture.get(), pTexture->mApiHandle, pData, D3D12_RESOURCE_DIMENSION_TEXTURE3D, (mipLevels == kEntireMipChain));
        return pTexture->mApiHandle ? pTexture : nullptr;
        return nullptr;
    }

    // Texture Cube
    Texture::SharedPtr Texture::createCube(uint32_t width, uint32_t height, ResourceFormat format, uint32_t arraySize, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, 1, mipLevels, arraySize, format, Type::TextureCube));
        createTextureCommon(pTexture.get(), pTexture->mApiHandle, pData, D3D12_RESOURCE_DIMENSION_TEXTURE2D, (mipLevels == kEntireMipChain));
        return pTexture->mApiHandle ? pTexture : nullptr;
    }

    Texture::SharedPtr Texture::create2DMS(uint32_t width, uint32_t height, ResourceFormat format, uint32_t sampleCount, uint32_t arraySize, bool useFixedSampleLocations)
    {
        assert(useFixedSampleLocations == true);
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, 1, 1, arraySize, format, Type::Texture2DMultisample));
        createTextureCommon(pTexture.get(), pTexture->mApiHandle, nullptr, D3D12_RESOURCE_DIMENSION_TEXTURE2D, false);
        return pTexture->mApiHandle ? pTexture : nullptr;
    }

    Texture::SharedPtr Texture::create2DFromView(uint32_t apiHandle, uint32_t width, uint32_t height, ResourceFormat format)
    {
        return nullptr;
    }

    uint32_t Texture::getMipLevelDataSize(uint32_t mipLevel) const
    {
        UNSUPPORTED_IN_D3D12("Texture::getMipLevelDataSize");
        return 0;
    }

    void Texture::readSubresourceData(void* pData, uint32_t dataSize, uint32_t mipLevel, uint32_t arraySlice) const
    {
        UNSUPPORTED_IN_D3D12("Texture::readSubresourceData");
    }

    void Texture::uploadSubresourceData(const void* pData, uint32_t dataSize, uint32_t mipLevel, uint32_t arraySlice)
    {
        UNSUPPORTED_IN_D3D12("Texture::uploadSubresourceData()");
    }

    void Texture::compress2DTexture()
    {
        UNSUPPORTED_IN_D3D12("Texture::compress2DTexture");
    }

    Texture::SharedPtr Texture::createView(uint32_t firstArraySlice, uint32_t arraySize, uint32_t mostDetailedMip, uint32_t mipCount) const
    {
        UNSUPPORTED_IN_D3D12("createView");
        return nullptr;
    }

    void Texture::generateMips() const
    {
        if (gGenMips.pFullScreenPass == nullptr)
        {
            gGenMips.pFullScreenPass = FullScreenPass::create("Framework\\GenerateMips.hlsl");
            gGenMips.pVars = ProgramVars::create(gGenMips.pFullScreenPass->getProgram()->getActiveVersion()->getReflector());
            gGenMips.pState = PipelineState::create();
            Sampler::Desc desc;
            desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear).setAddressingMode(Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
            gGenMips.pVars->setSampler("gSampler", Sampler::create(desc));
        }

        RenderContext* pContext = gpDevice->getRenderContext().get();
        pContext->pushPipelineState(gGenMips.pState);
        pContext->pushProgramVars(gGenMips.pVars);

        for (uint32_t i = 0; i < mMipLevels - 1; i++)
        {
            // Create an FBO for the next mip level
            Fbo::SharedPtr pFbo = Fbo::create();
            pFbo->attachColorTarget(shared_from_this(), 0, i + 1, 0);
            gGenMips.pState->setFbo(pFbo);

            // Create the resource view
            gGenMips.pVars->setTexture(0, shared_from_this(), 0, mArraySize, i, 1);

            // Run the program
            gGenMips.pFullScreenPass->execute(pContext);
        }
        pContext->popPipelineState();
        pContext->popProgramVars();
    }

    void Texture::createNullViews()
    {
        if (sNullRTV.ptr != 0)
        {
            return;
        }

        // Create the RTV
        DescriptorHeap::SharedPtr& pRtvHeap = gpDevice->getRtvDescriptorHeap();

        // Create the render-target view
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        uint32_t rtvIndex = pRtvHeap->allocateHandle();
        sNullRTV = pRtvHeap->getCpuHandle(rtvIndex);
        gpDevice->getApiHandle()->CreateRenderTargetView(nullptr, &rtvDesc, sNullRTV);

        // Create the DSV
        DescriptorHeap::SharedPtr& pDsvHeap = gpDevice->getDsvDescriptorHeap();

        // Create the render-target view
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D16_UNORM;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        uint32_t dsvIndex = pDsvHeap->allocateHandle();
        sNullDSV = pDsvHeap->getCpuHandle(dsvIndex);
        gpDevice->getApiHandle()->CreateDepthStencilView(nullptr, &dsvDesc, sNullDSV);
    }
}