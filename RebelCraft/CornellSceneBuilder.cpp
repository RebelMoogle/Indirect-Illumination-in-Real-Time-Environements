#include "DXUT.h"
#include "CornellSceneBuilder.h"
#include "ParallelogramGeometry.h"
#include "SphereGeometry.h"
#include "BoxGeometry.h"
#include "App.h"
#include "DiffuseMaterial.h"
#include "OmniMaterial.h"
#include "OptixViewerCamera.h"
#include "SpotLight.h"
#include "Scene.h"

using namespace optix;

// ================================================================================
CornellSceneBuilder::CornellSceneBuilder(App* givenApp):
BaseSceneBuilder(givenApp)
{

}

// ================================================================================
CornellSceneBuilder::~CornellSceneBuilder()
{

}

// ================================================================================
bool CornellSceneBuilder::Init()
{
	sceneToBuild = new Scene(GetApp());
	
	// Floor (white)
	RebelGeometry* wall_floor = new ParallelogramGeometry(make_float3( 0.0f, 0.0f, 0.0f ),
														make_float3( 0.0f, 0.0f, 559.2f ),
														make_float3( 556.0f, 0.0f, 0.0f ) );
	// Ceiling (white)
	RebelGeometry* wall_ceil = new ParallelogramGeometry(	make_float3( 0.0f, 548.8f, 0.0f ),
														make_float3( 556.0f, 0.0f, 0.0f ),
														make_float3( 0.0f, 0.0f, 559.2f ) );
	// Back (white)
	RebelGeometry* wall_back = new ParallelogramGeometry(	make_float3( 0.0f, 0.0f, 559.2f),
														make_float3( 0.0f, 548.8f, 0.0f),
														make_float3( 556.0f, 0.0f, 0.0f) );
	// Right (green)
	RebelGeometry* wall_right = new ParallelogramGeometry(make_float3( 0.0f, 0.0f, 0.0f ),
														make_float3( 0.0f, 548.8f, 0.0f ),
														make_float3( 0.0f, 0.0f, 559.2f) );
	// Left (red)
	RebelGeometry* wall_left = new ParallelogramGeometry(	make_float3( 556.0f, 0.0f, 0.0f ),
														make_float3( 0.0f, 0.0f, 559.2f ),
														make_float3( 0.0f, 548.8f, 0.0f ) );

	// Short block (white)
	float short_trans_x = -50;
	float short_trans_z = 250;
	RebelGeometry* short0 = new ParallelogramGeometry(make_float3( 130.0f + short_trans_x, 165.0f, 65.0f + short_trans_z),
													make_float3( -48.0f, 0.0f, 160.0f),
													make_float3( 160.0f, 0.0f, 49.0f) );
	RebelGeometry* short1 = new ParallelogramGeometry(make_float3( 290.0f + short_trans_x, 0.0f, 114.0f + short_trans_z),
													make_float3( 0.0f, 165.0f, 0.0f),
													make_float3( -50.0f, 0.0f, 158.0f) );
	RebelGeometry* short2 = new ParallelogramGeometry(make_float3( 130.0f + short_trans_x, 0.0f, 65.0f + short_trans_z),
													make_float3( 0.0f, 165.0f, 0.0f),
													make_float3( 160.0f, 0.0f, 49.0f) );
	RebelGeometry* short3 = new ParallelogramGeometry(make_float3( 82.0f + short_trans_x, 0.0f, 225.0f + short_trans_z),
													make_float3( 0.0f, 165.0f, 0.0f),
													make_float3( 48.0f, 0.0f, -160.0f) );
	RebelGeometry* short4 = new ParallelogramGeometry(make_float3( 240.0f + short_trans_x, 0.0f, 272.0f + short_trans_z),
													make_float3( 0.0f, 165.0f, 0.0f),
													make_float3( -158.0f, 0.0f, -47.0f) );

	// Tall block (white)
	RebelGeometry* tall0 = new ParallelogramGeometry(	make_float3( 423.0f, 330.0f, 247.0f),
													make_float3( -158.0f, 0.0f, 49.0f),
													make_float3( 49.0f, 0.0f, 159.0f) );
	RebelGeometry* tall1 = new ParallelogramGeometry(	make_float3( 423.0f, 0.0f, 247.0f),
													make_float3( 0.0f, 330.0f, 0.0f),
													make_float3( 49.0f, 0.0f, 159.0f) );
	RebelGeometry* tall2 = new ParallelogramGeometry(	make_float3( 472.0f, 0.0f, 406.0f),
													make_float3( 0.0f, 330.0f, 0.0f),
													make_float3( -158.0f, 0.0f, 50.0f) );
	RebelGeometry* tall3 = new ParallelogramGeometry(	make_float3( 314.0f, 0.0f, 456.0f),
													make_float3( 0.0f, 330.0f, 0.0f),
													make_float3( -49.0f, 0.0f, -160.0f) );
	RebelGeometry* tall4 = new ParallelogramGeometry(	make_float3( 265.0f, 0.0f, 296.0f),
													make_float3( 0.0f, 330.0f, 0.0f),
													make_float3( 158.0f, 0.0f, -49.0f) );

	// Tall block (white)
	RebelGeometry* blocker0 = new ParallelogramGeometry(	make_float3( 0.0f, 300.0f, 359.2f),
														make_float3( 0.0f, 0.0f, 200.0f),
														make_float3( 200.0f, 0.0f, 0.0f ) );
	RebelGeometry* blocker1 = new ParallelogramGeometry(	make_float3( 0.0f, 270.0f, 559.2f),
														make_float3( 0.0f, 0.0f, -200.0f),
														make_float3( 200.0f, 0.0f, 0.0f ) );
	RebelGeometry* blocker2 = new ParallelogramGeometry(	make_float3( 0.0f, 270.0f, 359.2f),
														make_float3( 0.0f, 30.0f, 0.0f),
														make_float3( 200.0f, 0.0f, 0.0f ) );
	RebelGeometry* blocker3 = new ParallelogramGeometry(	make_float3( 200.0f, 300.0f, 359.2f),
														make_float3( 0.0f, 0.0f, 200.0f),
														make_float3( 0.0f, -30.0f, 0.0f ) );

	// Light (black)
	RebelGeometry* light = new ParallelogramGeometry(	make_float3( 343.0f, 548.7f, 227.0f),
													make_float3( 0.0f, 0.0f, 105.0f),
													make_float3( -130.0f, 0.0f, 0.0f) );

	SphereGeometry* sphere = new SphereGeometry( make_float3(300, 100, 200), 60 );
	SphereGeometry* sphere2 = new SphereGeometry( make_float3(450, 490, 400), 50 );

	//BoxGeometry* smallBox = new BoxGeometry(D3DXVECTOR3(20, 0, 250), D3DXVECTOR3(100, 100, 100));
	
	sceneToBuild->AddGeometry(wall_floor);
	sceneToBuild->AddGeometry(wall_ceil);
	sceneToBuild->AddGeometry(wall_back);
	sceneToBuild->AddGeometry(wall_right);
	sceneToBuild->AddGeometry(wall_left);

	//sceneToBuild->AddGeometry(smallBox);

	sceneToBuild->AddGeometry(short0);
	sceneToBuild->AddGeometry(short1);
	sceneToBuild->AddGeometry(short2);
	sceneToBuild->AddGeometry(short3);
	sceneToBuild->AddGeometry(short4);
	sceneToBuild->AddGeometry(tall0);
	sceneToBuild->AddGeometry(tall1);
	sceneToBuild->AddGeometry(tall2);
	sceneToBuild->AddGeometry(tall3);
	sceneToBuild->AddGeometry(tall4);
	sceneToBuild->AddGeometry(blocker0);
	sceneToBuild->AddGeometry(blocker1);
	sceneToBuild->AddGeometry(blocker2);
	sceneToBuild->AddGeometry(blocker3);
	sceneToBuild->AddGeometry(light);
	sceneToBuild->AddGeometry(sphere);
	sceneToBuild->AddGeometry(sphere2);
//	sceneToBuild->AddGeometry(lightSphere);
	
	const float3 white = make_float3( 0.8f, 0.8f, 0.8f );
	const float3 green = make_float3( 0.15f, 0.8f, 0.15f );
	const float3 red   = make_float3( 0.8f, 0.1f, 0.16f );
	const float3 blue  = make_float3( 0.15f, 0.15f, 0.8f );
	const float3 black = make_float3( 0.01f, 0.01f, 0.01f );
	const float3 zeros = make_float3(0.0f, 0.0f, 0.0f);
	//const float3 light = make_float3( 15.0f, 15.0f, 5.0f );

	OmniMaterial* matWhite =		new OmniMaterial(white, zeros, zeros, 1.0f);
	OmniMaterial* matWhiteSpec =	new OmniMaterial(white, make_float3( 0.3f, 0.3f, 0.3f ), zeros, 1.0f, 50.f);
	OmniMaterial* matGreen =		new OmniMaterial(green, zeros, zeros, 1.0f);
	OmniMaterial* matRed =			new OmniMaterial(red,	zeros, zeros, 1.0f);
	OmniMaterial* matBlack =		new OmniMaterial(black, zeros, zeros, 1.0f);
	OmniMaterial* matBlue =			new OmniMaterial(blue,	zeros, zeros, 1.0f);
	OmniMaterial* matTrans =		new OmniMaterial(zeros, zeros, white, RefractIdx_Glass);
	

	sceneToBuild->AddMaterial(matWhite);
	sceneToBuild->AddMaterial(matGreen);
	sceneToBuild->AddMaterial(matRed);
	sceneToBuild->AddMaterial(matBlue);
	sceneToBuild->AddMaterial(matBlack);
	sceneToBuild->AddMaterial(matWhiteSpec);
	sceneToBuild->AddMaterial(matTrans);
	
	sceneToBuild->CreateGeometryInstance(wall_floor, matWhite);
	sceneToBuild->CreateGeometryInstance(wall_ceil, matWhite);
	sceneToBuild->CreateGeometryInstance(wall_back, matBlue);
	sceneToBuild->CreateGeometryInstance(wall_right, matGreen);
	sceneToBuild->CreateGeometryInstance(wall_left, matRed);

	//sceneToBuild->CreateGeometryInstance(smallBox, matWhite);

	sceneToBuild->CreateGeometryInstance(short0, matWhite);
	sceneToBuild->CreateGeometryInstance(short1, matWhite);
	sceneToBuild->CreateGeometryInstance(short2, matWhite);
	sceneToBuild->CreateGeometryInstance(short3, matWhite);
	sceneToBuild->CreateGeometryInstance(short4, matWhite);
	sceneToBuild->CreateGeometryInstance(tall0, matWhite);
	sceneToBuild->CreateGeometryInstance(tall1, matWhite);
	sceneToBuild->CreateGeometryInstance(tall2, matWhite);
	sceneToBuild->CreateGeometryInstance(tall3, matWhite);
	sceneToBuild->CreateGeometryInstance(tall4, matWhite);
	sceneToBuild->CreateGeometryInstance(blocker0, matWhite);
	sceneToBuild->CreateGeometryInstance(blocker1, matWhite);
	sceneToBuild->CreateGeometryInstance(blocker2, matWhite);
	sceneToBuild->CreateGeometryInstance(blocker3, matWhite);
	sceneToBuild->CreateGeometryInstance(light, matWhite); // matBlack
	sceneToBuild->CreateGeometryInstance(sphere, matWhiteSpec);
	sceneToBuild->CreateGeometryInstance(sphere2, matWhiteSpec); //matWhiteSpec
	//sceneToBuild->CreateGeometryInstance(lightSphere, matBlue);
	
	OptixViewerCamera* sceneCamera = new OptixViewerCamera();
	sceneCamera->SetRotateButtons(true, false, false);
	sceneCamera->SetDrag(true);
	sceneCamera->SetEnableYAxisMovement(true);
	sceneCamera->SetEnablePositionMovement(true);
	sceneCamera->SetScalers(0.01f, 100);

	//WARNING: may only work in 80%
	sceneCamera->SetViewParams(&D3DXVECTOR3(278.0f, 273.0f, -800.0f ), &D3DXVECTOR3(278.0f, 273.0f, 0.0f));
	//sceneCamera->SetViewParams(&D3DXVECTOR3(200.0f, 800.0f, 200.0f ), &D3DXVECTOR3(200.0f, -90.0f, 210.0f));

		//(	make_float3( 278.0f, 273.0f, -800.0f ), // eye
		//										make_float3( 278.0f, 273.0f, 0.0f ),    // lookat
		//										make_float3( 0.0f, 1.0f,  0.0f ),       // up
		//										35.0f, 35.0f);	// Hfov, Vfov
	sceneToBuild->SetCamera(sceneCamera);

	SunLight* sun = new SunLight();
	sun->SetPosition(D3DXVECTOR3(300.0f, 700.0f, 300.0f));
	sun->SetPower(D3DXVECTOR4(1,1,1,1));
	//sceneToBuild->SetSunLight(sun);

	// Create spotlights.
	SpotLight* lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(400.0f, 500.7f, 227.0f));
	lightSource->SetDirection(D3DXVECTOR3(-0.2,-0.7, 0.1));
	lightSource->SetDistance(800.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,1,1,1));
	lightSource->SetPower(D3DXVECTOR4(1,1,1,1));

	sceneToBuild->AddSpotLight(lightSource);

	lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(200.0f, 520.7f, 200.0f));
	lightSource->SetDirection(D3DXVECTOR3(-0.2,-0.7, 0.5));
	lightSource->SetDistance(800.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,0.81f,0.41f,1));
	lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	sceneToBuild->AddSpotLight(lightSource);

	
	
	return true;
}