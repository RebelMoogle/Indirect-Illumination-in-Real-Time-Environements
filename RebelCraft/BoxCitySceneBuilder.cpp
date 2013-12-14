#include "DXUT.h"
#include "BoxCitySceneBuilder.h"
#include "BoxGeometry.h"
#include "SphereGeometry.h"
#include "App.h"
#include "OmniMaterial.h"
#include "OptixViewerCamera.h"
#include "SpotLight.h"
#include "Scene.h"
#include "random.h"

BoxCitySceneBuilder::BoxCitySceneBuilder(App* givenApp):
BaseSceneBuilder(givenApp)
{
}


BoxCitySceneBuilder::~BoxCitySceneBuilder(void)
{
}
bool BoxCitySceneBuilder::Init()
{
	sceneToBuild = new Scene(GetApp());

	// materials
	const optix::float3 white = optix::make_float3(0.7f, 0.7f, 0.7f);
	const optix::float3 green = optix::make_float3( 0.15f, 0.5f, 0.15f );
	const optix::float3 red   = optix::make_float3( 0.8f, 0.1f, 0.06f );
	const optix::float3 blue  = optix::make_float3( 0.15f, 0.15f, 0.5f );
	const optix::float3 black = optix::make_float3( 0.1f, 0.1f, 0.1f );
	const optix::float3 zeros = optix::make_float3(0.0f, 0.0f, 0.0f);

	OmniMaterial* matWhite =		new OmniMaterial(white, zeros, zeros, 1.0f);
	OmniMaterial* matWhiteSpec =	new OmniMaterial(white, optix::make_float3(0.1f, 0.1f, 0.1f), zeros, 1.0f, 25.0f);
	OmniMaterial* matBlackSpec =	new OmniMaterial(optix::make_float3( 0.15f, 0.4f, 0.25f ), optix::make_float3(0.01f, 0.1f, 0.05f), zeros, 1.0f, 50.0f);
	OmniMaterial* matGreen =		new OmniMaterial(green, zeros, zeros, 1.0f);
	OmniMaterial* matRed =			new OmniMaterial(red, zeros, zeros, 1.0f);
	OmniMaterial* matBlack =		new OmniMaterial(black, zeros, zeros, 1.0f);
	OmniMaterial* matRedSpec =		new OmniMaterial(red, optix::make_float3(0.1f, 0.01f, 0.02f), zeros, 1.0f, 25.0f);
	OmniMaterial* matBlue =			new OmniMaterial(blue, zeros, zeros, 1.0f);
	OmniMaterial* matYellow =		new OmniMaterial(red + green, zeros, zeros, 1.0f);
	OmniMaterial* matPurple =		new OmniMaterial(optix::make_float3( 0.35f, 0.1f, 0.5f ), zeros, zeros, 1.0f);
	OmniMaterial* matBlah =			new OmniMaterial(blue + green, zeros, zeros, 1.0f);
	OmniMaterial* matTrans =		new OmniMaterial(zeros, zeros, optix::make_float3(1.0f, 1.0f, 1.0f), RefractIdx_Glass, 100.0f);

	sceneToBuild->AddMaterial(matWhite);
	sceneToBuild->AddMaterial(matGreen);
	sceneToBuild->AddMaterial(matRed);
	sceneToBuild->AddMaterial(matBlue);
	sceneToBuild->AddMaterial(matBlack);
	sceneToBuild->AddMaterial(matBlackSpec);
	sceneToBuild->AddMaterial(matWhiteSpec);
	sceneToBuild->AddMaterial(matRedSpec);
	sceneToBuild->AddMaterial(matTrans);
	sceneToBuild->AddMaterial(matYellow);
	sceneToBuild->AddMaterial(matPurple);
	sceneToBuild->AddMaterial(matBlah);

	//geometry
	
	float cityWidth = 1100.0f;
	float cityDepth = 1100.0f;
	optix::uint2& seed = optix::make_uint2(42, 237);//rand(); //cityWidth;x


	// some sphere-action
	SphereGeometry* sphereSpec = new SphereGeometry( optix::make_float3(cityWidth/2, 200, cityDepth/2 - 300), 60 );
	sceneToBuild->AddGeometry(sphereSpec);

	sceneToBuild->CreateGeometryInstance(sphereSpec, matBlackSpec);

	SphereGeometry* sphereTrans = new SphereGeometry( optix::make_float3(cityWidth/2, 100, cityDepth/2 + 200), 40 );
	sceneToBuild->AddGeometry(sphereTrans);

	sceneToBuild->CreateGeometryInstance(sphereTrans, matRedSpec);

	SphereGeometry* sphere = new SphereGeometry( optix::make_float3(cityWidth/2 - 400, 120, cityDepth/2 + 250), 60 );
	sceneToBuild->AddGeometry(sphere);

	sceneToBuild->CreateGeometryInstance(sphere, matPurple);

	// floor
	BoxGeometry* cityGround = new BoxGeometry(D3DXVECTOR3(0, -10, 0), D3DXVECTOR3(cityWidth, 10, cityDepth));
	sceneToBuild->AddGeometry(cityGround);

	sceneToBuild->CreateGeometryInstance(cityGround, matWhite);

	float minHeight = 10.0f;
	float maxHeight = 200.0f;
	float minWidth = 10.0f;
	float maxWidth = 100.0f;
	float minDepth = 10.0f;
	float maxDepth = 100.0f;
	float streetMaxWidth = 50.0f;
	float maxAlleyWidth = 10.0f;
	// buildings
	//	- additions to buildings?

	// cornellBox room
	// ground at 128
	// height / width : (16) max
	// walls 

	BoxGeometry* cornellFloor = new BoxGeometry(D3DXVECTOR3(-1050.0f, 135.0f, cityDepth/2 - 100), D3DXVECTOR3(50.0f, 5.0f, 50.0f));
	sceneToBuild->AddGeometry(cornellFloor);
	sceneToBuild->CreateGeometryInstance(cornellFloor, matWhite);

	BoxGeometry* cornellLeftWall = new BoxGeometry(D3DXVECTOR3(-1050.0f, 140.0f, cityDepth/2 - 105), D3DXVECTOR3(50.0f, 50.0f, 5.0f));
	sceneToBuild->AddGeometry(cornellLeftWall);
	sceneToBuild->CreateGeometryInstance(cornellLeftWall, matGreen);

	BoxGeometry* cornellRightWall = new BoxGeometry(D3DXVECTOR3(-1050.0f, 140.0f, cityDepth/2 - 50), D3DXVECTOR3(50.0f, 50.0f, 5.0f));
	sceneToBuild->AddGeometry(cornellRightWall);
	sceneToBuild->CreateGeometryInstance(cornellRightWall, matRed);

	BoxGeometry* cornellRoof = new BoxGeometry(D3DXVECTOR3(-1050.0f, 190.0f, cityDepth/2 - 100), D3DXVECTOR3(50.0f, 5.0f, 50.0f));
	sceneToBuild->AddGeometry(cornellRoof);
	sceneToBuild->CreateGeometryInstance(cornellRoof, matWhite);

	BoxGeometry* cornellBack = new BoxGeometry(D3DXVECTOR3(-1055.0f, 140.0f, cityDepth/2 - 100), D3DXVECTOR3(5.0f, 50.0f, 50.0f));
	sceneToBuild->AddGeometry(cornellBack);
	sceneToBuild->CreateGeometryInstance(cornellBack, matWhite);

	BoxGeometry* Box1 = new BoxGeometry(D3DXVECTOR3(-1040.0f, 140.0f, cityDepth/2 - 70), D3DXVECTOR3(20.0f, 20.0f, 20.0f));
	sceneToBuild->AddGeometry(Box1);
	sceneToBuild->CreateGeometryInstance(Box1, matWhite);

	BoxGeometry* Box2 = new BoxGeometry(D3DXVECTOR3(-1020.0f, 140.0f, cityDepth/2 - 95), D3DXVECTOR3(20.0f, 40.0f, 20.0f));
	sceneToBuild->AddGeometry(Box2);
	sceneToBuild->CreateGeometryInstance(Box2, matWhite);

	// corridor 
	// length = 1024
	// height / width : (16)
	// put objects and spotlights on the walls
	BoxGeometry* corridorFloor = new BoxGeometry(D3DXVECTOR3(-1000.0f, 120.0f, cityDepth/2 -150), D3DXVECTOR3(1000.0f, 20.0f, 200.0f));
	sceneToBuild->AddGeometry(corridorFloor);
	sceneToBuild->CreateGeometryInstance(corridorFloor, matWhite);

	BoxGeometry* corridorLeftWall = new BoxGeometry(D3DXVECTOR3(-1000.0f, 140.0f, cityDepth/2 - 150), D3DXVECTOR3(900.0f, 50.0f, 50.0f));
	sceneToBuild->AddGeometry(corridorLeftWall);
	sceneToBuild->CreateGeometryInstance(corridorLeftWall, matBlue);

	BoxGeometry* corridorRightWall = new BoxGeometry(D3DXVECTOR3(-1000.0f, 140.0f, cityDepth/2 - 50), D3DXVECTOR3(900.0f, 50.0f, 50.0f));
	sceneToBuild->AddGeometry(corridorRightWall);
	sceneToBuild->CreateGeometryInstance(corridorRightWall, matBlue);

	BoxGeometry* corridorRoof = new BoxGeometry(D3DXVECTOR3(-1000.0f, 190.0f, cityDepth/2 -150), D3DXVECTOR3(900.0f, 20.0f, 200.0f));
	sceneToBuild->AddGeometry(corridorRoof);
	sceneToBuild->CreateGeometryInstance(corridorRoof, matGreen);

	//// DYNAMIC TEST BOX
	//BoxGeometry* DynBoxTest = new BoxGeometry(D3DXVECTOR3(cityWidth/2, 100.0f, cityDepth/2 + 50.0f), D3DXVECTOR3(20.0f, 20.0f, 20.0f), false);
	//sceneToBuild->AddGeometry(DynBoxTest);
	//sceneToBuild->CreateGeometryInstance(DynBoxTest, matRed);


	// CITAY	
	// loop ?
	float currentTotalRowWidth = 0; // add currentRowWidth when done With row
	float currentRowWidth = 0; // add difference of newWidth - oldWidth if newWidth > oldWidth
	float currentRowDepth = 0; // always add newDepth, until new row Started
	float streetWidth = 0;
	BoxGeometry* cityBuilding = new BoxGeometry(D3DXVECTOR3(cityWidth/2, 800.0f, cityDepth/2), D3DXVECTOR3(10, 10, 10));
	OmniMaterial* currentMaterial = matRed;
	
	////sunlight position test
	//sceneToBuild->AddGeometry(cityBuilding);
	//sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);

	do 
	{
		// ## create random numbers
		optix::float2 randomNumbers = rnd_from_uint2(seed);
		float boxWidth = optix::lerp(minWidth, maxWidth, randomNumbers.x);
		float boxDepth = optix::lerp(minWidth, maxWidth, randomNumbers.y);
		
		randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
		float boxHeight = optix::lerp(minHeight, maxHeight, randomNumbers.x);

		// ## new box geometry
		//TODO: cut off buildings at edge of city: boxWidth - (currentTotalWidth + boxWidth - cityWidth)
		float offsetWidth = std::max(0.0f, currentRowWidth  - boxWidth);
		float offsetDepth = randomNumbers.x * maxAlleyWidth;
		cityBuilding = new BoxGeometry(	D3DXVECTOR3(currentTotalRowWidth + offsetWidth / 2, 0, currentRowDepth + offsetDepth / 2), 
									D3DXVECTOR3(boxWidth, boxHeight, boxDepth));
		// ## add geometry
		sceneToBuild->AddGeometry(cityBuilding);

		//choose material
		switch ((INT)floor(randomNumbers.y * 10.0f))
		{
		case 0: 
			currentMaterial = matWhite;
			break;
		case 1: 
			currentMaterial = matWhite;
			break;
		case 2: 
			currentMaterial = matWhite;
			break;
		case 3: 
			currentMaterial = matWhite;
			break;
		case 4: 
			currentMaterial = matWhite;
			break;
		case 5: 
			currentMaterial = matBlue;
			break;
		default: 
			currentMaterial = matWhite;
			break;
		}

		sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);

		//create further geometry instances in the offsets
		if(offsetWidth/2 >= 1.0f)
		{
			//create some geometry on the sides with maximum width offsetwidth
			// LEFT SIDE
			randomNumbers = rnd_from_uint2(seed);
			float extWidth = optix::lerp(0.5f, offsetWidth/2, randomNumbers.x);
			float extDepth = optix::lerp(1.0f, boxDepth, randomNumbers.y);

			randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
			float extHeight = optix::lerp(1.0f, boxHeight, randomNumbers.x);

			if(extWidth > 10.0f)
				extHeight = std::min(10.0f, extHeight);
			else
				extWidth = std::min(10.0f, extWidth);


			// ## new box geometry
			//TODO: cut off buildings at edge of city: boxWidth - (currentTotalWidth + boxWidth - cityWidth)
			float offsetHeight = randomNumbers.y * (boxHeight  - extHeight);
			randomNumbers = rnd_from_uint2(seed);
			float offsetExtDepth = randomNumbers.x * (boxDepth - extDepth);
			cityBuilding = new BoxGeometry(	D3DXVECTOR3(	currentTotalRowWidth + offsetWidth/2 - extWidth, 
															offsetHeight, 
															currentRowDepth +  offsetDepth/2 + offsetExtDepth), 
				D3DXVECTOR3(extWidth, extHeight, extDepth));
			// ## add geometry
			sceneToBuild->AddGeometry(cityBuilding);

			//choose material
			switch ((INT)floor(randomNumbers.y * 10.0f))
			{
			case 0: 
				currentMaterial = matWhite;
				break;
			case 1: 
				currentMaterial = matGreen;
				break;
			case 2: 
				currentMaterial = matWhite;
				break;
			case 3: 
				currentMaterial = matBlue;
				break;
			case 4: 
				currentMaterial = matWhite;
				break;
			case 5: 
				currentMaterial = matRed;
				break;
			default: 
				currentMaterial = matBlah;
				break;
			}
			sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);

			// RIGHT SIDE
			randomNumbers = rnd_from_uint2(seed);
			extWidth = optix::lerp(0.5f, offsetWidth/2, randomNumbers.x);
			extDepth = optix::lerp(1.0f, boxDepth, randomNumbers.y);

			randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
			extHeight = optix::lerp(1.0f, boxHeight, randomNumbers.x);

			if(extWidth > 10.0f)
				extHeight = std::min(10.0f, extHeight);
			else
				extWidth = std::min(10.0f, extWidth);
			
			// ## new box geometry
			//TODO: cut off buildings at edge of city: boxWidth - (currentTotalWidth + boxWidth - cityWidth)
			offsetHeight = randomNumbers.y * (boxHeight  - extHeight);
			randomNumbers = rnd_from_uint2(seed);
			offsetExtDepth = randomNumbers.x * (boxDepth - extDepth);
			cityBuilding = new BoxGeometry(	D3DXVECTOR3(	currentTotalRowWidth + boxWidth + offsetWidth/2, 
															offsetHeight, 
															currentRowDepth +  offsetDepth/2 + offsetExtDepth), 
											D3DXVECTOR3(extWidth, extHeight, extDepth));
			// ## add geometry
			sceneToBuild->AddGeometry(cityBuilding);

			//choose material
			switch ((INT)floor(randomNumbers.y * 10.0f))
			{
			case 0: 
				currentMaterial = matWhite;
				break;
			case 1: 
				currentMaterial = matGreen;
				break;
			case 2: 
				currentMaterial = matWhite;
				break;
			case 3: 
				currentMaterial = matBlue;
				break;
			case 4: 
				currentMaterial = matWhite;
				break;
			case 5: 
				currentMaterial = matRed;
				break;
			default: 
				currentMaterial = matBlah;
				break;
			}
			sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);

		}

		if(offsetDepth/2 >= 1.0f)
		{
			// create some geometry on the near and far side with max width offsetDepth / 2
			// FRONT SIDE
			randomNumbers = rnd_from_uint2(seed);
			float extWidth = optix::lerp(0.5f, boxWidth, randomNumbers.x);
			float extDepth = optix::lerp(1.0f, offsetDepth/2, randomNumbers.y);

			randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
			float extHeight = optix::lerp(1.0f, boxHeight, randomNumbers.x);

			if(extDepth > 10.0f)
				extHeight = std::min(10.0f, extHeight);
			else
				extDepth = std::min(10.0f, extDepth);


			// ## new box geometry
			float offsetHeight = randomNumbers.y * (boxHeight  - extHeight);
			randomNumbers = rnd_from_uint2(seed);
			float offsetExtWidth = randomNumbers.x * (boxWidth - extWidth);
			cityBuilding = new BoxGeometry(	D3DXVECTOR3( currentTotalRowWidth + offsetWidth/2 + offsetExtWidth, 
				offsetHeight, 
				currentRowDepth +  offsetDepth/2 - extDepth), 
				D3DXVECTOR3(extWidth, extHeight, extDepth));
			// ## add geometry
			sceneToBuild->AddGeometry(cityBuilding);

			//choose material
			switch ((INT)floor(randomNumbers.y * 10.0f))
			{
			case 0: 
				currentMaterial = matWhite;
				break;
			case 1: 
				currentMaterial = matGreen;
				break;
			case 2: 
				currentMaterial = matWhite;
				break;
			case 3: 
				currentMaterial = matBlue;
				break;
			case 4: 
				currentMaterial = matWhite;
				break;
			case 5: 
				currentMaterial = matRed;
				break;
			default: 
				currentMaterial = matBlah;
				break;
			}
			sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);

			// BACK SIDE
			randomNumbers = rnd_from_uint2(seed);
			extWidth = optix::lerp(0.5f, boxWidth, randomNumbers.x);
			extDepth = optix::lerp(1.0f, offsetDepth/2, randomNumbers.y);

			randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
			extHeight = optix::lerp(1.0f, boxHeight, randomNumbers.x);

			if(extDepth > 10.0f)
				extHeight = std::min(10.0f, extHeight);
			else
				extDepth = std::min(10.0f, extDepth);

			// ## new box geometry
			//TODO: cut off buildings at edge of city: boxWidth - (currentTotalWidth + boxWidth - cityWidth)
			offsetHeight = randomNumbers.y * (boxHeight  - extHeight);
			randomNumbers = rnd_from_uint2(seed);
			offsetExtWidth = randomNumbers.x * (boxWidth - extWidth);
			cityBuilding = new BoxGeometry(	D3DXVECTOR3(	currentTotalRowWidth + offsetExtWidth + offsetWidth/2, 
				offsetHeight, 
				currentRowDepth +  offsetDepth/2 + boxDepth), 
				D3DXVECTOR3(extWidth, extHeight, extDepth));
			// ## add geometry
			sceneToBuild->AddGeometry(cityBuilding);

			//choose material
			switch ((INT)floor(randomNumbers.y * 10.0f))
			{
			case 0: 
				currentMaterial = matWhite;
				break;
			case 1: 
				currentMaterial = matGreen;
				break;
			case 2: 
				currentMaterial = matWhite;
				break;
			case 3: 
				currentMaterial = matBlue;
				break;
			case 4: 
				currentMaterial = matWhite;
				break;
			case 5: 
				currentMaterial = matRed;
				break;
			default: 
				currentMaterial = matBlah;
				break;
			}
			sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);
		}

		// TOP EXTENSION create some geometry on top
		// create some geometry on the near and far side with max width offsetDepth / 2
		// FRONT SIDE
		randomNumbers = rnd_from_uint2(seed);
		float extWidth = optix::lerp(0.5f, boxWidth, randomNumbers.x);
		float extDepth = optix::lerp(1.0f, boxDepth, randomNumbers.y);

		randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
		float extHeight = optix::lerp(1.0f, 10.0f, randomNumbers.x);

		// ## new box geometry
		randomNumbers = rnd_from_uint2(seed);
		float offsetExtWidth = randomNumbers.x * (boxWidth - extWidth);
		float offsetExtDepth = randomNumbers.x * (boxDepth - extDepth);
		cityBuilding = new BoxGeometry(	D3DXVECTOR3( currentTotalRowWidth + offsetWidth/2 + offsetExtWidth, 
			boxHeight, 
			currentRowDepth +  offsetDepth/2 + offsetExtDepth), 
			D3DXVECTOR3(extWidth, extHeight, extDepth));
		// ## add geometry
		sceneToBuild->AddGeometry(cityBuilding);

		//choose material
		switch ((INT)floor(randomNumbers.y * 10.0f))
		{
		case 0: 
			currentMaterial = matWhite;
			break;
		case 1: 
			currentMaterial = matGreen;
			break;
		case 2: 
			currentMaterial = matWhite;
			break;
		case 3: 
			currentMaterial = matBlue;
			break;
		case 4: 
			currentMaterial = matWhite;
			break;
		case 5: 
			currentMaterial = matRed;
			break;
		default: 
			currentMaterial = matBlah;
			break;
		}
		sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);


		float curStreetWidth = randomNumbers.y * streetMaxWidth;
		//if(randomNumbers.x > 0.5f)
		//{
		//	// FLYING BOXCARS
		//	randomNumbers = rnd_from_uint2(seed);
		//	extWidth = optix::lerp(3.0f, 5.0f, randomNumbers.x);
		//	extDepth = optix::lerp(5.0f, 20.0f, randomNumbers.y);

		//	randomNumbers = rnd_from_uint2(seed); //randomNumbers.x, randomNumbers.y));
		//	extHeight = optix::lerp(3.0f, 5.0f, randomNumbers.x);

		//	// ## new box geometry
		//	//TODO: cut off buildings at edge of city: boxWidth - (currentTotalWidth + boxWidth - cityWidth)
		//	float offsetHeight = maxHeight /2;//randomNumbers.y * (boxHeight);
		//	randomNumbers = rnd_from_uint2(seed);
		//	offsetExtDepth = randomNumbers.x * (boxDepth - extDepth);
		//	cityBuilding = new BoxGeometry(	D3DXVECTOR3(	currentTotalRowWidth + boxWidth + offsetWidth + curStreetWidth/2 - extWidth/2, 
		//		offsetHeight, 
		//		currentRowDepth +  offsetDepth/2), 
		//		D3DXVECTOR3(extWidth, extHeight, extDepth), false);
		//	// ## add geometry
		//	sceneToBuild->AddGeometry(cityBuilding);

		//	//choose material
		//	switch ((INT)floor(randomNumbers.y * 10.0f))
		//	{
		//	case 0: 
		//		currentMaterial = matYellow;
		//		break;
		//	case 1: 
		//		currentMaterial = matGreen;
		//		break;
		//	case 2: 
		//		currentMaterial = matBlack;
		//		break;
		//	case 3: 
		//		currentMaterial = matBlue;
		//		break;
		//	case 4: 
		//		currentMaterial = matWhite;
		//		break;
		//	case 5: 
		//		currentMaterial = matPurple;
		//		break;
		//	default: 
		//		currentMaterial = matBlah;
		//		break;
		//	}
		//	sceneToBuild->CreateGeometryInstance(cityBuilding, currentMaterial);
		//}
		

		currentRowWidth = optix::max(currentRowWidth, boxWidth);
		currentRowDepth += boxDepth + offsetDepth;
		if(currentRowDepth + maxDepth >= cityDepth)
		{
			currentTotalRowWidth += currentRowWidth + curStreetWidth;
			currentRowWidth = 0;
			currentRowDepth = 0;
		}
	} while (currentTotalRowWidth + maxWidth < cityWidth);
	



	// create camera
	OptixViewerCamera* sceneCamera = new OptixViewerCamera();
	sceneCamera->SetRotateButtons(true, false, false);
	sceneCamera->SetDrag(true);
	sceneCamera->SetEnableYAxisMovement(true);
	sceneCamera->SetEnablePositionMovement(true);
	sceneCamera->SetScalers(0.01f, 200.0f);

	//WARNING: may only work in 80%
	sceneCamera->SetViewParams(&D3DXVECTOR3(-280.46f, 489.7f, 518.66f), &D3DXVECTOR3(-279.6f, 489.15f, 518.65f));
	//sceneCamera->SetViewParams(&D3DXVECTOR3(172.098f, 131.11f, 309.34f), &D3DXVECTOR3(171.9f, 130.79f, 310.27));
	//sceneCamera->SetViewParams(&D3DXVECTOR3(450.0f, 300.0f, -200.0f), &D3DXVECTOR3(650.0f, 0.0f, cityDepth/2));
	//sceneCamera->SetViewParams(&D3DXVECTOR3(200.0f, 800.0f, 200.0f ), &D3DXVECTOR3(200.0f, -90.0f, 210.0f));

	//(	make_float3( 278.0f, 273.0f, -800.0f ), // eye
	//										make_float3( 278.0f, 273.0f, 0.0f ),    // lookat
	//										make_float3( 0.0f, 1.0f,  0.0f ),       // up
	//										35.0f, 35.0f);	// Hfov, Vfov
	sceneToBuild->SetCamera(sceneCamera);

	SunLight* sun = new SunLight();

	// set new position to still view the whole area with the new direction
	D3DXVECTOR3 position = D3DXVECTOR3(cityWidth/2 - 200.0f, 800.0f, cityDepth/2 - 200.0f);
	// breaks if camera is pointing straight down (no GBUFFER?)

	sun->SetPosition(position);
	sun->SetPower(D3DXVECTOR4(1,1,1,1));
	sun->SetWidth(cityWidth);
	sun->SetHeight(maxHeight);
	sun->SetDepth(cityDepth);
	sun->ResetCamera();
	sceneToBuild->SetSunLight(sun);

	// Create spotlights.
	SpotLight* lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(-1025.0f, 188.0f, cityDepth/2 - 75));
	lightSource->SetDirection(D3DXVECTOR3(0.0,-0.9, 0.1));
	lightSource->SetDistance(50.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,1,1,1));
	lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	sceneToBuild->AddSpotLight(lightSource);

	lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(-900.0f, 188.0f, cityDepth/2 - 55));
	lightSource->SetDirection(D3DXVECTOR3(0.0, -0.9, 0.01));
	lightSource->SetDistance(50.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,1,1,1));
	lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	sceneToBuild->AddSpotLight(lightSource);

	lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(-700.0f, 188.0f, cityDepth/2 - 55));
	lightSource->SetDirection(D3DXVECTOR3(0.0, -0.9, 0.01));
	lightSource->SetDistance(50.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,1,1,1));
	lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	sceneToBuild->AddSpotLight(lightSource);

	lightSource = new SpotLight();
	lightSource->SetPosition(D3DXVECTOR3(-500.0f, 188.0f, cityDepth/2 - 55));
	lightSource->SetDirection(D3DXVECTOR3(0.0, -0.9, 0.01));
	lightSource->SetDistance(50.0f);
	lightSource->SetAngle(D3DX_PI/4);
	lightSource->SetDiffuseColor(D3DXVECTOR4(1,1,1,1));
	lightSource->SetPower(D3DXVECTOR4(1,0.81f,0.41f,1));

	sceneToBuild->AddSpotLight(lightSource);


	return true;
}