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

struct LightningVertex
{
	glm::vec3 pos;
	float intensity;
};

unsigned int divisions = 32;
std::vector<LightningVertex> lightningVertices;
GLuint vao = 0, vbo = 0;
int branchLevel = 0;

void AddBranch(const glm::vec3 &startPoint, float lengthMultiplier, float startingBranchIntensity)
{
	branchLevel++;

	glm::vec3 branch = glm::vec3(startPoint.x + Random::Float(-0.5f, 0.5f), startPoint.y - Random::Float(0.5f, 1.0f), startPoint.z /*+ Random::Float(-0.5f, 0.5f)*/);

	lightningVertices.push_back({ startPoint, startingBranchIntensity });
	lightningVertices.push_back({ branch, 0.9f });

	glm::vec3 dir = glm::normalize(branch - startPoint) * lengthMultiplier;
	
	float inv = 1.0f / 32.5f;

	float heightGradient = branch.y / 32.5f;
	float startLengthMult = 0.8f / branchLevel;
	float startHeightDecrease = 0.9f;
	float heightDecrease = startHeightDecrease * heightGradient;
	float branchIntensity = startingBranchIntensity - 0.15f;
	
	unsigned int nextID = lightningVertices.size() - 1;

	for (int i = 0; i < 16 / branchLevel; i++)
	{
		const LightningVertex &last = lightningVertices[nextID];
		glm::vec3 branchSegment = glm::vec3(dir.x * startLengthMult + last.pos.x, last.pos.y - heightDecrease + Random::Float(-1.0f * heightGradient, 1.0f * heightGradient), /* dir.z * startLengthMult +*/ last.pos.z);
		dir = glm::normalize(branchSegment - last.pos) * lengthMultiplier;
		//dir.y -= 0.05f;

		lightningVertices.push_back(lightningVertices[nextID]);		// Push back the last so we continue the branch where the last segment ended
		lightningVertices.push_back({ branchSegment, branchIntensity });

		float heightGradient = branchSegment.y * inv;
		heightDecrease = startHeightDecrease * heightGradient;

		//heightDecrease += 0.04f;
		startLengthMult -= 0.04f;
		branchIntensity -= 0.05f;

		nextID = lightningVertices.size() - 1;

		if (Random::Float() > 0.3f && i > 3 && i < 5)
		{
			if (branchLevel < 2)
				AddBranch(branchSegment, lengthMultiplier, startingBranchIntensity);
		}
	}
	
}

void GenerateLightning()
{
	glm::vec3 v = glm::vec3(0.0f);
	float heightIncrease = 0.5f;
	float xOffset = Random::Float(0.0f, 3.14f);
	float zOffset = Random::Float(0.0f, 3.14f);
	float leaderIntensity = 3.0f;
	float lightningHeight = (heightIncrease + divisions);

	lightningVertices.clear();
	lightningVertices.push_back({ glm::vec3(0.0f), leaderIntensity });

	for (unsigned int i = 0; i < divisions; i++)
	{
		v.x = Random::Float(-0.5f, 0.5f) + std::sin(xOffset) * 2.0f;
		v.y = Random::Float(0.5f, 1.5f) + heightIncrease;
		v.z = Random::Float(-0.5f, 0.5f) + std::cos(zOffset) * 2.0f;
		lightningVertices.push_back({ v, leaderIntensity });	

		if (Random::Float() > 0.32f && i > 3)
		{
			float heightGradient = (float)i / lightningHeight;
			float lengthMultiplier = glm::mix(0.15f, 2.5f, heightGradient);		// For the branches to be larger on the top and smaller on the bottom

			branchLevel = 0;
			AddBranch(v, lengthMultiplier, leaderIntensity - 2.1f);
		}
		
		if (i < divisions - 1)
			lightningVertices.push_back({ v, leaderIntensity });			// Push v again for the next line of the main leader to render correctly

		heightIncrease += 1.0f;
		xOffset += 0.5f;
		zOffset += 0.3f;
	}

	if (vao == 0)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	if (vbo == 0)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, lightningVertices.size() * sizeof(LightningVertex), lightningVertices.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
		glBindVertexArray(0);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, lightningVertices.size() * sizeof(LightningVertex), lightningVertices.data());
	}
}

void GenerateAnvilCrawler()
{
	glm::vec3 v = glm::vec3(0.0f);
	float heightIncrease = 0.5f;
	float xOffset = Random::Float(0.0f, 3.14f);
	float zOffset = Random::Float(0.0f, 3.14f);
	float leaderIntensity = 3.0f;
	float lightningHeight = (heightIncrease + divisions);

	lightningVertices.clear();
	lightningVertices.push_back({ glm::vec3(0.0f), leaderIntensity });

	for (unsigned int i = 0; i < divisions; i++)
	{
		v.x = Random::Float(-0.5f, 0.5f) + std::sin(xOffset) * 2.0f;
		v.y = Random::Float(0.5f, 1.5f) + heightIncrease;
		v.z = Random::Float(-0.5f, 0.5f) + std::cos(zOffset) * 2.0f;
		lightningVertices.push_back({ v, leaderIntensity });

		if (Random::Float() > 0.32f && i > 3)
		{
			float heightGradient = (float)i / lightningHeight;
			float lengthMultiplier = glm::mix(0.15f, 2.5f, heightGradient);		// For the branches to be larger on the top and smaller on the bottom

			branchLevel = 0;
			AddBranch(v, lengthMultiplier, leaderIntensity - 2.1f);
		}

		if (i < divisions - 1)
			lightningVertices.push_back({ v, leaderIntensity });			// Push v again for the next line of the main leader to render correctly

		heightIncrease += 1.0f;
		xOffset += 0.5f;
		zOffset += 0.3f;
	}

	if (vao == 0)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	if (vbo == 0)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, lightningVertices.size() * sizeof(LightningVertex), lightningVertices.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
		glBindVertexArray(0);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, lightningVertices.size() * sizeof(LightningVertex), lightningVertices.data());
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
	camera.SetProjectionMatrix(75.0f, (float)width / height, 0.25f, 100.0f);
	camera.SetPos(glm::vec3(0.0f, 15.0f, 35.0f));
	//camera.SetPos(glm::vec3(0.0f, 2.0f, 5.0f));
	camera.SetMoveSpeed(16.0f);

	Cube cube;
	cube.Load();
	Shader cubeShader;
	cubeShader.Load("Data/Shaders/Lightning/cube.vert", "Data/Shaders/Lightning/cube.frag");

	Shader lightningShader;
	lightningShader.Load("Data/Shaders/Lightning/lightning.vert", "Data/Shaders/Lightning/lightning.frag");

	GenerateAnvilCrawler();

	Profiler &profiler = window.GetProfiler();

	float lightningAlpha = 0.5f;
	float timer = 0.0f;
	bool performStrike = false;
	float lightIntensity = 0.0f;
	bool genLightning = false;
	bool makeVisible = false;

	//glLineWidth(2.0f);

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

		//profiler.BeginQuery("Lightning");

		if (genLightning)
		{
			GenerateAnvilCrawler();
			genLightning = false;
		}

		if (performStrike)
		{
			float deltaTime = window.GetDeltaTime();

			timer += deltaTime;

			
			if (lightningAlpha < 0.4f)
				lightningAlpha += deltaTime * 8.0f;
			else
				lightningAlpha = std::sin(Random::Float(-1.0f, 1.0f));

			lightIntensity = lightningAlpha * 0.5f + 0.5f;

			if (timer >= 0.35f)
			{
				performStrike = false;
				timer = 0.0f;
				lightningAlpha = -1.0f;
				lightIntensity = 0.0f;
			}
		}
		
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));
		m = glm::scale(m, glm::vec3(32.0f, 1.0f, 32.0f));

		cubeShader.Use();
		cubeShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		cubeShader.SetMat4("modelMatrix", m);
		cubeShader.SetFloat("lightIntensity", lightIntensity);
		cube.Render();

		if (performStrike || makeVisible)
		{
			m = glm::mat4(1.0f);

			lightningShader.Use();
			lightningShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
			lightningShader.SetMat4("modelMatrix", m);
			lightningShader.SetFloat("lightningAlpha", lightningAlpha);
			glBindVertexArray(vao);
			glDrawArrays(GL_LINES, 0, lightningVertices.size());
		}
		//profiler.EndQuery();

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::Text("Press WASD to move");
		ImGui::Text("Right click and move the mouse to look around");
		ImGui::DragFloat("lightningAlpha", &lightningAlpha, 0.01f);
		if (ImGui::Button("Generate lightning"))
		{
			genLightning = true;
		}
		if (ImGui::Button("Strike"))
		{
			genLightning = true;
			performStrike = true;
		}
		if (ImGui::Checkbox("Visible", &makeVisible))
		{
			lightningAlpha = 0.5f;
		}
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

		profiler.EndQuery();
		profiler.EndFrame();

		window.SwapBuffers();
	}

	window.Dispose();

	return 0;
}
