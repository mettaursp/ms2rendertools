#pragma once

#include "RenderOperation.h"
#include "Dimensions.h"

namespace GraphicsEngine
{
	class HDRColorCorrectionOperation : public RenderOperation
	{
	public:
		struct LuminescenceModeEnum
		{
			enum LuminescenceMode
			{
				Photometric,
				Digital,
				DigitalAccurate
			};
		};

		struct RangeFittingModeEnum
		{
			enum RangeFittingMode
			{
				Exposure,
				Burnout,
				Reinhard
			};
		};

		typedef LuminescenceModeEnum::LuminescenceMode LuminescenceMode;
		typedef RangeFittingModeEnum::RangeFittingMode RangeFittingMode;

		LuminescenceMode LuminescenceType = LuminescenceMode::DigitalAccurate;
		RangeFittingMode RangeFittingType = RangeFittingMode::Burnout;
		float Exposure = 1;
		float BurnoutCutoff = 1.5f;

		std::weak_ptr<class Texture> Input;
		std::weak_ptr<class FrameBuffer> Output;

		void Initialize();
		void Update(float) {}

		void Render();

		std::shared_ptr<FrameBuffer> GetLuminescenceBuffer() const;

		void Resize(int width, int height);

		Instantiable;

		Inherits_Class(RenderOperation);

		Reflected(HDRColorCorrectionOperation);

	private:
		std::weak_ptr<FrameBuffer> LuminescenceBuffer;
		std::weak_ptr<FrameBuffer> LuminescenceBackBuffer;
		std::weak_ptr<FrameBuffer> LuminescenceCacheBuffer;
		Dimensions Resolution;
	};
}

namespace Enum
{
	typedef GraphicsEngine::HDRColorCorrectionOperation::LuminescenceMode LuminescenceMode;
	typedef GraphicsEngine::HDRColorCorrectionOperation::RangeFittingMode RangeFittingMode;
}

namespace Engine
{
	Declare_Enum(LuminescenceMode);
	Declare_Enum(RangeFittingMode);
}
