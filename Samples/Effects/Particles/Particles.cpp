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
#include "Particles.h"
#include "API/D3D/FalcorD3D.h"
#include <limits>

void Particles::onGuiRender()
{
    float floatMax = std::numeric_limits<float>::max();
    float floatMin = std::numeric_limits<float>::min();
    mpGui->addFloatVar("Duration", mParticleSystem.Emitter.duration, 0.f);
    mpGui->addFloatVar("RandDuration", mParticleSystem.Emitter.randDuration, 0.f);
    mpGui->addFloatVar("Frequency", mParticleSystem.Emitter.emitFrequency, 0.01f);
    int32_t emitCount = mParticleSystem.Emitter.emitCount;
    mpGui->addIntVar("EmitCount", emitCount, 0);
    mParticleSystem.Emitter.emitCount = emitCount;
    mpGui->addIntVar("RandEmitCount", mParticleSystem.Emitter.randEmitCount, 0);
    mpGui->addFloat3Var("SpawnPos", mParticleSystem.Emitter.spawnPos, floatMin, floatMax);
    mpGui->addFloat3Var("RandSpawnPos", mParticleSystem.Emitter.randSpawnPos, floatMin, floatMax);
    mpGui->addFloat3Var("Velocity", mParticleSystem.Emitter.vel, floatMin, floatMax);
    mpGui->addFloat3Var("RandVel", mParticleSystem.Emitter.randVel, floatMin, floatMax);
    mpGui->addFloat3Var("Accel", mParticleSystem.Emitter.accel, floatMin, floatMax);
    mpGui->addFloat3Var("RandAccel", mParticleSystem.Emitter.randAccel, floatMin, floatMax);
    mpGui->addFloatVar("Scale", mParticleSystem.Emitter.scale, 0.001f);
    mpGui->addFloatVar("RandScale", mParticleSystem.Emitter.randScale, 0.001f);
    mpGui->addFloatVar("Growth", mParticleSystem.Emitter.growth);
    mpGui->addFloatVar("RandGrowth", mParticleSystem.Emitter.randGrowth);
}

void Particles::onLoad()
{
    mpRenderContext->initCommandSignatures();
    mParticleSystem.init(mpRenderContext.get(), 4096, "Effects/ParticleConstColor.ps.hlsl");
    mpCamera = Camera::create();
    mpCamController.attachCamera(mpCamera);
    mpTex = createTextureFromFile("C:/Users/clavelle/Desktop/TestParticle.png", true, false);
    mParticleSystem.getDrawVars()->setSrv(2, mpTex->getSRV());
}

void Particles::onFrameRender()
{
	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
 	mpRenderContext->clearFbo(mpDefaultFBO.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpCamController.update();

    mParticleSystem.update(mpRenderContext.get(), frameRate().getLastFrameTime());
    mParticleSystem.render(mpRenderContext.get(), mpCamera->getViewMatrix(), mpCamera->getProjMatrix());
}

void Particles::onShutdown()
{
}

bool Particles::onKeyEvent(const KeyboardEvent& keyEvent)
{
    mpCamController.onKeyEvent(keyEvent);
    return false;
}

bool Particles::onMouseEvent(const MouseEvent& mouseEvent)
{
    mpCamController.onMouseEvent(mouseEvent);
    return false;
}

void Particles::onDataReload()
{
}

void Particles::onResizeSwapChain()
{
    float height = (float)mpDefaultFBO->getHeight();
    float width = (float)mpDefaultFBO->getWidth();

    mpCamera->setFocalLength(21.0f);
    float aspectRatio = (width / height);
    mpCamera->setAspectRatio(aspectRatio);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    Particles sample;
    SampleConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.resizableWindow = true;
    sample.run(config);
}