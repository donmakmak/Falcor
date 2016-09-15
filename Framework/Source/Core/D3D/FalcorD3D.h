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
#define NOMINMAX
#include <d3dcompiler.h>
#include "Core/Formats.h"
#include <comdef.h>
#include <dxgiformat.h>

#ifndef FALCOR_D3D
#define FALCOR_D3D
#endif

namespace Falcor
{
    /*!
    *  \addtogroup Falcor
    *  @{
    */

    struct DxgiFormatDesc
    {
        ResourceFormat falcorFormat;
        DXGI_FORMAT dxgiFormat;
    };

    extern const DxgiFormatDesc kDxgiFormatDesc[];

    /** Get the DXGI format
    */
    inline DXGI_FORMAT getDxgiFormat(ResourceFormat format)
    {
        assert(kDxgiFormatDesc[(uint32_t)format].falcorFormat == format);
        return kDxgiFormatDesc[(uint32_t)format].dxgiFormat;
    }

    /** Get D3D_FEATURE_LEVEL
    */
    D3D_FEATURE_LEVEL getD3DFeatureLevel(uint32_t majorVersion, uint32_t minorVersion);

    /** Log a message if hr indicates an error
    */
    void d3dTraceHR(const std::string& Msg, HRESULT hr);
    /*! @} */
}

#define MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))

__forceinline BOOL dxBool(bool b) {return b ? TRUE : FALSE;}

#ifdef _LOG_ENABLED
#define d3d_call(a) {HRESULT hr = a; if(FAILED(hr)) { d3dTraceHR( #a, hr); } }
#else
#define d3d_call(a) a
#endif

#ifdef FALCOR_D3D11
#include "D3D11/FalcorD3D11.h"
#endif

#ifdef FALCOR_D3D12
#include "D3D12/FalcorD3D12.h"
#endif

#pragma comment(lib, "d3dcompiler.lib")

#define UNSUPPORTED_IN_D3D(msg_) {Falcor::Logger::log(Falcor::Logger::Level::Warning, msg_ + std::string(" is not supported in D3D. Ignoring call."));}