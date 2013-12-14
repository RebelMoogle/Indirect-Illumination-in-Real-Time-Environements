#pragma once

//########################
// name BoxCitySceneBuilder class
//	shall build a city of boxes.
//	boxes will be axis aligned

#include "BaseSceneBuilder.h"

class RebelGeometry;
class App;

class BoxCitySceneBuilder : public BaseSceneBuilder
{
public:
	explicit BoxCitySceneBuilder(App*);

	~BoxCitySceneBuilder(void);

	// creates the scene elements
	bool Init();

};

