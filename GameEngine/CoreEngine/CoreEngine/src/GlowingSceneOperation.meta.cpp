#include "GlowingSceneOperation.h"

#include "HDRColorCorrectionOperation.h"

namespace GraphicsEngine
{
	Reflect_Inherited(GlowingSceneOperation, RenderOperation,
		Document_Class("");

		Document("");
		Archivable Class_Member(bool, WaterEnabled);

		Document("");
		Archivable Class_Member(bool, Detatch);

		Document("");
		Archivable Class_Member(bool, DrawSkyBox);

		Document("");
		Archivable Class_Member(std::weak_ptr<CubeMapTexture>, SkyBox);

		Document("");
		Archivable Class_Member(float, SkyBrightness);

		Document("");
		Archivable Class_Member(float, SkyBackgroundBrightness);

		Document("");
		Archivable Class_Member(RGBA, SkyColor);

		Document("");
		Archivable Class_Member(RGBA, SkyBackgroundColor);

		Document("");
		Archivable Class_Member(int, Radius);

		Document("");
		Archivable Class_Member(float, Sigma);

		Document("");
		Archivable Class_Member(Vector3, Resolution);

		Document("");
		Archivable Class_Member(Matrix3, Transform);

		Bind_Function(Configure,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter(int, width);
					
					Document("");
					Function_Parameter(int, height);

					Document("");
					Function_Parameter(std::shared_ptr<Scene>, scene);

					Document("");
					Function_Parameter_Default(std::shared_ptr<FrameBuffer>, output, nullptr);
				);
			
				Bind_Parameters_No_Return(Configure, width, height, scene);
			);
		);

		Bind_Function(GenerateNormalMap,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<Texture>);
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter(std::shared_ptr<Texture>, heightMap);
				);
			
				Bind_Parameters(GenerateNormalMap, heightMap);
			);
		);

		Bind_Function(SetScene,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
					Function_Parameter(std::shared_ptr<Scene>, scene)
				);
			
				Bind_Parameters_No_Return(SetScene, scene);
			);
		);

		Bind_Function(GetSceneBuffer,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<FrameBuffer>);
			
				Overload_Parameters();
			
				Bind_Parameters(GetSceneBuffer);
			);
		);

		Bind_Function(GetHDRColorCorrection,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<HDRColorCorrectionOperation>);
			
				Overload_Parameters();
			
				Bind_Parameters(GetHDRColorCorrection);
			);
		);

		Bind_Function(GetLightingBuffer,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<FrameBuffer>);
			
				Overload_Parameters();
			
				Bind_Parameters(GetLightingBuffer);
			);
		);

		Bind_Function(GetHorizontalPass,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<FrameBuffer>);
			
				Overload_Parameters();
			
				Bind_Parameters(GetHorizontalPass);
			);
		);

		Bind_Function(GetVerticalPass,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<FrameBuffer>);
			
				Overload_Parameters();
			
				Bind_Parameters(GetVerticalPass);
			);
		);
	);
}
