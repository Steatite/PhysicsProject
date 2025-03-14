#pragma once
#include "Trackable.h"
#include <string>
#include "KeyboardEvent.h"

class GraphicsSystem;
class InputSystem;
class Player;
class Timer;
class GameListener;
class EventSystem;
class Vector2D;
class MouseEvent;
class AxisEvent;
class Mesh3D;
class GameObject3D;

const std::string ASSET_PATH = "Assets/";//GAMEASSETS; //MAKE THIS RELATIVE PATH
const std::string SMURF_FILENAME = "smurf_sprites.png";
const std::string PROJECTILE_FILENAME = "Sphere_Glow.png";
const std::string BACKGROUND_FILEPATH = "room/room0000.png";

const float PROJECTILE_SPEED = 100.0f;

class Game : public Trackable
{
public:
	friend class GameListener;

	static Game* getInstance();
	static void cleanupInstance();

	void init(int screenWidth, int screenHeight, int fps = 60, bool debugMode = false);
	void cleanup();

	void startGame();

private:
	Game();
	~Game();

	void getInput();
	void update();
	void render();

	void debug(); //Just a bunch of stuff to do in Debug Mode

	void DPlayerMove(Vector2D loc); //Functions that begin with the prefix D are debug functions
	void DMousePress(int);
	void DMouseRelease(int);

	void quitGame();

	void onMouseMove(Vector2D);
	void onClick(const MouseEvent&);
	void onAxis(const AxisEvent&);

	void onToggleDrawMode();
	void onToggleShaders();
	void onShaderHotReload();
	void onToggleDebugMode();
	void onTogglePause();

	void handleCameraMovement(KeyCode key);

	static Game* mspInstance;

	GraphicsSystem* mpGraphicsSystem;

	Player* mpPlayerUnit;

	Timer* mpGameTimer;

	GameListener* mpGameListener;

	double mDeltaTime;
	bool mDebugMode, mIsPlaying;

	float mTimePerFrame;

	Mesh3D* mpCube;
	Mesh3D* mpPlane;
	Mesh3D* mpSphere;
	Mesh3D* mpLine;
	float mTimeElapsed;

	GameObject3D* mBall1;
	GameObject3D* mBall2;
	
	bool mIsPaused;
};
