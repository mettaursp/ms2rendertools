#pragma once

#include "RenderOperation.h"
#include "Object.h"

namespace GraphicsEngine
{
	class Camera;
}

namespace Engine
{
	namespace Editor
	{
		struct SelectionHandleTypeEnum
		{
			enum SelectionHandleType
			{
				Move,
				Resize,
				Rotate
			};
		};

		class Selection;
		

		class SelectionHandlesOperation : public GraphicsEngine::RenderOperation
		{
		public:
			void Initialize() {}
			void Update(float) {}

			bool IsLocalSpace = false;
			bool ObjectsShareHandles = true;
			Enum::SelectionHandleType HandleType = Enum::SelectionHandleType::Move;

			std::weak_ptr<Selection> ActiveSelection;
			std::weak_ptr<GraphicsEngine::Camera> CurrentCamera;

			void Render();

			Instantiable;

			Inherits_Class(GraphicsEngine::RenderOperation);

			Reflected(SelectionHandlesOperation);
		};
	}
}

namespace Enum
{
	typedef Engine::Editor::SelectionHandleTypeEnum::SelectionHandleType SelectionHandleType;
}