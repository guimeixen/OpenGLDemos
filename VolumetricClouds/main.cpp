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
#include "TimeOfDayManager.h"
#include "TextRenderer.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <fstream>
#include <iostream>

unsigned int cloudFBWidth = 0;
unsigned int cloudFBHeight = 0;
unsigned int cloudFBUpdateTextureWidth;
unsigned int cloudFBUpdateTextureHeight;
GLuint cloudsFB = 0;
GLuint cloudQuarterFB = 0;
GLuint cloudsTexture = 0;
GLuint cloudsQuarterTexture = 0;
GLuint prevFrameFB = 0;
GLuint prevFrameCloudTexture = 0;

void CreateFramebuffers()
{
	if (cloudsFB > 0)
		glDeleteFramebuffers(1, &cloudsFB);
	if (cloudsTexture > 0)
		glDeleteTextures(1, &cloudsTexture);

	glCreateFramebuffers(1, &cloudsFB);
	glCreateTextures(GL_TEXTURE_2D, 1, &cloudsTexture);

	glTextureParameteri(cloudsTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(cloudsTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(cloudsTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(cloudsTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(cloudsTexture, 1, GL_RGBA16F, cloudFBWidth, cloudFBHeight);

	glNamedFramebufferTexture(cloudsFB, GL_COLOR_ATTACHMENT0, cloudsTexture, 0);

	GLenum fboStatus = glCheckNamedFramebufferStatus(cloudsFB, GL_FRAMEBUFFER);

	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete: " << fboStatus << "\n";
		return;
	}

	if (prevFrameFB > 0)
		glDeleteFramebuffers(1, &prevFrameFB);
	if (prevFrameCloudTexture > 0)
		glDeleteTextures(1, &prevFrameCloudTexture);

	glCreateFramebuffers(1, &prevFrameFB);
	glCreateTextures(GL_TEXTURE_2D, 1, &prevFrameCloudTexture);

	glTextureParameteri(prevFrameCloudTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(prevFrameCloudTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(prevFrameCloudTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(prevFrameCloudTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(prevFrameCloudTexture, 1, GL_RGBA16F, cloudFBWidth, cloudFBHeight);

	glNamedFramebufferTexture(prevFrameFB, GL_COLOR_ATTACHMENT0, prevFrameCloudTexture, 0);

	fboStatus = glCheckNamedFramebufferStatus(prevFrameFB, GL_FRAMEBUFFER);

	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete: " << fboStatus << "\n";
		return;
	}

	if (cloudQuarterFB > 0)
		glDeleteFramebuffers(1, &cloudQuarterFB);
	if (cloudsQuarterTexture > 0)
		glDeleteTextures(1, &cloudsQuarterTexture);

	glCreateFramebuffers(1, &cloudQuarterFB);
	glCreateTextures(GL_TEXTURE_2D, 1, &cloudsQuarterTexture);

	glTextureParameteri(cloudsQuarterTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(cloudsQuarterTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(cloudsQuarterTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(cloudsQuarterTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(cloudsQuarterTexture, 1, GL_RGBA16F, cloudFBUpdateTextureWidth, cloudFBUpdateTextureHeight);

	glNamedFramebufferTexture(cloudQuarterFB, GL_COLOR_ATTACHMENT0, cloudsQuarterTexture, 0);

	fboStatus = glCheckNamedFramebufferStatus(cloudQuarterFB, GL_FRAMEBUFFER);

	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete: " << fboStatus << "\n";
		return;
	}
}

float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
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

	TimeOfDayManager tod;
	tod.Init();

	Camera camera;
	camera.SetProjectionMatrix(75.0f, (float)width / height, 0.5f, 1000.0f);
	camera.SetPos(glm::vec3(0.0f));

	Quad q;
	q.Load();

	Shader postQuadShader;
	postQuadShader.Load("Data/Shaders/VolClouds/post_effect_quad.vert", "Data/Shaders/VolClouds/post_effect_quad.frag");

	Shader cloudShader;
	cloudShader.Load("Data/Shaders/VolClouds/clouds.vert", "Data/Shaders/VolClouds/clouds.frag");

	Shader cloudReprojectionShader;
	cloudReprojectionShader.Load("Data/Shaders/VolClouds/quad.vert", "Data/Shaders/VolClouds/cloud_reprojection.frag");

	Shader cubeShader;
	cubeShader.Load("Data/Shaders/VolClouds/cube.vert", "Data/Shaders/VolClouds/cube.frag");

	Cube cube;
	cube.Load();
	Sphere sphere;
	sphere.Load();

	cloudFBWidth = width / 2;
	cloudFBHeight = height / 2;
	
	// Make sure the clouds texture is divisible by the block size
	unsigned int cloudFBUpdateBlockSize = 4;
	while (cloudFBWidth % cloudFBUpdateBlockSize != 0)
	{
		cloudFBWidth++;
	}
	while (cloudFBHeight % cloudFBUpdateBlockSize != 0)
	{
		cloudFBHeight++;
	}
	cloudFBUpdateTextureWidth = cloudFBWidth / cloudFBUpdateBlockSize;
	cloudFBUpdateTextureHeight = cloudFBHeight / cloudFBUpdateBlockSize;
	
	CreateFramebuffers();

	// Load 3d texture
	struct TexData
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};

	const unsigned int resolution = 128;
	TexData *noise = new TexData[resolution * resolution * resolution];

	FILE *file = nullptr;
	fopen_s(&file, "Data/Textures/noise.data", "rb");

	if (file == nullptr)
	{
		// Generate the noise if the file doesn't exist and save it to file
		float stepSize = 1.0f / resolution;
		unsigned int index = 0;
		glm::vec3 coord = glm::vec3(0.0f);
		const float cellCount = 4;

		for (unsigned int z = 0; z < resolution; z++)
		{
			for (unsigned int y = 0; y < resolution; y++)
			{
				for (unsigned int x = 0; x < resolution; x++)
				{
					coord.x = x * stepSize;
					coord.y = y * stepSize;
					coord.z = z * stepSize;

					float r = Random::Perlin3D(coord.x, coord.y, coord.z, 8.0f, 3.0f, 2.0f, 0.5f);
					//float g = Random::Perlin3D(coord.x, coord.y, coord.z, 16.0f, 4.0f, 2.0f, 0.5f);
					float worley0 = 1.0f - Random::WorleyNoise(coord, cellCount * 2.0f);
					float worley1 = 1.0f - Random::WorleyNoise(coord, cellCount * 8.0f);
					float worley2 = 1.0f - Random::WorleyNoise(coord, cellCount * 14.0f);

					float worleyFBM = worley0 * 0.625f + worley1 * 0.25f + worley2 * 0.125f;

					float perlinWorley = remap(r, 0.0f, 1.0f, worleyFBM, 1.0f);

					//worley0 = 1.0f - Random::WorleyNoise(coord, cellCount);
					worley1 = 1.0f - Random::WorleyNoise(coord, cellCount * 2);
					worley2 = 1.0f - Random::WorleyNoise(coord, cellCount * 4);
					float worley3 = 1.0f - Random::WorleyNoise(coord, cellCount * 8);
					float worley4 = 1.0f - Random::WorleyNoise(coord, cellCount * 16);

					float worleyFBM0 = worley1 * 0.625f + worley2 * 0.25f + worley3 * 0.125f;
					float worleyFBM1 = worley2 * 0.625f + worley3 * 0.25f + worley3 * 0.125f;
					float worleyFBM2 = worley3 * 0.75f + worley4 * 0.25f;

					index = z * resolution*resolution + y * resolution + x;
					noise[index].r = static_cast<unsigned char>(perlinWorley * 255.0f);
					noise[index].g = static_cast<unsigned char>(worleyFBM0 * 255.0f);
					noise[index].b = static_cast<unsigned char>(worleyFBM1 * 255.0f);
					noise[index].a = static_cast<unsigned char>(worleyFBM2 * 255.0f);
				}
			}
		}

		std::fstream file = std::fstream("Data/Textures/noise.data", std::ios::out | std::ios::binary);
		file.write((char*)&noise[0], resolution * resolution * resolution * sizeof(TexData));
		file.close();
	}
	else
	{
		fread(noise, 1, resolution * resolution * resolution * sizeof(TexData), file);
		fclose(file);
	}

	GLuint noiseTexture = utils::Load3DTexture(resolution, resolution, resolution, 4, GL_UNSIGNED_BYTE, GL_MIRRORED_REPEAT, noise);
	GLuint weatherTexture = utils::LoadTexture("Data/Textures/weather.png", false);
	GLuint curlNoiseTexture = utils::LoadTexture("Data/Textures/curlNoise.png", false);
	GLuint verticalCoverageTexture = utils::LoadTexture("Data/Textures/vertical_coverage.png", false, GL_CLAMP_TO_EDGE);
	GLuint highCloudsTexture = utils::LoadTexture("Data/Textures/high_clouds.png", false);

	delete[] noise;




	struct HighFreqNoise
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};
	const unsigned int highFreqRes = 32;
	HighFreqNoise *highFreqNoise = new HighFreqNoise[highFreqRes * highFreqRes * highFreqRes];

	fopen_s(&file, "Data/Textures/highFreqNoise.data", "rb");

	if (file == nullptr)
	{
		// Generate the noise if the file doesn't exist and save it to file
		float stepSize = 1.0f / highFreqRes;
		unsigned int index = 0;
		glm::vec3 coord = glm::vec3(0.0f);
		const float cellCount = 2;

		for (unsigned int z = 0; z < highFreqRes; z++)
		{
			for (unsigned int y = 0; y < highFreqRes; y++)
			{
				for (unsigned int x = 0; x < highFreqRes; x++)
				{
					coord.x = x * stepSize;
					coord.y = y * stepSize;
					coord.z = z * stepSize;

					float worley0 = 1.0f - Random::WorleyNoise(coord, cellCount);
					float worley1 = 1.0f - Random::WorleyNoise(coord, cellCount * 2.0f);
					float worley2 = 1.0f - Random::WorleyNoise(coord, cellCount * 4.0f);
					float worley3 = 1.0f - Random::WorleyNoise(coord, cellCount * 8.0f);

					float worleyFBM0 = worley0 * 0.625f + worley1 * 0.25f + worley2 * 0.125f;
					float worleyFBM1 = worley1 * 0.625f + worley2 * 0.25f + worley3 * 0.125f;
					float worleyFBM2 = worley2 * 0.75f + worley3 * 0.25f;

					index = z * highFreqRes * highFreqRes + y * highFreqRes + x;
					highFreqNoise[index].r = static_cast<unsigned char>(worleyFBM0 * 255.0f);
					highFreqNoise[index].g = static_cast<unsigned char>(worleyFBM1 * 255.0f);
					highFreqNoise[index].b = static_cast<unsigned char>(worleyFBM2 * 255.0f);
				}
			}
		}

		std::fstream file = std::fstream("Data/Textures/highFreqNoise.data", std::ios::out | std::ios::binary);
		file.write((char*)&highFreqNoise[0], highFreqRes * highFreqRes * highFreqRes * sizeof(HighFreqNoise));
		file.close();
	}
	else
	{
		fread(highFreqNoise, 1, highFreqRes * highFreqRes * highFreqRes * sizeof(HighFreqNoise), file);
		fclose(file);
	}

	GLuint highFreqNoiseTexture = utils::Load3DTexture(highFreqRes, highFreqRes, highFreqRes, 3, GL_UNSIGNED_BYTE, GL_MIRRORED_REPEAT, highFreqNoise);

	delete[] highFreqNoise;

	GLuint inscatterTexture = utils::Load3DTexture("Data/Textures/inscatter.raw", 256, 128, 32, 4, GL_FLOAT);
	GLuint transmittanceTexture = utils::LoadRAW2DTexture("Data/Textures/transmittance.raw", 256, 64, 3, GL_FLOAT);

	float timeOfDay = 12.0f;
	float coverage = 0.45f;
	float camSpeed = camera.GetMoveSpeed();
	float cloudStartHeight = 2200.0f;
	float cloudLayerThickness = 2250.0f;
	float timeScale = 0.001f;
	float hgForward = 0.6f;
	float hgBackward = -0.29f;
	float density = 2.27f;
	float directLightMult = 2.5f;
	glm::vec3 ambientTopColor = glm::vec3(1.0f);
	glm::vec3 ambientBottomColor = glm::vec3(0.5f, 0.71f, 1.0f);
	float ambientMult = 0.265f;
	float detailScale = 9.5f;
	float highCloudsCoverage = 0.55f;
	float highCloudsTimeScale = 0.01f;
	float silverLiningIntensity = 0.65f;
	float silverLiningSpread = 0.88f;
	float forwardSilverLiningIntensity = 0.25f;

	float camFov = camera.GetFOV();

	bool shaderReload = false;

	Profiler &profiler = window.GetProfiler();

	float timer = 0.0f;
	bool performStrike = false;
	float lightningIntensity = 0.0f;
	glm::vec3 lightningPos = glm::vec3(0.0f, 3200.0f, 0.0f);

	float lightShaftsDensity = 0.33f;
	float decay = 1.0f;
	float weight = 2.0f;
	float exposure = 0.0034f;

	glm::mat4 previousView;
	
	float frameNumbers_2[4];
	float frameNumbers_4[16];
	float frameNumbers_8[64];

	// For block size 2
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		frameNumbers_2[i] = i;
	}
	while (i-- > 0)
	{
		int k = frameNumbers_2[i];
		int j = (int)(Random::Float() * 1000.0f) % 4;
		frameNumbers_2[i] = frameNumbers_2[j];
		frameNumbers_2[j] = k;
	}

	// For block size 4
	i = 0;
	for (i = 0; i < 16; i++)
	{
		frameNumbers_4[i] = i;
	}
	while (i-- > 0)
	{
		int k = frameNumbers_4[i];
		int j = (int)(Random::Float() * 1000.0f) % 16;
		frameNumbers_4[i] = frameNumbers_4[j];
		frameNumbers_4[j] = k;
	}

	// For block size 8
	i = 0;
	for (i = 0; i < 64; i++)
	{
		frameNumbers_8[i] = i;
	}
	while (i-- > 0)
	{
		int k = frameNumbers_8[i];
		int j = (int)(Random::Float() * 1000.0f) % 64;
		frameNumbers_8[i] = frameNumbers_8[j];
		frameNumbers_8[j] = k;
	}

	bool firstFrame = true;
	int frameCount = 0;
	unsigned int frameNumber = frameNumbers_4[0];
	int updateBlockSizeIndex = 2;

	while (!window.ShouldClose())
	{
		window.Update();
		if (window.WasResized())
		{
			width = window.GetWidth();
			height = window.GetHeight();
			camera.SetProjectionMatrix(camera.GetFOV(), (float)width / height, 0.5f, 1000.0f);

			cloudFBWidth = width / 2;
			cloudFBHeight = height / 2;
			while (cloudFBWidth % cloudFBUpdateBlockSize != 0)
			{
				cloudFBWidth++;
			}
			while (cloudFBHeight % cloudFBUpdateBlockSize != 0)
			{
				cloudFBHeight++;
			}
			cloudFBUpdateTextureWidth = cloudFBWidth / cloudFBUpdateBlockSize;
			cloudFBUpdateTextureHeight = cloudFBHeight / cloudFBUpdateBlockSize;
			CreateFramebuffers();
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

		tod.Update(window.GetDeltaTime());
		const TimeInfo &timeInfo = tod.GetCurrentTimeInfo();

		if (Input::WasKeyPressed(GLFW_KEY_R))
		{
			cloudShader.Reload();
			postQuadShader.Reload();
		}

		if (performStrike)
		{
			float deltaTime = window.GetDeltaTime();

			timer += deltaTime;

			lightningIntensity = std::sin(Random::Float(-1.0f, 1.0f)) * 0.5f + 0.5f;
			lightningIntensity *= 2.0f;

			if (timer >= 0.35f)
			{
				performStrike = false;
				timer = 0.0f;
				lightningIntensity = 0.0f;
			}
		}

		// Create the jitter matrix that will be use to translate the projection matrix
		int x = frameNumber % cloudFBUpdateBlockSize;
		int y = frameNumber / cloudFBUpdateBlockSize;

		float offsetX = x * 2.0f / cloudFBWidth;		// The size of a pixel is 1/width but because the range we'er applying the offset is in [-1,1] we have to multiply by 2 to get the size of a pixel
		float offsetY = y * 2.0f / cloudFBHeight;

		glm::mat4 jitterMatrix = glm::mat4(1.0f);
		jitterMatrix[3] = glm::vec4(offsetX, offsetY, 0.0f, 1.0f);

		glm::mat4 proj = camera.GetProjectionMatrix();
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 invProj = glm::inverse(proj);
		glm::mat4 invProjJitter = invProj * jitterMatrix;
		glm::mat4 invView = glm::inverse(view);
	

		profiler.BeginQuery("Clouds");

		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindFramebuffer(GL_FRAMEBUFFER, cloudQuarterFB);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, cloudFBUpdateTextureWidth, cloudFBUpdateTextureHeight);

		cloudShader.Use();
		cloudShader.SetMat4("invProj", invProjJitter);
		cloudShader.SetMat4("invView", invView);
		cloudShader.SetVec3("camPos", camera.GetPos());
		cloudShader.SetFloat("cloudCoverage", coverage);
		cloudShader.SetFloat("cloudStartHeight", cloudStartHeight);
		cloudShader.SetFloat("cloudLayerThickness", cloudLayerThickness);
		cloudShader.SetFloat("cloudLayerTopHeight", cloudStartHeight + cloudLayerThickness);
		cloudShader.SetVec3("lightDir", glm::vec3(timeInfo.dirLightDirection.x*-1.0f, timeInfo.dirLightDirection.y, timeInfo.dirLightDirection.z));
		cloudShader.SetVec3("sunLightColor", timeInfo.dirLightColor);
		cloudShader.SetFloat("time", (float)glfwGetTime());
		cloudShader.SetFloat("timeScale", timeScale);
		cloudShader.SetFloat("hgForward", hgForward);
		cloudShader.SetFloat("hgBackward", hgBackward);
		cloudShader.SetFloat("densityMult", density);
		cloudShader.SetFloat("directLightMult", directLightMult);
		cloudShader.SetVec3("ambientBottomColor", ambientBottomColor);
		cloudShader.SetVec3("ambientTopColor", ambientTopColor);
		cloudShader.SetFloat("ambientMult", timeInfo.ambient);
		cloudShader.SetFloat("detailScale", detailScale);
		cloudShader.SetFloat("highCloudsCoverage", highCloudsCoverage);
		cloudShader.SetFloat("highCloudsTimeScale", highCloudsTimeScale);
		cloudShader.SetFloat("silverLiningIntensity", silverLiningIntensity);
		cloudShader.SetFloat("silverLiningSpread", silverLiningSpread);
		cloudShader.SetFloat("forwardSilverLiningIntensity", forwardSilverLiningIntensity);
		//cloudShader.SetFloat("lightningIntensity", lightningIntensity);
		//cloudShader.SetVec3("lightningPos", lightningPos);
		glBindTextureUnit(0, noiseTexture);
		glBindTextureUnit(1, weatherTexture);
		glBindTextureUnit(2, highFreqNoiseTexture);
		glBindTextureUnit(3, highCloudsTexture);
		glBindTextureUnit(4, verticalCoverageTexture);
		//glBindTextureUnit(7, curlNoiseTexture);
		q.Render();

		profiler.EndQuery();

		profiler.BeginQuery("Cloud reprojection");
		glBindFramebuffer(GL_FRAMEBUFFER, cloudsFB);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, cloudFBWidth, cloudFBHeight);

		cloudReprojectionShader.Use();
		cloudReprojectionShader.SetMat4("invProj", invProj);
		cloudReprojectionShader.SetMat4("invView", invView);
		cloudReprojectionShader.SetMat4("invProjView", invProj * invView);
		cloudReprojectionShader.SetMat4("prevView", previousView);
		cloudReprojectionShader.SetMat4("proj", proj);
		cloudReprojectionShader.SetVec2("cloudRTDim", glm::vec2(cloudFBWidth, cloudFBHeight));
		cloudReprojectionShader.SetVec2("cloudSubRTDim", glm::vec2(cloudFBUpdateTextureWidth, cloudFBUpdateTextureHeight));
		cloudReprojectionShader.SetFloat("updateBlockSize", cloudFBUpdateBlockSize);
		cloudReprojectionShader.SetFloat("frameNumber", frameNumber);

		glBindTextureUnit(0, cloudsQuarterTexture);
		glBindTextureUnit(1, prevFrameCloudTexture);

		q.Render();

		profiler.EndQuery();

		
		profiler.BeginQuery("Post process quad");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);

		glm::vec3 fakeLightPos = camera.GetPos() + glm::vec3(timeInfo.dirLightDirection.x*-1.0f, timeInfo.dirLightDirection.y, timeInfo.dirLightDirection.z) * 100.0f;
		glm::vec4 lightClipPos = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(fakeLightPos, 1.0f);
		lightClipPos.x /= lightClipPos.w;
		lightClipPos.y /= lightClipPos.w;
		lightClipPos.z /= lightClipPos.w;

		glm::vec2 lightScreenPos = glm::vec2((lightClipPos.x + 1.0f) / 2.0f, (lightClipPos.y + 1.0f) / 2.0f);

		float lightShaftsIntensity = 1.0f - std::sqrtf(lightScreenPos.x * lightScreenPos.x + lightScreenPos.y * lightScreenPos.y) * 0.15f;
		lightShaftsIntensity = std::min(1.0f, lightShaftsIntensity);
		lightShaftsIntensity = std::max(0.0f, lightShaftsIntensity);

		if (lightClipPos.z < 0.0f || lightClipPos.z > 1.0f)
			lightShaftsIntensity = 0.0f;
		else
			lightShaftsIntensity = 1.0f * (lightShaftsIntensity * lightShaftsIntensity * lightShaftsIntensity);

		postQuadShader.Use();
		postQuadShader.SetMat4("invProj", invProj);
		postQuadShader.SetMat4("invView", invView);
		postQuadShader.SetVec3("camPos", camera.GetPos());
		postQuadShader.SetVec3("lightDir", glm::vec3(timeInfo.dirLightDirection.x*-1.0f, timeInfo.dirLightDirection.y, timeInfo.dirLightDirection.z));
		postQuadShader.SetVec2("lightScreenPos", lightScreenPos);
		postQuadShader.SetFloat("lightShaftsIntensity", lightShaftsIntensity);
		postQuadShader.SetVec3("lightShaftsColor", timeInfo.dirLightColor);
		postQuadShader.SetFloat("density", lightShaftsDensity);
		postQuadShader.SetFloat("decay", decay);
		postQuadShader.SetFloat("weight", weight);
		postQuadShader.SetFloat("exposure", exposure);
		glBindTextureUnit(0, cloudsTexture);
		glBindTextureUnit(1, transmittanceTexture);
		glBindTextureUnit(2, inscatterTexture);
		q.Render();
		
		profiler.EndQuery();

		glBlitNamedFramebuffer(cloudsFB, prevFrameFB, 0, 0, cloudFBWidth, cloudFBHeight, 0, 0, cloudFBWidth, cloudFBHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		if (ImGui::DragFloat("Time of day", &timeOfDay, 0.02f, 0.0f, 24.0f, "%.2f"))
		{
			if (timeOfDay > 24.0f)
				timeOfDay = 24.0f;
			else if (timeOfDay < 0.0f)
				timeOfDay = 0.0f;
			tod.SetCurrentTime(timeOfDay);
		}
		if (ImGui::SliderFloat("FOV", &camFov, 0.0f, 180.0f))
			camera.SetFOV(camFov);		
		ImGui::SliderFloat("Coverage", &coverage, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("High Clouds Coverage", &highCloudsCoverage, 0.0f, 1.0f, "%.2f");

		const char* items[] = { "1", "2", "4", "8" };
		if (ImGui::Combo("Update block size", &updateBlockSizeIndex, items, 4))
		{
			if (updateBlockSizeIndex == 0)
				cloudFBUpdateBlockSize = 1;
			else if (updateBlockSizeIndex == 1)
				cloudFBUpdateBlockSize = 2;
			else if (updateBlockSizeIndex == 2)
				cloudFBUpdateBlockSize = 4;
			else if (updateBlockSizeIndex == 3)
				cloudFBUpdateBlockSize = 8;

			while (cloudFBWidth % cloudFBUpdateBlockSize != 0)
			{
				cloudFBWidth++;
			}
			while (cloudFBHeight % cloudFBUpdateBlockSize != 0)
			{
				cloudFBHeight++;
			}
			cloudFBUpdateTextureWidth = cloudFBWidth / cloudFBUpdateBlockSize;
			cloudFBUpdateTextureHeight = cloudFBHeight / cloudFBUpdateBlockSize;

			CreateFramebuffers();
		}

		ImGui::SliderFloat("Cloud start height", &cloudStartHeight, 0.0f, 5000.0f, "%.1f");
		ImGui::SliderFloat("Cloud layer thickness", &cloudLayerThickness, 0.0f, 12000.0f, "%.1f");
		ImGui::SliderFloat("Detail scale", &detailScale, 0.0f, 16.0f);
		ImGui::DragFloat("Time scale", &timeScale, 0.001f, 0.0f, 0.009f, "%.5f");
		ImGui::DragFloat("High clouds time scale", &highCloudsTimeScale, 0.001f, 0.0f, 0.05f, "%.4f");
		if (ImGui::CollapsingHeader("Lighting"))
		{
			ImGui::SliderFloat("Eccentricity", &hgForward, -1.0f, 1.0f);
			ImGui::SliderFloat("Forward silver lining intensity", &forwardSilverLiningIntensity, 0.0f, 1.0f);
			ImGui::SliderFloat("Silver lining intensity", &silverLiningIntensity, 0.0f, 1.0f);
			ImGui::SliderFloat("Silver lining spread", &silverLiningSpread, 0.0f, 1.0f);
			ImGui::SliderFloat("Density mult", &density, 0.0f, 8.0f);
			ImGui::SliderFloat("Direct Light mult", &directLightMult, 0.0f, 4.0f);
			ImGui::ColorEdit3("Ambient bottom color", &ambientBottomColor.x);
			ImGui::ColorEdit3("Ambient top color", &ambientTopColor.x);
			ImGui::SliderFloat("Ambient mult", &ambientMult, 0.0f, 4.0f);
			ImGui::Separator();
		}	
		if (ImGui::CollapsingHeader("Light shafts"))
		{
			ImGui::SliderFloat("Light shafts density", &lightShaftsDensity, 0.0f, 1.0f);
			ImGui::SliderFloat("Light shafts decay", &decay, 0.0f, 1.1f);
			ImGui::SliderFloat("Light shafts weight", &weight, 0.0f, 15.0f);
			ImGui::SliderFloat("Light shafts exposure", &exposure, 0.0f, 0.005f, "%.4f");
			ImGui::Separator();
		}		
		if (ImGui::SliderFloat("Cam speed", &camSpeed, 0.0f, 3000.0f, "%.1f"))
			camera.SetMoveSpeed(camSpeed);
		if (ImGui::Button("Lightning"))
		{
			performStrike = true;
			lightningPos.x = Random::Float(-5000.0f, 5000.0f);
			lightningPos.z = Random::Float(-5000.0f, 5000.0f);
		}
		/*ImGui::Text("Press WASD to move");
		ImGui::Text("Right click and move the mouse to look around");*/

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		profiler.EndQuery();
		if (Input::IsKeyPressed(GLFW_KEY_SPACE))
		{
			profiler.BeginQuery("Text");
			//glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);

			textRenderer.AddText("CPU time: " + std::to_string(window.GetDeltaTime() * 1000.0f) + " ms", glm::vec2(30.0f, 460.0f), glm::vec2(0.3f, 0.3f));
			textRenderer.AddText(profiler.GetResults(), glm::vec2(30.0f, 420.0f), glm::vec2(0.3f, 0.3f));
			textRenderer.Render();
			glDepthMask(GL_TRUE);

			profiler.EndQuery();
		}
		profiler.EndFrame();

		previousView = view;

		
		//frameNumber = (frameNumber + 1) % (cloudFBUpdateBlockSize * cloudFBUpdateBlockSize);
		frameCount++;
		if (cloudFBUpdateBlockSize == 2)
			frameNumber = frameNumbers_2[frameCount % (cloudFBUpdateBlockSize * cloudFBUpdateBlockSize)];
		else if (cloudFBUpdateBlockSize == 4)
			frameNumber = frameNumbers_4[frameCount % (cloudFBUpdateBlockSize * cloudFBUpdateBlockSize)];
		else if (cloudFBUpdateBlockSize == 8)
			frameNumber = frameNumbers_8[frameCount % (cloudFBUpdateBlockSize * cloudFBUpdateBlockSize)];
		
		window.SwapBuffers();	
	}

	q.Dispose();
	cube.Dispose();

	cloudShader.Dispose();
	postQuadShader.Dispose();
	cubeShader.Dispose();

	glDeleteTextures(1, &noiseTexture);
	glDeleteTextures(1, &transmittanceTexture);
	glDeleteTextures(1, &inscatterTexture);
	glDeleteTextures(1, &weatherTexture);
	glDeleteTextures(1, &curlNoiseTexture);
	glDeleteTextures(1, &verticalCoverageTexture);
	glDeleteTextures(1, &highCloudsTexture);

	window.Dispose();

	return 0;
}
