#include "InputContext.h"

#include "LuaInput.h"
#include "InputSubscriber.h"

namespace GraphicsEngine
{
	Reflect_Inherited(InputContext, Object,
		Document_Class("");

		Document("");
		Archivable Class_Member(bool, Enabled);

		Document("");
		Archivable Class_Member(std::weak_ptr<DeviceTransform>, Device);

		Document("");
		Archivable Class_Member(std::weak_ptr<Engine::UserInput>, InputSource);

		Bind_Function(GetFocus,

			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<InputSubscriber>);
				
				Overload_Parameters(
					Document("");
					Function_Parameter(LuaEnum<Enum::BoundDevice>, device);
				);

				Bind_Parameters(GetFocus, device);
			);
		);
	);
}