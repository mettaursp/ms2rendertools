#pragma once

#include "RenderOperation.h"
#include "Object.h"
#include "RGBA.h"
#include "Aabb.h"
#include "Matrix3.h"
#include "InputEnums.h"

namespace GraphicsEngine
{
	class Camera;
	class Scene;
	class SceneObject;
}

struct SelectionHandleTypeEnum
{
	enum SelectionHandleType
	{
		Move,
		Resize,
		Rotate
	};
};

namespace Enum
{
	typedef SelectionHandleTypeEnum::SelectionHandleType SelectionHandleType;
}

namespace Engine
{
	class InputObject;

	namespace Editor
	{

		struct SelectedAxisEnum
		{
			enum SelectedAxis
			{
				None,

				AxisX,
				AxisY,
				AxisZ
			};
		};

		class Selection;
		
		class SelectionHandlesOperation : public GraphicsEngine::RenderOperation
		{
		public:
			void Initialize() {}
			void Update(float) {}

			bool IsActive = true;
			bool DrawSelectionBox = true;
			bool DrawHandles = true;
			Vector3 Resolution;
			float SelectionRayDistance = 10000;

			bool IsLocalSpace = false;
			bool ObjectsShareHandles = true;
			Enum::SelectionHandleType HandleType = Enum::SelectionHandleType::Move;

			RGBA BoxColor = RGBA(0, 0.75f, 1);
			RGBA HoverBoxColor = RGBA(0.85f, 0.95f, 1);
			RGBA HandleColorX = RGBA(1, 0.2f, 0.2f);
			RGBA HandleColorY = RGBA(0.2f, 1, 0.2f);
			RGBA HandleColorZ = RGBA(0.2f, 0.2f, 1);

			std::weak_ptr<Selection> ActiveSelection;
			std::weak_ptr<GraphicsEngine::Scene> TargetScene;
			std::weak_ptr<GraphicsEngine::Camera> CurrentCamera;

			std::weak_ptr<InputObject> MousePosition = GetInput(Enum::InputCode::MousePosition);
			std::weak_ptr<InputObject> SelectButton = GetInput(Enum::InputCode::MouseLeft);
			std::weak_ptr<InputObject> RequiredModifierKey;
			std::weak_ptr<InputObject> MultiSelectModifierKey = GetInput(Enum::InputCode::LeftControl);

			void Render();

			Instantiable;

			Inherits_Class(GraphicsEngine::RenderOperation);

			Reflected(SelectionHandlesOperation);

		private:
			typedef SelectedAxisEnum::SelectedAxis SelectedAxis;

			Selection* ActiveSelectionRaw = nullptr;

			void ProcessInput();
			Aabb DrawSelection(const std::shared_ptr<GraphicsEngine::Camera>& camera, const std::shared_ptr<GraphicsEngine::SceneObject>& object, bool isHovered);
			void DrawSelection(const std::shared_ptr<GraphicsEngine::Camera>& camera, const Aabb& box, const Matrix3& transformation, bool drawHandles, bool isHovered);
			void ProcessObjectInput(const std::shared_ptr<GraphicsEngine::SceneObject>& object, const Vector3& intersection);
			void ProcessObjectInput(const std::shared_ptr<GraphicsEngine::SceneObject>& object, const Vector3& intersection, SelectedAxis axis);

			static std::shared_ptr<InputObject> GetInput(Enum::InputCode code);
		};
	}
}

namespace Engine
{
	Declare_Enum(SelectionHandleType);
}