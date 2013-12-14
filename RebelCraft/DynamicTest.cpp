#include "DXUT.h"
#include "DynamicTest.h"
#include "BoxGeometry.h"
#include "App.h"
#include "OmniMaterial.h"
#include "OptixViewerCamera.h"
#include "SpotLight.h"
#include "Scene.h"
#include "random.h"

DynamicTest::DynamicTest(App* givenApp):
BaseSceneBuilder(givenApp)
{
}


DynamicTest::~DynamicTest(void)
{
}
bool DynamicTest::Init()
{
	sceneToBuild = new Scene(GetApp());

	// materials
	const optix::float3 white = optix::make_float3(0.7f, 0.7f, 0.7f);
	const optix::float3 green = optix::make_float3( 0.15f, 0.5f, 0.15f );
	const optix::float3 red   = optix::make_float3( 0.8f, 0.1f, 0.06f );
	const optix::float3 blue  = optix::make_float3( 0.15f, 0.15f, 0.5f );
	const optix::float3 black = optix::make_float3( 0.01f, 0.01f, 0.01f );
	const optix::float3 zeros = optix::make_float3(0.0f, 0.0f, 0.0f);

	OmniMaterial* matWhite = new OmniMaterial(white, zeros, zeros, 1.0f);
	OmniMaterial* matWhiteSpec = new OmniMaterial(white, white, zeros, 1.0f);
	OmniMaterial* matGreen = new OmniMaterial(green, zeros, zeros, 1.0f);
	OmniMaterial* matRed = new OmniMaterial(red, zeros, zeros, 1.0f);
	OmniMaterial* matBlack = new OmniMaterial(black, white, zeros, 1.0f);
	OmniMaterial* matBlue = new OmniMaterial(blue, blue, zeros, 1.0f);
	OmniMaterial* matYellow = new OmniMaterial(red + green, zeros, zeros, 1.0f);
	OmniMaterial* matPurple = new OmniMaterial(red + blue, zeros, zeros, 1.0f);
	OmniMaterial* matBlah = new OmniMaterial(blue + green, zeros, zeros, 1.0f);
	OmniMaterial* matTrans = new OmniMaterial(zeros, zeros, white, RefractIdx_Glass);

	sceneToBuild->AddMaterial(matWhite);
	sceneToBuild->AddMaterial(matGreen);
	sceneToBuild->AddMaterial(matRed);
	sceneToBuild->AddMaterial(matBlue);
	sceneToBuild->AddMaterial(matBlack);
	sceneToBuild->AddMaterial(matWhiteSpec);
	sceneToBuild->AddMaterial(matTrans);
	sceneToBuild->AddMaterial(matYellow);
	sceneToBuild->AddMaterial(matPurple);
	sceneToBuild->AddMaterial(matBlah);

	//geometry

	float cityWidth = 400.0f;
	float cityDepth = 400.0f;
	optix::uint2& seed = optix::make_uint2(42, 237);//rand(); //cityWidth;x

	// floor
	BoxGeometry* testGround = new BoxGeometry(D3DXVECTOR3(0, -10, 0), D3DXVECTOR3(cityWidth, 10, cityDepth));
	sceneToBuild->AddGeometry(testGround);

	sceneToBuild->CreateGeometryInstance(testGround, matWhite);

	BoxGeometry* testWall = new BoxGeometry(D3DXVECTOR3(cityWidth/2 - 40.0f, 0, 0), D3DXVECTOR3(10.0f, 50.0f, cityDepth));
	sceneToBuild->AddGeometry(testWall);
	sceneToBuild->CreateGeometryInstance(testWall, matWhite);

	BoxGeometry* testWall2 = new BoxGeometry(D3DXVECTOR3(cityWidth/2 + 40.0f, 0, 0), D3DXVECTOR3(10.0f, 50.0f, cityDepth));
	sceneToBuild->AddGeometry(testWall2);
	sceneToBuild->CreateGeometryInstance(testWall2, matWhite);

	// buildings
	//	- additions to buildings?


	// FLYING BOXCARS
	float extWidth = 20.0f;
	float extDepth = 20.0f;
	float extHeight = 20.0f;

	// ## new box geometry
	//TODO: cut off buildings at edge of city: boxWidth - (currentTotalWidth + boxWidth - cityWidth)
	float offsetHeight = 20.0f;//randomNumbers.y * (boxHeight);
	float offsetDepth = cityDepth/2;
	float offsetWidth = cityDepth/2;
	BoxGeometry* movingBox = new BoxGeometry(	D3DXVECTOR3(offsetWidth, offsetHeight, offsetDepth), 
		D3DXVECTOR3(extWidth, extHeight, extDepth), false);
	// ## add geometry
	sceneToBuild->AddGeometry(movingBox);
	sceneToBuild->CreateGeometryInstance(movingBox, matRed);

	//movingBox = new BoxGeometry(	D3DXVECTOR3(offsetWidth, offsetHeight, offsetDepth  - 100.0f), 
	//	D3DXVECTOR3(extWidth, extHeight, extDepth), true);
	//// ## add geometry
	//sceneToBuild->AddGeometry(movingBox);
	//sceneToBuild->CreateGeometryInstance(movingBox, matGreen);

	//movingBox = new BoxGeometry(	D3DXVECTOR3(offsetWidth, offsetHeight, offsetDepth  + 100.0f), 
	//	D3DXVECTOR3(extWidth, extHeight, extDepth), true);
	//// ## add geometry
	//sceneToBuild->AddGeometry(movingBox);
	//sceneToBuild->CreateGeometryInstance(movingBox, matGreen);
	

	// create camera
	OptixViewerCamera* sceneCamera = new OptixViewerCamera();
	sceneCamera->SetRotateButtons(true, false, false);
	sceneCamera->SetDrag(true);
	sceneCamera->SetEnableYAxisMovement(true);
	sceneCamera->SetEnablePositionMovement(true);
	sceneCamera->SetScalers(0.01f, 200.0f);

	//WARNING: may only work in 80%
	sceneCamera->SetViewParams(&D3DXVECTOR3(cityWidth/2, 160.0f, 0), &D3DXVECTOR3(cityWidth/2, 0.0f, cityDepth/2));
	//sceneCamera->SetViewParams(&D3DXVECTOR3(200.0f, 800.0f, 200.0f ), &D3DXVECTOR3(200.0f, -90.0f, 210.0f));

	//(	make_float3( 278.0f, 273.0f, -800.0f ), // eye
	//										make_float3( 278.0f, 273.0f, 0.0f ),    // lookat
	//										make_float3( 0.0f, 1.0f,  0.0f ),       // up
	//										35.0f, 35.0f);	// Hfov, Vfov
	sceneToBuild->SetCamera(sceneCamera);

	SunLight* sun = new SunLight();

	// set new position to still view the whole area with the new direction
	D3DXVECTOR3 position = D3DXVECTOR3(cityWidth/2 + 100.0f, 500.0f, cityWidth/2);
	// breaks if camera is pointing straight down (no GBUFFER?)

	sun->SetPosition(position);
	sun->SetPower(D3DXVECTOR4(1,1,1,1));
	sun->SetWidth(cityWidth + 10.0f);
	sun->SetHeight(500.0f);
	sun->SetDepth(cityDepth + 10.0f);
	sun->ResetCamera();
	sceneToBuild->SetSunLight(sun);

	// Create spotlights.
	SpotLight* lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(cityWidth*2, 188.0f, cityDepth/2));
	lightSource->SetDirection(D3DXVECTOR3(0.0,-0.9, 0.1));
	lightSource->SetDistance(50.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,1,1,1));
	lightSource->SetPower(D3DXVECTOR4(0.0,0.0,0.0,1));

	sceneToBuild->AddSpotLight(lightSource);
	//lightSource = new SpotLight();
	//lightSource->SetPosition(D3DXVECTOR3(200.0f, 820.7f, 400.0f));
	//lightSource->SetDirection(D3DXVECTOR3(-0.2,-0.7, 0.5));
	//lightSource->SetDistance(800.0f);
	//lightSource->SetAngle(D3DX_PI/4);
	//lightSource->SetDiffuseColor(D3DXVECTOR4(1,0.81f,0.41f,1));
	//lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	//sceneToBuild->AddSpotLight(lightSource);

	//lightSource = new SpotLight();
	//lightSource->SetPosition(D3DXVECTOR3(400.0f, 800.7f, 300.0f));
	//lightSource->SetDirection(D3DXVECTOR3(-0.2,-0.7, 0.1));
	//lightSource->SetDistance(800.0f);
	//lightSource->SetAngle(D3DX_PI/4);
	//lightSource->SetDiffuseColor(D3DXVECTOR4(1,0.81f,0.41f,1));
	//lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	//sceneToBuild->AddSpotLight(lightSource);

	//lightSource = new SpotLight();
	//lightSource->SetPosition(D3DXVECTOR3(1000.0f, 800.7f, 600.0f));
	//lightSource->SetDirection(D3DXVECTOR3(-0.3,-0.8, 0.1));
	//lightSource->SetDistance(800.0f);
	//lightSource->SetAngle(D3DX_PI/4);
	//lightSource->SetDiffuseColor(D3DXVECTOR4(0.81f,0.81f,0.41f,1));
	//lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	//sceneToBuild->AddSpotLight(lightSource);



	return true;
}