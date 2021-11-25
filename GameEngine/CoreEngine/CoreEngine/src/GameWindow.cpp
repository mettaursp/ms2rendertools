#include "GameWindow.h"

#include "LuaInput.h"
#include "EngineException.h"

namespace GraphicsEngine
{
	std::shared_ptr<Engine::UserInput> GameWindow::GetInput() const
	{
		return WindowInput.lock();
	}

	void GameWindow::Configure(Window* window)
	{
		ActiveWindow = window;

		std::shared_ptr<Engine::UserInput> userInput = Engine::Create<Engine::UserInput>();

		userInput->Configure(window->Input);
		userInput->SetParent(This.lock());

		WindowInput = userInput;
	}

	int GameWindow::GetRefreshRate()
	{
		if (ActiveWindow == nullptr)
			throw GameException("GameWindow.GetRefreshRate: Attempt to use unconfigured GameWindow.");

		return ActiveWindow->RefreshRate;
	}

	Vector3 GameWindow::GetResolution()
	{
		if (ActiveWindow == nullptr)
			throw GameException("GameWindow.GetRefreshRate: Attempt to use unconfigured GameWindow.");

		return ActiveWindow->Resolution;
	}

	void GameWindow::SetMousePosition(const Vector3& position)
	{
		if (ActiveWindow == nullptr)
			throw GameException("GameWindow.GetRefreshRate: Attempt to use unconfigured GameWindow.");

		ActiveWindow->SetMousePosition(position);
	}
}