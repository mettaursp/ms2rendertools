#pragma once

#include "Object.h"
#include "Window.h"

namespace Engine
{
	class UserInput;
}

namespace GraphicsEngine
{
	class GameWindow : public Engine::Object
	{
	public:
		void Initialize() {}
		void Update(float) {}

		void Configure(Window* window);

		std::shared_ptr<Engine::UserInput> GetInput() const;
		int GetRefreshRate();
		Vector3 GetResolution();
		void SetMousePosition(const Vector3& position);

	private:
		Window* ActiveWindow = nullptr;
		std::weak_ptr<Engine::UserInput> WindowInput;

		Instantiable;

		Inherits_Class(Object);

		Reflected(GameWindow);
	};
}
