#include "SelectionHandlesOperation.h"

#include "Selection.h"
#include "LuaInput.h"
#include "Scene.h"
#include "Camera.h"

namespace Engine
{
	Enum_Definition(SelectionHandleType,
		Document_Enum("");
		
		Enum_Item(Move);
		Enum_Item(Resize);
		Enum_Item(Rotate);
	);

	namespace Editor
	{
		using GraphicsEngine::RenderOperation;

		Reflect_Inherited(SelectionHandlesOperation, RenderOperation,
			Document_Class("");

			Document("");
			Archivable Class_Member(bool, IsActive);
			
			Document("");
			Archivable Class_Member(bool, DrawSelectionBox);
			
			Document("");
			Archivable Class_Member(bool, DrawHandles);
			
			Document("");
			Archivable Class_Member(Vector3, Resolution);
			
			Document("");
			Archivable Class_Member(float, SelectionRayDistance);
			
			Document("");
			Archivable Class_Member(bool, IsLocalSpace);
			
			Document("");
			Archivable Class_Member(bool, ObjectsShareHandles);
			
			Document("");
			Archivable Class_Member(LuaEnum<Enum::SelectionHandleType>, HandleType);
			
			Document("");
			Archivable Class_Member(RGBA, BoxColor);
			
			Document("");
			Archivable Class_Member(RGBA, HoverBoxColor);
			
			Document("");
			Archivable Class_Member(RGBA, HandleColorX);
			
			Document("");
			Archivable Class_Member(RGBA, HandleColorY);
			
			Document("");
			Archivable Class_Member(RGBA, HandleColorZ);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<Selection>, ActiveSelection);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<GraphicsEngine::Scene>, TargetScene);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<GraphicsEngine::Camera>, CurrentCamera);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, MousePosition);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, SelectButton);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, RequiredModifierKey);
		);
	}
}