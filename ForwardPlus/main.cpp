#include "Window.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "ResourcesLoader.h"
#include "Random.h"
#include "Quad.h"
#include "Shader.h"
#include "Camera.h"
#include "Cube.h"
#include "Sphere.h"
#include "Utils.h"
#include "TextRenderer.h"
#include "Model.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <iostream>

int main()
{
	unsigned int width = 1280;
	unsigned int height = 720;

	Window window;
	window.Init(width, height);

	TextRenderer textRenderer;
	textRenderer.Init("Data/Fonts/arial_sdf.fnt", "Data/Fonts/arial_sdf.png");
	textRenderer.Resize(width, height);

	Camera camera;
	camera.SetProjectionMatrix(75.0f, (float)width / height, 0.25f, 100.0f);
	camera.SetPos(glm::vec3(0.0f, 0.0f, 0.0f));
	camera.SetMoveSpeed(8.0f);

	Profiler &profiler = window.GetProfiler();

	Shader frustumShader;
	frustumShader.Load("Data/Shaders/Fplus/frustum.comp");
	Shader lightCullingShader;
	lightCullingShader.Load("Data/Shaders/Fplus/light_cull.comp");

	unsigned int lightTileSize = 16;
	unsigned int numTiles = (unsigned int)(std::ceil((float)width / lightTileSize) * std::ceil((float)height / lightTileSize));			// Also the number of frustums
	unsigned int numWorkGroupsX = (unsigned int)std::ceil((float)width / lightTileSize / 16);
	unsigned int numWorkGroupsY = (unsigned int)std::ceil((float)height / lightTileSize / 16);

	glm::uvec2 numFrustums = glm::uvec2((unsigned int)std::ceil((float)width / lightTileSize), (unsigned int)std::ceil((float)height / lightTileSize));

	struct ShaderPlane
	{
		glm::vec3 normal;
		float d;
	};
	struct ShaderFrustum
	{
		ShaderPlane planes[4];
	};

	struct Light
	{
		//unsigned int type;
		glm::vec4 positionRadiusWS;
		glm::vec4 positionRadiusVS;
		glm::vec4 color;
		//float radius;
	};
	Model trashCan;
	trashCan.Load("Data/Models/crytek-sponza/sponza1.obj");

	std::vector<Light> lights(1024);

	for (size_t i = 0; i < lights.size(); i++)
	{
		Light &l = lights[i];
		l.positionRadiusWS = glm::vec4(Random::Float(-8.0f, 8.0f), Random::Float(0.0f, 10.0f), Random::Float(-8.0f, 8.0f), 2.0f);
		l.color = glm::vec4(Random::Float(), Random::Float(), Random::Float(), 1.0f);
	}

	//Light l;
	//l.positionVS = glm::vec4(0.0f, 0.0f, 0.0f, 2.0f);
	//l.radius = 5.0f;
	//l.type = 0;

	GLuint lightsSSBO;
	glCreateBuffers(1, &lightsSSBO);
	glNamedBufferStorage(lightsSSBO, sizeof(Light) * lights.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint frustumsSSBO;
	glCreateBuffers(1, &frustumsSSBO);
	glNamedBufferStorage(frustumsSSBO, sizeof(ShaderFrustum) * numTiles, nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint avgLightsPerTile = 256;

	GLuint opaqueLightIndexListSSBO;
	glCreateBuffers(1, &opaqueLightIndexListSSBO);
	glNamedBufferStorage(opaqueLightIndexListSSBO, avgLightsPerTile * numTiles * sizeof(GLuint), nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint opaqueLightIndexCounterSSBO;
	glCreateBuffers(1, &opaqueLightIndexCounterSSBO);
	glNamedBufferStorage(opaqueLightIndexCounterSSBO, sizeof(GLuint), nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint opaqueLightGridTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &opaqueLightGridTexture);
	glTextureParameteri(opaqueLightGridTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(opaqueLightGridTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(opaqueLightGridTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(opaqueLightGridTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(opaqueLightGridTexture, 1, GL_RG32UI, (GLsizei)std::ceil((float)width / lightTileSize), (GLsizei)std::ceil((float)height / lightTileSize));

	GLuint debugTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &debugTexture);
	glTextureParameteri(debugTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(debugTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(debugTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(debugTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(debugTexture, 1, GL_R16F, (GLsizei)std::ceil((float)width / lightTileSize), (GLsizei)std::ceil((float)height / lightTileSize));

	GLuint depthPrePassTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &depthPrePassTexture);
	glTextureParameteri(depthPrePassTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthPrePassTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthPrePassTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(depthPrePassTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(depthPrePassTexture, 1, GL_DEPTH_COMPONENT24, width, height);

	GLuint depthPrePassFB;
	glCreateFramebuffers(1, &depthPrePassFB);
	glNamedFramebufferTexture(depthPrePassFB, GL_DEPTH_ATTACHMENT, depthPrePassTexture, 0);
	glNamedFramebufferDrawBuffer(depthPrePassFB, GL_NONE);
	glNamedFramebufferReadBuffer(depthPrePassFB, GL_NONE);

	if (glCheckNamedFramebufferStatus(depthPrePassFB, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Failed to create framebuffer\n";

	

	Shader depthPrePassShader;
	depthPrePassShader.Load("Data/Shaders/Fplus/depth_prepass.vert", "Data/Shaders/Fplus/depth_prepass.frag");
	Shader modelShader;
	modelShader.Load("Data/Shaders/Fplus/model.vert", "Data/Shaders/Fplus/model.frag");
	Shader modelForwardShader;
	modelForwardShader.Load("Data/Shaders/Fplus/model.vert", "Data/Shaders/Fplus/model_forward.frag");

	Shader debugQuadShader;
	debugQuadShader.Load("Data/Shaders/Fplus/debug_quad.vert", "Data/Shaders/Fplus/debug_quad.frag");

	Quad q;
	q.Load();
	
	//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

	// To reset the ac
	// Could also use double buffering for the buffers
	/*std::cout << glGetError() << '\n';
	glInvalidateBufferData(opaqueLightIndexCounter);
	unsigned int data = 0;
	std::cout << glGetError() << '\n';
	glClearNamedBufferData(opaqueLightIndexCounter, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);		// Use red_integer for non-normalized integer source data format.
	std::cout << glGetError() << '\n';*/

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	bool fullscreenDebugQuad = false;
	bool useForward = false;

	while (!window.ShouldClose())
	{
		window.Update();
		if (window.WasResized())
		{
			width = window.GetWidth();
			height = window.GetHeight();
			glViewport(0, 0, width, height);
		}

		if (Input::MouseMoved() && Input::IsMouseButtonDown(1))
			camera.Look();
		else if (!Input::IsMouseButtonDown(1))
			camera.firstMouse = true;

		if (Input::IsKeyPressed(GLFW_KEY_W))
			camera.Move(FORWARD, window.GetDeltaTime());
		if (Input::IsKeyPressed(GLFW_KEY_S))
			camera.Move(BACKWARD, window.GetDeltaTime());
		if (Input::IsKeyPressed(GLFW_KEY_A))
			camera.Move(LEFT, window.GetDeltaTime());
		if (Input::IsKeyPressed(GLFW_KEY_D))
			camera.Move(RIGHT, window.GetDeltaTime());

		const glm::mat4 &view = camera.GetViewMatrix();
		glm::mat4 t;
		for (size_t i = 0; i < lights.size(); i++)
		{
			Light &l = lights[i];
			//l.positionRadiusWS = glm::vec4(float(i * 1.2f) * std::sin(window.GetElapsedTime()), 0.75f, std::cos(window.GetElapsedTime()) * 1.5f * float(i), 2.0f);
			l.positionRadiusWS.y += window.GetDeltaTime() * 4.0f;
			if (l.positionRadiusWS.y > 10.0f)
				l.positionRadiusWS.y = Random::Float(-4.0f, 4.0f);

			t = view * glm::translate(glm::mat4(1.0f), glm::vec3(l.positionRadiusWS.x, l.positionRadiusWS.y, l.positionRadiusWS.z));
			l.positionRadiusVS = t[3];
			l.positionRadiusVS.w = 2.0f;
		}

		glNamedBufferSubData(lightsSSBO, 0, sizeof(Light) * lights.size(), lights.data());

		glm::mat4 modelTransform;
		modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

		if (!useForward)
		{
			profiler.BeginQuery("Depth Prepass");

			glBindFramebuffer(GL_FRAMEBUFFER, depthPrePassFB);
			glClear(GL_DEPTH_BUFFER_BIT);

			depthPrePassShader.Use();
			depthPrePassShader.SetMat4("proj", camera.GetProjectionMatrix());
			depthPrePassShader.SetMat4("view", camera.GetViewMatrix());
			depthPrePassShader.SetMat4("modelMatrix", modelTransform);

			trashCan.Render();

			profiler.EndQuery();

			profiler.BeginQuery("Frustums dispatch");

			glInvalidateBufferData(opaqueLightIndexCounterSSBO);
			unsigned int data = 0;
			glClearNamedBufferData(opaqueLightIndexCounterSSBO, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);		// Use red_integer for non-normalized integer source data format.

			//unsigned int clearImage[2] = { 0,0 };
			//glClearTexImage(opaqueLightGridTexture, 0, GL_RG_INTEGER, GL_UNSIGNED_INT, clearImage);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, frustumsSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, opaqueLightIndexListSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, opaqueLightIndexCounterSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lightsSSBO);
			glBindImageTexture(4, opaqueLightGridTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32UI);
			glBindImageTexture(6, debugTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
			glBindTextureUnit(5, depthPrePassTexture);

			frustumShader.Use();
			frustumShader.SetMat4("invProj", glm::inverse(camera.GetProjectionMatrix()));
			frustumShader.SetVec2("screenRes", glm::vec2(width, height));
			frustumShader.SetUVec2("numFrustums", numFrustums);

			glDispatchCompute(numWorkGroupsX, numWorkGroupsY, 1);

			// We intend to read the buffer in the light culling shader
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			profiler.EndQuery();
			profiler.BeginQuery("Light culling");

			glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
			glm::mat4 lightP = camera.GetViewMatrix() * m;

			lightCullingShader.Use();
			lightCullingShader.SetMat4("invProj", glm::inverse(camera.GetProjectionMatrix()));
			lightCullingShader.SetMat4("proj", camera.GetProjectionMatrix());
			lightCullingShader.SetUint("numLights", unsigned int(lights.size()));
			lightCullingShader.SetVec2("screenRes", glm::vec2((float)width, (float)height));

			glDispatchCompute((GLuint)std::ceil((float)width / lightTileSize), (GLuint)std::ceil((float)height / lightTileSize), 1);

			// We're going to read the light indices buffer and acces the light grid image
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			profiler.EndQuery();
		}

		profiler.BeginQuery("Final shading");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (useForward)
		{
			modelForwardShader.Use();
			modelForwardShader.SetMat4("proj", camera.GetProjectionMatrix());
			modelForwardShader.SetMat4("view", camera.GetViewMatrix());
			modelForwardShader.SetMat4("modelMatrix", modelTransform);
			modelForwardShader.SetUint("numLights", lights.size());
		}
		else
		{
			modelShader.Use();
			modelShader.SetMat4("proj", camera.GetProjectionMatrix());
			modelShader.SetMat4("view", camera.GetViewMatrix());
			modelShader.SetMat4("modelMatrix", modelTransform);
		}

		

		trashCan.Render();		

		profiler.EndQuery();
		profiler.BeginQuery("Debug quad");

		debugQuadShader.Use();

		if (fullscreenDebugQuad)
			debugQuadShader.SetVec2("transScale", glm::vec2(0.0f, 1.0f));
		else
			debugQuadShader.SetVec2("transScale", glm::vec2(-0.66f, 0.3f));

		q.Render();

		profiler.EndQuery();


		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::Checkbox("Fullscreen debug quad", &fullscreenDebugQuad);
		ImGui::Checkbox("Use Forward", &useForward);
		ImGui::Text("Press WASD to move");
		ImGui::Text("Right click and move the mouse to look around");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		profiler.EndQuery();

		profiler.BeginQuery("Text");
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);

		textRenderer.AddText("CPU time: " + std::to_string(window.GetDeltaTime() * 1000.0f) + " ms", glm::vec2(30.0f, 460.0f), glm::vec2(0.3f, 0.3f));
		textRenderer.AddText(profiler.GetResults(), glm::vec2(30.0f, 420.0f), glm::vec2(0.3f, 0.3f));
		textRenderer.Render();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		profiler.EndQuery();
		profiler.EndFrame();

		window.SwapBuffers();
	}

	window.Dispose();

	return 0;
}
