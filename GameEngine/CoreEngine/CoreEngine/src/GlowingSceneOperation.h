#pragma once

#include "DrawSceneOperation.h"
#include "BlurOperation.h"
#include "ShadingOperation.h"
#include "CubeMap.h"
#include "LightBuffer.h"

namespace GraphicsEngine
{
	
	class GlowingSceneOperation : public RenderOperation
	{
	public:
		bool WaterEnabled = false;
		bool Detatch = true;
		bool DrawSkyBox = false;
		std::weak_ptr<CubeMapTexture> SkyBox;
		float SkyBrightness = 1;
		float SkyBackgroundBrightness = 1;
		RGBA SkyColor;
		RGBA SkyBackgroundColor;
		int Radius = 1;
		float Sigma = 1;
		Vector3 Resolution;
		Matrix3 Transform;
		std::weak_ptr<FrameBuffer> Output;
		std::weak_ptr<Light> DebugViewLight;

		void Initialize();
		void Configure(int width, int height, const std::shared_ptr<Scene>& scene, const std::shared_ptr<FrameBuffer>& output = nullptr);
		void SetScene(const std::shared_ptr<Scene>& scene);

		void Update(float);
		void Render();
		std::shared_ptr<FrameBuffer> GetSceneBuffer() const;
		std::shared_ptr<FrameBuffer> GetLuminescenceBuffer() const;
		std::shared_ptr<FrameBuffer> GetLightingBuffer() const;
		std::shared_ptr<FrameBuffer> GetHorizontalPass() const;
		std::shared_ptr<FrameBuffer> GetVerticalPass() const;
		std::shared_ptr<Texture> GenerateNormalMap(const std::shared_ptr<Texture>& heightMap);
		std::shared_ptr<class HDRColorCorrectionOperation> GetHDRColorCorrection() const;

	private:
		std::weak_ptr<FrameBuffer> SceneBuffer;
		std::weak_ptr<FrameBuffer> LightingBuffer;
		std::weak_ptr<FrameBuffer> AccumulationBuffer;
		std::weak_ptr<FrameBuffer> HDRBuffer;
		std::weak_ptr<FrameBuffer> WaterBuffer;
		std::weak_ptr<FrameBuffer> NormalMapGenBuffer;
		std::weak_ptr<Camera> ShadowCamera;

		std::weak_ptr<FrameBuffer> BlurResult;
		std::weak_ptr<DrawSceneOperation> SceneDraw;
		std::weak_ptr<BlurOperation> Blur;
		std::weak_ptr<ShadingOperation> Shader;
		std::weak_ptr<class HDRColorCorrectionOperation> HDRColorCorrection;
		LightBuffer Lights;

		typedef std::pair < std::weak_ptr<Texture>, std::weak_ptr<Texture> > TexturePair;
		typedef std::vector<TexturePair> TexturePairVector;

		TexturePairVector NormalMapQueue;

		Instantiable;

		Inherits_Class(RenderOperation);

		Reflected(GlowingSceneOperation);
	};
}

