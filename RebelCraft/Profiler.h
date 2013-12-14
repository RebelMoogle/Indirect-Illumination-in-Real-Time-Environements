//=================================================================================================
//
//	Query Profiling Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "DXUT.h"
#include <map>

class App;

class Profiler
{

public:

    static Profiler GlobalProfiler;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* immContext);

    void StartProfile(const std::wstring& name);
    void EndProfile(const std::wstring& name);

    void EndFrame(App*);

protected:
	struct ProfileData
	{
		ID3D11Query* DisjointQuery[5];
		ID3D11Query* TimestampStartQuery[5];
		ID3D11Query* TimestampEndQuery[5];
		BOOL QueryStarted;
		BOOL QueryFinished;

		ProfileData() : QueryStarted(FALSE), QueryFinished(FALSE) 
		{
			ZeroMemory(DisjointQuery, sizeof(ID3D11Query) * 5);
			ZeroMemory(TimestampStartQuery, sizeof(ID3D11Query) * 5);
			ZeroMemory(TimestampEndQuery, sizeof(ID3D11Query) * 5);
		}
	};

    typedef std::map<std::wstring, ProfileData> ProfileMap;

    ProfileMap profiles;
    UINT64 currFrame;

    ID3D11Device* device;
    ID3D11DeviceContext* context;
};

class ProfileBlock
{
public:

	ProfileBlock(const std::wstring& name);
	~ProfileBlock();

protected:

	std::wstring name;
};
