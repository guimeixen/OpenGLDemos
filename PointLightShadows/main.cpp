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
	camera.SetPos(glm::vec3(0.0f, 0.4f, 0.0f));
	camera.SetMoveSpeed(2.0f);
	//camera.SetMoveSpeed(0.0f);

	Profiler &profiler = window.GetProfiler();

	Shader modelShader;
	modelShader.Load("Data/Shaders/PointLightShadows/model.vert", "Data/Shaders/PointLightShadows/model.frag");
	Shader cubeShader;
	cubeShader.Load("Data/Shaders/PointLightShadows/cube.vert", "Data/Shaders/PointLightShadows/cube.frag");
	Shader shadowShader;
	shadowShader.Load("Data/Shaders/PointLightShadows/shadow.vert", "Data/Shaders/PointLightShadows/shadow.frag");
	Shader shadowGeometryShader;
	shadowGeometryShader.Load("Data/Shaders/PointLightShadows/shadow_g.vert", "Data/Shaders/PointLightShadows/shadow_g.geom","Data/Shaders/PointLightShadows/shadow_g.frag");

	Model cornell;
	cornell.Load("Data/Models/cornell2.obj");
	Cube cube;
	cube.Load();

	const unsigned int SHADOW_MAP_RES = 1024;

	GLuint shadowTexture;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &shadowTexture);
	glTextureParameteri(shadowTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(shadowTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(shadowTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(shadowTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(shadowTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(shadowTexture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(shadowTexture, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glTextureStorage2D(shadowTexture, 1, GL_DEPTH_COMPONENT24, SHADOW_MAP_RES, SHADOW_MAP_RES);
	
	GLuint shadowFB;
	glCreateFramebuffers(1, &shadowFB);
	glNamedFramebufferTexture(shadowFB, GL_DEPTH_ATTACHMENT, shadowTexture, 0);
	glNamedFramebufferDrawBuffer(shadowFB, GL_NONE);
	glNamedFramebufferReadBuffer(shadowFB, GL_NONE);

	if (glCheckNamedFramebufferStatus(shadowFB, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Failed to create framebuffer\n";
	

	// sponza
	//glm::vec3 lightPos = glm::vec3(0.0f, 7.0f, -3.5f);
	//float lightRadius = 25.0f;
	// cornell
	glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, -2.0f);
	float lightRadius = 10.0f;
	glm::vec3 lightColor = glm::vec3(1.0f);
	float lightIntensity = 1.0f;
	bool animateLight = true;
	bool useGeometry = false;

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

		if (animateLight)
		{
			lightPos.x = std::sin((float)glfwGetTime()) * 0.8f;
			lightPos.y = std::cos((float)glfwGetTime()) * 0.8f;
		}
		
		float aspect = (float)SHADOW_MAP_RES / SHADOW_MAP_RES;
		float near = 0.1f;
		float far = 25.0f;		// set to light radius

		glm::mat4 proj = glm::perspective(glm::radians(90.0f), aspect, near, far);

		glm::mat4 lightTransforms[6];
		lightTransforms[0] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		lightTransforms[1] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		lightTransforms[2] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		lightTransforms[3] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		lightTransforms[4] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		lightTransforms[5] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
		//m = glm::scale(m, glm::vec3(0.4f));

		profiler.BeginQuery("Shadows");

		glBindFramebuffer(GL_FRAMEBUFFER, shadowFB);
		glViewport(0, 0, SHADOW_MAP_RES, SHADOW_MAP_RES);

		if (useGeometry)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
			shadowGeometryShader.Use();
			shadowGeometryShader.SetVec3("lightPos", lightPos);
			shadowGeometryShader.SetFloat("farPlane", far);		
			shadowGeometryShader.SetMat4Array("lightTransform[0]", lightTransforms, 6);
			shadowGeometryShader.SetMat4("modelMatrix", m);
			cornell.Render();
		}
		else
		{
			shadowShader.Use();
			shadowShader.SetVec3("lightPos", lightPos);
			shadowShader.SetFloat("farPlane", far);
			
			for (unsigned int i = 0; i < 6; i++)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, shadowTexture, 0);
				glClear(GL_DEPTH_BUFFER_BIT);
				shadowShader.SetMat4("lightTransform", lightTransforms[i]);
				shadowShader.SetMat4("modelMatrix", m);
				cornell.Render();
			}
		}

		profiler.EndQuery();

		profiler.BeginQuery("Scene");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindTextureUnit(1, shadowTexture);

		modelShader.Use();
		modelShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		modelShader.SetMat4("modelMatrix", m);
		modelShader.SetVec3("lightPos", lightPos);
		modelShader.SetFloat("lightRadius", lightRadius);
		modelShader.SetVec3("lightColor", lightColor);
		modelShader.SetFloat("lightIntensity", lightIntensity);
		modelShader.SetFloat("farPlane", far);
		cornell.Render();

		// Light cube
		m = glm::translate(glm::mat4(1.0f), lightPos);
		m = glm::scale(m, glm::vec3(0.15f));

		cubeShader.Use();
		cubeShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		cubeShader.SetMat4("modelMatrix", m);
		cube.Render();

		profiler.EndQuery();

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::Text("Press WASD to move");
		ImGui::Text("Right click and move the mouse to look around");
		ImGui::DragFloat3("Light pos", &lightPos.x, 0.05f);
		ImGui::DragFloat("Light radius", &lightRadius, 0.1f);
		ImGui::ColorEdit3("Light color", &lightColor.x);
		ImGui::DragFloat("Light intensity", &lightIntensity, 0.1f);
		ImGui::Checkbox("Animate light", &animateLight);
		ImGui::Checkbox("Use geometry shader", &useGeometry);
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
