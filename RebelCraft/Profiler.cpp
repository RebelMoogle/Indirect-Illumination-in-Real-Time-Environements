//=================================================================================================
//
//	Query Profiling Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================
#include "DXUT.h"
#include "Profiler.h"
#include "App.h"


using std::wstring;
using std::map;

// == Profiler ====================================================================================

Profiler Profiler::GlobalProfiler;

void Profiler::Initialize(ID3D11Device* device, ID3D11DeviceContext* immContext)
{
    this->device = device;
    this->context = immContext;
	currFrame = 0;
}

void Profiler::StartProfile(const wstring& name)
{
    //if(PROFILING)
    //    return;

    ProfileData* profileData = &profiles[name];
	//profileData.

    _ASSERT(profileData->QueryStarted == FALSE);
    _ASSERT(profileData->QueryFinished == FALSE);
    
    if(profileData->DisjointQuery[currFrame] == NULL)
    {
        // Create the queries
        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;
        HRESULT hr = device->CreateQuery(&desc, &profileData->DisjointQuery[currFrame]);
		if(FAILED(hr))
			DXUT_ERR_MSGBOX(L" disjoint ts query not created", hr);

        desc.Query = D3D11_QUERY_TIMESTAMP;
        hr = device->CreateQuery(&desc, &profileData->TimestampStartQuery[currFrame]);
		if(FAILED(hr))
			DXUT_ERR_MSGBOX(L"timestamp query not created", hr);

        hr = device->CreateQuery(&desc, &profileData->TimestampEndQuery[currFrame]);
		if(FAILED(hr))
			DXUT_ERR_MSGBOX(L"timestamp2 query not created", hr);
    }

    // Start a disjoint query first
    context->Begin(profileData->DisjointQuery[currFrame]);

    // Insert the start timestamp    
    context->End(profileData->TimestampStartQuery[currFrame]);

    profileData->QueryStarted = TRUE;
}

void Profiler::EndProfile(const wstring& name)
{
   /* if(EnableProfiling::Value == EnableProfiling::Disabled)
        return;*/

    ProfileData& profileData = profiles[name];
    _ASSERT(profileData.QueryStarted == TRUE);
    _ASSERT(profileData.QueryFinished == FALSE);

    // Insert the end timestamp    
    context->End(profileData.TimestampEndQuery[currFrame]);

    // End the disjoint query
    context->End(profileData.DisjointQuery[currFrame]);

    profileData.QueryStarted = FALSE;
    profileData.QueryFinished = TRUE;
}

void Profiler::EndFrame(App* parentApp)
{
    //if(EnableProfiling::Value == EnableProfiling::Disabled)
    //    return;

    currFrame = (currFrame + 1) % 5;    

    //float queryTime = 0.0f;

    // Iterate over all of the profiles
    ProfileMap::iterator iter;
    for(iter = profiles.begin(); iter != profiles.end(); iter++)
    {
        ProfileData& profile = (*iter).second;
        if(profile.QueryFinished == FALSE)
            continue;

        profile.QueryFinished = FALSE;

        if(profile.DisjointQuery[currFrame] == NULL)
            continue;

        //timer.Update();

        // Get the query data
        UINT64 startTime = 0;
        while(context->GetData(profile.TimestampStartQuery[currFrame], &startTime, sizeof(startTime), 0) != S_OK);

        UINT64 endTime = 0;
        while(context->GetData(profile.TimestampEndQuery[currFrame], &endTime, sizeof(endTime), 0) != S_OK);

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        while(context->GetData(profile.DisjointQuery[currFrame], &disjointData, sizeof(disjointData), 0) != S_OK);

        //timer.Update();
        //queryTime += timer.DeltaMillisecondsF();

        float time = 0.0f;
        if(disjointData.Disjoint == FALSE)
        {
            UINT64 delta = endTime - startTime;
            float frequency = static_cast<float>(disjointData.Frequency);
            time = (delta / frequency) * 1000.0f;
        }        

        //wstring output = (*iter).first + L": " + ToString(time) + L"ms";
        //spriteRenderer.RenderText(spriteFont, output.c_str(), transform, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		parentApp->textToRender << "(GPU), " << (*iter).first << ": " << time << "ms \n";

        //transform._42 += 25.0f;
    }

    //transform._42 += 25.0f;    

    //wstring output = L"Time spent waiting for queries: " + ToString(queryTime) + L"ms";
    //spriteRenderer.RenderText(spriteFont, output.c_str(), transform, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
}


// == ProfileBlock ================================================================================

ProfileBlock::ProfileBlock(const std::wstring& name) : name(name)
{
	Profiler::GlobalProfiler.StartProfile(name);
}

ProfileBlock::~ProfileBlock()
{
	Profiler::GlobalProfiler.EndProfile(name);
}