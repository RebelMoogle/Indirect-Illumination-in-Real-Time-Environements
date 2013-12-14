#pragma once

//########################
// name BoxCitySceneBuilder class
//	shall build a city of boxes.
//	boxes will be axis aligned

#include "BaseSceneBuilder.h"

class RebelGeometry;
class App;

class DynamicTest : public BaseSceneBuilder
{
public:
	explicit DynamicTest(App*);

	~DynamicTest(void);

	// creates the scene elements
	bool Init();

};

