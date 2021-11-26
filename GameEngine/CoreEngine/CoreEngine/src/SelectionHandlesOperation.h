#pragma once

#include "RenderOperation.h"
#include "Object.h"
#include "RGBA.h"
#include "Aabb.h"
#include "Matrix3.h"
#include "InputEnums.h"
#include "Constants.h"

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

class Mesh;
class MeshData;
class Ray;

namespace Engine
{
	class InputObject;
	class Transform;

	namespace Editor
	{

		struct SelectedAxisEnum
		{
			enum SelectedAxis
			{
				None,

				AxisX,
				AxisY,
				AxisZ,

				AxisNegativeX,
				AxisNegativeY,
				AxisNegativeZ
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

			bool SnapsToGrid = false;
			float MinimumObjectSize = 0.01f;
			float GridLength = 1;
			float SnappingAngle = PI / 4.f;

			bool IsLocalSpace = false;
			bool ObjectsShareHandles = true;
			Enum::SelectionHandleType HandleType = Enum::SelectionHandleType::Move;

			RGBA BoxColor = RGBA(0, 0.75f, 1);
			RGBA HoverBoxColor = RGBA(0.85f, 0.95f, 1);
			RGBA HandleColorX = RGBA(1, 0.2f, 0.2f);
			RGBA HandleColorY = RGBA(0.2f, 1, 0.2f);
			RGBA HandleColorZ = RGBA(0.2f, 0.2f, 1);

			RGBA HoverHandleColorX = RGBA(1, 0.75f, 0.75f);
			RGBA HoverHandleColorY = RGBA(0.75f, 1, 0.75f);
			RGBA HoverHandleColorZ = RGBA(0.75f, 0.75f, 1);

			float ArrowHandleMinSize = 0.15f;
			float ArrowHandleScaling = 0.15f;
			float ArrowHandleMinOffset = 0.1f;
			float ArrowHandleScaledOffset = 0.1f;

			float SphereHandleMinSize = 0.1f;
			float SphereHandleScaling = 0.1f;
			float SphereHandleMinOffset = 0.2f;
			float SphereHandleScaledOffset = 0.2f;

			std::weak_ptr<Selection> ActiveSelection;
			std::weak_ptr<GraphicsEngine::Scene> TargetScene;
			std::weak_ptr<GraphicsEngine::Camera> CurrentCamera;

			std::weak_ptr<InputObject> MousePosition = GetInput(Enum::InputCode::MousePosition);
			std::weak_ptr<InputObject> SelectButton = GetInput(Enum::InputCode::MouseLeft);
			std::weak_ptr<InputObject> DragButton = GetInput(Enum::InputCode::MouseLeft);
			std::weak_ptr<InputObject> ResetButton = GetInput(Enum::InputCode::MouseRight);
			std::weak_ptr<InputObject> RequiredModifierKey;
			std::weak_ptr<InputObject> RequiredDragModifierKey;
			std::weak_ptr<InputObject> MultiSelectModifierKey = GetInput(Enum::InputCode::LeftControl);

			std::weak_ptr<InputObject> MoveToolKey = GetInput(Enum::InputCode::T);
			std::weak_ptr<InputObject> ResizeToolKey = GetInput(Enum::InputCode::G);
			std::weak_ptr<InputObject> RotateToolKey = GetInput(Enum::InputCode::R);
			std::weak_ptr<InputObject> ToggleEnabledKey = GetInput(Enum::InputCode::End);
			std::weak_ptr<InputObject> SelectToolKey = GetInput(Enum::InputCode::F);

			std::weak_ptr<InputObject> ToggleLocalSpaceKey = GetInput(Enum::InputCode::L);
			std::weak_ptr<InputObject> ToggleGroupSelectKey = GetInput(Enum::InputCode::O);

			void Render();

			Instantiable;

			Inherits_Class(GraphicsEngine::RenderOperation);

			Reflected(SelectionHandlesOperation);

		private:
			typedef SelectedAxisEnum::SelectedAxis SelectedAxis;

			struct ObjectHandleHit
			{
				bool HitHandle = false;
				float ClosestDistance = std::numeric_limits<float>::max();
				GraphicsEngine::SceneObject* Object = nullptr;
				SelectedAxis Axis = SelectedAxis::None;
				Matrix3 Transformation;
				const Mesh* RenderMesh = nullptr;
			};

			struct HandleMesh
			{
				const MeshData* Data = nullptr;
				const Mesh* RenderMesh = nullptr;
			};

			struct MovingObjectData
			{
				std::shared_ptr<GraphicsEngine::SceneObject> Object;
				std::shared_ptr<Transform> ObjectTransform;
				Matrix3 InitialTransformation;
				Matrix3 ParentInverseTransformation;
				Matrix3 ParentTransformation;
			};

			typedef std::vector<MovingObjectData> MovingObjectVector;

			int SelectedObjects = 0;
			Selection* ActiveSelectionRaw = nullptr;
			const HandleMesh RingMesh = GetCoreMesh("CoreRing");
			const HandleMesh ArrowMesh = GetCoreMesh("CoreArrow");
			const HandleMesh SphereMesh = GetCoreMesh("CoreSphere");

			bool IsMovingObject = false;
			bool MovingInLocalSpace = false;
			SelectedAxis CurrentMovementHandle = SelectedAxis::None;
			Vector3 InitialCursorPoint;
			Vector3 HandlePosition;
			Vector3 HandleAxis;
			Aabb MovingBox;
			MovingObjectVector MovingObjects;

			RGBA GetColor(SelectedAxis axis) const;
			RGBA GetHoverColor(SelectedAxis axis) const;
			Vector3 GetMouseHandlePoint(const std::shared_ptr<GraphicsEngine::Camera>& camera, const Ray& mouseRay) const;
			float GetMouseHandleValue(const std::shared_ptr<GraphicsEngine::Camera>& camera, const Vector3& mousePoint) const;
			void UpdateObject(int index, float handleValue);
			HandleMesh GetCoreMesh(const std::string& name) const;
			void ProcessInput();
			Aabb DrawSelection(ObjectHandleHit& closestHit, const Ray& ray, const std::shared_ptr<GraphicsEngine::Camera>& camera, const std::shared_ptr<GraphicsEngine::SceneObject>& object, bool isHovered);
			bool DrawSelection(ObjectHandleHit& closestHit, const Ray& ray, const std::shared_ptr<GraphicsEngine::Camera>& camera, const Aabb& box, const Matrix3& transformation, const Matrix3& transformationInverse, bool drawHandles, bool isHovered);
			bool DrawAxisHandle(const HandleMesh& mesh, const Ray& ray, const Matrix3& transformation, const Matrix3& inverseTransformation, ObjectHandleHit& closestHit, SelectedAxis axis);
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