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

GLuint gBuffer = 0, ssaoFB = 0, ssaoBlurFB = 0;
GLuint albedoTexture = 0, normalsTexture = 0, depthTexture = 0;
GLuint ssaoFBTexture = 0;
GLuint ssaoBlurFBTexture = 0;

void CreateFramebuffers(unsigned int width, unsigned int height)
{
	if (gBuffer > 0)
	{
		glDeleteFramebuffers(1, &gBuffer);
		glDeleteTextures(1, &albedoTexture);
		glDeleteTextures(1, &normalsTexture);
		glDeleteTextures(1, &depthTexture);
	}

	glCreateFramebuffers(1, &gBuffer);

	glCreateTextures(GL_TEXTURE_2D, 1, &albedoTexture);
	glTextureParameteri(albedoTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(albedoTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(albedoTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(albedoTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(albedoTexture, 1, GL_RGBA8, width, height);

	glCreateTextures(GL_TEXTURE_2D, 1, &normalsTexture);
	glTextureParameteri(normalsTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(normalsTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(normalsTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(normalsTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(normalsTexture, 1, GL_RGBA16F, width, height);

	glCreateTextures(GL_TEXTURE_2D, 1, &depthTexture);
	glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(depthTexture, 1, GL_DEPTH_COMPONENT24, width, height);

	glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, albedoTexture, 0);
	glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, normalsTexture, 0);
	glNamedFramebufferTexture(gBuffer, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(gBuffer, 2, attachments);

	if (glCheckNamedFramebufferStatus(gBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Failed to create framebuffer\n";
	}


	if (ssaoFB > 0)
	{
		glDeleteFramebuffers(1, &ssaoFB);
		glDeleteTextures(1, &ssaoFBTexture);	
	}

	glCreateFramebuffers(1, &ssaoFB);

	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoFBTexture);
	glTextureParameteri(ssaoFBTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ssaoFBTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ssaoFBTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoFBTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(ssaoFBTexture, 1, GL_R8, width, height);

	glNamedFramebufferTexture(ssaoFB, GL_COLOR_ATTACHMENT0, ssaoFBTexture, 0);

	if (glCheckNamedFramebufferStatus(ssaoFB, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Failed to create framebuffer\n";
	}

	if (ssaoBlurFB > 0)
	{
		glDeleteFramebuffers(1, &ssaoBlurFB);
		glDeleteTextures(1, &ssaoBlurFBTexture);		
	}

	glCreateFramebuffers(1, &ssaoBlurFB);

	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoBlurFBTexture);
	glTextureParameteri(ssaoBlurFBTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ssaoBlurFBTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ssaoBlurFBTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoBlurFBTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(ssaoBlurFBTexture, 1, GL_R8, width, height);

	glNamedFramebufferTexture(ssaoBlurFB, GL_COLOR_ATTACHMENT0, ssaoBlurFBTexture, 0);

	if (glCheckNamedFramebufferStatus(ssaoBlurFB, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Failed to create framebuffer\n";
	}
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
	camera.SetProjectionMatrix(75.0f, (float)width / height, 0.2f, 100.0f);
	camera.SetPos(glm::vec3(10.0f, 1.7f, 0.0f));
	camera.SetYaw(180.0f);

	Shader cubeShader;
	cubeShader.Load("Data/Shaders/SSAO/cube.vert", "Data/Shaders/SSAO/cube.frag");
	Shader skyboxShader;
	skyboxShader.Load("Data/Shaders/SSAO/skybox.vert", "Data/Shaders/SSAO/skybox.frag");

	Cube cube;
	cube.Load();

	Model dragon;
	dragon.Load("Data/Models/crytek-sponza/sponza1.obj");

	/*Model platform;
	platform.Load("Data/Models/platform.obj");*/

	struct Vertex
	{
		glm::vec4 posuv;
		int cornerid;
	};

	Vertex vertices[6];
	vertices[0] = { glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f), 0 };
	vertices[1] = { glm::vec4(-1.0f, -1.0f,	0.0f, 0.0f), 2 };
	vertices[2] = { glm::vec4(1.0f, -1.0f,	1.0f, 0.0f), 3 };
	vertices[3] = { glm::vec4(-1.0f, 1.0f,	0.0f, 1.0f), 0 };
	vertices[4] = { glm::vec4(1.0f, -1.0f,	1.0f, 0.0f), 3 };
	vertices[5] = { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1 };

	GLuint quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_INT, 5 * sizeof(float), (GLvoid*)(4 * sizeof(float)));
	glBindVertexArray(0);

	Shader postShader;
	postShader.Load("Data/Shaders/SSAO/quad.vert", "Data/Shaders/SSAO/post_process.frag");

	Shader ssaoShader;
	ssaoShader.Load("Data/Shaders/SSAO/quad.vert", "Data/Shaders/SSAO/ssao.frag");

	Shader ssaoBlurShader;
	ssaoBlurShader.Load("Data/Shaders/SSAO/quad.vert", "Data/Shaders/SSAO/ssao_blur.frag");

	Profiler &profiler = window.GetProfiler();

	CreateFramebuffers(width, height);

	// Generate the sample kernel in tangent space. Normal points in the positive z direction
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	const unsigned int SAMPLES = 8;		// Also change in shader
	glm::vec3 ssaoKernel[SAMPLES];
	glm::vec3 sample;
	for (unsigned int i = 0; i < SAMPLES; i++)
	{
		sample.x = randomFloats(generator) * 2.0f - 1.0f;
		sample.y = randomFloats(generator) * 2.0f - 1.0f;
		sample.z = randomFloats(generator);		// Between 0 and 1 so we have hemisphere sample kernel

		// Leaving the sample as above they would be randomly distributed in the sample kernel
		// but we rather plave a larger weight on occlusions close to the fragment.
		// To do that we use an accelerating interpolation function
		float scale = (float)i / SAMPLES;		// Goes from 0 to 1
		scale = glm::mix(0.1f, 1.0f, scale * scale);		// scale * scale goes from 0 to 1. It starts more slowly and accelarates towards the end to 1
		sample = glm::normalize(sample) * scale;		// Multiply by scale to place the sample closer or further away from the fragment depending on the value of scale
		ssaoKernel[i] = sample;
	}

	// Random kernel rotations vectors
	const unsigned int MAX_ROT_VECTORS = 16;
	glm::vec3 ssaoNoise[MAX_ROT_VECTORS];
	glm::vec3 noise;
	for (unsigned int i = 0; i < MAX_ROT_VECTORS; i++)
	{
		noise.x = randomFloats(generator) * 2.0f - 1.0f;
		noise.y = randomFloats(generator) * 2.0f - 1.0f;
		noise.z = 0.0f,

		ssaoNoise[i] = glm::normalize(noise);
	}

	GLuint ssaoNoiseTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoNoiseTexture);
	glTextureParameteri(ssaoNoiseTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(ssaoNoiseTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(ssaoNoiseTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoNoiseTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(ssaoNoiseTexture, 1, GL_RGB32F, 4, 4);
	glTextureSubImage2D(ssaoNoiseTexture, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, ssaoNoise);

	const char *faces[6];
	faces[0] = "Data/Textures/skybox/right.png";
	faces[1] = "Data/Textures/skybox/left.png";
	faces[2] = "Data/Textures/skybox/up.png";
	faces[3] = "Data/Textures/skybox/down.png";
	faces[4] = "Data/Textures/skybox/back.png";
	faces[5] = "Data/Textures/skybox/front.png";
	GLuint skyboxTexture = utils::LoadCubemap(faces);

	faces[0] = "Data/Textures/skybox/irradiance/right.png";
	faces[1] = "Data/Textures/skybox/irradiance/left.png";
	faces[2] = "Data/Textures/skybox/irradiance/up.png";
	faces[3] = "Data/Textures/skybox/irradiance/down.png";
	faces[4] = "Data/Textures/skybox/irradiance/back.png";
	faces[5] = "Data/Textures/skybox/irradiance/front.png";
	GLuint irradianceTexture = utils::LoadCubemap(faces);

	// It's important the clear color is black so the normals on empty regions are 0,0,0. Otherwise there cloud be some occlusion depending where the camera is looking.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	float radius = 0.5f;
	float intensity = 1.0f;
	bool enableSSAO = false;
	bool ssaoOnly = false;

	while (!window.ShouldClose())
	{
		window.Update();
		if (window.WasResized())
		{
			width = window.GetWidth();
			height = window.GetHeight();
			glViewport(0, 0, width, height);
			CreateFramebuffers(width, height);
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

		profiler.BeginQuery("GBuffer");

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glviewpo
		cubeShader.Use();
		cubeShader.SetMat4("projectionMatrix", camera.GetProjectionMatrix());
		cubeShader.SetMat4("viewMatrix", camera.GetViewMatrix());
		cubeShader.SetVec3("camPos", camera.GetPos());

		//glBindTextureUnit(0, irradianceTexture);

		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		cubeShader.SetMat4("modelMatrix", m);
		dragon.Render();

		/*m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
		//m = glm::scale(m, glm::vec3(3.0f));
		cubeShader.SetMat4("modelMatrix", m);
		platform.Render();*/

		glBindTextureUnit(0, skyboxTexture);
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_FRONT);
		skyboxShader.Use();
		skyboxShader.SetMat4("projView", camera.GetProjectionMatrix() * glm::mat4(glm::mat3(camera.GetViewMatrix())));
		cube.Render();
		glCullFace(GL_BACK);
		glDepthFunc(GL_LESS);

		profiler.EndQuery();

		profiler.BeginQuery("SSAO");
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFB);
		glClear(GL_COLOR_BUFFER_BIT);

		ssaoShader.Use();
		ssaoShader.SetMat4("invProj", glm::inverse(camera.GetProjectionMatrix()));
		ssaoShader.SetMat4("projectionMatrix", camera.GetProjectionMatrix());
		ssaoShader.SetMat4("viewMatrix", camera.GetViewMatrix());
		ssaoShader.SetVec2("noiseScale", glm::vec2(width / 4.0f, height / 4.0f));
		ssaoShader.SetFloat("radius", radius);
		ssaoShader.SetFloat("intensity", intensity);

		const Frustum &frustum = camera.GetFrustum();
		glm::vec3 cornersViewSpace[] = { glm::vec3(camera.GetViewMatrix() * glm::vec4(frustum.ftl, 1.0f)), glm::vec3(camera.GetViewMatrix() * glm::vec4(frustum.ftr, 1.0f)), glm::vec3(camera.GetViewMatrix() * glm::vec4(frustum.fbl, 1.0f)), glm::vec3(camera.GetViewMatrix() * glm::vec4(frustum.fbr, 1.0f)) };
		glUniform3fv(glGetUniformLocation(ssaoShader.GetProgram(), "frustumFarCornersViewSpace"), 4, &cornersViewSpace[0].x);

		glUniform3fv(glGetUniformLocation(ssaoShader.GetProgram(), "samples"), SAMPLES, &ssaoKernel[0].x);

		glBindTextureUnit(0, albedoTexture);
		glBindTextureUnit(1, normalsTexture);
		glBindTextureUnit(2, depthTexture);
		glBindTextureUnit(3, ssaoNoiseTexture);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		profiler.EndQuery();

		profiler.BeginQuery("SSAO Blur");
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFB);
		glClear(GL_COLOR_BUFFER_BIT);

		ssaoBlurShader.Use();
		glBindTextureUnit(0, ssaoFBTexture);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		profiler.EndQuery();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		profiler.BeginQuery("Post quad");
		postShader.Use();
		postShader.SetMat4("invProj", glm::inverse(camera.GetProjectionMatrix()));
		postShader.SetVec2("noiseScale", glm::vec2(width / 4.0f, height / 4.0f));
		postShader.SetInt("enableSSAO", (int)enableSSAO);
		postShader.SetInt("ssaoOnly", (int)ssaoOnly);

		glUniform3fv(glGetUniformLocation(postShader.GetProgram(), "frustumFarCornersViewSpace"), 4, &cornersViewSpace[0].x);

		glBindTextureUnit(0, albedoTexture);
		glBindTextureUnit(1, normalsTexture);
		glBindTextureUnit(2, depthTexture);
		glBindTextureUnit(3, ssaoNoiseTexture);
		glBindTextureUnit(4, ssaoBlurFBTexture);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		profiler.EndQuery();

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::Text("Press WASD to move");
		ImGui::Text("Right click and move the mouse to look around");
		ImGui::DragFloat("Radius", &radius, 0.02f);
		ImGui::DragFloat("Intensity", &intensity, 0.05f);
		ImGui::Checkbox("SSAO", &enableSSAO);
		ImGui::Checkbox("SSAO only", &ssaoOnly);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		profiler.EndQuery();

		profiler.BeginQuery("Text");
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		//glDepthMask(GL_FALSE);

		textRenderer.AddText("CPU time: " + std::to_string(window.GetDeltaTime() * 1000.0f) + " ms", glm::vec2(30.0f, 460.0f), glm::vec2(0.3f, 0.3f));
		textRenderer.AddText(profiler.GetResults(), glm::vec2(30.0f, 420.0f), glm::vec2(0.3f, 0.3f));
		textRenderer.Render();
		//glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		profiler.EndQuery();
		profiler.EndFrame();

		window.SwapBuffers();
	}

	cube.Dispose();
	cubeShader.Dispose();
	window.Dispose();

	return 0;
}
