//--------------------------------------------------------------------------------------
// File: RebelCraft.cpp
//
// Empty starting point for new Direct3D 9 and/or Direct3D 11 applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "App.h"
#include "StaticCamera.h"
//#include "ShaderDefines.h"
//#include "Shader.h"
//#include "ShaderStructures.h"
#include <sstream>
#include <intsafe.h>



////-------------------------------------------------------------------------------------
//// enumerations
////-------------------------------------------------------------------------------------
//enum SCENE_SELECTION 
//{
//	POWER_PLANT,
//	SPONZA,
//};
//
//enum 
//{
//	UI_TOGGLEFULLSCREEN,
//	UI_FACENORMALS,
//	UI_SELECTEDSCENE,
//	UI_LIGHTS,
//	UI_CAMERALIGHT,
//};
//
//enum 
//{
//	HUD_GENERIC = 0,
//	HUD_PARTITIONS,
//	HUD_FILTERING,
//	HUD_EXPERT,
//	HUD_NUM,
//};

//-------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------
App* gApp = 0;


//D3DXMATRIXA16 gWorld;
//ID3D11ShaderResourceView* gSkybox = 0;

////DXUT GUI
//CDXUTDialogResourceManager gDialogMgr;
//CD3DSettingsDlg gSettings;
//CDXUTDialog gHUD[HUD_NUM];
//CDXUTCheckBox* gAnimateLight = 0;
//CDXUTComboBox* gSceneSelect = 0;
//CDXUTComboBox* gLightCamSelect = 0;
//CDXUTSlider* gLights = 0;
//CDXUTTextHelper* gText = 0;

bool gDisplayUI = false;

// Any UI state passed directly to rendering shaders
//UIConstants gUIConstants;

//-------------------------------------------------------------------------------------
// Forward Declarations
//-------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void CALLBACK OnGUIEvent(UINT eventID, INT controlID, CDXUTControl* control, void* userContext);
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

//void LoadSkybox(LPCWSTR fileName);

void DestroyApp();
//void InitScene(ID3D11Device* d3dDevice);
//void DestroyScene();

void InitUI();

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									   DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{

	static bool firstTime = true;
	if (firstTime)
	{
		firstTime = false;
		if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
		}
	}

	pDeviceSettings->d3d11.sd.SampleDesc.Count = 1;
	pDeviceSettings->d3d11.sd.SampleDesc.Quality = 0;

	pDeviceSettings->d3d11.AutoCreateDepthStencil = true;

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									  void* pUserContext )
{
	if (gApp->D3DCreateDevice(pd3dDevice, pBackBufferSurfaceDesc))
		return S_OK;
	else return S_FALSE;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										  const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	if (gApp->D3DCreateSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc))
		return S_OK;
	else return S_FALSE;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	gApp->Move(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								  double fTime, float fElapsedTime, void* pUserContext )
{
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(DXUTGetDXGIBackBufferSurfaceDesc()->Width);
	viewport.Height = static_cast<float>(DXUTGetDXGIBackBufferSurfaceDesc()->Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	
	gApp->Render(pd3dDevice, pd3dImmediateContext, &viewport, fElapsedTime);

	

#ifdef _DEBUG
		//draw debug stuff
		
#endif _DEBUG

	//}
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	if (gApp)
		gApp->D3DReleaseSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{	
	if (gApp)
		gApp->D3DReleaseDevice();

	DXUTGetGlobalResourceCache().OnDestroyDevice();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						  bool* pbNoFurtherProcessing, void* pUserContext )
{

	if(gApp)
		gApp->HandleMessages(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{


}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
					   bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
					   int xPos, int yPos, void* pUserContext )
{
}

////--------------------------------------------------------------------------------------
//// Handle GUI Events
////--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT eventID, INT controlID, CDXUTControl* control, void* userContext )
{
	//switch( controlID )
	//{
	//case IDC_TOGGLEFULLSCREEN:
	//	DXUTToggleFullScreen(); break;
	//case IDC_TOGGLEREF:
	//	DXUTToggleREF(); break;
	//case IDC_CHANGEDEVICE:
	//	g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;
	//}

}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	DXUTSetIsInGammaCorrectMode(true);
	
	// Perform any application-level initialization here

	gApp = new App();

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTSetHotkeyHandling(true, true, false);
	DXUTCreateWindow( L"RebelCraft", hInstance );

	// require 11-level hardware
//	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, false, 1920, 1200 );
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1024, 768 );

	DXUTMainLoop(); // Enter into the DXUT render loop

	// Perform any application-level cleanup here
	SAFE_DELETE(gApp);

	return DXUTGetExitCode();
}
