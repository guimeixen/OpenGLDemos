#include "Window.h"

#include "Random.h"
#include "ResourcesLoader.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <iostream>

Window::Window()
{
}

bool Window::Init(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;

	if (glfwInit() != GLFW_TRUE)
	{
		std::cout << "Failed to initialize GLFW!\n";
		return false;
	}
	else
	{
		std::cout << "GLFW successfuly initialized\n";
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	window = glfwCreateWindow(width, height, "Game", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetWindowPos(window, 550, 50);

	glfwSetWindowUserPointer(window, this);

	glfwSetKeyCallback(window, WindowKeyboardCallback);
	glfwSetCursorPosCallback(window, WindowMouseCallback);
	glfwSetMouseButtonCallback(window, WindowMouseButtonCallback);
	glfwSetWindowFocusCallback(window, WindowFocusCallback);
	glfwSetCharCallback(window, WindowCharCallback);
	glfwSetFramebufferSizeCallback(window, WindowFramebufferSizeCallback);

	glfwSetCursorPos(window, width / 2.0f, height / 2.0f);

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	monitorWidth = mode->width;
	monitorHeight = mode->height;

	//glfwSetScrollCallback(window, WindowScrollCallback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW\n";
		return false;
	}
	else
	{
		std::cout << "GLEW sucessfully initialized\n";
	}

	std::cout << "GL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glGetError();

	// IMGUI
	ImGui::CreateContext();
	ImGuiStyle &style = ImGui::GetStyle();
	style.FrameBorderSize = 1.0f;
	style.WindowRounding = 0.0f;

	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init();

	profiler.Init();
	Random::Init();

	return true;
}

void Window::Dispose()
{
	profiler.Dispose();

	ResourcesLoader::Dispose();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void Window::Update()
{
	inputManager.Reset();
	inputManager.Update();

	glfwPollEvents();

	double currentTime = glfwGetTime();
	deltaTime = (float)currentTime - lastTime;
	lastTime = (float)currentTime;

	elapseTime += deltaTime;
}

void Window::SwapBuffers()
{
	glfwSwapBuffers(window);
}

bool Window::WasResized()
{
	if (wasResized)
	{
		wasResized = false;
		return true;
	}
	return false;
}

void Window::UpdateKeys(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	inputManager.UpdateKeys(key, scancode, action, mods);

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		if (wireframe == false)
		{
			wireframe = true;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			wireframe = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (key == GLFW_KEY_F11 && action == GLFW_RELEASE)
	{
		int count = 0;
		const GLFWvidmode *vidModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

		int maxRefreshRate = 0;

		width = 0;
		height = 0;

		if (!isFullscreen)
		{
			for (int i = 0; i < count; i++)
			{
				if (vidModes[i].width > width && vidModes[i].height > height)
				{
					width = vidModes[i].width;
					height = vidModes[i].height;

					if (vidModes[i].refreshRate > maxRefreshRate)
						maxRefreshRate = vidModes[i].refreshRate;
				}
				//std::cout << vidModes[i].width << " " << vidModes[i].height << " " << vidModes[i].refreshRate << " " << vidModes[i].redBits << " " << vidModes[i].greenBits << " " << vidModes[i].blueBits << std::endl;
			}
			std::cout << width << " " << height << " " << maxRefreshRate << std::endl;
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, width, height, maxRefreshRate);

			isFullscreen = true;
		}
		else
		{
			width = 1280;
			height = 720;

			// Find max refresh rate for 1280x720
			for (int i = 0; i < count; i++)
			{
				if (vidModes[i].width == width && vidModes[i].height == height && vidModes[i].refreshRate > maxRefreshRate)
				{
					maxRefreshRate = vidModes[i].refreshRate;

					if (maxRefreshRate == 60)			// We dont want more than 60hz
						break;
				}
			}
			//std::cout << "Windowed refresh rate:" << maxRefreshRate << std::endl;
			glfwSetWindowMonitor(window, nullptr, 550, 50, width, height, maxRefreshRate);

			isFullscreen = false;
		}

		wasResized = true;
	}
}

void Window::UpdateMousePosition(float xpos, float ypos)
{
	inputManager.SetMousePosition(glm::vec2(xpos, ypos));
}

void Window::UpdateMouseButtonState(int button, int action, int mods)
{
	inputManager.SetMouseButtonState(button, action);
}

void Window::UpdateScroll(double xoffset, double yoffset)
{
	inputManager.SetScrollWheelYOffset(static_cast<float>(yoffset));
}

void Window::UpdateChar(unsigned int c)
{
	inputManager.UpdateChar(c);
}

void Window::UpdateFocus(int focused)
{
	if (focused == GLFW_TRUE)
		isFocused = true;
	else if (focused == GLFW_FALSE)
		isFocused = false;
}

void Window::UpdateFramebufferSize(int width, int height)
{
	//std::cout << "width : " << width << "    "  << height << '\n';
	this->width = static_cast<unsigned int>(width);
	this->height = static_cast<unsigned int>(height);

	if (width == 0 && height == 0)
		isMinimized = true;
	else
	{
		isMinimized = false;
		wasResized = true;
	}
}