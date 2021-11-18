#include "Selection.h"

namespace Engine
{
	namespace Editor
	{

		void Selection::AddObject(const std::shared_ptr<GraphicsEngine::SceneObject>& object)
		{
			if (object == nullptr) return;

			if (FindIndex(object) == -1)
			{
				SelectedObjects.push_back(object);

				SelectionChanged.Fire(this);
			}
		}

		void Selection::RemoveObject(const std::shared_ptr<GraphicsEngine::SceneObject>& object)
		{
			if (object == nullptr) return;

			int index = FindIndex(object);

			if (index != -1)
			{
				std::swap(SelectedObjects[index], SelectedObjects.back());

				SelectedObjects.pop_back();

				SelectionChanged.Fire(this);
			}
		}

		bool Selection::IsObjectSelected(const std::shared_ptr<GraphicsEngine::SceneObject>& object)
		{
			if (object == nullptr) return false;

			return FindIndex(object) != -1;
		}

		void Selection::ToggleObjectSelection(const std::shared_ptr<GraphicsEngine::SceneObject>& object)
		{
			if (object == nullptr) return;

			int index = FindIndex(object);

			if (index == -1)
				SelectedObjects.push_back(object);
			else
			{
				std::swap(SelectedObjects[index], SelectedObjects.back());

				SelectedObjects.pop_back();
			}

			SelectionChanged.Fire(this);
		}

		void Selection::Clear()
		{
			SelectedObjects.clear();
		}

		int Selection::GetObjectCount() const
		{
			return int(SelectedObjects.size());
		}

		const std::shared_ptr<GraphicsEngine::SceneObject>& Selection::GetObject(int index)
		{
			static const std::shared_ptr<GraphicsEngine::SceneObject> null;

			if (index < 0 || index >= int(SelectedObjects.size()))
				return null;

			return SelectedObjects[index];
		}

		int Selection::FindIndex(const std::shared_ptr<GraphicsEngine::SceneObject>& object)
		{
			for (int i = 0; i < GetObjectCount(); ++i)
				if (SelectedObjects[i] == object)
					return i;

			return -1;
		}
	}
}