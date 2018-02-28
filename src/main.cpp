/* Base code for shadow mapping lab */
/* This code is incomplete - follow tasks listed in handout */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <random>
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
	shared_ptr<Program> SceneProg;
	shared_ptr<Program> SSAOProg;

	// Shapes
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> dog;
	shared_ptr<Shape> dragon;
	shared_ptr<Shape> stairs;

	// Debug Settings
	bool ShowSceneColor = false;
	bool ShowSceneNormals = false;
	bool ShowSceneDepth = false;

	GLuint SceneFBO;
	GLuint SceneColorTexture;
	GLuint SceneNormalsTexture;
	GLuint SceneDepthTexture;

	GLuint SSAONoiseTexture;
	vector<float> ssaoKernel;

	vec3 g_light = vec3(1, 1, 1);
	int g_width = -1;
	int g_height = -1;

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
	vec3 cameraPos = vec3(0, 0, 8);
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
			cPhi = glm::max(cPhi, -3.1415f / 2.f);
			cPhi = glm::min(cPhi, 3.1415f / 2.f);
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

		case GLFW_KEY_J:
			ShowSceneColor = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_K:
			ShowSceneNormals = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_L:
			ShowSceneDepth = (action != GLFW_RELEASE);
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
		CHECKED_GL_CALL(glViewport(0, 0, g_width = w, g_height = h));
		setGBufferTextureSize(g_width, g_height);
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

		unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };

		CHECKED_GL_CALL(glGenVertexArrays(1, &GroundVertexArray));
		CHECKED_GL_CALL(glBindVertexArray(GroundVertexArray));

		GLuint GroundPositionBuffer, GroundNormalBuffer, GroundIndexBuffer;

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

	void setGBufferTextureSize(const int width, const int height)
	{
		glBindTexture(GL_TEXTURE_2D, SceneColorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, SceneNormalsTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, SceneDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	// set up the FBO for GBuffer (scene colors, normals, and a depth texture)
	void initGBuffer()
	{
		// generate the FBO for the shadow depth
		glGenFramebuffers(1, &SceneFBO);

		// generate the color texture
		glGenTextures(1, &SceneColorTexture);
		glBindTexture(GL_TEXTURE_2D, SceneColorTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// generate the normals texture
		glGenTextures(1, &SceneNormalsTexture);
		glBindTexture(GL_TEXTURE_2D, SceneNormalsTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// generate the depth texture
		glGenTextures(1, &SceneDepthTexture);
		glBindTexture(GL_TEXTURE_2D, SceneDepthTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		setGBufferTextureSize(g_width, g_height);

		// bind with framebuffer's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, SceneFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SceneColorTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, SceneNormalsTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, SceneDepthTexture, 0);

		const GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		uint Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (Status != GL_FRAMEBUFFER_COMPLETE)
		{
			cerr << "Failed to properly create framebuffer!" << endl;

			string Problem;
			switch (Status)
			{
			case GL_FRAMEBUFFER_UNDEFINED:
				Problem = "Undefined - the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist.";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				Problem = "Incomplete attachment - one or more of the framebuffer attachment points are framebuffer incomplete.";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				Problem = "Incomplete missing attachment - the framebuffer does not have at least one image attached to it.";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				Problem = "Incomplete draw buffer - the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for one or more color attachment point(s) named by GL_DRAW_BUFFERi.";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				Problem = "Incomplete read buffer - GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.";
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				Problem = "Unsupported - the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				Problem = "Incomplete multisample.";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				Problem = "Incomplete layer targets - one or more framebuffer attachments is layered, and one or more populated attachment is not layered, or all populated color attachments are not from textures of the same target.";
				break;
			default:
				Problem = "Unknown.";
				break;
			}

			cerr << "Framebuffer problem: " << Problem << endl;
			exit(1);
		}

		CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void init(string const & RESOURCE_DIR)
	{
		GLSL::checkVersion();

		glfwGetFramebufferSize(windowManager->getHandle(), &g_width, &g_height);

		// Set background color
		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
		// Enable z-buffer test
		glEnable(GL_DEPTH_TEST);

		// Initialize mesh
		dog = make_shared<Shape>();
		dog->loadMesh(RESOURCE_DIR + "dog.obj");
		dog->resize();
		dog->init();

		sphere = make_shared<Shape>();
		sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
		sphere->resize();
		sphere->init();

		dragon = make_shared<Shape>();
		dragon->loadMesh(RESOURCE_DIR + "dragon10k.obj");
		dragon->resize();
		dragon->init();

		stairs = make_shared<Shape>();
		stairs->loadMesh(RESOURCE_DIR + "staircase.obj");
		stairs->resize();
		stairs->init();

		// Initialize the GLSL programs

		SceneProg = make_shared<Program>();
		SceneProg->setVerbose(true);
		SceneProg->setShaderNames(RESOURCE_DIR + "scene_vert.glsl", RESOURCE_DIR + "scene_frag.glsl");
		if (! SceneProg->init())
		{
			exit(1);
		}

		SSAOProg = make_shared<Program>();
		SSAOProg->setVerbose(true);
		SSAOProg->setShaderNames(RESOURCE_DIR + "ssao_vert.glsl", RESOURCE_DIR + "ssao_frag.glsl");
		if (! SSAOProg->init())
		{
			exit(1);
		}

		////////////////////////
		// Intialize textures //
		////////////////////////

		SceneProg->addUniform("P");
		SceneProg->addUniform("M");
		SceneProg->addUniform("V");
		SceneProg->addUniform("lightDir");
		SceneProg->addAttribute("vertPos");
		SceneProg->addAttribute("vertNor");
		SceneProg->addUniform("materialColor");

		SSAOProg->addUniform("sceneColorTex");
		SSAOProg->addUniform("sceneNormalsTex");
		SSAOProg->addUniform("sceneDepthTex");
		SSAOProg->addUniform("noiseTex");
		SSAOProg->addUniform("uMode");
		SSAOProg->addUniform("P");
		SSAOProg->addUniform("sampleVectors");
		SSAOProg->addAttribute("vertPos");

		initGBuffer();
		initSSAOKernel();
	}

	// simple linear interpolation helper function
	static float lerp(float const a, float const b, float const f)
	{
		return a + f * (b - a);
	}

	// set up two sources of randomness for our SSAO implementation:
	// 1) A set of random sample vectors within a unit hemisphere oriented +Z
	// 2) A set of random 2D vectors that can be used to randomly rotate the hemisphere once aligned along the normal
	void initSSAOKernel()
	{
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;

		// Sample kernel - random vectors within unit hemisphere
		int const numSamples = 64; // If you change this, you must update the shader as well!
		for (uint i = 0; i < numSamples; ++i)
		{
			float const a = randomFloats(generator);
			float const b = randomFloats(generator);
			float const c = randomFloats(generator);

			float const VerticalBias = 0.1f;
			vec3 sample = vec3(a * 2 - 1, b * 2 - 1, c * (1 - VerticalBias) + VerticalBias);
			sample = normalize(sample);
			sample *= randomFloats(generator);

			// Scale samples s.t. they're more aligned to center of kernel
			float scale = float(i) / numSamples;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample.x);
			ssaoKernel.push_back(sample.y);
			ssaoKernel.push_back(sample.z);
		}

		// Noise texture - random vec2s for hemisphere orientation
		vector<float> NoiseData;
		uint const NoiseTexSize = 4;
		for (uint i = 0; i < NoiseTexSize * NoiseTexSize; i++)
		{
			NoiseData.push_back(randomFloats(generator) * 2 - 1);
			NoiseData.push_back(randomFloats(generator) * 2 - 1);
		}

		CHECKED_GL_CALL(glGenTextures(1, &SSAONoiseTexture));
		CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, SSAONoiseTexture));
		CHECKED_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, NoiseTexSize, NoiseTexSize, 0, GL_RG, GL_FLOAT, NoiseData.data()));
		CHECKED_GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
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

	void SetView(shared_ptr<Program> curShade)
	{
		mat4 Cam = glm::lookAt(cameraPos, cameraLookAt, vec3(0, 1, 0));
		CHECKED_GL_CALL(glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam)));
	}

	void SetModel(vec3 trans, float rotY, float sc, shared_ptr<Program> curS)
	{
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 Rot = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 Scale = glm::scale(glm::mat4(1.0f), vec3(sc));
		mat4 ctm = Trans * Rot * Scale;
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)));
	}



	////////////
	// Render //
	////////////

	// Draw the dog, sphere, dragon, and stairs and ground plane
	void DrawScene()
	{
		SceneProg->bind();

		SetProjectionMatrix(SceneProg);
		SetView(SceneProg);

		CHECKED_GL_CALL(glUniform3f(SceneProg->getUniform("lightDir"), g_light.x, g_light.y, g_light.z));

		// draw the dog mesh
		SetModel(vec3(-1, -0.75f, 0), 0, 1, SceneProg);
		CHECKED_GL_CALL(glUniform3f(SceneProg->getUniform("materialColor"), 0.8f, 0.2f, 0.2f));
		dog->draw(SceneProg);

		// draw the sphere mesh
		SetModel(vec3(1, -1, 0), 0, 1, SceneProg);
		CHECKED_GL_CALL(glUniform3f(SceneProg->getUniform("materialColor"), 0.2f, 0.2f, 0.8f));
		sphere->draw(SceneProg);

		// draw the dragon mesh
		SetModel(vec3(-1, 0.5f, -4), radians(90.f), 3, SceneProg);
		CHECKED_GL_CALL(glUniform3f(SceneProg->getUniform("materialColor"), 0.2f, 0.8f, 0.8f));
		dragon->draw(SceneProg);

		// draw the stairs mesh
		SetModel(vec3(1, 1, -9), radians(180.f), 3, SceneProg);
		CHECKED_GL_CALL(glUniform3f(SceneProg->getUniform("materialColor"), 0.8f, 0.6f, 0.2f));
		stairs->draw(SceneProg);

		// draw the ground plane
		SetModel(vec3(0, 0, 0), 0, 1, SceneProg);
		CHECKED_GL_CALL(glUniform3f(SceneProg->getUniform("materialColor"), 0.2f, 0.8f, 0.2f));
		CHECKED_GL_CALL(glBindVertexArray(GroundVertexArray));
		CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, GroundIndexCount, GL_UNSIGNED_SHORT, 0));
		CHECKED_GL_CALL(glBindVertexArray(0));

		SceneProg->unbind();
	}

	void UpdateCamera()
	{
		float t1 = (float) glfwGetTime();

		float const dT = (t1 - t0);
		t0 = t1;

		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 forward = glm::vec3(cos(cTheta) * cos(cPhi), sin(cPhi), sin(cTheta) * cos(cPhi));
		glm::vec3 right = glm::normalize(glm::cross(forward, up));

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
	
	/* let's draw */
	void render()
	{
		UpdateCamera();


		// First render the scene into the gbuffer

		CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, SceneFBO));
		CHECKED_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		DrawScene();


		// Now do the SSAO pass - copy gbuffer values to screen + plus do SSAO calculation

		CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		CHECKED_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		SSAOProg->bind();
		CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
		CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, SceneColorTexture));
		CHECKED_GL_CALL(glUniform1i(SSAOProg->getUniform("sceneColorTex"), 0));

		CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE1));
		CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, SceneNormalsTexture));
		CHECKED_GL_CALL(glUniform1i(SSAOProg->getUniform("sceneNormalsTex"), 1));

		CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE2));
		CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, SceneDepthTexture));
		CHECKED_GL_CALL(glUniform1i(SSAOProg->getUniform("sceneDepthTex"), 2));

		CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE3));
		CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, SSAONoiseTexture));
		CHECKED_GL_CALL(glUniform1i(SSAOProg->getUniform("noiseTex"), 3));

		int PassMode = 0;

		if (ShowSceneColor)
		{
			PassMode = 1;
		}
		else if (ShowSceneNormals)
		{
			PassMode = 2;
		}
		else if (ShowSceneDepth)
		{
			PassMode = 3;
		}

		CHECKED_GL_CALL(glUniform1i(SSAOProg->getUniform("uMode"), PassMode));
		CHECKED_GL_CALL(glUniform3fv(SSAOProg->getUniform("sampleVectors"), (int) ssaoKernel.size() / 3, ssaoKernel.data()));

		SetProjectionMatrix(SSAOProg); // We need the projection matrix for a few things

		CHECKED_GL_CALL(glBindVertexArray(QuadVertexArray));
		CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
		SSAOProg->unbind();
	}

};

int main(int argc, char **argv)
{
	std::string resourceDir = "../resources/";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1024, 768);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom();

	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		application->render();
		glfwSwapBuffers(windowManager->getHandle());
		glfwPollEvents();
	}

	windowManager->shutdown();
	return 0;
}
