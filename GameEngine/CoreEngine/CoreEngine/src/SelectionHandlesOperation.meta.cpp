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
			Archivable Class_Member(bool, SnapsToGrid);

			Document("");
			Archivable Class_Member(float, MinimumObjectSize);

			Document("");
			Archivable Class_Member(float, GridLength);
			
			Document("");
			Archivable Class_Member(float, SnappingAngle);
			
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
			Archivable Class_Member(RGBA, HoverHandleColorX);

			Document("");
			Archivable Class_Member(RGBA, HoverHandleColorY);

			Document("");
			Archivable Class_Member(RGBA, HoverHandleColorZ);

			Document("");
			Archivable Class_Member(float, ArrowHandleMinSize);

			Document("");
			Archivable Class_Member(float, ArrowHandleScaling);

			Document("");
			Archivable Class_Member(float, ArrowHandleMinOffset);

			Document("");
			Archivable Class_Member(float, ArrowHandleScaledOffset);

			Document("");
			Archivable Class_Member(float, SphereHandleMinSize);

			Document("");
			Archivable Class_Member(float, SphereHandleScaling);

			Document("");
			Archivable Class_Member(float, SphereHandleMinOffset);

			Document("");
			Archivable Class_Member(float, SphereHandleScaledOffset);
			
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
			Archivable Class_Member(std::weak_ptr<InputObject>, DragButton);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, ResetButton);
			
			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, RequiredModifierKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, RequiredDragModifierKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, MultiSelectModifierKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, MoveToolKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, ResizeToolKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, RotateToolKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, ToggleEnabledKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, SelectToolKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, ToggleLocalSpaceKey);

			Document("");
			Archivable Class_Member(std::weak_ptr<InputObject>, ToggleGroupSelectKey);
		);
	}
}