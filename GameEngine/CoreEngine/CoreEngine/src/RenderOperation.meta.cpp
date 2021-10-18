#include "RenderOperation.h"

namespace GraphicsEngine
{
	Reflect_Inherited(RenderOperation, Object,
		Document_Class("");
		
		Document("");
		Archivable Class_Member(bool, RenderAutomatically);

		Bind_Function(Render,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters();
			
				Bind_Parameters_No_Return(Render);
			);
		);
	);
}
