#pragma once

#include <optix_world.h>
#include <optix_d3d11_interop.h>

class App;

using namespace optix;
class OptixAPI
{
public:
	//OptixAPI(void);
	explicit OptixAPI(App*);
	~OptixAPI(void);

	void Init(ID3D11Device*);

	//void Launch(UINT, RTsize, RTsize, RTsize);

	Context GetContext();

private:

	Context optixContext;

	App* parentApp;
};

