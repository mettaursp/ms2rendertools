#include "SelectionHandlesOperation.h"

#include "Camera.h"
#include "Selection.h"
#include "Scene.h"
#include "LuaInput.h"
#include "Graphics.h"
#include "Engine.h"
#include "GameEngine.h"
#include "GameWindow.h"

namespace Engine
{
	namespace Editor
	{
		void SelectionHandlesOperation::Render()
		{
			std::shared_ptr<Selection> selection = ActiveSelection.lock();
			std::shared_ptr<GraphicsEngine::Camera> camera = CurrentCamera.lock();
			std::shared_ptr<GraphicsEngine::Scene> scene = TargetScene.lock();

			if (selection == nullptr || camera == nullptr || scene == nullptr) return;

			ActiveSelectionRaw = selection.get();

			ProcessInput();

			if (!IsActive) return;

			Aabb bounds;

			Programs::Screen->Use();

			for (int i = 0; i < selection->GetObjectCount(); ++i)
			{
				const std::shared_ptr<GraphicsEngine::SceneObject>& object = selection->GetObject(i);

				Aabb objectBounds = DrawSelection(camera, object, false);

				if (ObjectsShareHandles)
				{
					if (i == 0)
						bounds = objectBounds;
					else
					{
						bounds.Expand(objectBounds.Min);
						bounds.Expand(objectBounds.Max);
					}
				}
			}

			if (ObjectsShareHandles)
				DrawSelection(camera, bounds, Matrix3(), true, false);

			std::shared_ptr<InputObject> mousePosition = MousePosition.lock();
			std::shared_ptr<InputObject> selectButton = SelectButton.lock();
			std::shared_ptr<InputObject> requiredModifierKey = RequiredModifierKey.lock();
			std::shared_ptr<InputObject> multiSelectModifierKey = MultiSelectModifierKey.lock();

			if (mousePosition == nullptr || selectButton == nullptr) return;

			bool canSelect = requiredModifierKey == nullptr || requiredModifierKey->GetState();
			
			Ray mouseRay = camera->GetRay(mousePosition->GetPosition().X, mousePosition->GetPosition().Y, Resolution.X, Resolution.Y, SelectionRayDistance);

			SceneRayCastResults hit;

			hit.Distance = SelectionRayDistance + 1;

			scene->CastRay(mouseRay, [&hit](const SceneRayCastResults& results)
			{
				if (results.Distance < hit.Distance)
					hit = results;
			});

			if (hit.Hit != nullptr)
			{
				std::shared_ptr<GraphicsEngine::SceneObject> hitObject = hit.Hit->Cast<GraphicsEngine::SceneObject>();

				if (selectButton->GetState() && selectButton->GetStateChanged())
				{
					if (multiSelectModifierKey != nullptr && !multiSelectModifierKey->GetState())
						selection->Clear();

					ProcessObjectInput(hitObject, hit.Intersection);
				}

				DrawSelection(camera, hitObject, true);
			}
		}

		void SelectionHandlesOperation::ProcessInput()
		{

		}

		Aabb SelectionHandlesOperation::DrawSelection(const std::shared_ptr<GraphicsEngine::Camera>& camera, const std::shared_ptr<GraphicsEngine::SceneObject>& object, bool isHovered)
		{
			Aabb objectBounds = object->GetBoundingBox();
			Aabb objectLocalBounds = object->GetLocalBoundingBox();
			Matrix3 objectTransformation = object->GetTransformation();

			if (IsLocalSpace)
				DrawSelection(camera, objectLocalBounds, objectTransformation, !isHovered && !ObjectsShareHandles, isHovered);

			if (!(ObjectsShareHandles && IsLocalSpace))
				DrawSelection(camera, objectBounds, Matrix3(), !isHovered && !ObjectsShareHandles && !IsLocalSpace, isHovered);

			return objectBounds;
		}

		void SelectionHandlesOperation::DrawSelection(const std::shared_ptr<GraphicsEngine::Camera>& camera, const Aabb& box, const Matrix3& transformation, bool drawHandles, bool isHovered)
		{
			Matrix3 boxTransformation = transformation * Matrix3(box.GetCenter())* Matrix3::NewScale(0.5f * box.GetSize());

			Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(boxTransformation));
			Programs::Screen->color.Set(isHovered ? HoverBoxColor : BoxColor);

			Programs::Screen->CoreMeshes.WireCube->Draw();

			if (drawHandles)
			{
				Vector3 center = transformation * box.GetCenter();
				Vector3 size = box.GetSize();

				Matrix3 rotation = Matrix3().ExtractRotation(transformation, center);

				if (HandleType == Enum::SelectionHandleType::Rotate)
				{
					float boxSize = 0.5f * std::sqrtf(2) * std::max(std::max(size.X, size.Y), size.Z);

					rotation = rotation * Matrix3::NewScale(boxSize, boxSize, boxSize);

					Matrix3 rotationX(center, -rotation.UpVector(), rotation.RightVector(), rotation.FrontVector());
					Matrix3 rotationZ(center, rotation.RightVector(), -rotation.FrontVector(), rotation.UpVector());

					Programs::Screen->color.Set(HandleColorX);
					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationX));

					Programs::Screen->CoreMeshes.Ring->Draw();

					Programs::Screen->color.Set(HandleColorY);
					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotation));

					Programs::Screen->CoreMeshes.Ring->Draw();

					Programs::Screen->color.Set(HandleColorZ);
					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationZ));

					Programs::Screen->CoreMeshes.Ring->Draw();
				}
				else
				{
					const Mesh* mesh = HandleType == Enum::SelectionHandleType::Move ? Programs::Screen->CoreMeshes.Arrow : Programs::Screen->CoreMeshes.Sphere;

					// x
					Programs::Screen->color.Set(HandleColorX);

					Matrix3 rotationX(center + 0.65f * size.X * rotation.RightVector(), 0.5f * -rotation.UpVector(), 0.5f * rotation.RightVector(), 0.5f * rotation.FrontVector());

					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationX));

					mesh->Draw();

					rotationX.SetVectors(center - 0.65f * size.X * rotation.RightVector(), 0.5f * rotation.UpVector(), 0.5f * -rotation.RightVector(), 0.5f * rotation.FrontVector());

					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationX));

					mesh->Draw();

					// y
					Programs::Screen->color.Set(HandleColorY);

					Matrix3 rotationY(center + 0.65f * size.Y * rotation.UpVector(), 0.5f * rotation.RightVector(), 0.5f * rotation.UpVector(), 0.5f * rotation.FrontVector());;

					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationY));

					mesh->Draw();

					rotationY.SetVectors(center - 0.65f * size.Y * rotation.UpVector(), 0.5f * rotation.RightVector(), 0.5f * -rotation.UpVector(), 0.5f * -rotation.FrontVector());

					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationY));

					mesh->Draw();

					// z
					Programs::Screen->color.Set(HandleColorZ);

					Matrix3 rotationZ(center + 0.65f * size.Z * rotation.FrontVector(), 0.5f * rotation.RightVector(), 0.5f * rotation.FrontVector(), 0.5f * -rotation.UpVector());

					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationZ));

					mesh->Draw();

					rotationZ.SetVectors(center - 0.65f * size.Z * rotation.FrontVector(), 0.5f * rotation.RightVector(), 0.5f * -rotation.FrontVector(), 0.5f * rotation.UpVector());

					Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(rotationZ));

					mesh->Draw();
				}
			}
		}

		void SelectionHandlesOperation::ProcessObjectInput(const std::shared_ptr<GraphicsEngine::SceneObject>& object, const Vector3& intersection)
		{
			ActiveSelectionRaw->ToggleObjectSelection(object);
		}

		void SelectionHandlesOperation::ProcessObjectInput(const std::shared_ptr<GraphicsEngine::SceneObject>& object, const Vector3& intersection, SelectedAxis axis)
		{

		}

		std::shared_ptr<InputObject> SelectionHandlesOperation::GetInput(Enum::InputCode code)
		{
			return Root()->GetPrimaryWindow()->GetInput()->GetInput(code);
		}
	}
}