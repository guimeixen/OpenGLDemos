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

	Quad q;
	q.Load();

	Shader fxaaShader;
	fxaaShader.Load("Data/Shaders/FXAA/quad.vert", "Data/Shaders/FXAA/fxaa.frag");
	Shader modelShader;
	modelShader.Load("Data/Shaders/FXAA/model.vert", "Data/Shaders/FXAA/model.frag");

	Model trashCan;
	trashCan.Load("Data/Models/trash_can.obj");

	GLuint sceneTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &sceneTexture);
	glTextureParameteri(sceneTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(sceneTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(sceneTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(sceneTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(sceneTexture, 1, GL_RGBA8, width, height);

	GLuint rbo;
	glCreateRenderbuffers(1, &rbo);
	glNamedRenderbufferStorage(rbo, GL_DEPTH_COMPONENT24, width, height);

	GLuint sceneFB;
	glCreateFramebuffers(1, &sceneFB);
	glNamedFramebufferTexture(sceneFB, GL_COLOR_ATTACHMENT0, sceneTexture, 0);
	glNamedFramebufferRenderbuffer(sceneFB, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckNamedFramebufferStatus(sceneFB, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Failed to create framebuffer\n";

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	bool enableFXAA = true;

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

		glBindFramebuffer(GL_FRAMEBUFFER, sceneFB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		profiler.BeginQuery("Model render");

		modelShader.Use();
		modelShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		modelShader.SetMat4("modelMatrix", glm::mat4(1.0f));
		trashCan.Render();

		profiler.EndQuery();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		profiler.BeginQuery("FXAA");

		fxaaShader.Use();
		fxaaShader.SetVec2("invScreenRes", glm::vec2(1.0f / width, 1.0f / height));
		fxaaShader.SetBool("enableFXAA", enableFXAA);
		glBindTextureUnit(0, sceneTexture);
		q.Render();

		profiler.EndQuery();

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::Checkbox("Enable FXAA", &enableFXAA);
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

	q.Dispose();

	window.Dispose();

	return 0;
}
