#ifndef __RES_DEBUG__
#define __RES_DEBUG__

#include <string>

class Debug
{
	public:

		// Redirects string to debugger.
		static void OutputString(const char* String);

		// Prints an information.
		static void PrintInfo(const std::string& Source, const std::string& Information);
		// Prints a warning.
		static void PrintWarning(const std::string& Source, const std::string& Warning);
		// Prints an error.
		static void PrintError(const std::string& Source, const std::string& Error, long ErrorCode = 0L);
		// Prints a message.
		static void Print(const std::string& Source, const std::string& Category, const std::string& Message);

};

#endif