#pragma once
#include <exception>
#include <source_location>
#include <string>

class InternalEngineException : public std::exception
{
public:
	const char* Message = nullptr;
	std::source_location Caller;
	std::string ErrorMessage;

	InternalEngineException(const char* message, const std::source_location& caller = std::source_location::current()) throw();
	InternalEngineException(const std::string& message, const std::source_location& caller = std::source_location::current()) throw();

	virtual const char* what() throw()
	{
		return ErrorMessage.c_str();
	}
};

class EngineException : public InternalEngineException
{
public:
	EngineException(const char* message, const std::source_location& caller = std::source_location::current()) throw() : InternalEngineException(message, caller) {}
	EngineException(const std::string& message, const std::source_location& caller = std::source_location::current()) throw() : InternalEngineException(message, caller) {}
};

class GameException : public InternalEngineException
{
public:
	GameException(const char* message, const std::source_location& caller = std::source_location::current()) throw() : InternalEngineException(message, caller) {}
	GameException(const std::string& message, const std::source_location& caller = std::source_location::current()) throw() : InternalEngineException(message, caller) {}
};