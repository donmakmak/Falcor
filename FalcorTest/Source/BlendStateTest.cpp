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
#include "BlendStateTest.h"
#include <iostream>

void BlendStateTest::addTests()
{
    addTestToList<TestCreate>();
    addTestToList<TestRtArray>();
}

testing_func(BlendStateTest, TestCreate)
{
    glm::vec4 blendFactor(1.f);
    BlendState::BlendOp blendOp = BlendState::BlendOp::Add;
    BlendState::BlendFunc blendFunc = BlendState::BlendFunc::Zero;

    TestDesc desc;
    //Pick some random assortment of parameters and ensure they match after being passed to state
    desc.setAlphaToCoverage(true);
    desc.setBlendFactor(blendFactor);
    desc.setIndependentBlend(true);
    desc.setRenderTargetWriteMask(0, true, true, true, true);
    desc.setRtBlend(0, true);
    desc.setRtParams(0, blendOp, blendOp, blendFunc, blendFunc, blendFunc, blendFunc);

    BlendState::SharedPtr blendState = nullptr;
    blendState = BlendState::create(desc);

    if (doStatesMatch(blendState, desc))
    {
        return TEST_PASS;
    }
    else
    {
        return test_fail("Blend state doesn't match desc used to create");
    }
}

testing_func(BlendStateTest, TestRtArray)
{
    const int32_t numBlendOps = 5;
    const int32_t numBlendFuncs = 16;

    //create default
    TestDesc desc;
    //Set parameters in the RT array and ensure they match created state
    for (uint32_t i = 0; i < Fbo::getMaxColorTargetCount(); ++i)
    {
        BlendState::BlendOp blendOp = static_cast<BlendState::BlendOp>(i % numBlendOps);
        BlendState::BlendFunc blendFunc = static_cast<BlendState::BlendFunc>(i % numBlendFuncs);

        desc.setRtParams(i, blendOp, blendOp, blendFunc, blendFunc, blendFunc, blendFunc);
        BlendState::SharedPtr state = BlendState::create(desc);
        if (!doStatesMatch(state, desc))
        {
            return test_fail("Render target desc doesn't match ones used to create");
        }
    }

    return TEST_PASS;
}

bool BlendStateTest::doStatesMatch(const BlendState::SharedPtr state, const TestDesc& desc)
{
    //check settings not in the rt array
    if (state->isAlphaToCoverageEnabled() != desc.mAlphaToCoverageEnabled ||
        state->getBlendFactor() != desc.mBlendFactor ||
        state->isIndependentBlendEnabled() != desc.mEnableIndependentBlend ||
        state->getRtCount() != desc.mRtDesc.size())
    {
        return false;
    }

    //check the rt array
    for (uint32_t i = 0; i < state->getRtCount(); ++i)
    {
        BlendState::Desc::RenderTargetDesc rtDesc = state->getRtDesc(i);
        BlendState::Desc::RenderTargetDesc otherRtDesc = desc.mRtDesc[i];
        if (rtDesc.writeMask.writeRed != otherRtDesc.writeMask.writeRed ||
            rtDesc.writeMask.writeGreen != otherRtDesc.writeMask.writeGreen ||
            rtDesc.writeMask.writeBlue != otherRtDesc.writeMask.writeBlue ||
            rtDesc.writeMask.writeAlpha != otherRtDesc.writeMask.writeAlpha ||
            rtDesc.blendEnabled != otherRtDesc.blendEnabled ||
            rtDesc.rgbBlendOp != otherRtDesc.rgbBlendOp ||
            rtDesc.alphaBlendOp != otherRtDesc.alphaBlendOp ||
            rtDesc.srcRgbFunc != otherRtDesc.srcRgbFunc ||
            rtDesc.dstRgbFunc != otherRtDesc.dstRgbFunc ||
            rtDesc.srcAlphaFunc != otherRtDesc.srcAlphaFunc ||
            rtDesc.dstAlphaFunc != otherRtDesc.dstAlphaFunc)
        {
            return false;
        }

    }

    return true;
}

int main()
{
    BlendStateTest bst;
    bst.init();
    bst.run();
    return 0;
}