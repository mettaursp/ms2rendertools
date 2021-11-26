#pragma once

#include "Object.h"
#include "Vector3.h"
#include "Matrix3.h"
#include "Quaternion.h"
#include "TaskScheduler.h"

namespace Engine
{
	class Transform : public Object
	{
	public:
		virtual ~Transform() {}

		void Initialize() {}

		void Update(float delta);

		bool HasMoved() const;
		bool HasMoved();

		void SetStatic(bool isStatic);
		bool IsTransformStatic() const;

		void SetTransformation(const Matrix3& matrix);
		const Matrix3& GetTransformation();

		void SetInheritsTransformation(bool inherits);
		bool InheritsTransformation() const;

		Vector3 GetPosition() const;
		Vector3 GetPosition();
		void SetPosition(const Vector3& position);
		void Move(const Vector3& offset);

		Vector3 GetWorldPosition() const;
		Vector3 GetWorldPosition();
		const Matrix3& GetWorldTransformation() const;
		const Matrix3& GetWorldTransformation();
		const Matrix3& GetWorldTransformationInverse() const;
		const Matrix3& GetWorldTransformationInverse();
		const Matrix3& GetWorldRotation() const;
		const Matrix3& GetWorldRotation();
		Quaternion GetWorldOrientation() const;
		Quaternion GetWorldOrientation();
		const Matrix3& GetWorldNormalTransformation() const;
		const Matrix3& GetWorldNormalTransformation() ;

		Quaternion GetOrientation() const;
		Quaternion GetOrientation();
		void SetOrientation(const Quaternion& orientation);
		void Rotate(const Quaternion& rotation);
		void Rotate(const Vector3 axis, float angle);

		Vector3 GetEulerAngles() const;
		Vector3 GetEulerAngles();
		void SetEulerAngles(const Vector3& angles);
		void SetEulerAngles(float pitch, float roll, float yaw);
		void Rotate(const Vector3& angles);
		void Rotate(float pitch, float roll, float yaw);

		Vector3 GetEulerAnglesYaw() const;
		Vector3 GetEulerAnglesYaw();
		void SetEulerAnglesYaw(const Vector3& angles);
		void SetEulerAnglesYaw(float yaw, float pitch, float roll);
		void RotateYaw(const Vector3& angles);
		void RotateYaw(float yaw, float pitch, float roll);

		Vector3 GetScale() const;
		Vector3 GetScale();
		void SetScale(const Vector3& scale);
		void Rescale(const Vector3& scale);

		void TransformBy(const Matrix3& transformation);
		void TransformBy(const Quaternion& transformation, const Vector3& point = Vector3());
		void TransformByRelative(const Matrix3& transformation);
		void TransformByRelative(const Quaternion& transformation, const Vector3& point = Vector3());

		Event<Transform*> TransformMoved;

	private:
		Matrix3 Transformation;
		Matrix3 WorldTransformation;
		Matrix3 WorldTransformationInverse;
		Matrix3 WorldRotation;
		Matrix3 WorldNormalTransformation;

		bool HasChanged();
		void Recompute();

		bool IsStaticTransformation = true;
		bool InheritTransformation = true;
		bool Moved = false;
		bool HadParent = false;
		Matrix3 OldParentTransform;

		Instantiable;

		Inherits_Class(Object);

		Reflected(Transform);
	};
}
