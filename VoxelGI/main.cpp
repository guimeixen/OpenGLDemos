#include "Window.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "ResourcesLoader.h"
#include "Shader.h"
#include "Camera.h"
#include "Cube.h"
#include "Utils.h"
#include "TextRenderer.h"
#include "Model.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <iostream>

struct DrawElementsIndirectCommand
{
	GLuint vertexCount;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

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
	camera.SetProjectionMatrix(75.0f, (float)width / height, 0.1f, 64.0f);
	camera.SetPos(glm::vec3(0.0f, 0.0f, 2.0f));

	Model model;
	model.Load("Data/Models/crytek-sponza/sponza1.obj");

	Shader shader;
	shader.Load("Data/Shaders/VoxelGI/voxel_cone_tracing.vert", "Data/Shaders/VoxelGI/voxel_cone_tracing.frag");

	Shader voxelizationShader;
	voxelizationShader.Load("Data/Shaders/VoxelGI/voxelization.vert", "Data/Shaders/VoxelGI/voxelization.geom", "Data/Shaders/VoxelGI/voxelization.frag");

	Shader voxelCubeShader;
	voxelCubeShader.Load("Data/Shaders/VoxelGI/cube.vert", "Data/Shaders/VoxelGI/cube.frag");

	Shader fillDataComputeShader;
	fillDataComputeShader.Load("Data/Shaders/VoxelGI/fill_buffer.comp");

	Shader shadowShader;
	shadowShader.Load("Data/Shaders/VoxelGI/shadow.vert", "Data/Shaders/VoxelGI/shadow.frag");

	Shader mipmapShader;
	mipmapShader.Load("Data/Shaders/VoxelGI/mipmap.comp");

	Cube cube;
	cube.Load();

	const unsigned int VOXEL_RES = 256;
	float voxelGridSize = 32.0f;

	GLuint voxelTexture;
	glCreateTextures(GL_TEXTURE_3D, 1, &voxelTexture);
	glTextureParameteri(voxelTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(voxelTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTextureParameteri(voxelTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glTextureParameteri(voxelTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(voxelTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const unsigned int mipLevels = 9;
	glTextureStorage3D(voxelTexture, mipLevels, GL_RGBA8, VOXEL_RES, VOXEL_RES, VOXEL_RES);
	unsigned char clearColor[4] = { 0, 0, 0, 0 };
	glClearTexImage(voxelTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &clearColor);
	//glTextureSubImage3D(voxelTexture, 0, 0, 0, 0, VOXEL_RES, VOXEL_RES, VOXEL_RES, GL_RGBA, GL_UNSIGNED_BYTE, v.data());

	float size = 0.5f;
	float vertices[] = {
		-size, -size, -size,
		-size, -size, size,
		size, -size, size,
		size, -size, -size,
		-size, size, -size,
		-size, size, size,
		size, size, size,
		size, size, -size,
	};

	unsigned short indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		0,5,4,
		1,5,0,

		2,7,6,
		3,7,2
	};

	GLuint vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

	DrawElementsIndirectCommand cmd;
	cmd.baseInstance = 0;
	cmd.baseVertex = 0;
	cmd.firstIndex = 0;
	cmd.instanceCount = 0;
	cmd.vertexCount = 36;

	GLuint voxelsBuffer;
	glGenBuffers(1, &voxelsBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, voxelsBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_COPY);

	//std::vector<float> vv(VOXEL_RES * VOXEL_RES * VOXEL_RES * 16);

	GLuint ssbo = 0;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, VOXEL_RES * VOXEL_RES * VOXEL_RES * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, voxelsBuffer);

	// Shadows FBO

	GLuint SHADOW_RES = 1024;

	GLuint shadowsTex;
	glGenTextures(1, &shadowsTex);
	glBindTexture(GL_TEXTURE_2D, shadowsTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	GLuint shadowsFBO;
	glGenFramebuffers(1, &shadowsFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowsTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	unsigned int localSize = 4;

	bool voxelized = false;
	bool showVoxels = false;

	// Settings
	float samplingFactor = 1.0f;
	float bounceStrength = 2.0f;
	float aoFalloff = 725.0f;
	bool enableIndirect = true;
	float specAperture = 0.4f;
	float diffAperture = 0.67f;

	glm::vec3 lightPos = glm::vec3(8.0f, 16.0f, -4.6f);

	Profiler &profiler = window.GetProfiler();

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

		if (Input::WasKeyPressed(GLFW_KEY_1))
			showVoxels = !showVoxels;

		if (Input::IsKeyPressed(GLFW_KEY_T))
			lightPos.x += window.GetDeltaTime();
		if (Input::IsKeyPressed(GLFW_KEY_G))
			lightPos.x -= window.GetDeltaTime();
		if (Input::IsKeyPressed(GLFW_KEY_Y))
			lightPos.y += window.GetDeltaTime();
		if (Input::IsKeyPressed(GLFW_KEY_H))
			lightPos.y -= window.GetDeltaTime();
		if (Input::IsKeyPressed(GLFW_KEY_U))
			lightPos.z += window.GetDeltaTime();
		if (Input::IsKeyPressed(GLFW_KEY_J))
			lightPos.z -= window.GetDeltaTime();

		if (Input::IsKeyPressed(GLFW_KEY_V))
			voxelized = false;
	
		profiler.BeginQuery("Shadow");

		glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOW_RES, SHADOW_RES);

		glm::mat4 lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -4.0f, 34.0f);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightDir = glm::normalize(lightPos);
		glm::mat4 lightSpaceMatrix = lightProj * lightView;

		shadowShader.Use();
		shadowShader.SetMat4("mvp", lightSpaceMatrix);
		model.Render();

		profiler.EndQuery();

		profiler.BeginQuery("Main clear");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.15f, 0.5f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		glBindTextureUnit(4, shadowsTex);

		profiler.EndQuery();

		// VOXELIZATION

		if (!voxelized)
		{
			profiler.BeginQuery("Voxelization");
			voxelizationShader.Use();

			glViewport(0, 0, VOXEL_RES, VOXEL_RES);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			glBindImageTexture(3, voxelTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

			float halfSize = voxelGridSize / 2.0f;

			glm::mat4 ortho = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, halfSize, voxelGridSize * 1.5f);
			glm::mat4 projX = ortho * glm::lookAt(glm::vec3(voxelGridSize, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			glm::mat4 projY = ortho * glm::lookAt(glm::vec3(0, voxelGridSize, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
			glm::mat4 projZ = ortho * glm::lookAt(glm::vec3(0, 0, voxelGridSize), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

			voxelizationShader.SetMat4("orthoProjX", projX);
			voxelizationShader.SetMat4("orthoProjY", projY);
			voxelizationShader.SetMat4("orthoProjZ", projZ);
			voxelizationShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
			voxelizationShader.SetVec3("lightDir", lightDir);
			voxelizationShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

			voxelizationShader.SetMat4("modelMatrix", glm::mat4(1.0f));
			voxelizationShader.SetVec3("emissiveColor", glm::vec3(0.0f));
			model.Render();

			/*glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f));
			voxelizationShader.SetMat4("modelMatrix", m);
			voxelizationShader.SetVec3("emissiveColor", glm::vec3(1.0f, 0.0f, 0.0f));
			cube.Render();*/

			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glViewport(0, 0, width, height);

			profiler.EndQuery();

			profiler.BeginQuery("Voxel Mipmapping");

			mipmapShader.Use();
			glBindTextureUnit(0, voxelTexture);

			unsigned int secondMipRes = VOXEL_RES >> 1;

			for (int i = 0; i < mipLevels - 1; i++)
			{
				unsigned int dstMipRes = secondMipRes >> i;
				mipmapShader.SetInt("dstMipRes", (int)dstMipRes);
				mipmapShader.SetInt("srcMipLevel", i);
				glBindImageTexture(0, voxelTexture, i + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

				unsigned int count = dstMipRes / 4;

				glDispatchCompute(count, count, count);
			}

			profiler.EndQuery();
		}

		voxelized = true;

		profiler.BeginQuery("Voxel Cone Tracing");

		glBindTextureUnit(3, voxelTexture);

		glViewport(0, 0, width, height);

		if (showVoxels)
		{
			fillDataComputeShader.Use();
			glBindImageTexture(3, voxelTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
			glDispatchCompute(VOXEL_RES / localSize, VOXEL_RES / localSize, VOXEL_RES / localSize);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			voxelCubeShader.Use();
			voxelCubeShader.SetMat4("projectionMatrix", camera.GetProjectionMatrix());
			voxelCubeShader.SetMat4("viewMatrix", camera.GetViewMatrix());
			voxelCubeShader.SetFloat("voxelGridSize", voxelGridSize);
			voxelCubeShader.SetFloat("volumeSize", (float)VOXEL_RES);
			glBindVertexArray(vao);
			//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, voxelsBuffer);
			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			shader.Use();
			shader.SetMat4("projectionMatrix", camera.GetProjectionMatrix());
			shader.SetMat4("viewMatrix", camera.GetViewMatrix());
			shader.SetVec3("camPos", camera.GetPos());
			shader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
			shader.SetFloat("volumeSize", (float)VOXEL_RES);
			shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
			shader.SetVec3("lightDir", lightDir);

			shader.SetFloat("samplingFactor", samplingFactor);
			shader.SetFloat("bounceStrength", bounceStrength);
			shader.SetFloat("aoFalloff", aoFalloff);
			shader.SetInt("enableIndirect", enableIndirect);
			//shader.SetFloat("specAperture", specAperture);
			shader.SetFloat("diffAperture", diffAperture);

			//glm::mat4 m = glm::translate(glm::mat4(), glm::vec3(0.0f));
			shader.SetMat4("modelMatrix", glm::mat4(1.0f));
			model.Render();
		}

		profiler.EndQuery();

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::SliderFloat("Sampling Factor", &samplingFactor, 0.1f, 1.5f);
		ImGui::SliderFloat("Bounce Strength", &bounceStrength, 0.0f, 5.0f);
		ImGui::SliderFloat("AO Falloff", &aoFalloff, 0.0f, 1500.0f);
		ImGui::SliderFloat("Specular Aperture", &specAperture, 0.01f, 1.0f);
		ImGui::SliderFloat("Diffuse Aperture", &diffAperture, 0.01f, 1.0f);
		ImGui::Checkbox("Enable GI", &enableIndirect);
		ImGui::Text("Press F1 to show voxels");
		ImGui::Text("Press Q for wireframe view");
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
