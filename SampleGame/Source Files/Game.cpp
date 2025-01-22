#include "Game.h"
#include "GraphicsSystem.h"
#include "Color.h"
#include "Animation.h"
#include "AnimationData.h"
#include "Player.h"
#include "Timer.h"
#include "AnimationManager.h"
#include "EventSystem.h"
#include "GameObject2DManager.h"
#include "Texture2D.h"
#include "InputSystem.h"
#include "MouseEvent.h"
#include "GameListener.h"
#include "AxisEvent.h"

#include <iostream>
#include <Shader.h>
#include <cassert>

using namespace std;

Game* Game::mspInstance = nullptr;

Game* Game::getInstance()
{
	if (!mspInstance)
		mspInstance = new Game();

	return mspInstance;
}

void Game::cleanupInstance()
{
	if (mspInstance)
	{
		delete mspInstance;
		mspInstance = nullptr;
	}
}

void Game::startGame()
{
	mIsPlaying = true;
	mpGameTimer->start();
	while (mIsPlaying)    // Detect window close button or ESC key
	{
		mDeltaTime = mpGameTimer->getElapsedTime();
		mpGameTimer->start();

		getInput();
		update();
		render();

		debug();

		while(mpGameTimer->getElapsedTime() < mTimePerFrame)
			mpGameTimer->sleepUntilElapsed(mTimePerFrame);
			
	}
}

Game::Game()
{
	mpGraphicsSystem = nullptr;
	mpPlayerUnit = nullptr;
	mpGameTimer = nullptr;

	mDebugMode = false;
	mIsPlaying = false;
	mTimePerFrame = 0.0f;

	mDeltaTime = 0.0f;
}

Game::~Game()
{
	
}

void Game::init(int screenWidth, int screenHeight, int fps, bool debugMode)
{

	mpGraphicsSystem = GraphicsSystem::getInstance();

	assert(mpGraphicsSystem->init(screenWidth, screenHeight));

	mpGraphicsSystem->setActiveShaderProgram("Textured");

	Texture2D* smurfTexture = mpGraphicsSystem->createAndAddTexture2D("smurf", ASSET_PATH + SMURF_FILENAME, true);
	Texture2D* projTexture = mpGraphicsSystem->createAndAddTexture2D("proj", ASSET_PATH + PROJECTILE_FILENAME, true);
	Texture2D* bgTexture = mpGraphicsSystem->createAndAddTexture2D("bg", ASSET_PATH + BACKGROUND_FILEPATH);

	Vector2D bgScale = Vector2D(1.0f, 0.75f);
	Sprite* bgSprite = mpGraphicsSystem->createAndAddSprite("Background", &bgTexture, Vector2D::Zero(), bgTexture->getSize(), bgScale);
	mpGraphicsSystem->setBackground(mpGraphicsSystem->createAndAddGameObject2D(bgSprite, Vector2D(21.5f, 15.5f)));

	mpGraphicsSystem->createAndAddAnimationData("smurf", &smurfTexture, 4, 4);
	mpGraphicsSystem->createAndAddAnimationData("proj", &projTexture, 1, 13, 0.25f);


	Animation* playerAnim = mpGraphicsSystem->createAndAddAnimation("smurf", 16, true);
	mpPlayerUnit = new Player(playerAnim, 50.0f, Vector2D(3.0f, 3.0f));
	mpGraphicsSystem->addGameObject2D(mpPlayerUnit);

	mpGameListener = new GameListener();
	EventSystem::getInstance()->addListener(Event::KEYBOARD_EVENT, mpGameListener);
	EventSystem::getInstance()->addListener(Event::MOUSE_EVENT, mpGameListener);
	EventSystem::getInstance()->addListener(Event::AXIS_EVENT, mpGameListener);

	mpGameTimer = new Timer();

	mTimePerFrame = 1.0f / fps;
	mDebugMode = debugMode;

	srand(time(NULL));
}

void Game::cleanup()
{
	delete mpGameListener;
	mpGameListener = nullptr;

	delete mpPlayerUnit;
	mpPlayerUnit = nullptr;

	EventSystem::cleanupInstance();

	mpGraphicsSystem->cleanup();

	GraphicsSystem::cleanupInstance();
	mpGraphicsSystem = nullptr;

	delete mpGameTimer;
	mpGameTimer = nullptr;
}

void Game::getInput()
{
	InputSystem::getInstance()->update();
}

void Game::update()
{
	mpGraphicsSystem->setIntegerUniform("Textured", "uTexture0", 0);
	mpGraphicsSystem->setVec2Uniform("Textured", "uResolution", mpGraphicsSystem->getDisplayResolution());

	mpGraphicsSystem->getAnimation(0)->update(mDeltaTime);

	mpGraphicsSystem->update(mDeltaTime);
}

void Game::render()
{
	mpGraphicsSystem->drawInternalObjects();
	mpGraphicsSystem->render();
}

void Game::debug()
{
	if(mDebugMode)
	{
		//cout << "Frame Length: " << mDeltaTime << ", which is equal to " << 1 / mDeltaTime << " FPS." << endl;
	}
}

void Game::DPlayerMove(Vector2D loc)
{
	cout << "Player move to: " << loc << endl;
}

void Game::DMousePress(int button)
{
	cout << "Mouse Button pressed with ID: " << button << endl;
}

void Game::DMouseRelease(int button)
{
	cout << "Mouse Button released with ID: " << button << endl;
}

void Game::quitGame()
{
	mIsPlaying = false;
	cout << "QUIT" << endl;
}

void Game::onMouseMove(Vector2D mouseLoc)
{

}

void Game::onClick(const MouseEvent& event)
{

}

void Game::onToggleDrawMode()
{
	switch (mpGraphicsSystem->getDrawMode())
	{
	case GraphicsSystem::DrawMode::Fill:
		mpGraphicsSystem->setDrawMode(GraphicsSystem::DrawMode::Wireframe);
		break;

	case GraphicsSystem::DrawMode::Wireframe:
		mpGraphicsSystem->setDrawMode(GraphicsSystem::DrawMode::Fill);
		break;
	}
}

void Game::onToggleShaders()
{
	if (mpGraphicsSystem->getCurrentShaderProgram() == "Textured")
	{
		mpGraphicsSystem->setActiveShaderProgram("Color");
		Vector4D green = Vector4D(0.0f, 1.0f, 0.0f, 1.0f);
		mpGraphicsSystem->setVec4Uniform("Color", "uColor", green);
	}
	else
		mpGraphicsSystem->setActiveShaderProgram("Textured");
}

void Game::onShaderHotReload()
{
	//Hot Reload Shaders... Should probably do this through the managers (to not add a line for every new shader)
	mpGraphicsSystem->reloadShader("Textured Vert");
	mpGraphicsSystem->reloadShader("Textured Frag");
	mpGraphicsSystem->reloadShader("Basic Vert");
	mpGraphicsSystem->reloadShader("Color Frag");

	mpGraphicsSystem->linkShaderProgram("Textured");
	mpGraphicsSystem->linkShaderProgram("Color");
}

void Game::onToggleDebugMode()
{
	mDebugMode = !mDebugMode;

	mpGraphicsSystem->setDebugMode(mDebugMode);
}

void Game::onAxis(const AxisEvent& ev)
{
	mpPlayerUnit->setMoveDirection(ev.getState());
}