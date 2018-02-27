/* Base code for shadow mapping lab */
/* This code is incomplete - follow tasks listed in handout */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tiny_obj_loader/tiny_obj_loader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "WindowManager.h"


using namespace std;
using namespace glm;



class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Shaders
	shared_ptr<Program> DepthProg;
	shared_ptr<Program> DepthProgDebug;
	shared_ptr<Program> ShadowProg;
	shared_ptr<Program> DebugProg;

	// Shapes
	shared_ptr<Shape> world;
	shared_ptr<Shape> shape;

	// Textures
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;

	// Debug Settings
	bool SHOW_LIGHT_COLOR = false;
	bool SHOW_LIGHT_DEPTH = false;

	GLuint ShadowMapFBO;
	const GLuint ShadowMapWidth = 1024, ShadowMapHeight = 1024;
	GLuint ShadowMapDepthTexture;

	glm::vec3 g_light = glm::vec3(1, 1, 1);

	// Ground Plane vertex data
	GLuint GroundVertexArray;
	int GroundIndexCount;

	// Screen Quad vertex data (for debugging)
	GLuint QuadVertexArray;
	GLuint QuadVertexBuffer;


	/////////////////
	// Camera Data //
	/////////////////

	// Previous frame start time (for time-based movement)
	float t0 = 0;

	vec3 cameraLookAt;

	float cTheta = - 3.14159f / 2.f;
	float cPhi = 0;
	bool mouseDown = false;

	double lastX = 0;
	double lastY = 0;
	float cameraRotateSpeed = 0.005f;

	bool moveForward = false;
	bool moveBack = false;
	bool moveLeft = false;
	bool moveRight = false;
	glm::vec3 cameraPos;
	float cameraMoveSpeed = 12.0f;


	/////////////////////
	// Event Callbacks //
	/////////////////////

	void mouseCallback(GLFWwindow* window, int but, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			mouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			mouseDown = false;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (mouseDown)
		{
			cTheta += (float) (xpos - lastX) * cameraRotateSpeed;
			cPhi -= (float) (ypos - lastY) * cameraRotateSpeed;
		}

		lastX = xpos;
		lastY = ypos;
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			moveForward = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_S:
			moveBack = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_A:
			moveLeft = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_D:
			moveRight = (action != GLFW_RELEASE);
			break;

		case GLFW_KEY_K:
			SHOW_LIGHT_COLOR = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_L:
			SHOW_LIGHT_DEPTH = (action != GLFW_RELEASE);
			break;
		};

		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_1:
				cameraMoveSpeed = 1.f;
				break;
			case GLFW_KEY_2:
				cameraMoveSpeed = 3.f;
				break;
			case GLFW_KEY_3:
				cameraMoveSpeed = 6.f;
				break;
			case GLFW_KEY_4:
				cameraMoveSpeed = 12.f;
				break;
			case GLFW_KEY_5:
				cameraMoveSpeed = 24.f;
				break;

			case GLFW_KEY_Q:
				g_light.x += 0.25;
				break;
			case GLFW_KEY_E:
				g_light.x -= 0.25;
				break;
			}
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void resizeCallback(GLFWwindow* window, int w, int h)
	{
	}



	///////////////////////
	// Geometry Creation //
	///////////////////////

	// Create Geometry
	void initGeom()
	{
		initGround();
		initQuad();
	}

	// Create the ground plane
	void initGround()
	{
		const float groundSize = 20;
		const float groundY = -1.5;

		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		const float GrndPos[] =
		{
			-groundSize, groundY, -groundSize,
			-groundSize, groundY,  groundSize,
			 groundSize, groundY,  groundSize,
			 groundSize, groundY, -groundSize,
		};

		const float GrndNorm[] =
		{
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
		};

		const float GrndTex[] =
		{
			0, 0, // back
			0, 1,
			1, 1,
			1, 0,
		};

		unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };

		CHECKED_GL_CALL(glGenVertexArrays(1, &GroundVertexArray));
		CHECKED_GL_CALL(glBindVertexArray(GroundVertexArray));

		GLuint GroundPositionBuffer, GroundNormalBuffer, GroundTexCoordBuffer, GroundIndexBuffer;

		CHECKED_GL_CALL(glGenBuffers(1, &GroundPositionBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, GroundPositionBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW));
		CHECKED_GL_CALL(glEnableVertexAttribArray(0));
		CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));

		CHECKED_GL_CALL(glGenBuffers(1, &GroundNormalBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, GroundNormalBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW));
		CHECKED_GL_CALL(glEnableVertexAttribArray(1));
		CHECKED_GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));

		CHECKED_GL_CALL(glGenBuffers(1, &GroundTexCoordBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, GroundTexCoordBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW));
		CHECKED_GL_CALL(glEnableVertexAttribArray(2));
		CHECKED_GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));

		GroundIndexCount = 6;
		CHECKED_GL_CALL(glGenBuffers(1, &GroundIndexBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundIndexBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW));

		CHECKED_GL_CALL(glBindVertexArray(0));
	}

	// geometry set up for a quad
	void initQuad()
	{
		// now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &QuadVertexArray);
		glBindVertexArray(QuadVertexArray);

		const GLfloat g_quad_vertex_buffer_data[] =
		{
			-1.0f, -1.0f,  0.0f,
			 1.0f, -1.0f,  0.0f,
			-1.0f,  1.0f,  0.0f,
			-1.0f,  1.0f,  0.0f,
			 1.0f, -1.0f,  0.0f,
			 1.0f,  1.0f,  0.0f,
		};

		glGenBuffers(1, &QuadVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, QuadVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

		glBindVertexArray(0);
	}



	///////////
	// Setup //
	///////////

	// set up the FBO for the light's depth
	void initShadow()
	{
		// generate the FBO for the shadow depth
		glGenFramebuffers(1, &ShadowMapFBO);

		// generate the texture
		glGenTextures(1, &ShadowMapDepthTexture);
		glBindTexture(GL_TEXTURE_2D, ShadowMapDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapWidth, ShadowMapHeight,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// bind with framebuffer's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMapDepthTexture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void init(string const & RESOURCE_DIR)
	{
		GLSL::checkVersion();

		// Set background color
		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
		// Enable z-buffer test
		glEnable(GL_DEPTH_TEST);

		// Initialize mesh
		shape = make_shared<Shape>();
		shape->loadMesh(RESOURCE_DIR + "dog.obj");
		shape->resize();
		shape->init();

		world = make_shared<Shape>();
		world->loadMesh(RESOURCE_DIR + "sphere.obj");
		world->resize();
		world->init();

		// Initialize the GLSL programs
		DepthProg = make_shared<Program>();
		DepthProg->setVerbose(true);
		DepthProg->setShaderNames(RESOURCE_DIR + "depth_vert.glsl", RESOURCE_DIR + "depth_frag.glsl");
		DepthProg->init();

		DepthProgDebug = make_shared<Program>();
		DepthProgDebug->setVerbose(true);
		DepthProgDebug->setShaderNames(RESOURCE_DIR + "depth_vertDebug.glsl", RESOURCE_DIR + "depth_fragDebug.glsl");
		DepthProgDebug->init();

		ShadowProg = make_shared<Program>();
		ShadowProg->setVerbose(true);
		ShadowProg->setShaderNames(RESOURCE_DIR + "shadow_vert.glsl", RESOURCE_DIR + "shadow_frag.glsl");
		ShadowProg->init();

		DebugProg = make_shared<Program>();
		DebugProg->setVerbose(true);
		DebugProg->setShaderNames(RESOURCE_DIR + "pass_vert.glsl", RESOURCE_DIR + "pass_texfrag.glsl");
		DebugProg->init();

		////////////////////////
		// Intialize textures //
		////////////////////////

		texture0 = make_shared<Texture>();
		texture0->setFilename(RESOURCE_DIR + "crate.jpg");
		texture0->init();
		texture0->setUnit(0);
		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture1 = make_shared<Texture>();
		texture1->setFilename(RESOURCE_DIR + "world.jpg");
		texture1->init();
		texture1->setUnit(0);
		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture2 = make_shared<Texture>();
		texture2->setFilename(RESOURCE_DIR + "grass.jpg");
		texture2->init();
		texture2->setUnit(0);
		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Add uniform and attributes to each of the programs
		DepthProg->addUniform("LP");
		DepthProg->addUniform("LV");
		DepthProg->addUniform("M");
		DepthProg->addAttribute("vertPos");
		// un-needed, but easier then modifying shape
		DepthProg->addAttribute("vertNor");
		DepthProg->addAttribute("vertTex");

		DepthProgDebug->addUniform("LP");
		DepthProgDebug->addUniform("LV");
		DepthProgDebug->addUniform("M");
		DepthProgDebug->addAttribute("vertPos");
		// un-needed, but easier then modifying shape
		DepthProgDebug->addAttribute("vertNor");
		DepthProgDebug->addAttribute("vertTex");
		DepthProgDebug->addUniform("Texture0");

		ShadowProg->addUniform("P");
		ShadowProg->addUniform("M");
		ShadowProg->addUniform("V");
		ShadowProg->addUniform("LS");
		ShadowProg->addUniform("lightDir");
		ShadowProg->addAttribute("vertPos");
		ShadowProg->addAttribute("vertNor");
		ShadowProg->addAttribute("vertTex");
		ShadowProg->addUniform("Texture0");
		ShadowProg->addUniform("shadowDepth");

		DebugProg->addUniform("texBuf");
		DebugProg->addAttribute("vertPos");

		initShadow();
	}



	////////////////
	// Transforms //
	////////////////

	void SetProjectionMatrix(shared_ptr<Program> curShade)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float) height;
		mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 100.0f);
		CHECKED_GL_CALL(glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(Projection)));
	}

	/* TODO fix */
	mat4 SetOrthoMatrix(shared_ptr<Program> curShade)
	{
		mat4 ortho = mat4(1.0);

		// fill in the glUniform call to send to the right shader!

		return ortho;
	}

	/* camera controls - do not change */
	void SetView(shared_ptr<Program> curShade)
	{
		mat4 Cam = glm::lookAt(cameraPos, cameraLookAt, vec3(0, 1, 0));
		CHECKED_GL_CALL(glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam)));
	}

	/* TODO fix */
	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up)
	{
		mat4 Cam = mat4(1.0);

		// fill in the glUniform call to send to the right shader!

		return Cam;
	}

	/* model transforms */
	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS)
	{
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 ctm = Trans;
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)));
	}

	void SetModel(mat4 ctm, shared_ptr<Program> curS)
	{
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)));
	}



	////////////
	// Render //
	////////////

	/*
	Draw the dog, sphere and ground plane
	Textures can be turned on an off (as shadow map depth drawing does not need textures)
	*/
	void drawScene(shared_ptr<Program> shader, GLint texID, int TexOn)
	{
		if (TexOn)
		{
			texture0->bind(texID);
		}
		//draw the dog mesh
		SetModel(vec3(-1, 0, -5), 0, 0, 1, shader);
		shape->draw(shader);

		if (TexOn)
		{
			texture1->bind(texID);
		}
		//draw the world sphere
		SetModel(vec3(1, 0, -5), 0, 0, 1, shader);
		world->draw(shader);

		if (TexOn)
		{
			texture2->bind(texID);
		}

		//draw the ground plane
		SetModel(vec3(0, -1, 0), 0, 0, 1, shader);
		CHECKED_GL_CALL(glBindVertexArray(GroundVertexArray));
		CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, GroundIndexCount, GL_UNSIGNED_SHORT, 0));
		CHECKED_GL_CALL(glBindVertexArray(0));
	}

	void render_UpdateCamera()
	{
		float t1 = (float) glfwGetTime();

		float const dT = (t1 - t0);
		t0 = t1;

		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 forward = glm::vec3(cos(cTheta) * cos(cPhi), sin(cPhi), sin(cTheta) * cos(cPhi));
		glm::vec3 right = glm::cross(forward, up);

		if (moveForward)
			cameraPos += forward * cameraMoveSpeed * dT;
		if (moveBack)
			cameraPos -= forward * cameraMoveSpeed * dT;
		if (moveLeft)
			cameraPos -= right * cameraMoveSpeed * dT;
		if (moveRight)
			cameraPos += right * cameraMoveSpeed * dT;

		cameraLookAt = cameraPos + forward;
	}

	glm::mat4 render_ShadowMap()
	{
		mat4 L;

		// set up light's depth map
		glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		// set up shadow shader
		// render scene
		DepthProg->bind();

		// TODO you will need to fix these to return correct matrices
		mat4 LO = SetOrthoMatrix(DepthProg);
		mat4 LV = SetLightView(DepthProg, g_light, vec3(0, 0, 0), vec3(0, 1, 0));
		drawScene(DepthProg, 0, 0);
		DepthProg->unbind();
		glCullFace(GL_BACK);

		L = LO * LV;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return L;
	}

	/* let's draw */
	void render()
	{
		render_UpdateCamera();
		mat4 L = render_ShadowMap();


		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (SHOW_LIGHT_COLOR)
		{
			// geometry style debug on light - test transforms, draw geometry from light
			// perspective
			DepthProgDebug->bind();
			// render scene from light's point of view
			SetOrthoMatrix(DepthProgDebug);
			SetLightView(DepthProgDebug, g_light, vec3(0, 0, 0), vec3(0, 1, 0));
			drawScene(DepthProgDebug, DepthProgDebug->getUniform("Texture0"), 1);
			DepthProgDebug->unbind();
		}
		else if (SHOW_LIGHT_DEPTH)
		{
			/* code to draw the light depth buffer */
			// actually draw the light depth map
			DebugProg->bind();
			CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
			CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, ShadowMapDepthTexture));
			CHECKED_GL_CALL(glUniform1i(DebugProg->getUniform("texBuf"), 0));
			CHECKED_GL_CALL(glBindVertexArray(QuadVertexArray));
			CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
			DebugProg->unbind();
		}
		else
		{
			// now render the scene like normal
			// set up shadow shader
			ShadowProg->bind();

			/* TODO: also set up light depth map */

			CHECKED_GL_CALL(glUniform3f(ShadowProg->getUniform("lightDir"), g_light.x, g_light.y, g_light.z));

			// view/proj matrices
			SetProjectionMatrix(ShadowProg);
			SetView(ShadowProg);

			// TODO: is there other uniform data that must be sent?

			drawScene(ShadowProg, ShadowProg->getUniform("Texture0"), 1);
			ShadowProg->unbind();
		}
	}

};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources/";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1024, 768);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
