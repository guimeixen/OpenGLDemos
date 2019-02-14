#include "Window.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\matrix_access.hpp"

#include "ResourcesLoader.h"
#include "Random.h"
#include "Quad.h"
#include "Shader.h"
#include "Camera.h"
#include "Cube.h"
#include "Sphere.h"
#include "Utils.h"
#include "TextRenderer.h"
#include "Plane.h"
#include "Model.h"
#include "TimeOfDayManager.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <fstream>
#include <iostream>

float waterHeight = 0.0f;
Plane waterTopPlane, waterBottomPlane;

void IntersectFrustumEdgeWaterPlane(const glm::vec3 &start, const glm::vec3 &end, std::vector<glm::vec3>& points)
{
	glm::vec3 delta = end - start;
	glm::vec3 dir = glm::normalize(delta);
	float length = glm::length(delta);

	float distance = 0.0f;

	if (waterTopPlane.IntersectRay(start, dir, distance))
	{
		if (distance <= length)
		{
			glm::vec3 hitPos = start + dir * distance;

			points.push_back(glm::vec3(hitPos.x, waterHeight, hitPos.z));
		}
	}

	if (waterBottomPlane.IntersectRay(start, dir, distance))
	{
		if (distance <= length)
		{
			glm::vec3 hitPos = start + dir * distance;

			points.push_back(glm::vec3(hitPos.x, waterHeight, hitPos.z));
		}
	}
}

void IntersectFrustumWaterPlane(const Frustum &frustum, std::vector<glm::vec3>& points)
{
	glm::vec3 corners[8];
	corners[0] = frustum.nbl;
	corners[1] = frustum.ntl;
	corners[2] = frustum.ntr;
	corners[3] = frustum.nbr;

	corners[4] = frustum.fbl;
	corners[5] = frustum.ftl;
	corners[6] = frustum.ftr;
	corners[7] = frustum.fbr;

	IntersectFrustumEdgeWaterPlane(corners[0], corners[1], points);
	IntersectFrustumEdgeWaterPlane(corners[1], corners[2], points);
	IntersectFrustumEdgeWaterPlane(corners[2], corners[3], points);
	IntersectFrustumEdgeWaterPlane(corners[3], corners[0], points);

	IntersectFrustumEdgeWaterPlane(corners[4], corners[5], points);
	IntersectFrustumEdgeWaterPlane(corners[5], corners[6], points);
	IntersectFrustumEdgeWaterPlane(corners[6], corners[7], points);
	IntersectFrustumEdgeWaterPlane(corners[7], corners[4], points);

	IntersectFrustumEdgeWaterPlane(corners[0], corners[4], points);
	IntersectFrustumEdgeWaterPlane(corners[1], corners[5], points);
	IntersectFrustumEdgeWaterPlane(corners[2], corners[6], points);
	IntersectFrustumEdgeWaterPlane(corners[3], corners[7], points);
}

bool SegmentPlaneIntersection(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &n, float d, glm::vec3 &q)
{
	glm::vec3 ab = b - a;
	float t = (d - glm::dot(n, a)) / glm::dot(n, ab);

	if (t > -0.0f && t <= 1.0f)
	{
		q = a + t * ab;
		return true;
	}

	return false;
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
	camera.SetProjectionMatrix(75.0f, (float)width / height, 0.2f, 1000.0f);
	camera.SetPos(glm::vec3(0.0f));

	TimeOfDayManager tod;
	tod.Init();

	Shader shader;
	shader.Load("Data/Shaders/ProjGridWater/cube.vert", "Data/Shaders/ProjGridWater/cube.frag");

	Shader waterShader;
	waterShader.Load("Data/Shaders/ProjGridWater/water.vert", "Data/Shaders/ProjGridWater/water.frag");

	Shader skydomeShader;
	skydomeShader.Load("Data/Shaders/ProjGridWater/skydome.vert", "Data/Shaders/ProjGridWater/skydome.frag");

	Sphere skydome;
	skydome.Load();

	Model dragon;
	dragon.Load("Data/Models/dragon.obj");
	Model platform;
	platform.Load("Data/Models/platform.obj");

	GLuint inscatterTexture = utils::Load3DTexture("Data/Textures/inscatter.raw", 256, 128, 32, 4, GL_FLOAT);
	GLuint transmittanceTexture = utils::LoadRAW2DTexture("Data/Textures/transmittance.raw", 256, 64, 3, GL_FLOAT);
	GLuint normalMap = utils::LoadTexture("Data/Textures/oceanwaves_ddn2.png", false);

	const unsigned int resolution = 128;
	std::vector<glm::vec2> vertices(resolution * resolution);
	std::vector<unsigned short> indices((resolution - 1) * (resolution - 1) * 6);

	for (size_t x = 0; x < resolution; x++)
	{
		for (size_t z = 0; z < resolution; z++)
		{
			vertices[x * resolution + z] = glm::vec2((float)x / (resolution - 1), (float)z / (resolution - 1));
		}
	}

	int index = 0;

	for (size_t x = 0; x < resolution - 1; x++)
	{
		for (size_t z = 0; z < resolution - 1; z++)
		{
			indices[index++] = (x + 0) * resolution + (z + 0);
			indices[index++] = (x + 0) * resolution + (z + 1);
			indices[index++] = (x + 1) * resolution + (z + 1);

			indices[index++] = (x + 0) * resolution + (z + 0);
			indices[index++] = (x + 1) * resolution + (z + 1);
			indices[index++] = (x + 1) * resolution + (z + 0);
		}
	}

	GLuint vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

	glBindVertexArray(0);

	GLuint reflectionFBO;
	glGenFramebuffers(1, &reflectionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);

	GLuint reflectionTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &reflectionTexture);
	glTextureParameteri(reflectionTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(reflectionTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(reflectionTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(reflectionTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(reflectionTexture, 1, GL_RGBA8, width / 2, height / 2);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glNamedRenderbufferStorage(rbo, GL_DEPTH_COMPONENT24, width / 2, height / 2);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete: " << fboStatus << "\n";
		return -1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint refractionFBO;
	glCreateFramebuffers(1, &refractionFBO);

	GLuint refractionTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &refractionTexture);
	glTextureParameteri(refractionTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(refractionTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(refractionTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(refractionTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureStorage2D(refractionTexture, 1, GL_RGBA8, width / 2, height / 2);

	GLuint refractionDepth;
	glCreateTextures(GL_TEXTURE_2D, 1, &refractionDepth);
	glTextureParameteri(refractionDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(refractionDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(refractionDepth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(refractionDepth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureStorage2D(refractionDepth, 1, GL_DEPTH_COMPONENT24, width / 2, height / 2);

	glNamedFramebufferTexture(refractionFBO, GL_COLOR_ATTACHMENT0, refractionTexture, 0);
	glNamedFramebufferTexture(refractionFBO, GL_DEPTH_ATTACHMENT, refractionDepth, 0);

	if (glCheckNamedFramebufferStatus(refractionFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete: " << fboStatus << "\n";
		return -1;
	}

	std::vector<glm::vec3> points;
	glm::vec4 viewCorners[4];
	glm::mat4 viewFrame;
	float sum = 1.5f;
	float scale = 18.0f;
	float timeOfDay = 12.0f;

	glm::vec2 normalMapOffset0 = glm::vec2();
	glm::vec2 normalMapOffset1 = glm::vec2();

	Plane waterPlane;
	waterPlane.SetNormalAndPoint(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, waterHeight, 0.0f));
	waterBottomPlane.SetNormalAndPoint(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, waterHeight - sum, 0.0f));
	waterTopPlane.SetNormalAndPoint(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, waterHeight + sum, 0.0f));

	glm::vec3 frustumCornersWorld[8];

	int frustumIndices[12][2] = {
		{ 0,1 },
	{ 1,2 },
	{ 2,3 },
	{ 3,0 },
	{ 4,5 },
	{ 5,6 },
	{ 6,7 },
	{ 7,4 },
	{ 0,4 },
	{ 1,5 },
	{ 2,6 },
	{ 3,7 }
	};

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

		tod.Update(window.GetDeltaTime());
		const TimeInfo &timeInfo = tod.GetCurrentTimeInfo();

		points.clear();
		//IntersectFrustumWaterPlane(camera.GetFrustum(), points);

		/*if (points.size() > 0)
		{*/
		glm::vec3 camPos = camera.GetPos();

		float range = std::max(0.0f, 10.0f) + 5.0f;

		if (camPos.y < waterHeight)
		{
			camPos.y = std::min(camPos.y, waterHeight - range);
		}
		else
		{
			camPos.y = std::max(camPos.y, waterHeight + range);
		}

		//float minCamHeight = waterHeight + sum + 10.0f;
		//camPos.y = std::max(minCamHeight, camPos.y);

		glm::vec3 focus = camera.GetPos() + camera.GetForward() * scale;
		focus.y = waterHeight;

		viewFrame = glm::lookAt(camPos, focus, glm::vec3(0.0f, 1.0f, 0.0f));

		// Construct view and projection matrices
		glm::mat4 projectorViewProj = camera.GetProjectionMatrix() * viewFrame;

		const Frustum &frustum = camera.GetFrustum();
		frustumCornersWorld[0] = frustum.nbl;
		frustumCornersWorld[1] = frustum.ntl;
		frustumCornersWorld[2] = frustum.ntr;
		frustumCornersWorld[3] = frustum.nbr;

		frustumCornersWorld[4] = frustum.fbl;
		frustumCornersWorld[5] = frustum.ftl;
		frustumCornersWorld[6] = frustum.ftr;
		frustumCornersWorld[7] = frustum.fbr;

		range = std::max(1.0f, 10.0f);

		// For each corner if its world space position is  
		// between the wave range then add it to the list.
		for (size_t i = 0; i < 8; i++)
		{
			if (frustumCornersWorld[i].y <= waterHeight + range && frustumCornersWorld[i].y >= waterHeight - range)
			{
				points.push_back(frustumCornersWorld[i]);
			}
		}

		// Now take each segment in the frustum box and check
		// to see if it intersects the ocean plane on both the
		// upper and lower ranges.
		for (size_t i = 0; i < 12; i++)
		{
			glm::vec3 p0 = frustumCornersWorld[frustumIndices[i][0]];
			glm::vec3 p1 = frustumCornersWorld[frustumIndices[i][1]];

			glm::vec3 max, min;
			if (SegmentPlaneIntersection(p0, p1, glm::vec3(0.0f, 1.0f, 0.0f), waterHeight + range, max))
			{
				points.push_back(max);
			}

			if (SegmentPlaneIntersection(p0, p1, glm::vec3(0.0f, 1.0f, 0.0f), waterHeight - range, min))
			{
				points.push_back(min);
			}
		}

		float xmin = std::numeric_limits<float>::max();
		float ymin = std::numeric_limits<float>::max();
		float xmax = std::numeric_limits<float>::min();
		float ymax = std::numeric_limits<float>::min();
		glm::vec4 q = glm::vec4(0.0f);
		glm::vec4 p = glm::vec4(0.0f);

		// Now convert each world space position into
		// projector screen space. The min/max x/y values
		// are then used for the range conversion matrix.
		// Calculate the x and y span of vVisible in projector space
		for (size_t i = 0; i < points.size(); i++)
		{
			// Project the points of intersection between the frustum and the waterTop or waterBottom plane to the waterPlane
			q.x = points[i].x;
			q.y = waterHeight;
			q.z = points[i].z;
			q.w = 1.0f;

			// Now transform the points to projector space
			p = projectorViewProj * q;
			p.x /= p.w;
			p.y /= p.w;

			if (p.x < xmin) xmin = p.x;
			if (p.y < ymin) ymin = p.y;
			if (p.x > xmax) xmax = p.x;
			if (p.y > ymax) ymax = p.y;
		}

		// Create a matrix that transform the [0,1] range to [xmin,xmax] and [ymin,ymax] and leave the z and w intact
		glm::mat4 rangeMap;
		rangeMap = glm::row(rangeMap, 0, glm::vec4(xmax - xmin, 0.0f, 0.0f, xmin));
		rangeMap = glm::row(rangeMap, 1, glm::vec4(0.0f, ymax - ymin, 0.0f, ymin));
		rangeMap = glm::row(rangeMap, 2, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
		rangeMap = glm::row(rangeMap, 3, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		// Now update the projector matrix with the range conversion matrix
		glm::mat4 projectorToWorld = glm::inverse(projectorViewProj) * rangeMap;

		glm::vec2 ndcCorners[4];
		ndcCorners[0] = glm::vec2(0.0f, 0.0f);
		ndcCorners[1] = glm::vec2(1.0f, 0.0f);
		ndcCorners[2] = glm::vec2(1.0f, 1.0f);
		ndcCorners[3] = glm::vec2(0.0f, 1.0f);

		// Now transform the corners of the 
		for (int i = 0; i < 4; i++)
		{
			glm::vec4 a, b;

			// Transform the ndc corners to world space
			a = projectorToWorld * glm::vec4(ndcCorners[i].x, ndcCorners[i].y, -1.0f, 1.0f);
			b = projectorToWorld * glm::vec4(ndcCorners[i].x, ndcCorners[i].y, 1.0f, 1.0f);

			// And calculate the intersection between the line made by this two points and the water plane
			// in homogeneous space
			// The rest of the grid vertices will then be interpolated in the vertex shader
			float h = waterHeight;

			glm::vec4 ab = b - a;

			float t = (a.w * h - a.y) / (ab.y - ab.w * h);

			viewCorners[i] = a + ab * t;
		}
		//}

		profiler.BeginQuery("Reflection");

		glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
		glViewport(0, 0, width / 2, height / 2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);

		camPos = camera.GetPos();
		float dist = 2.0f * (camPos.y - waterHeight);
		camPos.y -= dist;
		camera.SetPos(camPos);
		float pitch = camera.GetPitch();
		camera.SetPitch(-pitch);


		skydomeShader.Use();
		skydomeShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		skydomeShader.SetVec3("camPos", camPos);
		skydomeShader.SetVec3("lightDir", timeInfo.dirLightDirection);
		glBindTextureUnit(0, transmittanceTexture);
		glBindTextureUnit(1, inscatterTexture);
		skydome.Render();
		glDepthFunc(GL_LESS);
		glCullFace(GL_BACK);

		shader.Use();
		shader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		shader.SetVec3("sunDir", timeInfo.dirLightDirection);
		shader.SetVec3("sunLightColor", timeInfo.dirLightColor);

		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -10.0f));
		shader.SetMat4("modelMatrix", m);
		dragon.Render();

		m = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, 0.0f));
		m = glm::scale(m, glm::vec3(8.0f));
		shader.SetMat4("modelMatrix", m);
		platform.Render();

		camPos.y += dist;
		camera.SetPos(camPos);
		camera.SetPitch(pitch);

		profiler.EndQuery();

		profiler.BeginQuery("Refraction");

		glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
		glClearColor(0.08f, 0.16f, 0.3f, 1.0f);
		glViewport(0, 0, width / 2, height / 2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();
		shader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		shader.SetVec3("sunDir", timeInfo.dirLightDirection);
		shader.SetVec3("sunLightColor", timeInfo.dirLightColor);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, 0.0f));
		m = glm::scale(m, glm::vec3(8.0f));
		shader.SetMat4("modelMatrix", m);
		platform.Render();

		profiler.EndQuery();

		profiler.BeginQuery("Main");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);
		skydomeShader.Use();
		skydomeShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		skydomeShader.SetVec3("camPos", camera.GetPos());
		skydomeShader.SetVec3("lightDir", timeInfo.dirLightDirection);
		glBindTextureUnit(0, transmittanceTexture);
		glBindTextureUnit(1, inscatterTexture);
		skydome.Render();
		glDepthFunc(GL_LESS);
		glCullFace(GL_BACK);

		shader.Use();
		shader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
		shader.SetVec3("sunDir", timeInfo.dirLightDirection);
		shader.SetVec3("sunLightColor", timeInfo.dirLightColor);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -10.0f));
		shader.SetMat4("modelMatrix", m);
		dragon.Render();

		m = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, 0.0f));
		m = glm::scale(m, glm::vec3(8.0f));
		shader.SetMat4("modelMatrix", m);
		platform.Render();

		if (points.size() > 0)
		{
			float nAngle0 = 42.0f * (3.14159f / 180.0f);
			float nAngle1 = 76.0f * (3.14159f / 180.0f);
			float deltaTime = window.GetDeltaTime();
			normalMapOffset0 += glm::vec2(glm::cos(nAngle0), glm::sin(nAngle0)) * 0.055f * deltaTime;
			normalMapOffset1 += glm::vec2(glm::cos(nAngle1), glm::sin(nAngle1)) * 0.025f * deltaTime;

			waterShader.Use();
			waterShader.SetMat4("projView", camera.GetProjectionMatrix() * camera.GetViewMatrix());
			waterShader.SetMat4("projectionMatrix", camera.GetProjectionMatrix());
			waterShader.SetMat4("viewFrame", viewFrame);
			waterShader.SetVec4("viewCorner0", viewCorners[0]);
			waterShader.SetVec4("viewCorner1", viewCorners[1]);
			waterShader.SetVec4("viewCorner2", viewCorners[2]);
			waterShader.SetVec4("viewCorner3", viewCorners[3]);
			waterShader.SetVec3("camPos", camera.GetPos());
			waterShader.SetFloat("time", window.GetElapsedTime());
			waterShader.SetVec4("normalMapOffset", glm::vec4(normalMapOffset0.x, normalMapOffset0.y, normalMapOffset1.x, normalMapOffset1.y));
			waterShader.SetVec3("sunDir", timeInfo.dirLightDirection);
			waterShader.SetVec3("sunLightColor", timeInfo.dirLightColor);

			glBindTextureUnit(0, reflectionTexture);
			glBindTextureUnit(1, refractionTexture);
			glBindTextureUnit(2, refractionDepth);
			glBindTextureUnit(3, normalMap);
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
		}

		profiler.EndQuery();

		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		ImGui::Text("Press WASD to move");
		ImGui::Text("Right click and move the mouse to look around");
		if (ImGui::SliderFloat("Time of day", &timeOfDay, 0.0f, 24.0f, "%.1f"))
		{
			tod.SetCurrentTime(timeOfDay);
		}
		ImGui::SliderFloat("Scale", &scale, 0.0f, 100.0f);
		if (ImGui::SliderFloat("Sum", &sum, 0.0f, 15.0f))
		{
			waterBottomPlane.SetNormalAndPoint(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, waterHeight - sum, 0.0f));
			waterTopPlane.SetNormalAndPoint(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, waterHeight + sum, 0.0f));
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

	shader.Dispose();
	waterShader.Dispose();
	dragon.Dispose();

	window.Dispose();

	return 0;
}
