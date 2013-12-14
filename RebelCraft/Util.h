#ifndef __RES_UTIL__
#define __RES_UTIL__

#include <sstream>

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef DEG2RAD
#define DEG2RAD( a ) ( a * D3DX_PI / 180.0f )
#endif


class Util
{
	public:

	template <typename T>
	static std::string ToStr ( T Number )
	{
		std::stringstream ss;
		ss << Number;
		return ss.str();
	}

	template <typename T>
	static T ToNumber ( const std::string &Text )
	{
		std::stringstream ss(Text);
		T result;
		return ss >> result ? result : 0;
	}

	// Converts most value types to wstring.
	template <class T> 
	static std::wstring ToWStr( const T &t )
	{
		std::wostringstream oss;
		oss << t;
		return std::wstring (oss.str());
	}

	// converts wstring to string.
	static std::string ToStr( const std::wstring& string );
	// converts string to wstring.
	static std::wstring ToWStr( const std::string& string );

	static inline optix::float3 Float3FromD3DXVECTOR3(const D3DXVECTOR3* givenVector)
	{
		return optix::make_float3( givenVector->x, givenVector->y, givenVector->z );
	}

};

#endif