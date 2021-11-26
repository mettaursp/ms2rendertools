#include "EngineException.h"

#include <sstream>

InternalEngineException::InternalEngineException(const char* message, const std::source_location& caller) throw()
{
	Caller = caller;

	std::stringstream error;

	error << Caller.file_name() << " [" << Caller.line() << "]: " << Caller.function_name() << ": " << Message;

	ErrorMessage = error.str();
}
InternalEngineException::InternalEngineException(const std::string& message, const std::source_location& caller) throw()
{
	Caller = caller;

	std::stringstream error;

	error << Caller.file_name() << " [" << Caller.line() << "]: " << Caller.function_name() << ": " << Message;

	ErrorMessage = error.str();
	Message = ErrorMessage.c_str();
}