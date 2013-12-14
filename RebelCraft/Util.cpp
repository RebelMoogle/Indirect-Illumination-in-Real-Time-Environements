#include "DXUT.h"
#include "Util.h"
#include "File.h"
#include "Debug.h"

// =======================================================================================================
std::string Util::ToStr( const std::wstring& string )
{
	// TODO: Unsafe conversion!
	std::string res = std::string(string.begin(), string.end());
	res.assign(string.begin(), string.end());
	return res;
}

// =======================================================================================================
std::wstring Util::ToWStr( const std::string& string )
{
	// TODO: Unsafe conversion!
	std::wstring res = std::wstring(string.begin(), string.end());
	res.assign(string.begin(), string.end());
	return res;
}
