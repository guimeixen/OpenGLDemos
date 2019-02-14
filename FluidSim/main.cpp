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

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <iostream>

struct Surface
{
	GLuint fb;
	GLuint texture;
};

Surface CreateSurface(unsigned int width, unsigned int height, int nComponents, bool floatData)
{
	Surface s = {};

	glCreateTextures(GL_TEXTURE_2D, 1, &s.texture);
	glTextureParameteri(s.texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(s.texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(s.texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(s.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (nComponents == 1)
		glTextureStorage2D(s.texture, 1, floatData ? GL_R16F : GL_R8, width, height);
	else if (nComponents == 2)
		glTextureStorage2D(s.texture, 1, floatData ? GL_RG16F : GL_RG8, width, height);
	else if (nComponents == 3)
		glTextureStorage2D(s.texture, 1, floatData ? GL_RGB16F : GL_RGB8, width, height);


	glCreateFramebuffers(1, &s.fb);
	glNamedFramebufferTexture(s.fb, GL_COLOR_ATTACHMENT0, s.texture, 0);

	GLenum fboStatus = glCheckNamedFramebufferStatus(s.fb, GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete: " << fboStatus << "\n";

	return s;
}

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
	camera.SetMoveSpeed(0.0f);

	Profiler &profiler = window.GetProfiler();

	Quad q;
	q.Load();

	Shader advectionShader;
	advectionShader.Load("Data/Shaders/FluidSim/quad.vert", "Data/Shaders/FluidSim/advection.frag");
	Shader divergenceShader;
	divergenceShader.Load("Data/Shaders/FluidSim/quad.vert", "Data/Shaders/FluidSim/divergence.frag");

	Shader visShader;
	visShader.Load("Data/Shaders/FluidSim/quad.vert", "Data/Shaders/FluidSim/vis.frag");


	const unsigned int simulationWidth = width;
	const unsigned int simulationHeight = height;

	Surface velocity[2];
	velocity[0] = CreateSurface(simulationWidth, simulationHeight, 2, true);
	velocity[1] = CreateSurface(simulationWidth, simulationHeight, 2, true);

	Surface color[2];
	color[0] = CreateSurface(simulationWidth, simulationHeight, 3, false);
	color[1] = CreateSurface(simulationWidth, simulationHeight, 3, false);

	Surface divergence;
	divergence = CreateSurface(simulationWidth, simulationHeight, 3, true);

	/*Surface pressure[2];
	pressure[0] = CreateSurface(simulationWidth, simulationHeight, 1, true);
	pressure[1] = CreateSurface(simulationWidth, simulationHeight, 1, true);
	*/
	struct rgb
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	glm::vec2 *data = new glm::vec2[simulationWidth * simulationHeight];
	rgb *colorData = new rgb[simulationWidth * simulationHeight];
	int index = 0;
	for (int y = 0; y < simulationHeight; y++)
	{
		for (int x = 0; x < simulationWidth; x++)
		{
			data[index] = glm::vec2(std::sin((float)y / simulationHeight * 3.14159f * 4.0f), std::sin((float)x / simulationWidth * 3.14159f * 4.0f));
			unsigned char r = (unsigned char)((Random::Perlin2D((float)x / simulationWidth, (float)y / simulationHeight, 8.0f) * 0.5f + 0.5f) * 255.0f);
			colorData[index] = { r,r,r };
			index++;
		}
	}
	glTextureSubImage2D(velocity[0].texture, 0, 0, 0, simulationWidth, simulationHeight, GL_RG, GL_FLOAT, data);
	glTextureSubImage2D(velocity[1].texture, 0, 0, 0, simulationWidth, simulationHeight, GL_RG, GL_FLOAT, data);

	glTextureSubImage2D(color[0].texture, 0, 0, 0, simulationWidth, simulationHeight, GL_RGB, GL_UNSIGNED_BYTE, colorData);
	glTextureSubImage2D(color[1].texture, 0, 0, 0, simulationWidth, simulationHeight, GL_RGB, GL_UNSIGNED_BYTE, colorData);

	delete[] data;



	unsigned int vIndex = 0;
	unsigned int cindex = 0;
	float timeStep = 0.0125f;

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

		// Advect velocity
		/*glBindFramebuffer(GL_FRAMEBUFFER, velocity[vIndex].fb);
		//glClear(GL_COLOR_BUFFER_BIT);
		advectionShader.Use();
		advectionShader.SetFloat("timeStep", timeStep);

		glBindTextureUnit(0, velocity[1 - vIndex].texture);
		glBindTextureUnit(1, velocity[1 - vIndex].texture);
		q.Render();*/
		
		// Advect the color
		glBindFramebuffer(GL_FRAMEBUFFER, color[vIndex].fb);
		//glClear(GL_COLOR_BUFFER_BIT);
		advectionShader.Use();
		advectionShader.SetFloat("timeStep", timeStep);

		glBindTextureUnit(0, velocity[vIndex].texture);
		glBindTextureUnit(1, color[1-cindex].texture);
		q.Render();

		// Compute the divergence
		glBindFramebuffer(GL_FRAMEBUFFER, divergence.fb);
		glClear(GL_COLOR_BUFFER_BIT);
		divergenceShader.Use();

		glBindTextureUnit(0, velocity[vIndex].texture);
		q.Render();

		// Pressure


		// Subtract gradient

		// Display
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		visShader.Use();
		glBindTextureUnit(0, divergence.texture);
		q.Render();

		vIndex = 1 - vIndex;
		cindex = 1 - cindex;

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
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
