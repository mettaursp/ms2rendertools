#include "SelectionHandlesOperation.h"

#include "Camera.h"
#include "Selection.h"
#include "Scene.h"
#include "LuaInput.h"
#include "Graphics.h"
#include "Engine.h"
#include "GameEngine.h"
#include "GameWindow.h"
#include "ModelAsset.h"
#include "Transform.h"

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

			glEnable(GL_DEPTH_TEST); CheckGLErrors();
			Graphics::ClearScreen(GL_DEPTH_BUFFER_BIT); CheckGLErrors();

			Aabb bounds;
			ObjectHandleHit closestHit;

			std::shared_ptr<InputObject> mousePosition = MousePosition.lock();

			if (mousePosition == nullptr) return;

			Programs::Screen->Use();

			Ray mouseRay = camera->GetRay(int(mousePosition->GetPosition().X), int(mousePosition->GetPosition().Y), int(Resolution.X), int(Resolution.Y), SelectionRayDistance);

			SelectedObjects = selection->GetObjectCount();

			for (int i = 0; i < selection->GetObjectCount(); ++i)
			{
				const std::shared_ptr<GraphicsEngine::SceneObject>& object = selection->GetObject(i);

				Aabb objectBounds = DrawSelection(closestHit, mouseRay, camera, object, false);

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

			if (ObjectsShareHandles && SelectedObjects > 0)
				DrawSelection(closestHit, mouseRay, camera, bounds, Matrix3(), Matrix3(), !(IsLocalSpace && SelectedObjects == 1), false);

			std::shared_ptr<InputObject> dragButton = DragButton.lock();
			std::shared_ptr<InputObject> resetButton = ResetButton.lock();

			if (closestHit.HitHandle)
			{
				Graphics::ClearScreen(GL_DEPTH_BUFFER_BIT); CheckGLErrors();

				Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(closestHit.Transformation));
				Programs::Screen->color.Set(GetHoverColor(closestHit.Axis));

				closestHit.RenderMesh->Draw();

				std::shared_ptr<InputObject> requiredDragModifierKey = RequiredDragModifierKey.lock();

				if (dragButton != nullptr && dragButton->GetState() && dragButton->GetStateChanged() && (requiredDragModifierKey == nullptr || requiredDragModifierKey->GetState()))
				{
					IsMovingObject = true;
					MovingInLocalSpace = IsLocalSpace && (!ObjectsShareHandles || SelectedObjects == 1);
					CurrentMovementHandle = closestHit.Axis;
					HandlePosition = closestHit.Transformation.Translation();
					HandleAxis = closestHit.Transformation.UpVector().Unit();
					InitialCursorPoint = GetMouseHandlePoint(camera, mouseRay);

					if (MovingInLocalSpace)
						MovingBox = closestHit.Object->GetLocalBoundingBox();
					else
						MovingBox = bounds;

					if (MovingObjects.size() > 0)
					{
						IsMovingObject = true;
					}

					if (closestHit.Object != nullptr)
					{
						const std::shared_ptr<GraphicsEngine::SceneObject> object = closestHit.Object->Cast<GraphicsEngine::SceneObject>();
						std::shared_ptr<Transform> objectTransform = object->GetComponent<Transform>();
						std::shared_ptr<Transform> objectParentTransform = objectTransform->GetComponent<Transform>();

						MovingObjects.push_back({ object, objectTransform, objectTransform->GetTransformation(), objectParentTransform == nullptr ? Matrix3() : objectParentTransform->GetWorldTransformationInverse(), objectParentTransform == nullptr ? Matrix3() : objectParentTransform->GetWorldTransformation() });
					}
					else
					{
						for (int i = 0; i < selection->GetObjectCount(); ++i)
						{
							const std::shared_ptr<GraphicsEngine::SceneObject> object = selection->GetObject(i);
							std::shared_ptr<Transform> objectTransform = object->GetComponent<Transform>();
							std::shared_ptr<Transform> objectParentTransform = objectTransform->GetComponent<Transform>();

							MovingObjects.push_back({ object, objectTransform, objectTransform->GetTransformation(), objectParentTransform == nullptr ? Matrix3() : objectParentTransform->GetWorldTransformationInverse(), objectParentTransform == nullptr ? Matrix3() : objectParentTransform->GetWorldTransformation() });
						}
					}
				}
			}

			if (dragButton == nullptr || !dragButton->GetState())
			{
				IsMovingObject = false;
				CurrentMovementHandle = SelectedAxis::None;
				MovingObjects.clear();
			}
			else if (IsMovingObject)
			{
				if (resetButton != nullptr && resetButton->GetState())
				{
					for (int i = 0; i < int(MovingObjects.size()); ++i)
						UpdateObject(i, 0);

					IsMovingObject = false;
					CurrentMovementHandle = SelectedAxis::None;
					MovingObjects.clear();
				}
				else
				{
					float handleValue = GetMouseHandleValue(camera, GetMouseHandlePoint(camera, mouseRay));

					std::cout << "handle value: " << handleValue << std::endl;

					for (int i = 0; i < int(MovingObjects.size()); ++i)
						UpdateObject(i, handleValue);
				}
			}

			if (IsMovingObject) return;

			std::shared_ptr<InputObject> selectButton = SelectButton.lock();
			std::shared_ptr<InputObject> requiredModifierKey = RequiredModifierKey.lock();
			std::shared_ptr<InputObject> multiSelectModifierKey = MultiSelectModifierKey.lock();

			if (selectButton == nullptr) return;

			bool canSelect = requiredModifierKey == nullptr || requiredModifierKey->GetState();

			if (!canSelect || closestHit.HitHandle) return;

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

				DrawSelection(closestHit, mouseRay, camera, hitObject, true);
			}
		}

		void SelectionHandlesOperation::ProcessInput()
		{
			if (IsMovingObject) return;

			std::shared_ptr<InputObject> moveToolKey = MoveToolKey.lock();
			std::shared_ptr<InputObject> resizeToolKey = ResizeToolKey.lock();
			std::shared_ptr<InputObject> rotateToolKey = RotateToolKey.lock();
			std::shared_ptr<InputObject> selectToolKey = SelectToolKey.lock();
			std::shared_ptr<InputObject> toggleEnabledKey = ToggleEnabledKey.lock();
			std::shared_ptr<InputObject> toggleLocalSpaceKey = ToggleLocalSpaceKey.lock();
			std::shared_ptr<InputObject> toggleGroupSelectKey = ToggleGroupSelectKey.lock();

			if (toggleEnabledKey != nullptr && toggleEnabledKey->GetState() && toggleEnabledKey->GetStateChanged())
				IsActive ^= true;

			if (moveToolKey != nullptr && moveToolKey->GetState() && moveToolKey->GetStateChanged())
			{
				DrawHandles = true;

				HandleType = Enum::SelectionHandleType::Move;
			}

			if (resizeToolKey != nullptr && resizeToolKey->GetState() && resizeToolKey->GetStateChanged())
			{
				DrawHandles = true;

				HandleType = Enum::SelectionHandleType::Resize;
			}

			if (rotateToolKey != nullptr && rotateToolKey->GetState() && rotateToolKey->GetStateChanged())
			{
				DrawHandles = true;

				HandleType = Enum::SelectionHandleType::Rotate;
			}

			if (toggleLocalSpaceKey != nullptr && toggleLocalSpaceKey->GetState() && toggleLocalSpaceKey->GetStateChanged())
				IsLocalSpace ^= true;

			if (toggleGroupSelectKey != nullptr && toggleGroupSelectKey->GetState() && toggleGroupSelectKey->GetStateChanged())
				ObjectsShareHandles ^= true;

			if (selectToolKey != nullptr && selectToolKey->GetState() && selectToolKey->GetStateChanged())
				DrawHandles = false;
		}

		Aabb SelectionHandlesOperation::DrawSelection(ObjectHandleHit& closestHit, const Ray& ray, const std::shared_ptr<GraphicsEngine::Camera>& camera, const std::shared_ptr<GraphicsEngine::SceneObject>& object, bool isHovered)
		{
			Aabb objectBounds = object->GetBoundingBox();
			Aabb objectLocalBounds = object->GetLocalBoundingBox();
			Matrix3 objectTransformation = object->GetTransformation();
			Matrix3 objectInverseTransformation = object->GetInverseTransformation();

			bool hitObject = false;

			if (IsLocalSpace && !(ObjectsShareHandles && SelectedObjects > 1))
				hitObject = DrawSelection(closestHit, ray, camera, objectLocalBounds, objectTransformation, objectInverseTransformation, !isHovered && (!ObjectsShareHandles || SelectedObjects == 1), isHovered);

			if (ObjectsShareHandles || !IsLocalSpace)
				hitObject |= DrawSelection(closestHit, ray, camera, objectBounds, Matrix3(), Matrix3(), /*!isHovered && (SelectedObjects == 1 || !IsLocalSpace)*/ false, isHovered);

			if (hitObject)
				closestHit.Object = object.get();

			return objectBounds;
		}

		void CastRay(const Ray& ray, const std::shared_ptr<Engine::ModelAsset>& asset, const Matrix3& transformation)
		{
			int meshID = asset->GetMeshID();

			const MeshData* data = MeshLoader::GetMeshData(meshID);

			float closest = std::numeric_limits<float>::max();
			bool hitObject = false;

			auto resultsProcessorLambda = [&closest, &hitObject](const SceneRayCastResults& results)
			{
				if (results.Distance < closest)
					closest = results.Distance;

				hitObject = true;
			};

			data->CastRay(Ray(transformation * ray.Start, transformation * ray.Direction), std::ref(resultsProcessorLambda));
		}

		SelectionHandlesOperation::HandleMesh SelectionHandlesOperation::GetCoreMesh(const std::string& name) const
		{
			int meshId = Graphics::GetCoreMesh(name)->GetMeshID();

			return HandleMesh{
				MeshLoader::GetMeshData(meshId),
				Programs::Screen->Meshes->GetMesh(meshId)
			};
		}

		RGBA SelectionHandlesOperation::GetColor(SelectedAxis axis) const
		{
			const RGBA white = RGBA(1, 1, 1, 1);

			switch (axis)
			{
			case SelectedAxis::None: return white;
			case SelectedAxis::AxisX: return HandleColorX;
			case SelectedAxis::AxisY: return HandleColorY;
			case SelectedAxis::AxisZ: return HandleColorZ;
			case SelectedAxis::AxisNegativeX: return HandleColorX;
			case SelectedAxis::AxisNegativeY: return HandleColorY;
			case SelectedAxis::AxisNegativeZ: return HandleColorZ;
			}

			return white;
		}

		RGBA SelectionHandlesOperation::GetHoverColor(SelectedAxis axis) const
		{
			const RGBA white = RGBA(1, 1, 1, 1);

			switch (axis)
			{
			case SelectedAxis::None: return white;
			case SelectedAxis::AxisX: return HoverHandleColorX;
			case SelectedAxis::AxisY: return HoverHandleColorY;
			case SelectedAxis::AxisZ: return HoverHandleColorZ;
			case SelectedAxis::AxisNegativeX: return HoverHandleColorX;
			case SelectedAxis::AxisNegativeY: return HoverHandleColorY;
			case SelectedAxis::AxisNegativeZ: return HoverHandleColorZ;
			}

			return white;
		}

		Vector3 SelectionHandlesOperation::GetMouseHandlePoint(const std::shared_ptr<GraphicsEngine::Camera>& camera, const Ray& mouseRay) const
		{
			Vector3 cameraLook = camera->GetTransformation().FrontVector();	

			if (HandleType == Enum::SelectionHandleType::Move || HandleType == Enum::SelectionHandleType::Resize)
			{
				Vector3 handleTangent = cameraLook.Cross(HandleAxis);

				if (std::abs(handleTangent.SquareLength()) < 1e-5f)
					return 0;

				Vector3 handlePlaneNormal = handleTangent.Cross(HandleAxis);

				float mouseDistanceToPlane = (handlePlaneNormal * (HandlePosition - mouseRay.Start)) / (handlePlaneNormal * mouseRay.Direction);

				return mouseRay.Start + mouseDistanceToPlane * mouseRay.Direction;
			}
			else
			{
				float mouseDistanceToPlane = (HandleAxis * (HandlePosition - mouseRay.Start)) / (HandleAxis * mouseRay.Direction);

				return mouseRay.Start + mouseDistanceToPlane * mouseRay.Direction;
			}
		}

		float SelectionHandlesOperation::GetMouseHandleValue(const std::shared_ptr<GraphicsEngine::Camera>& camera, const Vector3& mousePoint) const
		{
			if (HandleType == Enum::SelectionHandleType::Move || HandleType == Enum::SelectionHandleType::Resize)
				return (mousePoint - HandlePosition) * HandleAxis - (InitialCursorPoint - HandlePosition) * HandleAxis;
			else
			{
				Vector3 axis1 = (InitialCursorPoint - HandlePosition).Unit();
				Vector3 axis2 = (mousePoint - HandlePosition).Unit();

				float sin = axis1.Cross(axis2) * HandleAxis;
				float cos = axis1 * axis2;

				float angle = std::atan2(sin, cos);

				return angle;
			}
		}

		void SelectionHandlesOperation::UpdateObject(int index, float handleValue)
		{
			MovingObjectData& object = MovingObjects[index];

			if (handleValue == 0)
			{
				object.ObjectTransform->SetTransformation(object.InitialTransformation);

				return;
			}

			if (HandleType == Enum::SelectionHandleType::Move || HandleType == Enum::SelectionHandleType::Resize)
			{
				Vector3 movementAxis = object.ParentInverseTransformation * (handleValue * HandleAxis);

				if (HandleType == Enum::SelectionHandleType::Move)
					object.ObjectTransform->SetTransformation(Matrix3(movementAxis) * object.InitialTransformation);
				else
				{
					Vector3 scale(1, 1, 1);

					if (MovingInLocalSpace)
					{
						float axisSize = 1;

						if (CurrentMovementHandle == SelectedAxis::AxisX || CurrentMovementHandle == SelectedAxis::AxisNegativeX)
							axisSize = 0.5f * object.InitialTransformation.RightVector().Length() * MovingBox.GetSize().X;
						else if (CurrentMovementHandle == SelectedAxis::AxisY || CurrentMovementHandle == SelectedAxis::AxisNegativeY)
							axisSize = 0.5f * object.InitialTransformation.UpVector().Length() * MovingBox.GetSize().Y;
						else if (CurrentMovementHandle == SelectedAxis::AxisZ || CurrentMovementHandle == SelectedAxis::AxisNegativeZ)
							axisSize = 0.5f * object.InitialTransformation.FrontVector().Length() * MovingBox.GetSize().Z;
					
						float newAxisSize = std::max(axisSize + 0.5f * handleValue, MinimumObjectSize);

						movementAxis = object.ParentInverseTransformation * ((newAxisSize - axisSize) * HandleAxis);

						if (CurrentMovementHandle == SelectedAxis::AxisX || CurrentMovementHandle == SelectedAxis::AxisNegativeX)
							scale.X = newAxisSize / axisSize;
						else if (CurrentMovementHandle == SelectedAxis::AxisY || CurrentMovementHandle == SelectedAxis::AxisNegativeY)
							scale.Y = newAxisSize / axisSize;
						else if (CurrentMovementHandle == SelectedAxis::AxisZ || CurrentMovementHandle == SelectedAxis::AxisNegativeZ)
							scale.Z = newAxisSize / axisSize;

						object.ObjectTransform->SetTransformation(Matrix3(movementAxis) * object.InitialTransformation * Matrix3::NewScale(scale));
					}
					else
					{
						Vector3 boxSize = MovingBox.GetSize();
						float axisSize = 1;

						if (CurrentMovementHandle == SelectedAxis::AxisX || CurrentMovementHandle == SelectedAxis::AxisNegativeX)
							axisSize = boxSize.X;
						else if (CurrentMovementHandle == SelectedAxis::AxisY || CurrentMovementHandle == SelectedAxis::AxisNegativeY)
							axisSize = boxSize.Y;
						else if (CurrentMovementHandle == SelectedAxis::AxisZ || CurrentMovementHandle == SelectedAxis::AxisNegativeZ)
							axisSize = boxSize.Z;

						float newAxisSize = std::max(axisSize + handleValue, MinimumObjectSize);

						float axisScaleShift = newAxisSize / axisSize;

						scale = Vector3(axisScaleShift, axisScaleShift, axisScaleShift);

						axisScaleShift -= 1;

						if (CurrentMovementHandle == SelectedAxis::AxisNegativeX || CurrentMovementHandle == SelectedAxis::AxisNegativeY || CurrentMovementHandle == SelectedAxis::AxisNegativeZ)
							axisScaleShift *= -1;

						movementAxis = Vector3();

						movementAxis += axisScaleShift * boxSize.X * Vector3(1, 0, 0);
						movementAxis += axisScaleShift * boxSize.Y * Vector3(0, 1, 0);
						movementAxis += axisScaleShift * boxSize.Z * Vector3(0, 0, 1);

						movementAxis *= 0.5f;

						object.ObjectTransform->SetTransformation(object.ParentInverseTransformation * Matrix3(MovingBox.GetCenter() + movementAxis) * Matrix3::NewScale(scale) * Matrix3(-MovingBox.GetCenter()) * object.ParentTransformation * object.InitialTransformation);
					}

					//object.ObjectTransform->SetTransformation(Matrix3(movementAxis) * object.InitialTransformation * Matrix3::NewScale(scale));
				}
			}
			else
			{
				Matrix3 rotation;

				if (CurrentMovementHandle == SelectedAxis::AxisX)
					rotation.RotatePitch(handleValue);

				if (CurrentMovementHandle == SelectedAxis::AxisY)
					rotation.RotateYaw(handleValue);

				if (CurrentMovementHandle == SelectedAxis::AxisZ)
					rotation.RotateRoll(-handleValue);

				if (MovingInLocalSpace)
				{
					Matrix3 transformation = object.InitialTransformation;

					transformation.SetTranslation(Vector3());

					object.ObjectTransform->SetTransformation(rotation.SetTranslation(object.InitialTransformation.Translation()) * transformation);
				}
				else
				{

					object.ObjectTransform->SetTransformation(object.ParentInverseTransformation * Matrix3(MovingBox.GetCenter()) * rotation * Matrix3(-MovingBox.GetCenter()) * object.ParentTransformation * object.InitialTransformation);
				}
			}
		}

		bool SelectionHandlesOperation::DrawAxisHandle(const HandleMesh& mesh, const Ray& ray, const Matrix3& transformation, const Matrix3& cameraTransformation, ObjectHandleHit& closestHit, SelectedAxis axis)
		{
			bool hitObject = false;

			auto resultsProcessorLambda = [&closestHit, &hitObject](const SceneRayCastResults& results)
			{
				if (results.Distance < closestHit.ClosestDistance)
				{
					closestHit.ClosestDistance = results.Distance;

					hitObject = true;
				}
			};

			Matrix3 inverseTransformation(Vector3(), transformation.RightVector().InvertedLength(), transformation.UpVector().InvertedLength(), transformation.FrontVector().InvertedLength());

			inverseTransformation.Transpose();
			inverseTransformation.SetTransformedTranslation(-transformation.Translation());

			mesh.Data->CastRay(Ray(inverseTransformation * ray.Start, inverseTransformation * ray.Direction), std::ref(resultsProcessorLambda));

			if (hitObject)
			{
				closestHit.HitHandle = true;
				closestHit.Axis = axis;
				closestHit.Object = nullptr;
				closestHit.RenderMesh = mesh.RenderMesh;
				closestHit.Transformation = transformation;
			}

			Programs::Screen->transform.Set(cameraTransformation.FullMultiply(transformation));
			Programs::Screen->color.Set(GetColor(axis));

			mesh.RenderMesh->Draw();

			return hitObject;
		}

		bool SelectionHandlesOperation::DrawSelection(ObjectHandleHit& closestHit, const Ray& ray, const std::shared_ptr<GraphicsEngine::Camera>& camera, const Aabb& box, const Matrix3& transformation, const Matrix3& transformationInverse, bool drawHandles, bool isHovered)
		{
			GraphicsEngine::SceneObject* object = nullptr;

			Matrix3 boxTransformation = transformation * Matrix3(box.GetCenter())* Matrix3::NewScale(0.5f * box.GetSize());

			Programs::Screen->transform.Set(camera->GetProjection().FullMultiply(boxTransformation));
			Programs::Screen->color.Set(isHovered ? HoverBoxColor : BoxColor);

			Programs::Screen->CoreMeshes.WireCube->Draw();

			bool hitObject = false;

			if (drawHandles && DrawHandles)
			{
				Vector3 center = transformation * box.GetCenter();
				Vector3 size = box.GetSize().Scale(transformation.ExtractScale());
				float boxSize = 0.5f * std::sqrtf(2) * std::max(std::max(size.X, size.Y), size.Z);

				Matrix3 rotation = Matrix3().ExtractRotation(transformation);

				if (HandleType == Enum::SelectionHandleType::Rotate)
				{
					rotation = rotation * Matrix3::NewScale(boxSize, boxSize, boxSize);

					hitObject |= DrawAxisHandle(
						RingMesh,
						ray,
						Matrix3(center, -rotation.UpVector(), rotation.RightVector(), rotation.FrontVector()),
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisX
					);

					hitObject |= DrawAxisHandle(
						RingMesh,
						ray,
						Matrix3(center, rotation.RightVector(), rotation.UpVector(), rotation.FrontVector()),
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisY
					);

					hitObject |= DrawAxisHandle(
						RingMesh,
						ray,
						Matrix3(center, rotation.RightVector(), -rotation.FrontVector(), rotation.UpVector()),
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisZ
					);
				}
				else
				{
					const HandleMesh& handleMesh = HandleType == Enum::SelectionHandleType::Move ? ArrowMesh : SphereMesh;
					const Mesh* mesh = HandleType == Enum::SelectionHandleType::Move ? Programs::Screen->CoreMeshes.Arrow : Programs::Screen->CoreMeshes.Sphere;

					float minSize = HandleType == Enum::SelectionHandleType::Move ? ArrowHandleMinSize : SphereHandleMinSize;
					float scaling = HandleType == Enum::SelectionHandleType::Move ? ArrowHandleScaling : SphereHandleScaling;
					float minOffset = HandleType == Enum::SelectionHandleType::Move ? ArrowHandleMinOffset : SphereHandleMinOffset;
					float scaledOffset = HandleType == Enum::SelectionHandleType::Move ? ArrowHandleScaledOffset : SphereHandleScaledOffset;

					float scale = minSize + scaling * boxSize;
					float offset = minOffset + scaledOffset * boxSize;

					Vector3 right = scale * rotation.RightVector();
					Vector3 up = scale * rotation.UpVector();
					Vector3 front = scale * rotation.FrontVector();

					Vector3 rightOffset = (offset + 0.5f * size.X) * rotation.RightVector();
					Vector3 upOffset = (offset + 0.5f * size.Y) * rotation.UpVector();
					Vector3 frontOffset = (offset + 0.5f * size.Z) * rotation.FrontVector();

					// x
					Matrix3 rotationX(center + rightOffset, -up, right, front);

					hitObject |= DrawAxisHandle(
						handleMesh,
						ray,
						rotationX,
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisX
					);

					rotationX.SetVectors(center - rightOffset, up, -right, front);

					hitObject |= DrawAxisHandle(
						handleMesh,
						ray,
						rotationX,
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisNegativeX
					);

					// y
					Matrix3 rotationY(center + upOffset, right, up, front);

					hitObject |= DrawAxisHandle(
						handleMesh,
						ray,
						rotationY,
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisY
					);

					rotationY.SetVectors(center - upOffset, right, -up,-front);

					hitObject |= DrawAxisHandle(
						handleMesh,
						ray,
						rotationY,
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisNegativeY
					);

					// z
					Matrix3 rotationZ(center + frontOffset, right, front, -up);

					hitObject |= DrawAxisHandle(
						handleMesh,
						ray,
						rotationZ,
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisZ
					);

					rotationZ.SetVectors(center - frontOffset, right, -front, up);

					hitObject |= DrawAxisHandle(
						handleMesh,
						ray,
						rotationZ,
						camera->GetProjection(),
						closestHit,
						SelectedAxis::AxisNegativeZ
					);
				}
			}

			return hitObject;
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