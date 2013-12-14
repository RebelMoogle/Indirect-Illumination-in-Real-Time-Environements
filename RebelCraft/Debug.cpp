#include "DXUT.h"
#include "Debug.h"
#include "Util.h"
#include <sstream>

#ifdef WIN32
#include <Windows.h>
#endif

void Debug::OutputString(const char* String)
{
#ifdef WIN32
	OutputDebugStringA(String);
#else
	TODO: Text auf Konsole ausgeben.
#endif
}


// =======================================================================================================
void Debug::PrintInfo(const std::string& Source, const std::string& Information)
{
#ifdef _DEBUG
	Print(Source, "Info", Information);
#endif
}

// =======================================================================================================
void Debug::PrintWarning(const std::string& Source, const std::string& Warning)
{
#ifdef _DEBUG
	Print(Source, "Warning", Warning);
#endif
}

// =======================================================================================================
void Debug::PrintError(const std::string& Source, const std::string& Error, long ErrorCode)
{
#ifdef _DEBUG
	if (ErrorCode != 0)
		Print(Source, "Error", Error + " (hr = " + Util::ToStr(ErrorCode) + ")");
	else Print(Source, "Error", Error);
#endif
}

// =======================================================================================================
void Debug::Print(const std::string& Source, const std::string& Category, const std::string& Message)
{	
#ifdef _DEBUG
	std::stringstream s;
	s << "[" << Category << "] " << Source << " -> " << Message << std::endl;	
	
	OutputString(s.str().c_str());
#endif
}
