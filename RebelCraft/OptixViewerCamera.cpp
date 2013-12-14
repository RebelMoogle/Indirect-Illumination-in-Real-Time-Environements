#include "DXUT.h"
#include "Util.h"
#include "OptixViewerCamera.h"
#include "ContentLoader.h"
#include "OptixTypes.h"


OptixViewerCamera::OptixViewerCamera(void)
{

	cameraCBuffer = new ConstantBuffer<CameraConstants>();
}


OptixViewerCamera::~OptixViewerCamera(void)
{
}

void OptixViewerCamera::FrameMove( FLOAT fElapsedTime )
{
	CFirstPersonCamera::FrameMove(fElapsedTime);
	UpdateReferenceFrame();
}

void OptixViewerCamera::UpdateReferenceFrame()
{
	optix::float3 lookAt =  Util::Float3FromD3DXVECTOR3(GetLookAtPt());
	optix::float3 eye	=	Util::Float3FromD3DXVECTOR3(GetEyePt());
	
	optix::float3 upAxis	= optix::normalize(Util::Float3FromD3DXVECTOR3(GetWorldUp()));
	wAxis = lookAt - eye;

	float wLength = optix::length(wAxis);

	uAxis = optix::normalize(optix::cross(wAxis, upAxis));
	vAxis = optix::normalize(optix::cross(uAxis, wAxis));

	float horizontalFOV = m_fFOV;
	float aspectRatio = m_fAspect;
	float verticalFOV = horizontalFOV / aspectRatio;
	uAxis = -uAxis * (wLength * tanf(horizontalFOV*0.5f));
	vAxis = -vAxis * (wLength * tanf(verticalFOV*0.5f));

	if (rayGenerationProgram)
	{
		rayGenerationProgram["U"]->setFloat(uAxis);
		rayGenerationProgram["V"]->setFloat(vAxis);
		rayGenerationProgram["W"]->setFloat(wAxis);
		rayGenerationProgram["eye"]->setFloat(eye);
	}	

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	cameraCBuffer->Data.World	= identityMatrix;
	cameraCBuffer->Data.refractiveIndexETA = 1.0f;
	cameraCBuffer->Data.View	= *GetViewMatrix();
	cameraCBuffer->Data.cameraPosition = *GetEyePt();
	cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*GetProjMatrix());
}

bool OptixViewerCamera::OptixCreateDevice( optix::Context& Context )
{
	if(!ContentLoader::LoadOptixProgramFromFile("CUDA\\OptixViewerCamera.cu.ptx", "optix_viewer_camera", Context, &rayGenerationProgram)) return false;
	
	Context->setRayGenerationProgram(EntryType_OptixViewerCamera, rayGenerationProgram);	
	rayGenerationProgram["U"]->setFloat(uAxis);
	rayGenerationProgram["V"]->setFloat(vAxis);
	rayGenerationProgram["W"]->setFloat(wAxis);
	rayGenerationProgram["eye"]->setFloat(Util::Float3FromD3DXVECTOR3(CFirstPersonCamera::GetEyePt()));
	rayGenerationProgram["ray_type"]->setUint((UINT)RayType_DirectDiffuse);
	return true;
}

void OptixViewerCamera::OptixReleaseDevice()
{
	if (rayGenerationProgram)
		rayGenerationProgram->destroy();
}

bool OptixViewerCamera::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	return cameraCBuffer->D3DCreateDevice(Device,BackBufferSurfaceDesc);
}

void OptixViewerCamera::D3DReleaseDevice()
{
	cameraCBuffer->D3DReleaseDevice();
}


