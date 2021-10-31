#pragma once

#include <vector>

#include "Object.h"
#include "TaskScheduler.h"

namespace GraphicsEngine
{
	class SceneObject;
}

namespace Engine
{
	namespace Editor
	{
		class Selection : public Object
		{
		public:
			void Initialize() {}
			void Update(float) {}

			void AddObject(const std::shared_ptr<GraphicsEngine::SceneObject>& object);
			void RemoveObject(const std::shared_ptr<GraphicsEngine::SceneObject>& object);
			bool IsObjectSelected(const std::shared_ptr<GraphicsEngine::SceneObject>& object);
			void ToggleObjectSelection(const std::shared_ptr<GraphicsEngine::SceneObject>& object);
			void Clear();
			int GetObjectCount() const;
			const std::shared_ptr<GraphicsEngine::SceneObject>& GetObject(int index);

			Event<Selection*> SelectionChanged;

		private:
			typedef std::vector<std::shared_ptr<GraphicsEngine::SceneObject>> ObjectVector;

			ObjectVector SelectedObjects;

			int FindIndex(const std::shared_ptr<GraphicsEngine::SceneObject>& object);

			Instantiable;

			Inherits_Class(Object);

			Reflected(Selection);
		};
	}
}