#ifndef __CORNELL_SCENE_BUILDER__
#define __CORNELL_SCENE_BUILDER__

#include "BaseSceneBuilder.h"

class RebelGeometry;
class App;

// Creates a scene containing the cornell box.
class CornellSceneBuilder : public BaseSceneBuilder
{
	public:

		// Base constructor.
		explicit CornellSceneBuilder(App*);

		// Destructor. Deletes the scene.
		~CornellSceneBuilder();

		// Creates the scene elements.
		bool Init();

};

#endif