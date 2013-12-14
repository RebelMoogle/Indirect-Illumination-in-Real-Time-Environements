#pragma once

class App;
class Scene;

// Base class for scene builders.
// Since several scenes are analytic and others a stored in files, different scene builders are required.
// Derived class are responsible for creating and releasing of the scene only.
class BaseSceneBuilder
{
	public:

		// Base constructor.
		explicit BaseSceneBuilder(App*);

		// Destructor. Deletes the scene.
		virtual ~BaseSceneBuilder();

		// Creates the scene elements.
		virtual bool Init() = 0;

		// Gets the scene.
		Scene* GetScene();

		// Gets the parent window.
		App* GetApp();

	protected:

		// The scene.
		Scene* sceneToBuild;

	private:
		// Parent App.
		App* const parentApp;
		
};
