#include "DXUT.h"
#include "BaseSceneBuilder.h"
#include "Scene.h"

// ================================================================================
BaseSceneBuilder::BaseSceneBuilder(App* givenApp) :
sceneToBuild(NULL),
parentApp(givenApp)
{

}

// ================================================================================
BaseSceneBuilder::~BaseSceneBuilder()
{
	SAFE_DELETE(sceneToBuild);
}

// ================================================================================
Scene* BaseSceneBuilder::GetScene()
{
	return sceneToBuild;
}

// ================================================================================
App* BaseSceneBuilder::GetApp()
{
	return parentApp;
}
