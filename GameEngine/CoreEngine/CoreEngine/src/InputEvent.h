#pragma once

extern "C" {
#include <SDL.h>
}
#include <vector>

#include "TaskScheduler.h"
#include "InputEnums.h"
#include "Vector3.h"

class InputHandler;

class InputObject
{
public:
	Event<InputObject*> Began;
	Event<InputObject*> Changed;
	Event<InputObject*> Ended;
	Event<InputObject*> Refresh;

	bool GetState() const;
	Enum::InputState GetStateEnum(Enum::InputCode code) const;
	bool GetStateChanged() const;
	const Vector3& GetPosition() const;
	const Vector3& GetDelta() const;
	Enum::InputType GetType() const;
	Enum::InputCode GetCode() const;
	std::string GetName() const;
	Enum::BoundDevice GetDevice() const;

	friend class InputHandler;

private:
	bool IsDown = false;
	bool StateChanged = false;
	Vector3 Position;
	Vector3 Delta;
	Enum::InputType Type;
	Enum::InputCode Code;
	Enum::BoundDevice Device;
	const char* Name = nullptr;
};

class MouseInput
{
public:
	class Button
	{
	public:
		bool IsDown = false;

		Event<InputObject*> Down;
		Event<InputObject*> Up;
	};

	class Wheel : public Button
	{
	public:
		Event<InputObject*> Moved;
	};

	Button Left;
	Button Right;
	Wheel Wheel;
	Event<InputObject*> Moved;
};

class InputHandler
{
public:
	bool GetState(Enum::InputCode code) const;
	bool GetStateChanged(Enum::InputCode code) const;
	Enum::InputState GetStateEnum(Enum::InputCode code) const;
	const Vector3& GetPosition(Enum::InputCode code) const;
	const Vector3& GetDelta(Enum::InputCode code) const;
	Enum::InputType GetType(Enum::InputCode code) const;
	const char* GetName(Enum::InputCode code) const;
	InputObject& GetInput(Enum::InputCode code);
	const InputObject& GetInput(Enum::InputCode code) const;
	Enum::BoundDevice GetDevice(Enum::InputCode code) const;

	Event<InputObject*> InputBegan;
	Event<InputObject*> InputChanged;
	Event<InputObject*> InputEnded;

	class EventHandler
	{
	public:
		void ProcessEvent(SDL_Event& event);
		void FlushQueue();
		void ResetDeltas();

		friend class InputHandler;
	private:
		InputHandler* Target;
	};

	InputHandler(EventHandler& eventHandler);

	friend class EventHandler;

private:
	struct InputEvent
	{
		Enum::InputCode Code;
		Enum::InputState State;
	};

	typedef std::vector<InputEvent> InputVector;

	InputVector EventQueue;
	InputObject Inputs[Enum::InputCode::Codes];

	void ProcessKey(int keyCode, bool state);
	void ProcessMouse(int mouseCode, bool state);
	void ProcessEvent(SDL_Event& event);
	void SetInputState(InputObject& input, bool state);
	void FlushQueue();
	void ResetDeltas();
};
