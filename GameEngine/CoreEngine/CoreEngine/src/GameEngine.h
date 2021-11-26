#pragma once

#include <vector>
#include <map>

#include "Object.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace GraphicsEngine
{
	class GameWindow;
}

namespace Engine
{
	struct ProcessArguments
	{
		ProcessArguments() {}

		int GetArguments() const;
		std::string GetArgument(int index) const;
		bool HasArgumentFlag(const std::string& name) const;
		std::string GetArgumentValue(const std::string& name) const;

		void PushArgument(const std::string& argument);
		void InsertArgumentFlag(const std::string& name);
		void InsertArgument(const std::string& name, const std::string& value);

	private:
		typedef std::map<std::string, std::string> FlagMap;

		std::vector<std::string> Arguments;
		FlagMap ArgumentFlags;

		Base_Class;

		Reflected_Type(ProcessArguments);
	};

	Define_Value_Type(ProcessArguments);

	class GameEngine : public Object
	{
	public:
		virtual ~GameEngine() {}

		void Initialize() {}
		void Update(float delta) {}

		std::shared_ptr<GraphicsEngine::GameWindow> GetPrimaryWindow() const;
		void SetPrimaryWindow(const std::shared_ptr<GraphicsEngine::GameWindow>& primaryWindow);
		int GetArguments() const;
		std::string GetArgument(int index) const;
		bool HasArgumentFlag(const std::string& name) const;
		std::string GetArgumentValue(const std::string& name) const;

		int SpawnProcess(const std::string& scriptPath, const ProcessArguments& arguments = ProcessArguments()) const { return -1; }

		std::shared_ptr<GraphicsEngine::GameWindow> CreateWindow(const std::string& name, const std::string& title) const { return nullptr; }
		void CreateConsole(const std::string& name, const std::string& title) const { return; }

		void Configure(int argc, const char* const* argv);

	private:
		static const int StartArgument = 1;

		ProcessArguments Arguments;
		std::weak_ptr<GraphicsEngine::GameWindow> PrimaryWindow;

		Instantiable;

		Inherits_Class(Object);

		Reflected(GameEngine);
	};
}