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
#include "API/LowLevel/RootSignature.h"
#include "API/D3D/D3DState.h"
#include "API/Device.h"

namespace Falcor
{
    using RootParameterVec = std::vector<D3D12_ROOT_PARAMETER>;

    D3D12_SHADER_VISIBILITY getShaderVisibility(ShaderVisibility visibility)
    {
        // D3D12 doesn't support a combination of flags, it's either ALL or a single stage
        if (isPowerOf2(visibility) == false)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        else if ((visibility & ShaderVisibility::Vertex) != ShaderVisibility::None)
        {
            return D3D12_SHADER_VISIBILITY_VERTEX;
        }
        else if ((visibility & ShaderVisibility::Pixel) != ShaderVisibility::None)
        {
            return D3D12_SHADER_VISIBILITY_PIXEL;
        }
        else if ((visibility & ShaderVisibility::Geometry) != ShaderVisibility::None)
        {
            return D3D12_SHADER_VISIBILITY_GEOMETRY;
        }
        else if ((visibility & ShaderVisibility::Domain) != ShaderVisibility::None)
        {
            return D3D12_SHADER_VISIBILITY_DOMAIN;
        }
        else if ((visibility & ShaderVisibility::Hull) != ShaderVisibility::None)
        {
            return D3D12_SHADER_VISIBILITY_HULL;
        }
        should_not_get_here();
        return (D3D12_SHADER_VISIBILITY)-1;
    }

    D3D12_DESCRIPTOR_RANGE_TYPE getDescRangeType(RootSignature::DescType type)
    {
        switch (type)
        {
        case RootSignature::DescType::Srv:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        case RootSignature::DescType::Uav:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        case RootSignature::DescType::Cbv:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case RootSignature::DescType::Sampler:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        default:
            should_not_get_here();
            return (D3D12_DESCRIPTOR_RANGE_TYPE)-1;
        }
    }

    void convertCbvSet(const RootSignature::DescriptorSetLayout& set, D3D12_ROOT_PARAMETER& desc)
    {
        assert(set.getRangeCount() == 1);
        const auto& range = set.getRange(0);
        assert(range.type == RootSignature::DescType::Cbv && range.descCount == 1);

        desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        desc.Descriptor.RegisterSpace = range.regSpace;
        desc.Descriptor.ShaderRegister = range.baseRegIndex;
        desc.ShaderVisibility = getShaderVisibility(set.getVisibility());
    }

    void convertDescTable(const RootSignature::DescriptorSetLayout& falcorSet, D3D12_ROOT_PARAMETER& desc, std::vector<D3D12_DESCRIPTOR_RANGE>& d3dRange)
    {
        desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        desc.ShaderVisibility = getShaderVisibility(falcorSet.getVisibility());
        d3dRange.resize(falcorSet.getRangeCount());
        desc.DescriptorTable.NumDescriptorRanges = (uint32_t)falcorSet.getRangeCount();
        desc.DescriptorTable.pDescriptorRanges = d3dRange.data();

        for (size_t i = 0; i < falcorSet.getRangeCount(); i++)
        {
            const auto& falcorRange = falcorSet.getRange(i);
            d3dRange[i].BaseShaderRegister = falcorRange.baseRegIndex;
            d3dRange[i].NumDescriptors = falcorRange.descCount;
            d3dRange[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            d3dRange[i].RangeType = getDescRangeType(falcorRange.type);
            d3dRange[i].RegisterSpace = falcorRange.regSpace;
        }
    }

    bool RootSignature::apiInit()
    {
        mSizeInBytes = 0;
        RootParameterVec rootParams(mDesc.mSets.size());
        mElementByteOffset.resize(mDesc.mSets.size());

        // Descriptor sets. Need to allocate some space for the D3D12 tables
        std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> d3dRanges(mDesc.mSets.size());
        for (size_t i = 0 ; i < mDesc.mSets.size() ; i++)
        {
            const auto& set = mDesc.mSets[i];
            assert(set.getRangeCount() == 1);
            uint32_t byteOffset;
            if (set.getRangeCount() == 1 && set.getRange(0).type == DescType::Cbv)
            {
                convertCbvSet(set, rootParams[i]);
                byteOffset = 8;
            }
            else
            {
                convertDescTable(mDesc.mSets[i], rootParams[i], d3dRanges[i]);
                byteOffset = 4;
            }
            mElementByteOffset[i] = mSizeInBytes;
            mSizeInBytes += byteOffset;
        }

        // Create the root signature
        D3D12_ROOT_SIGNATURE_DESC desc;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        desc.pParameters = rootParams.data();
        desc.NumParameters = (uint32_t)rootParams.size();
        desc.pStaticSamplers = nullptr;
        desc.NumStaticSamplers = 0;

        ID3DBlobPtr pSigBlob;
        ID3DBlobPtr pErrorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pSigBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            std::string msg = convertBlobToString(pErrorBlob.GetInterfacePtr());
            logError(msg);
            return false;
        }

        if (mSizeInBytes > sizeof(uint32_t) * D3D12_MAX_ROOT_COST)
        {
            logError("Root-signature cost is too high. D3D12 root-signatures are limited to 64 DWORDs, trying to create a signature with " + std::to_string(mSizeInBytes / sizeof(uint32_t)) + " DWORDs");
            return false;
        }

        Device::ApiHandle pDevice = gpDevice->getApiHandle();
        d3d_call(pDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&mApiHandle)));
       
        return true;
    }
}
