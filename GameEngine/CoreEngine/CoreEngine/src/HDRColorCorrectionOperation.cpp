#include "HDRColorCorrectionOperation.h"

#include "Graphics.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Textures.h"

namespace GraphicsEngine
{
	void HDRColorCorrectionOperation::Initialize()
	{
		std::shared_ptr<FrameBuffer> luminescenceBuffer = FrameBuffer::Create(Resolution.Width, Resolution.Height, Textures::Create(Resolution.Width, Resolution.Height, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_FLOAT, GL_RG32F, GL_RG), true);
		std::shared_ptr<FrameBuffer> luminescenceBackBuffer = FrameBuffer::Create(Resolution.Width, Resolution.Height, Textures::Create(Resolution.Width, Resolution.Height, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_FLOAT, GL_RG32F, GL_RG), true);
		std::shared_ptr<FrameBuffer> luminescenceCacheBuffer = FrameBuffer::Create(Resolution.Width, Resolution.Height, Textures::Create(Resolution.Width, Resolution.Height, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_FLOAT, GL_RG32F, GL_RG), true);

		luminescenceBuffer->Name = "LuminescencePrimaryBuffer";
		luminescenceBackBuffer->Name = "LuminescenceSecondaryBuffer";
		luminescenceCacheBuffer->Name = "LuminescenceCacheBuffer";

		luminescenceBuffer->SetParent(This.lock());
		luminescenceBackBuffer->SetParent(This.lock());
		luminescenceCacheBuffer->SetParent(This.lock());

		LuminescenceBuffer = luminescenceBuffer;
		LuminescenceBackBuffer = luminescenceBackBuffer;
		LuminescenceCacheBuffer = luminescenceCacheBuffer;
	}

	void HDRColorCorrectionOperation::Render()
	{
		auto input = Input.lock();

		int width = input->GetWidth();
		int height = input->GetHeight();
		bool firstPass = true;

		std::shared_ptr<FrameBuffer> current = LuminescenceBuffer.lock();
		std::shared_ptr<FrameBuffer> back = LuminescenceBackBuffer.lock();

		Programs::Luminescence->Use();

		Programs::Luminescence->firstPass.Set(true);
		Programs::Luminescence->transform.Set(Matrix3());
		Programs::Luminescence->inputData.Set(input);

		Programs::Luminescence->resolution.Set(Graphics::ActiveWindow->Resolution);

		Programs::Luminescence->luminescenceMode.Set(LuminescenceType);

		while (width > 1 || height > 1)
		{
			Programs::Luminescence->bounds.Set(Vector3(float(width), float(height)));

			width += width & 1;
			height += height & 1;

			width >>= 1;
			height >>= 1;

			current->DrawTo(0, 0, width, height);

			Graphics::ClearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Programs::Luminescence->finalPass.Set(width == 1 && height == 1);

			Programs::Luminescence->CoreMeshes.Square->Draw();

			if (firstPass)
			{
				LuminescenceCacheBuffer.lock()->DrawTo();

				Graphics::ClearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				Programs::Luminescence->CoreMeshes.Square->Draw();

				firstPass = false;
			}

			std::swap(current, back);

			Programs::Luminescence->firstPass.Set(false);
			Programs::Luminescence->inputData.Set(back->GetTexture());
		}


		if (!Output.expired())
			Output.lock()->DrawTo();
		else
			FrameBuffer::Detatch();

		Graphics::ClearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Programs::ToneMap->Use();

		Programs::ToneMap->transform.Set(Matrix3());
		Programs::ToneMap->inputData.Set(input);
		Programs::ToneMap->luminescenceData.Set(back->GetTexture(), 1);
		Programs::ToneMap->resolution.Set(Graphics::ActiveWindow->Resolution);
		Programs::ToneMap->exposure.Set(Exposure);
		Programs::ToneMap->burnoutCutoff.Set(BurnoutCutoff);
		Programs::ToneMap->rangeFittingMode.Set(RangeFittingType);
		Programs::ToneMap->luminescenceMode.Set(LuminescenceType);

		Programs::ToneMap->CoreMeshes.Square->Draw();
	}

	std::shared_ptr<FrameBuffer> HDRColorCorrectionOperation::GetLuminescenceBuffer() const
	{
		return LuminescenceCacheBuffer.lock();
	}

	void HDRColorCorrectionOperation::Resize(int width, int height)
	{
		Resolution = Dimensions(width, height);

		LuminescenceBuffer.lock()->Resize(Resolution);
		LuminescenceBackBuffer.lock()->Resize(Resolution);
		LuminescenceCacheBuffer.lock()->Resize(Resolution);
	}
}