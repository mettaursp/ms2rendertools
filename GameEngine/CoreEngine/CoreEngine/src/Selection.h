#pragma once

#include <vector>

#include "Object.h"
#include "TaskScheduler.h"

namespace Engine
{
	namespace Editor
	{
		class Selection : public Object
		{
		public:
			void Initialize() {}
			void Update(float) {}

			void AddObject(const std::shared_ptr<Object>& object);
			void RemoveObject(const std::shared_ptr<Object>& object);
			void Clear();
			int GetObjectCount() const;

			Event<Selection*> SelectionChanged;

		private:
			typedef std::vector<std::shared_ptr<Object>> ObjectVector;

			ObjectVector SelectedObjects;

			int FindIndex(const std::shared_ptr<Object>& object);

			Instantiable;

			Inherits_Class(Object);

			Reflected(Selection);
		};
	}
}