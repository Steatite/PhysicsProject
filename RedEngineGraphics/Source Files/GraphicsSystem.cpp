//stb_image setup
#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

#include "GraphicsSystem.h"
#include "glad\glad.h"
#include "GLFW\glfw3.h"
#include <iostream>
#include "Mesh2D.h"
#include "Texture2D.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "ShaderManager.h"
#include "Sprite.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "Font.h"
#include "FontManager.h"
#include "DebugHUD.h"
#include "GridSystem.h"
#include "SpriteManager.h"
#include "GameObject2DManager.h"
#include "GameObject2D.h"
#include "AnimationData.h"
#include "Texture2DManager.h"
#include "Matrix3D.h"
#include "Camera2D.h"
#include "UIElement.h"
#include "UIScrollElement.h"

using namespace std;

GraphicsSystem* GraphicsSystem::mspInstance = nullptr;

GraphicsSystem* GraphicsSystem::getInstance()
{
	if (!mspInstance)
		mspInstance = new GraphicsSystem();

	return mspInstance;
}

void GraphicsSystem::cleanupInstance()
{
	if (mspInstance)
	{
		if (mspInstance->mInit)
			mspInstance->cleanup();

		delete mspInstance;
		mspInstance = nullptr;
	}
		
}

GraphicsSystem::GraphicsSystem()
{
	mInit = false;
	mDebugMode = false;
	mWindow = nullptr;
	mDrawMode = DrawMode::Fill;
	mpShaderManager = nullptr;
	mpAnimationManager = nullptr;
	mpSpriteManager = nullptr;
	mpDebugHUD = nullptr;
	mpFontManager = nullptr;
	mpGameObjectManager = nullptr;
	mpTexture2DManager = nullptr;
	mpBackground = nullptr;
}

GraphicsSystem::~GraphicsSystem()
{
	if (mInit)
		cleanup();
}

void _scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	GraphicsSystem::getInstance()->callScrollCallback(xoffset, yoffset);
}

bool GraphicsSystem::init(int displayWidth, int displayHeight)
{
	//Initialize OpenGL, and set our context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create Window, and verify
	mWindow = glfwCreateWindow(displayWidth, displayHeight, "LearnOpenGL", NULL, NULL);
	if (!mWindow)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(mWindow);

	mWindowResolution = Vector2D(displayWidth, displayHeight);

	//Load glad, and verify
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return false;
	}

	//Tell OpenGL our rendering frame/viewport
	glViewport(0, 0, displayWidth, displayHeight);

	//Determine default background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Set our callback function for resizing the window
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetScrollCallback(mWindow, _scrollCallback);

	mpShaderManager = ShaderManager::getInstance();
	mpShaderManager->init();
	setActiveShaderProgram("Textured");

	mpTexture2DManager = Texture2DManager::getInstance();
	mpTexture2DManager->init();

	mpSpriteManager = SpriteManager::getInstance();
	mpSpriteManager->init();

	mpAnimationManager = AnimationManager::getInstance();
	mpAnimationManager->init();

	mpFontManager = FontManager::getInstance();
	mpFontManager->init();

	mpGameObjectManager = GameObject2DManager::getInstance();
	mpGameObjectManager->init();

	mpDebugHUD = DebugHUD::getInstance();
	mpDebugHUD->addDebugValue("Current Shader Program: ", &GraphicsSystem::getCurrentShaderProgram);

	mpCamera = new Camera2D(Vector2D(0, 0), mWindowResolution);

	mpGridSystem = GridSystem::getInstance();
	mpGridSystem->init(displayWidth, displayHeight);

	mpTexture2DManager = Texture2DManager::getInstance();
	mpTexture2DManager->init();

	mpSpriteManager = SpriteManager::getInstance();
	mpSpriteManager->init();

	mpAnimationManager = AnimationManager::getInstance();
	mpAnimationManager->init();

	mpFontManager = FontManager::getInstance();
	mpFontManager->init();

	mpGameObjectManager = GameObject2DManager::getInstance();
	mpGameObjectManager->init();

	mpDebugHUD = DebugHUD::getInstance();
	mpDebugHUD->addDebugValue("Current Shader Program: ", &GraphicsSystem::getCurrentShaderProgram);

	mpCamera = new Camera2D(Vector2D(0, 0), mWindowResolution);

	mpGridSystem = GridSystem::getInstance();
	mpGridSystem->init(displayWidth, displayHeight);

	cout << "Well here we are!" << endl;

	mCurrentShaderProgram = "";

	mInit = true;
	return true;
}

void GraphicsSystem::cleanup()
{
	mpGameObjectManager->cleanup();
	GameObject2DManager::cleanupInstance();

	mpShaderManager->cleanup();
	ShaderManager::cleanupInstance();

	mpAnimationManager->cleanup();
	AnimationManager::cleanupInstance();

	mpSpriteManager->cleanup();
	SpriteManager::cleanupInstance();

	mpTexture2DManager->cleanup();
	Texture2DManager::cleanupInstance();

	mpFontManager->cleanup();
	FontManager::cleanupInstance();

	DebugHUD::cleanupInstnace();

	mpGridSystem->cleanup();
	GridSystem::cleanupInstance();

	delete mpCamera;
	mpCamera = nullptr;

	glfwTerminate();
	mInit = false;
}

bool GraphicsSystem::render()
{
	if(mDebugMode)
		drawDebugInfo();

	if (glfwWindowShouldClose(mWindow))
		return false;

	glfwSwapBuffers(mWindow);
	glfwPollEvents();

	glClear(GL_COLOR_BUFFER_BIT);

	if (mpBackground)
		draw(mpBackground);

	//drawGrid();

	return true;
}

void GraphicsSystem::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	GraphicsSystem::getInstance()->OnWindowSizeUpdated(width, height);

	glViewport(0, 0, width, height);
}

void GraphicsSystem::OnWindowSizeUpdated(int width, int height)
{
	mWindowResolution = Vector2D(width, height);
}

void GraphicsSystem::draw(Mesh2D& mesh)
{
	assert(mCurrentShaderProgram != "");
	setActiveShaderProgram(mCurrentShaderProgram);

	if (mesh.mVBO == -1)
	{
		initMesh2D(&mesh);

		bindMesh2D(&mesh);

		
		for (int i = 0; i < mesh.mTextureDataCount; i++)
		{
			if (mesh.mTextureData[i]->mTOI == -1)
				initTexture2D(mesh.mTextureData[i]);
			bindTexture2D(mesh.mTextureData[i], i);
		}


		//Packing and linking only need to occur on mesh init, as the data is stored in the VAO
		packGPUData(mesh);

		//Copy draw order data into bound buffer (EBO)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.mDrawCount, mesh.mDrawOrder, GL_STATIC_DRAW);

		linkGPUData(mesh);
		
	}
	else
	{
		for (int i = 0; i < mesh.mTextureDataCount; i++)
		{
			bindTexture2D(mesh.mTextureData[i], i);
		}

		bindMesh2D(&mesh);
	}

	glDrawElements(convertMeshType(mesh.mMeshType), mesh.mDrawCount, GL_UNSIGNED_INT, 0);
}

void GraphicsSystem::draw(Sprite& sprite, Vector2D location, float angle, bool useTopAnchoring)
{
	float pi = 3.14159265358979323846f;

	internalDrawSprite(sprite);

	float yOffset =
		useTopAnchoring ?
		-sprite.getSize().getY() / 2.0f * sprite.getScale().getY() * cos(angle * pi / 180.0f) :
		0.0f;

	float xOffset =
		useTopAnchoring ?
		sprite.getSize().getY() / 2.0f * sprite.getScale().getY() * sin(angle * pi / 180.0f) :
		0.0f;

	Matrix3D scaleMatrix = Matrix3D(
		sprite.mScale.getX(), 0.0f, 0.0f,
		0.0f, sprite.mScale.getY(), 0.0f,
		0.0f, 0.0f, 1.0f
	);

	Matrix3D rotationMatrix = Matrix3D(
		cos(angle * pi / 180.0f), -sin(angle * pi / 180.0f), 0.0f,
		sin(angle * pi / 180.0f), cos(angle * pi / 180.0f), 0.0f,
		0.0f, 0.0f, 1.0f
	);

	Matrix3D translationMatrix = Matrix3D(
		1.0f, 0.0f, location.getX() * mpGridSystem->getGridBoxWidth() + xOffset,
		0.0f, 1.0f, location.getY() * mpGridSystem->getGridBoxHeight() + yOffset,
		0.0f, 0.0f, 1.0f
	);
	
	Matrix3D modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	

	Matrix3D viewMatrix = Matrix3D(
		1.0f, 0.0f, -mpCamera->getLoc().getX() * mpGridSystem->getGridBoxWidth(),
		0.0f, 1.0f, -mpCamera->getLoc().getY() * mpGridSystem->getGridBoxHeight(),
		0.0f, 0.0f, 1.0f
	);

	setMat3Uniform(mCurrentShaderProgram, "uModelMat", modelMatrix); //These game objects shouldn't need these uniforms sent here. Need some gs funciton to be called by game.
	setMat3Uniform(mCurrentShaderProgram, "uViewMat", viewMatrix);

	setActiveShaderProgram(mCurrentShaderProgram);

	glDrawElements(convertMeshType(sprite.mpMesh->mMeshType), sprite.mpMesh->mDrawCount, GL_UNSIGNED_INT, 0);
}

void GraphicsSystem::drawUI(Sprite& sprite, Vector2D location, Vector2D lowerBound, Vector2D upperBound)
{
	internalDrawSprite(sprite);


	// Matrix Math should DEF be moved to the GPU
	Matrix3D modelMatrix = Matrix3D(
		sprite.mScale.getX(), 0.0f, location.getX() * mpCamera->getResolution().getX(),
		0.0f, sprite.mScale.getY(), location.getY() * mpCamera->getResolution().getY(),
		0.0f, 0.0f, 1.0f
	);

	Matrix3D viewMatrix = Matrix3D(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	);

	if (upperBound == Vector2D::Zero())
		upperBound = Vector2D(1.0f);

	setMat3Uniform(mCurrentShaderProgram, "uModelMat", modelMatrix); //These game objects shouldn't need these uniforms sent here. Need some gs funciton to be called by game.
	setMat3Uniform(mCurrentShaderProgram, "uViewMat", viewMatrix);
	setVec2Uniform(mCurrentShaderProgram, "uLowerBound", lowerBound);
	setVec2Uniform(mCurrentShaderProgram, "uUpperBound", upperBound);

	setActiveShaderProgram(mCurrentShaderProgram);

	glDrawElements(convertMeshType(sprite.mpMesh->mMeshType), sprite.mpMesh->mDrawCount, GL_UNSIGNED_INT, 0);
}

void GraphicsSystem::internalDrawSprite(Sprite& sprite)
{
	setActiveShaderProgram(mCurrentShaderProgram);

	if (sprite.mpMesh->mVBO == -1)
	{
		initMesh2D(sprite.mpMesh);

		bindMesh2D(sprite.mpMesh);


		for (int i = 0; i < sprite.mpMesh->mTextureDataCount; i++)
		{
			if (sprite.mpMesh->mTextureData[i]->mTOI == -1)
				initTexture2D(sprite.mpMesh->mTextureData[i]);
			bindTexture2D(sprite.mpMesh->mTextureData[i], i);
		}


		//Packing and linking only need to occur on mesh init, as the data is stored in the VAO
		packGPUData(*sprite.mpMesh, sprite.mSize, sprite.mAnchoring);

		//Copy draw order data into bound buffer (EBO)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * sprite.mpMesh->mDrawCount, sprite.mpMesh->mDrawOrder, GL_STATIC_DRAW);

		linkGPUData(*sprite.mpMesh);

	}
	else
	{
		for (int i = 0; i < sprite.mpMesh->mTextureDataCount; i++)
		{
			bindTexture2D(sprite.mpMesh->mTextureData[i], i);
		}

		bindMesh2D(sprite.mpMesh);
	}
}

void GraphicsSystem::draw(Animation& animation, Vector2D location)
{
	Sprite* currentSprite = animation.getCurrentSprite();
	if (currentSprite == nullptr)
		return;

	draw(*currentSprite, location);
}

void GraphicsSystem::drawUI(Animation& animation, Vector2D location)
{
	Sprite* currentSprite = animation.getCurrentSprite();
	if (currentSprite == nullptr)
		return;

	drawUI(*currentSprite, location);
}

void GraphicsSystem::draw(GameObject2D* obj)
{
	switch (obj->mDrawingMode)
	{
	case GameObject2D::SpriteMode:
		draw(*obj->mImage.s, obj->getLoc(), obj->getRotation(), obj->getIsUsingTopAnchoring());
		break;
	case GameObject2D::AnimationMode:
		draw(*obj->mImage.a, obj->getLoc());
		break;
	}
}

void GraphicsSystem::drawUI(GameObject2D* obj)
{
	switch (obj->mDrawingMode)
	{
	case GameObject2D::SpriteMode:
		drawUI(*obj->mImage.s, obj->getLoc());
		break;
	case GameObject2D::AnimationMode:
		drawUI(*obj->mImage.a, obj->getLoc());
		break;
	}
}

void GraphicsSystem::drawUI(UIElement* obj)
{
	UIScrollElement* scrollWindow = dynamic_cast<UIScrollElement*>(obj->getParent());

	Rect bounds;

	if (scrollWindow)
		bounds = scrollWindow->getBounds();

	switch (obj->mDrawingMode)
	{
	case GameObject2D::SpriteMode:
		drawUI(*obj->mImage.s, obj->getLoc(), bounds.lower, bounds.upper);
		break;
	case GameObject2D::AnimationMode:
		drawUI(*obj->mImage.a, obj->getLoc());
		break;
	}
}

void GraphicsSystem::drawUI(string text, string fontKey, string shaderProgram, Vector2D loc, Vector3D color, float scale)
{
	//No need to set active shader program, as setting a uniform sets the shader program as active
	setVec3Uniform(shaderProgram, "textColor", color);
	setVec2Uniform(shaderProgram, "uResolution", mpCamera->getResolution());

	Font* font = mpFontManager->getFont(fontKey);

	//Init
	if (font->mCharacters.empty())
	{
		initFont(font);
	}

	if (mpFontManager->mVBO == -1)
	{
		glGenVertexArrays(1, &mpFontManager->mVAO);
		glGenBuffers(1, &mpFontManager->mVBO);
		glBindVertexArray(mpFontManager->mVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mpFontManager->mVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
	//Bind
	else
	{
		glBindVertexArray(mpFontManager->mVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mpFontManager->mVBO);
	}

	//Pack Data & Draw

	for (string::const_iterator c = text.begin(); c != text.end(); c++)
	{
		Font::Character ch = font->mCharacters.at(*c);

		float x = loc.getX() + ch.bearing.getX() * scale / mpCamera->getResolution().getX();
		float y = loc.getY() - (ch.size.getY() - ch.bearing.getY()) * scale / mpCamera->getResolution().getY();

		float w = ch.size.getX() * scale / mpCamera->getResolution().getX();
		float h = ch.size.getY() * scale / mpCamera->getResolution().getY();

		float verticies[6][4] = { //Should this be changed? I think the x, y position might be flipped from game on y.
			{x,		y + h,	0.0f, 0.0f },
			{x,		y,		0.0f, 1.0f },
			{x + w,	y,		1.0f, 1.0f },

			{x,		y + h,	0.0f, 0.0f },
			{x + w,	y,		1.0f, 1.0f },
			{x + w, y + h,	1.0f, 0.0f }
		};

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticies), verticies);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		loc.setX(loc.getX() + (ch.advance >> 6) * scale / mpCamera->getResolution().getX());
	}

	setActiveShaderProgram(mCurrentShaderProgram);
}

void GraphicsSystem::drawDebugInfo()
{
	mpDebugHUD->draw();
}

ShaderObjectIndex GraphicsSystem::sdCreateShader(SHADER_TYPE type)
{
	switch (type)
	{
	case VERTEX_SHADER:
		return glCreateShader(GL_VERTEX_SHADER);
		break;
	case FRAGMENT_SHADER:
		return glCreateShader(GL_FRAGMENT_SHADER);
		break;
	}

	return -1;
}

bool GraphicsSystem::sdCompileShader(ShaderObjectIndex soi, string code)
{
	const char* charCode = code.c_str();
	//Attach the Vertex Shader Code and Compile
	glShaderSource(soi, 1, &charCode, NULL);
	glCompileShader(soi);

	int success;
	glGetShaderiv(soi, GL_COMPILE_STATUS, &success);

	return success;
}

string GraphicsSystem::sdCollectDebugInfo(ShaderObjectIndex soi)
{
	char infoLog[512];

	glGetShaderInfoLog(soi, 512, NULL, infoLog);

	return infoLog;
}

ShaderProgramIndex GraphicsSystem::spCreateShaderProgram()
{
	return glCreateProgram();
}

void GraphicsSystem::spAttachShaderToProgram(ShaderProgramIndex spi, ShaderObjectIndex soi)
{
	glAttachShader(spi, soi);
}

bool GraphicsSystem::spLinkProgram(ShaderProgramIndex spi)
{
	glLinkProgram(spi);

	int success;
	glGetProgramiv(spi, GL_LINK_STATUS, &success);

	return success;
}

string GraphicsSystem::spCollectDebugInfo(ShaderProgramIndex spi)
{
	char infoLog[512];

	glGetProgramInfoLog(spi, 512, NULL, infoLog);

	return infoLog;
}

void GraphicsSystem::sdDeleteShader(ShaderObjectIndex shader)
{
	glDeleteShader(shader);
}

void GraphicsSystem::spActivateFloatAttribute(int index, int dimensions)
{
	glVertexAttribPointer(index, dimensions, GL_FLOAT, GL_FALSE, dimensions * sizeof(float), (void*)0);
	glEnableVertexAttribArray(index);
}

void GraphicsSystem::setActiveShaderProgram(string program)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	glUseProgram(sp->mSPI);

	mCurrentShaderProgram = program;
}

void GraphicsSystem::setDrawMode(DrawMode mode)
{
	switch (mode)
	{
	case DrawMode::Fill:
		mDrawMode = DrawMode::Fill;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case DrawMode::Wireframe:
		mDrawMode = DrawMode::Wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}
}

void GraphicsSystem::setFloatUniform(string program, string uniformName, float value)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	glUniform1f(uniformLocation, value);
}

void GraphicsSystem::setIntegerUniform(string program, string uniformName, int value)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	glUniform1i(uniformLocation, value);
}

void GraphicsSystem::setVec2Uniform(std::string program, std::string uniformName, Vector2D value)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	glUniform2f(uniformLocation, value.getX(), value.getY());
}

void GraphicsSystem::setVec3Uniform(std::string program, std::string uniformName, Vector3D value)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	glUniform3f(uniformLocation, value.getX(), value.getY(), value.getZ());
}

void GraphicsSystem::setVec4Uniform(std::string program, std::string uniformName, Vector4D value)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	glUniform4f(uniformLocation, value.getX(), value.getY(), value.getZ(), value.getW());
}

void GraphicsSystem::setMat3Uniform(std::string program, std::string uniformName, Sprite& sprite, Vector2D location)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	float mat[] = {
		sprite.mScale.getX(), 0.0f, location.getX() * mpGridSystem->getGridBoxWidth(),
		0.0f, sprite.mScale.getY(), location.getY() * mpGridSystem->getGridBoxHeight(),
		0.0f, 0.0f, 1.0f
	};
	glUniformMatrix3fv(uniformLocation, 1, false, mat);
}

void GraphicsSystem::setMat3Uniform(std::string program, std::string uniformName, Matrix3D matrix)
{
	ShaderProgram* sp = ShaderManager::getInstance()->getShaderProgram(program);

	int uniformLocation = glGetUniformLocation(sp->mSPI, uniformName.c_str());

	if (uniformLocation == -1)
		return;

	glUseProgram(sp->mSPI);
	glUniformMatrix3fv(uniformLocation, 1, false, matrix.convertToFloatArray());
}

float GraphicsSystem::getTime()
{
	return glfwGetTime();
}

void GraphicsSystem::initTexture2D(Texture2D* texture)
{
	glGenTextures(1, &texture->mTOI);
	glBindTexture(GL_TEXTURE_2D, texture->mTOI);

	if(texture->mHasAlpha)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->mWidth, texture->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->mData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->mWidth, texture->mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->mData);

	glGenerateMipmap(GL_TEXTURE_2D);

	texture->freeRawData();
}

void GraphicsSystem::initMesh2D(Mesh2D* mesh)
{
	//Setup Vertex Buffer Object (VBO)
	glGenBuffers(1, &mesh->mVBO);

	//Setup Vertex Array Object (VAO)
	glGenVertexArrays(1, &mesh->mVAO);

	//Setup Element Buffer Object
	glGenBuffers(1, &mesh->mEBO);
}

void GraphicsSystem::initFont(Font* font)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 32; c <= 126; c++)
	{
		if (FT_Load_Char(font->mFontFace, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph: " << c << std::endl;
			continue;
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			font->mFontFace->glyph->bitmap.width,
			font->mFontFace->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			font->mFontFace->glyph->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Font::Character character
		{
			texture,
			Vector2D(font->mFontFace->glyph->bitmap.width, font->mFontFace->glyph->bitmap.rows),
			Vector2D(font->mFontFace->glyph->bitmap_left, font->mFontFace->glyph->bitmap_top),
			font->mFontFace->glyph->advance.x
		};

		font->mCharacters.emplace(c, character);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	FT_Done_Face(font->mFontFace);
}

void GraphicsSystem::bindMesh2D(Mesh2D* mesh)
{
	//Bind VAO to OpenGL
	glBindVertexArray(mesh->mVAO);

	//Bind VBO to OpenGL
	glBindBuffer(GL_ARRAY_BUFFER, mesh->mVBO);

	//Bind EBO to VAO (Don't unbind EBO before VAO [VAO remembers all])
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mEBO);
}

void GraphicsSystem::bindTexture2D(Texture2D* texture, unsigned int textureLocation)
{
	glActiveTexture(GL_TEXTURE0 + textureLocation);
	glBindTexture(GL_TEXTURE_2D, texture->mTOI);
}

void GraphicsSystem::packGPUData(Mesh2D& mesh, Vector2D size, ImageAnchor anchoring)
{
	unsigned int valuesPerVertex = (mesh.mHasColorData) ? 6 : 3;
	valuesPerVertex += (mesh.mTextureDataCount) ? 2 : 0;
	unsigned int numOfFloats = valuesPerVertex * (double)mesh.mVertexCount;

	float* verticies = new float[numOfFloats];

	Vector2D offset;
	float largerSide = size.getX() > size.getY() ? size.getX() : size.getY();
	float halfXSize = size.getX() / 2.0f;
	float halfYSize = size.getX() / 2.0f;

	switch (anchoring)
	{
	case ImageAnchor::BottomLeft:
		offset = Vector2D::Zero();
		break;

	case ImageAnchor::Center:
		offset = Vector2D(-halfXSize, -halfYSize);
	}

	for (int i = 0; i < mesh.mVertexCount; i++)
	{
		verticies[i * valuesPerVertex] = mesh.getVertexAt(i).getX() * largerSide + offset.getX();
		verticies[i * valuesPerVertex + 1] = mesh.getVertexAt(i).getY() * largerSide + offset.getY();
		verticies[i * valuesPerVertex + 2] = 1.0f; //2D Objects are drawn at Z = 1

		if (mesh.mHasColorData)
		{
			verticies[i * valuesPerVertex + 3] = mesh.mColorData[i].getX();
			verticies[i * valuesPerVertex + 4] = mesh.mColorData[i].getY();
			verticies[i * valuesPerVertex + 5] = mesh.mColorData[i].getZ();

			if (mesh.mTextureDataCount)
			{
				verticies[i * valuesPerVertex + 6] = mesh.mTextureCoords[i].getX();
				verticies[i * valuesPerVertex + 7] = mesh.mTextureCoords[i].getY();
			}
		}
	}

	//Copy data into bound buffer (VBO)
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numOfFloats, verticies, GL_STATIC_DRAW);

	delete[] verticies;
	verticies = nullptr;
}

void GraphicsSystem::linkGPUData(Mesh2D& mesh)
{
	//Linking Vertex Attributes
	if (mesh.mTextureDataCount)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	else if (mesh.mHasColorData)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	else
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
}

bool GraphicsSystem::createAndAddShader(string key, SHADER_TYPE type, string filename)
{
	return mpShaderManager->createAndAddShader(key, type, filename);
}

void GraphicsSystem::removeShader(string key)
{
	mpShaderManager->removeShader(key);
}

bool GraphicsSystem::reloadShader(string key)
{
	return mpShaderManager->reloadShader(key);
}

void GraphicsSystem::createAndAddShaderProgram(string key)
{
	mpShaderManager->createAndAddShaderProgram(key);
}

bool GraphicsSystem::createAndAddShaderProgram(string key, string vertexShader, string fragmentShader)
{
	return mpShaderManager->createAndAddShaderProgram(key, vertexShader, fragmentShader);
}

void GraphicsSystem::removeShaderProgram(string key)
{
	mpShaderManager->removeShaderProgram(key);
}

bool GraphicsSystem::attachShaderToProgram(string programKey, string shaderKey)
{
	return mpShaderManager->attachShaderToProgram(programKey, shaderKey);
}

void GraphicsSystem::activateFloatAttributeOnProgram(string key, int index, int dimensions)
{
	mpShaderManager->activateFloatAttributeOnProgram(key, index, dimensions);
}

bool GraphicsSystem::linkShaderProgram(string key)
{
	return mpShaderManager->linkShaderProgram(key);
}

AnimationData* GraphicsSystem::createAndAddAnimationData(string key, Texture2D** texture, int numHorizontal, int numVertical, Vector2D scale)
{
	return mpAnimationManager->createAndAddAnimationData(key, texture, numHorizontal, numVertical, scale);
}

void GraphicsSystem::removeAnimationData(string key)
{
	mpAnimationManager->removeAnimationData(key);
}

Animation* GraphicsSystem::createAndAddAnimation(string animationDataKey, int mFPS, bool isLooping)
{
	return mpAnimationManager->createAndAddAnimation(animationDataKey, mFPS, isLooping);
}

void GraphicsSystem::removeAndDeleteAnimation(int id)
{
	mpAnimationManager->removeAndDeleteAnimation(id);
}

void GraphicsSystem::removeAndDeleteAnimation(Animation* anim)
{
	mpAnimationManager->removeAndDeleteAnimation(anim);
}

Animation* GraphicsSystem::getAnimation(int id)
{
	return mpAnimationManager->getAnimation(id);
}

void GraphicsSystem::update(float deltaTime)
{
	mpGameObjectManager->updateAll(deltaTime);
}

bool GraphicsSystem::_imGetKey(unsigned int keyCode, GraphicsSystemIMKey key)
{
	return glfwGetKey(mWindow, keyCode);
}

bool GraphicsSystem::_imGetMouseButton(unsigned int mouseCode, GraphicsSystemIMKey key)
{
	return glfwGetMouseButton(mWindow, mouseCode) == GLFW_PRESS;
}

Vector2D GraphicsSystem::_imGetMousePosition(GraphicsSystemIMKey key)
{
	double x, y;
	glfwGetCursorPos(mWindow, &x, &y);
	y = mWindowResolution.getY() - y;

	x /= mWindowResolution.getX();
	y /= mWindowResolution.getY();

	return Vector2D(x, y);
}

void GraphicsSystem::createAndAddFont(string key, string filepath, int pointSize)
{
	mpFontManager->createAndAddFont(key, filepath, pointSize);
}

void GraphicsSystem::removeAndDeleteFont(string key)
{
	mpFontManager->removeAndDeleteFont(key);
}

Texture2D* GraphicsSystem::createAndAddTexture2D(std::string key, std::string filepath, bool hasAlpha)
{
	return mpTexture2DManager->createAndAddTexture2D(key, filepath, hasAlpha);
}

void GraphicsSystem::removeAndDeleteTexture2D(std::string key)
{
	mpTexture2DManager->removeAndDeleteTexture2D(key);
}

Texture2D* GraphicsSystem::getTexture2D(std::string key)
{
	return mpTexture2DManager->getTexture2D(key);
}

void GraphicsSystem::addToDebugHUD(std::string text) 
{
	mpDebugHUD->addDebugValue(text); 
}

void GraphicsSystem::addPersistantToDebugHUD(std::string text)
{
	mpDebugHUD->addPersistantDebugValue(text);
}

Sprite* GraphicsSystem::createAndAddSprite(string key, Texture2D** texture, Vector2D textureStartLoc, Vector2D size, Vector2D scale, ImageAnchor anchoring)
{
	return mpSpriteManager->createAndAddSprite(key, texture, textureStartLoc, size, scale, anchoring);
}

void GraphicsSystem::removeAndDeleteSprite(string key)
{
	mpSpriteManager->removeAndDeleteSprite(key);
}

Sprite* GraphicsSystem::getSprite(string key)
{
	return mpSpriteManager->getSprite(key);
}

GameObject2D* GraphicsSystem::createGameObject2D(Sprite* sprite, Vector2D loc)
{
	return mpGameObjectManager->createGameObject2D(sprite, loc);
}

GameObject2D* GraphicsSystem::createAndAddGameObject2D(Sprite* sprite, Vector2D loc, bool useTopAnchoring)
{
	return mpGameObjectManager->createAndAddGameObject2D(sprite, loc, useTopAnchoring);
}

GameObject2D* GraphicsSystem::createAndAddGameObject2D(Animation* anim, Vector2D loc)
{
	return mpGameObjectManager->createAndAddGameObject2D(anim, loc);
}

void GraphicsSystem::addGameObject2D(GameObject2D* obj)
{
	mpGameObjectManager->addGameObject2D(obj);
}

void GraphicsSystem::removeAndDeleteGameObject2D(GameObject2D* obj)
{
	mpGameObjectManager->removeAndDeleteGameObject2D(obj);
}

void GraphicsSystem::removeAndDeleteGameObject2D(int id)
{
	mpGameObjectManager->removeAndDeleteGameObject2D(id);
}

void GraphicsSystem::drawGrid()
{

	string previousShader = mCurrentShaderProgram;
	setActiveShaderProgram("ColorUI");

	Vector4D white = Vector4D(1.0f, 1.0f, 1.0f, 1.0f);
	setVec4Uniform("ColorUI", "uColor", white);

	setVec2Uniform("ColorUI", "uResolution", mpCamera->getResolution());

	float boxHeight = mpGridSystem->getGridBoxHeight();
	float boxWidth = mpGridSystem->getGridBoxWidth();
	int rows = getDisplayHeight() / boxHeight + 1;
	int columns = getDisplayWidth() / boxWidth + 1;

	float cameraPixelOffsetX = (mpCamera->getLoc().getX() - (int)mpCamera->getLoc().getX()) * boxWidth;
	float cameraPixelOffsetY = (mpCamera->getLoc().getY() - (int)mpCamera->getLoc().getY()) * boxWidth;

	for (int i = 0; i < rows; i++)
	{
		Vector2D verticies[] = {
			Vector2D(0.0f, i * boxHeight - cameraPixelOffsetY),
			Vector2D(getDisplayWidth() * 1.0f, i * boxHeight - cameraPixelOffsetY)
		};

		unsigned int drawOrder[] = { 0, 1 };

		Mesh2D mesh(verticies, 2, drawOrder, 2, MeshType::Lines);
		draw(mesh);
	}

	for (int i = 0; i < columns; i++)
	{
		Vector2D verticies[] = {
			Vector2D(i * boxWidth - cameraPixelOffsetX, 0.0f),
			Vector2D(i * boxWidth - cameraPixelOffsetX, getDisplayHeight() * 1.0f)
		};

		unsigned int drawOrder[] = { 0, 1 };

		Mesh2D mesh(verticies, 2, drawOrder, 2, MeshType::Lines);
		draw(mesh);
	}

	setActiveShaderProgram(previousShader);
}

unsigned int GraphicsSystem::convertMeshType(MeshType meshType)
{
	switch (meshType)
	{
	case MeshType::Triangles:
		return GL_TRIANGLES;

	case MeshType::Lines:
		return GL_LINES;
	}
}

Vector2D GraphicsSystem::convertToGridCoordinates(Vector2D pixelCoordinates)
{
	return mpGridSystem->convertPixelsToGrid(pixelCoordinates);
}

Vector2D GraphicsSystem::convertToScreenCoordinates(Vector2D pixelCoordinates)
{
	Vector2D cameraResolution = mpCamera->getResolution();

	return Vector2D(pixelCoordinates.getX() / cameraResolution.getX(), pixelCoordinates.getY() / cameraResolution.getY());
}

Vector2D GraphicsSystem::convertScreenToGridCoordinates(Vector2D screenCoordinates)
{
	return mpGridSystem->convertScreenToGrid(screenCoordinates);
}

void GraphicsSystem::cleanupMesh2D(Mesh2D* mesh)
{
	glDeleteBuffers(1, &mesh->mVBO);

	glDeleteVertexArrays(1, &mesh->mVAO);

	glDeleteBuffers(1, &mesh->mEBO);
}

void GraphicsSystem::cleanupTexture2D(Texture2D* texture)
{
	glDeleteTextures(1, &texture->mTOI);
}

void GraphicsSystem::setCursorHidden(bool isHidden)
{
	if (isHidden)
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	else
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void GraphicsSystem::_imSetScrollCallback(ScrollFunctionCallback callback, GraphicsSystemIMKey key)
{
	mScrollCallback = callback;
}

void GraphicsSystem::callScrollCallback(double xOffset, double yOffset)
{
	if(mScrollCallback)
		mScrollCallback(xOffset, yOffset);
}

void GraphicsSystem::drawInternalObjects()
{
	mpGameObjectManager->drawAll();
}

void GraphicsSystem::setBackgroundColor(Vector3D color)
{
	glClearColor(color.getX(), color.getY(), color.getZ(), 1.0f);
}