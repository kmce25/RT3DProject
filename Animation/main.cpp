// Based loosly on the first triangle OpenGL tutorial
// http://www.opengl.org/wiki/Tutorial:_OpenGL_3.1_The_First_Triangle_%28C%2B%2B/Win%29
// This program will render two triangles
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <stack>
#include "md2model.h"
#include "Entity.h"
#include "bass.h"
#include "Audio.h"

using namespace std;

#define DEG_TO_RADIAN 0.017453293

/// SKYBOX ///
glm::vec3 SkyBoxVertices[36] =
{
	glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, -1.0f,  1.0f),
	glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1.0f, -1.0f,  1.0f),
	glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, -1.0f)
};
unsigned int cubemapTexture;
GLuint shaderSkybox;


/// GLOBALS ///
GLuint meshIndexCount = 0;
GLuint md2VertCount1= 0;
GLuint md2VertCount2 = 0;
GLuint md2VertCount4 = 0;
GLuint md2VertCount5 = 0;
GLuint md2VertCount6 = 0;
GLuint md2VertCount7 = 0;
GLuint meshObjects[5];
GLuint shaderProgram;
std::vector<Entity> entities;
float windowWidth = 800.0f, windowHeight = 600.0f;

/// MD2 ///
md2model carModel;
md2model goblinModel;
md2model building1Model;
md2model building2Model;

/// TEXTURES & MATERIALS ///
GLuint textures[3];

rt3d::materialStruct material0 = {
	{ 0.2f, 0.4f, 0.2f, 1.0f }, // ambient
	{ 0.5f, 1.0f, 0.5f, 1.0f }, // diffuse
	{ 0.0f, 0.1f, 0.0f, 1.0f }, // specular
	2.0f  // shininess
};
rt3d::materialStruct material1 = {
	{ 0.4f, 0.4f, 1.0f, 1.0f }, // ambient
	{ 0.8f, 0.8f, 1.0f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	1.0f  // shininess
};


/// PLAYER CHARACTER ///
glm::vec3 playerCarPos(0, 0.0f, 0);
int currentAnim = 0; //Starts Standing
GLfloat playerRotation = 90.0f;

/// PLAYER CAMERA ///
glm::vec3 eye(playerCarPos.x-5.0f, playerCarPos.y+1.5f, playerCarPos.z);
glm::vec3 at(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::vec3 eyeReturn(0.0f, 0.0f, 0.0f);
GLfloat cameraRotation = playerRotation;
GLfloat cameraRotationReturn = 0.0f;
stack<glm::mat4> mvStack;
bool noClipMode = false;
bool cameraAlternatable = true;

/// MINIMAP CAMERA ///
glm::vec3 Minieye(playerCarPos.x, playerCarPos.y + 10.0f, playerCarPos.z);
glm::vec3 Miniat(playerCarPos.x, playerCarPos.y, playerCarPos.z);
glm::vec3 Miniup(0.5f, 0.0f, 0.0f);

rt3d::lightStruct light0 = {
	{0.1f, 0.1f, 0.1f, 1.0f}, // ambient
	{1.0f, 1.0f, 1.0f, 1.0f}, // diffuse
	{1.0f, 1.0f, 1.0f, 1.0f}, // specular
	{0.0f, 0.0f, 0.0f, 1.0f}  // position
};
glm::vec4 lightPos(0, 1.0f, 0, 1.0f); //light position



/// Set up rendering context ///
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
        rt3d::exitFatalError("Unable to initialize SDL"); 
	  
    // Request an OpenGL 3.0 context.
	
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)
 
    // Create a window with the prescribed dimensions
    window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if (!window) // Check window was created OK
        rt3d::exitFatalError("Unable to create window");
 
    context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
    SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

/// Texture loading function ///
GLuint loadBitmap(char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

	// load file - using core SDL library
 	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	SDL_PixelFormat *format = tmpSurface->format;
	
	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format-> Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format-> Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D,0,internalFormat,tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		SDL_Surface *tmpSurface;
		tmpSurface = SDL_LoadBMP(faces[i].c_str());
		if (!tmpSurface) {
			std::cout << "Error loading bitmap" << std::endl;
		}

		SDL_PixelFormat *format = tmpSurface->format;

		GLuint externalFormat, internalFormat;
		if (format->Amask) {
			internalFormat = GL_RGBA;
			externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
		}
		else {
			internalFormat = GL_RGB;
			externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0, externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
		SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer

	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}



/// INITIALISE ///
void init(void) {
	// For this simple example we'll be using the most basic of shader programs
	shaderProgram = rt3d::initShaders("phong-tex.vert","phong-tex.frag");
	shaderSkybox = rt3d::initShaders("skybox.vert", "skybox.frag");

	glUseProgram(shaderProgram);
	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, material0);

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("Cube1.obj", verts, norms, tex_coords, indices);
	GLuint size = indices.size();
	meshIndexCount = size;
	textures[0] = loadBitmap("fabric.bmp");
	meshObjects[0] = rt3d::createMesh(verts.size()/3, verts.data(), nullptr, norms.data(), tex_coords.data(), size, indices.data());

	//////////skybox
	glUseProgram(shaderSkybox);
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * 3 * 2 * 6 * sizeof(GLfloat), &SkyBoxVertices, GL_STATIC_DRAW); // 3 coords, 3 verts, 2 tris per face, 6 faces
	glVertexAttribPointer((GLuint)RT3D_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0); // make sure that the position in the shade is called in_Position
	glEnableVertexAttribArray(0);
	meshObjects[3] = skyboxVAO;

	// skybox textures
	vector<std::string> faces
	{
		std::string("./resources/textures/skybox/right.bmp"),
		std::string("./resources/textures/skybox/left.bmp"),
		std::string("./resources/textures/skybox/top.bmp"),
		std::string("./resources/textures/skybox/bottom.bmp"),
		std::string("./resources/textures/skybox/front.bmp"),
		std::string("./resources/textures/skybox/back.bmp")
	};
	cubemapTexture = loadCubemap(faces);
	////



	entities.push_back(Entity(glm::vec3(0.0f, -0.1f, 0.0f), glm::vec3(20.0f, 0.1f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), textures[0], meshObjects[0], meshIndexCount, material0, false, false));

	textures[1] = loadBitmap("Car.bmp");
	meshObjects[1] = carModel.ReadMD2Model("CarModel.MD2");
	md2VertCount1= carModel.getVertDataCount();
	entities.push_back(Entity(glm::vec3(5.0f, 0.0f, -5.0f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(180.0f, 0.0f, 0.0f), textures[1], meshObjects[1], md2VertCount1, material1, true, true));

	textures[2] = loadBitmap("hobGoblin2.bmp");
	meshObjects[2] = goblinModel.ReadMD2Model("tris.MD2");
	md2VertCount2 = goblinModel.getVertDataCount();
	entities.push_back(Entity(glm::vec3(5.0f, 1.2f, -5.0f), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(90.0f, 0.0f, 0.0f), textures[2], meshObjects[2], md2VertCount2, material1, true, false));


	textures[4] = loadBitmap("./Resources/textures/Building/Building1Texture1.bmp");
	meshObjects[4] = building1Model.ReadMD2Model("./Resources/md2models/Buildings/SmallBuilding.MD2");
	md2VertCount4 = building1Model.getVertDataCount();
	entities.push_back(Entity(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), textures[4], meshObjects[4], md2VertCount4, material1, true, false));

	textures[7] = loadBitmap("./Resources/textures/Building/houseblendTexture.bmp");
	meshObjects[7] = building2Model.ReadMD2Model("./Resources/md2models/Buildings/Building.MD2");
	md2VertCount7 = building2Model.getVertDataCount();
	entities.push_back(Entity(glm::vec3(40.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), textures[7], meshObjects[7], md2VertCount7, material1, true, false));


	/*textures[5] = loadBitmap("./Resources/textures/Building/BuildingCatTexture.bmp");
	meshObjects[5] = building1Model.ReadMD2Model("./Resources/md2models/Buildings/SmallBuilding.MD2");
	md2VertCount5 = building1Model.getVertDataCount();
	entities.push_back(Entity(glm::vec3(5.0f, 1.2f, -5.0f), glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(180.0f, 0.0f, 0.0f), textures[5], meshObjects[5], md2VertCount5, material1, true, false));*/

	/*textures[6] = loadBitmap("./Resources/textures/Building/Building3Texture.bmp");
	meshObjects[6] = building2Model.ReadMD2Model("./Resources/md2models/Buildings/Building.MD2");
	md2VertCount6 = building2Model.getVertDataCount();
	entities.push_back(Entity(glm::vec3(5.0f, 1.2f, -5.0f), glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(180.0f, 0.0f, 0.0f), textures[6], meshObjects[6], md2VertCount6, material1, true, false));*/




	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

}

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::sin(angle*DEG_TO_RADIAN), pos.y, pos.z - d*std::cos(angle*DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(angle*DEG_TO_RADIAN), pos.y, pos.z + d*std::sin(angle*DEG_TO_RADIAN));
}

/// Make the player camera rotate around the player in third person ///
void rotateCameraAroundPlayer(float angle) {
	cameraRotation += angle;
	
	eye.x -= playerCarPos.x;
	eye.z -= playerCarPos.z;

	float newX = eye.x*std::cos(-angle*DEG_TO_RADIAN) +eye.z*std::sin(-angle*DEG_TO_RADIAN);
	float newZ = -eye.x*std::sin(-angle*DEG_TO_RADIAN) +eye.z*std::cos(-angle*DEG_TO_RADIAN);

	eye.x = (newX + playerCarPos.x);
	eye.z = (newZ + playerCarPos.z);
}

void update(void) {
	const	Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (noClipMode) // How the camera controls when detached from the player characer
	{
		if (keys[SDL_SCANCODE_W]) { eye = moveForward(eye, cameraRotation, 0.3f); ; } //Move forward
		if (keys[SDL_SCANCODE_S]) { eye = moveForward(eye, cameraRotation, -0.3f); } //Move back
		if (keys[SDL_SCANCODE_A]) { cameraRotation -= 1.4f; } //Rotate left 
		if (keys[SDL_SCANCODE_D]) { cameraRotation += 1.4f; } //Rotate right
		if (keys[SDL_SCANCODE_Q]) { eye.y += 0.13f; } //Move up
		if (keys[SDL_SCANCODE_E]) { eye.y -= 0.13f; } //Move down
		if (keys[SDL_SCANCODE_R] && cameraAlternatable==true) {  noClipMode = false; eye = eyeReturn; cameraRotation = cameraRotationReturn; cameraAlternatable = false;  } //Disable NoClip mode, return to following the player character
		if (!keys[SDL_SCANCODE_R]) { cameraAlternatable = true; }
	}
	else // How the camera and player control when the camera follows the player characer
	{
		if (keys[SDL_SCANCODE_W]) { playerCarPos = moveForward(playerCarPos, cameraRotation, 0.1f);eye = moveForward(eye, cameraRotation, 0.1f); currentAnim = 1; }//Move forward
		if (!keys[SDL_SCANCODE_W]) { if (currentAnim = 1) { currentAnim = 0; } }//Not moving forward, use resting animation
		if (keys[SDL_SCANCODE_S]) { playerCarPos = moveForward(playerCarPos, cameraRotation, -0.05f);eye = moveForward(eye, cameraRotation, -0.05f); }//Move back
		if (keys[SDL_SCANCODE_A]) { rotateCameraAroundPlayer(-1.5f) ;	} //Rotate left 
		if (keys[SDL_SCANCODE_D]) { rotateCameraAroundPlayer(1.5f); } //Rotate right
		if (keys[SDL_SCANCODE_R] && cameraAlternatable == true) { eyeReturn = eye; cameraRotationReturn = cameraRotation; noClipMode = true;  cameraAlternatable = false; currentAnim = 0;	} //Ename NoClip mode, begin moving independantly of the player character
		if (!keys[SDL_SCANCODE_R]) { cameraAlternatable = true; }
		playerRotation = cameraRotation; //Rotate the player character with the camera
		//Miniat = glm::vec3(playerCarPos.x, playerCarPos.y, playerCarPos.z);
	}

	/// Audio Control ///
	if (keys[SDL_SCANCODE_1]) {
		BASS_Pause();
	}

	if (keys[SDL_SCANCODE_2]) {
		BASS_Start();
	}

	if (keys[SDL_SCANCODE_3])
	{
		Audio::LoadSample("Audio/CarAudio.wav");
	}

	

	Minieye = glm::vec3(playerCarPos.x, playerCarPos.y + 10.0f, playerCarPos.z); //Centre the minimap camera above the player character

	for (int it = 0; it < entities.size(); it++)
	{
		if (entities[it].getIsPlayer())
		{
			entities[it].updatePosition(glm::vec3(playerCarPos.x, playerCarPos.y, playerCarPos.z));
			entities[it].updateRotation(-playerRotation);
		}
	}
}


void drawMainContents(SDL_Window * window, GLfloat scale) {
/* Draw a cube for ground plane
glBindTexture(GL_TEXTURE_2D, textures[0]);
mvStack.push(mvStack.top());
mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, -0.1f, 0.0f));
mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
rt3d::setMaterial(shaderProgram, material0);
rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
mvStack.pop();*/




}

void draw(SDL_Window * window) {
	/// Set-up the screen ///
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f,0.5f,0.5f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN),windowWidth/windowHeight,1.0f,50.0f);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	GLfloat scale(1.0f); // Allow easy scaling of complete scene
	
	glm::mat4 modelview(1.0); // Set base position for scene
	mvStack.push(modelview);

	at = moveForward(eye, cameraRotation,1.0f);
	mvStack.top() = glm::lookAt(eye,at,up);

	/// Skybox ///
	glDisable(GL_CULL_FACE); // Disable culling and depth test
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shaderSkybox);

	/// Set view and projection matrix ///
	glm::mat4 projection_skybox = glm::perspective(float(45.0f*DEG_TO_RADIAN), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
	rt3d::setUniformMatrix4fv(shaderSkybox, "projection", glm::value_ptr(projection_skybox)); // JR note this is a 45 degree projection because we are looking at the face of a cube
	glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
	mvStack.push(glm::mat4(mvRotOnlyMat3)); // only use the rotational part, i.e. upper 3x3
	rt3d::setUniformMatrix4fv(shaderSkybox, "modelview", glm::value_ptr(mvStack.top()));
	mvStack.pop();

	glBindVertexArray(meshObjects[3]); //bind the VAO
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); // bind the cubemap
	glDrawArrays(GL_TRIANGLES, 0, 36); // draw
	glBindVertexArray(0); // unbind the VAO (by binding to zero)

	glEnable(GL_CULL_FACE); // Enable culling and depth test to draw rest of scene
	glEnable(GL_DEPTH_TEST);

	glUseProgram(shaderProgram);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	glm::vec4 playerCarLightPos(playerCarPos.x, playerCarPos.y + 1.0, playerCarPos.z, 1.0f);
	playerCarLightPos = playerCarLightPos*mvStack.top();
	rt3d::setLightPos(shaderProgram, glm::value_ptr(playerCarLightPos));

	glDepthMask(GL_TRUE); // make sure depth test is on
						  
	// Animate the md2 model, and update the mesh with new vertex data
	goblinModel.Animate(currentAnim, 0.1f);
	rt3d::updateMesh(meshObjects[2], RT3D_VERTEX, goblinModel.getAnimVerts(), goblinModel.getVertDataSize());

	/// DRAW MAIN VIEW ///
	for (int it = 0; it < entities.size(); it++)
	{
		entities[it].Draw(mvStack, projection, shaderProgram, playerRotation);
	}

	
	//mvStack.top() = glm::lookAt(Minieye, Miniat, Miniup);
	//mvStack.pop();
	projection = glm::perspective(float(90.0f*DEG_TO_RADIAN), windowWidth / windowHeight, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	modelview=glm::mat4(1.0); // Set base position for scene
	mvStack.push(modelview);

	//mvStack.top() = glm::lookAt(Minieye, Miniat, Miniup);



	glViewport(0, windowHeight - (windowHeight / 4), windowWidth / 4, windowHeight / 4);
	////////
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f,0.5f,0.5f,1.0f);
	//glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

	//glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN),windowWidth/windowHeight,1.0f,50.0f);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	//GLfloat scale(1.0f); // Allow easy scaling of complete scene
	
	//glm::mat4 modelview(1.0); // Set base position for scene
	mvStack.push(modelview);

	Miniat = moveForward(playerCarPos, playerRotation,1.0f);
	//mvStack.top() = glm::lookAt(Minieye, Miniat,up
	mvStack.top() = glm::lookAt(Minieye, Miniat,up);
	////////



	/// DRAW MINIMAP VIEW ///
	for (int it = 0; it < entities.size(); it++)
	{
		entities[it].Draw(mvStack, projection, shaderProgram, playerRotation);
	}


	glViewport(0, 0, windowWidth, windowHeight);

	mvStack.pop(); // Initial matrix
	glDepthMask(GL_TRUE);

    SDL_GL_SwapWindow(window); // Swap buffers
}


int main(int argc, char *argv[]) {
    SDL_Window * hWindow; // Window handle
    SDL_GLContext glContext; // OpenGL context handle
    hWindow = setupRC(glContext); // Create window and render context 

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // GlewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit (1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();
	Audio::LoadSample("Audio/GTARadio.wav");
	bool running = true; // Set running to true
	SDL_Event sdlEvent;  // Variable to detect SDL events
	while (running)	{	// The event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // Call the draw function
	}

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(hWindow);
    SDL_Quit();
    return 0;
}