#include "HDRColorCorrectionOperation.h"

#include "FrameBuffer.h"

namespace Engine
{
	using Enum::LuminescenceMode;
	using Enum::RangeFittingMode;

	Enum_Definition(LuminescenceMode,
		Document_Enum("");

		Document_Item("");
		Enum_Item(Photometric);

		Document_Item("");
		Enum_Item(Digital);

		Document_Item("");
		Enum_Item(DigitalAccurate);
	);

	Enum_Definition(RangeFittingMode,
		Document_Enum("");

		Document_Item("");
		Enum_Item(Exposure);

		Document_Item("");
		Enum_Item(Burnout);

		Document_Item("");
		Enum_Item(Reinhard);
	);
}

namespace GraphicsEngine
{
	Reflect_Inherited(HDRColorCorrectionOperation, RenderOperation,
		Document_Class("");

		Document("");
		Archivable Class_Member(LuaEnum<Enum::LuminescenceMode>, LuminescenceType);

		Document("");
		Archivable Class_Member(LuaEnum<Enum::RangeFittingMode>, RangeFittingType);

		Document("");
		Archivable Class_Member(float, Exposure);

		Document("");
		Archivable Class_Member(float, BurnoutCutoff);

		Document("");
		Archivable Class_Member(std::weak_ptr<class Texture>, Input);

		Document("");
		Archivable Class_Member(std::weak_ptr<class FrameBuffer>, Output);

		Bind_Function(GetLuminescenceBuffer,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<FrameBuffer>);
			
				Overload_Parameters();
			
				Bind_Parameters(GetLuminescenceBuffer);
			);
		);

		Bind_Function(Resize,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters(
					Document("");
					Function_Parameter(int, width);

					Document("");
					Function_Parameter(int, height);
				);
			
				Bind_Parameters_No_Return(Resize, width, height);
			);
		);
	);
}