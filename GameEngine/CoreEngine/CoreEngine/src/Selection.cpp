#include "Selection.h"

namespace Engine
{
	namespace Editor
	{

		void Selection::AddObject(const std::shared_ptr<Object>& object)
		{
			if (object == nullptr) return;

			if (FindIndex(object) == -1)
			{
				SelectedObjects.push_back(object);

				SelectionChanged.Fire(this);
			}
		}

		void Selection::RemoveObject(const std::shared_ptr<Object>& object)
		{
			if (object == nullptr) return;

			int index = FindIndex(object);

			if (index == -1)
			{
				std::swap(SelectedObjects[index], SelectedObjects.back());

				SelectedObjects.pop_back();

				SelectionChanged.Fire(this);
			}
		}

		void Selection::Clear()
		{
			SelectedObjects.clear();
		}

		int Selection::GetObjectCount() const
		{
			return int(SelectedObjects.size());
		}

		int Selection::FindIndex(const std::shared_ptr<Object>& object)
		{
			for (int i = 0; i < GetObjectCount(); ++i)
				if (SelectedObjects[i] == object)
					return i;

			return -1;
		}
	}
}