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

namespace Falcor
{
    template<typename ViewType>
    ViewType getViewDimension(Texture::Type type, bool isArray);

    template<typename ViewDesc>
    void initializeSrvDesc(ResourceFormat format, Texture::Type type, uint32_t firstArraySlice, uint32_t arraySize, uint32_t mostDetailedMip, uint32_t mipCount, bool isTextureArray, ViewDesc& desc)
    {
        // create SRV
        desc = {};
        desc.Format = getDxgiFormat(format);
        desc.ViewDimension = getViewDimension<decltype(desc.ViewDimension)>(type, isTextureArray);

        switch(type)
        {
        case Texture::Type::Texture1D:
            if(arraySize > 1)
            {
                desc.Texture1DArray.MipLevels = mipCount;
                desc.Texture1DArray.MostDetailedMip = mostDetailedMip;
                desc.Texture1DArray.ArraySize = arraySize;
                desc.Texture1DArray.FirstArraySlice = firstArraySlice;
            }
            else
            {
                desc.Texture1D.MipLevels = mipCount;
                desc.Texture1D.MostDetailedMip = mostDetailedMip;
            }
            break;
        case Texture::Type::Texture2D:
            if(isTextureArray)
            {
                desc.Texture2DArray.MipLevels = mipCount;
                desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
                desc.Texture2DArray.ArraySize = arraySize;
                desc.Texture2DArray.FirstArraySlice = firstArraySlice;
            }
            else
            {
                desc.Texture2D.MipLevels = mipCount;
                desc.Texture2D.MostDetailedMip = mostDetailedMip;
            }
            break;
        case Texture::Type::Texture2DMultisample:
            if(arraySize > 1)
            {
                desc.Texture2DMSArray.ArraySize = arraySize;
                desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
            }
            break;
        case Texture::Type::Texture3D:
            assert(arraySize == 1);
            desc.Texture3D.MipLevels = mipCount;
            desc.Texture3D.MostDetailedMip = mostDetailedMip;
            break;
        case Texture::Type::TextureCube:
            if(arraySize > 1)
            {
                desc.TextureCubeArray.First2DArrayFace = 0;
                desc.TextureCubeArray.NumCubes = arraySize;
                desc.TextureCubeArray.MipLevels = mipCount;
                desc.TextureCubeArray.MostDetailedMip = mostDetailedMip;
            }
            else
            {
                desc.TextureCube.MipLevels = mipCount;
                desc.TextureCube.MostDetailedMip = mostDetailedMip;
            }
            break;
        default:
            should_not_get_here();
        }
#ifdef FALCOR_D3D12
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#endif
    }

    template<typename DescType>
    inline void initializeRtvDesc(const Texture* pTexture, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize, DescType& desc)
    {
        desc = {};
        uint32_t arrayMultiplier = (pTexture->getType() == Texture::Type::TextureCube) ? 6 : 1;

        if(arraySize == Texture::kEntireArraySlice)
        {
            arraySize = pTexture->getArraySize() - firstArraySlice;
        }

        desc.ViewDimension = getViewDimension<decltype(desc.ViewDimension)>(pTexture->getType(), pTexture->getArraySize() > 1);

        switch(pTexture->getType())
        {
        case Texture::Type::Texture1D:
            if(pTexture->getArraySize() > 1)
            {
                desc.Texture1DArray.ArraySize = arraySize;
                desc.Texture1DArray.FirstArraySlice = firstArraySlice;
                desc.Texture1DArray.MipSlice = mipLevel;
            }
            else
            {
                desc.Texture1D.MipSlice = mipLevel;
            }
            break;
        case Texture::Type::Texture2D:
        case Texture::Type::TextureCube:
            if(pTexture->getArraySize() * arrayMultiplier > 1)
            {
                desc.Texture2DArray.ArraySize = arraySize * arrayMultiplier;
                desc.Texture2DArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                desc.Texture2DArray.MipSlice = mipLevel;
            }
            else
            {
                desc.Texture2D.MipSlice = mipLevel;
            }
            break;
        case Texture::Type::Texture3D:
            desc.Texture3D.FirstWSlice = firstArraySlice;
            desc.Texture3D.MipSlice = mipLevel;
            desc.Texture3D.WSize = arraySize;
            break;
        case Texture::Type::Texture2DMultisample:
            if(pTexture->getArraySize() > 1)
            {
                desc.Texture2DMSArray.ArraySize = arraySize;
                desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
            }
            break;
        default:
            should_not_get_here();
        }
        desc.Format = getDxgiFormat(pTexture->getFormat());
    }

    template<typename DescType>
    inline void initializeDsvDesc(const Texture* pTexture, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize, DescType& desc)
    {
        desc = {};
        uint32_t arrayMultiplier = (pTexture->getType() == Texture::Type::TextureCube) ? 6 : 1;

        if(arraySize == Fbo::kAttachEntireMipLevel)
        {
            arraySize = pTexture->getArraySize() - firstArraySlice;
        }

        desc.ViewDimension = getViewDimension<decltype(desc.ViewDimension)>(pTexture->getType(), pTexture->getArraySize() > 1);

        switch(pTexture->getType())
        {
        case Texture::Type::Texture1D:
            if(pTexture->getArraySize() > 1)
            {
                desc.Texture1DArray.ArraySize = arraySize;
                desc.Texture1DArray.FirstArraySlice = firstArraySlice;
                desc.Texture1DArray.MipSlice = mipLevel;
            }
            else
            {
                desc.Texture1D.MipSlice = mipLevel;
            }
            break;
        case Texture::Type::Texture2D:
        case Texture::Type::TextureCube:
            if(pTexture->getArraySize() * arrayMultiplier > 1)
            {
                desc.Texture2DArray.ArraySize = arraySize * arrayMultiplier;
                desc.Texture2DArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                desc.Texture2DArray.MipSlice = mipLevel;
            }
            else
            {
                desc.Texture2D.MipSlice = mipLevel;
            }
            break;
        case Texture::Type::Texture2DMultisample:
            if(pTexture->getArraySize() > 1)
            {
                desc.Texture2DMSArray.ArraySize = arraySize;
                desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
            }
            break;
        default:
            should_not_get_here();
        }
        desc.Format = getDxgiFormat(pTexture->getFormat());
    }
}