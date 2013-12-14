#include "DXUT.h"
#include "OptixAPI.h"
#include "OptixTypes.h"
#include "App.h"
//
using namespace optix;
OptixAPI::OptixAPI(App* givenApp)/*:
optixContext(NULL)*/
{
	parentApp = givenApp;
}


OptixAPI::~OptixAPI(void)
{
	optixContext->destroy();
}


void OptixAPI::Init(ID3D11Device* d3dDevice)
{
	optixContext = Context::create();
	optixContext->setD3D11Device(d3dDevice);
	optixContext->validate();

	optixContext->setRayTypeCount(RAY_TYPE_COUNT);
	optixContext->setEntryPointCount(ENTRY_TYPE_COUNT);

	// Final validate and compile.
	optixContext->validate();	
	optixContext->compile();

	optixContext["scene_epsilon"]->setFloat(0.01f);

	return;
}

optix::Context OptixAPI::GetContext()
{
	return optixContext;
}

//void OptixAPI::Launch( UINT entryIndex, RTsize width, RTsize height, RTsize depth)
//{
//	optixContext->launch(entryIndex, width, height, depth);
//}



