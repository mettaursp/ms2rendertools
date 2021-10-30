#pragma once

#include "Object.h"

namespace Engine
{
	class InputObject;

	namespace Editor
	{
		class SelectionHandlesOperation;

		class Selector : public Object
		{
		public:
			void Initialize() {}
			void Update(float) {}

			std::weak_ptr<InputObject> MousePosition;
			std::weak_ptr<InputObject> SelectButton;
			std::weak_ptr<InputObject> RequiredModifierKey;

			std::weak_ptr<SelectionHandlesOperation> SelectionHandles;
		};
	}
}