#include "FrameBuffer.h"

#include "Texture.h"

namespace GraphicsEngine
{
	Reflect_Inherited(FrameBuffer, Object,
		Document_Class("");
		
		Document("");
		Static Archivable Class_Member(Dimensions, WindowSize);

		Bind_Function(GetTexture,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<Texture>);
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter_Default(unsigned int, attachmentSlot, 0);
				);
			
				Bind_Parameters(GetTexture, attachmentSlot);
			);
		);

		Bind_Function(Create,

			Document("");
			Static Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<FrameBuffer>);

				Overload_Parameters
				(
					Document("");
					Function_Parameter(unsigned int, width);

					Document("");
					Function_Parameter(unsigned int, height);

					Document("");
					Function_Parameter_Default(std::shared_ptr<Texture>, colorAttachment0, nullptr);
					
					Document("");
					Function_Parameter_Default(bool, setParent, true);

					Document("");
					Function_Parameter_Default(bool, depthBufferEnabled, true);
				);

				Static_Bind_Parameters(Create, width, height, colorAttachment0, setParent, depthBufferEnabled);
			);
		);
	);
}