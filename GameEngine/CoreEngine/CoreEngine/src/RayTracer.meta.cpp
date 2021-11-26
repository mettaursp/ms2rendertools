#include "RayTracer.h"

#include "Texture.h"
#include "Scene.h"

namespace GraphicsEngine
{
	using Engine::Object;

	Reflect_Inherited(RayTracer, Object,
		Document_Class("");

		Document("");
		Archivable Class_Member(int, BatchWidth);

		Document("");
		Archivable Class_Member(int, BatchHeight);

		Document("");
		Archivable Class_Member(int, MaxBounces);

		Document("");
		Archivable Class_Member(int, Samples);

		Document("");
		Archivable Class_Member(std::weak_ptr<Scene>, CurrentScene);

		Bind_Function(SetMaxThreads,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter(int, maxThreads);
				);
			
				Bind_Parameters_No_Return(SetMaxThreads, maxThreads);
			);
		);

		Bind_Function(GetMaxThreads,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(int);
			
				Overload_Parameters
				(
				);
			
				Bind_Parameters(GetMaxThreads);
			);
		);

		Bind_Function(GetHardwareThreads,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(int);
			
				Overload_Parameters
				(
				);
			
				Bind_Parameters(GetHardwareThreads);
			);
		);

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
				);
			
				Bind_Parameters_No_Return(Configure, width, height);
			);
		);

		Bind_Function(SetViewport,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter(int, minX);

					Document("");
					Function_Parameter(int, minY);

					Document("");
					Function_Parameter(int, maxX);

					Document("");
					Function_Parameter(int, maxY);
				);
			
				Bind_Parameters_No_Return(SetViewport, minX, minY, maxX, maxY);
			);
		);

		Bind_Function(Render,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
				);
			
				Bind_Parameters_No_Return(Render);
			);
		);

		Bind_Function(DrawTo,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter(std::shared_ptr<Texture>, texture);
				);
			
				Bind_Parameters_No_Return(DrawTo, texture);
			);
		);

		Bind_Function(Save,
		
			Document("");
			Function_Overload
			(
				Document("");
				Returns_Nothing;
			
				Overload_Parameters
				(
					Document("");
					Function_Parameter(std::string, filePath);
				);
			
				Bind_Parameters_No_Return(Save, filePath);
			);
		);
	);
}