#include <iostream>

#include <glad/glad.h>
#include <GLFW\glfw3.h>

#include "opengl/Shader.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <theme.h>

#include "PerlinNoise1DVisualizer.h"
#include "PerlinNoise2DVisualizer.h"

#ifdef _WIN32
	#include <Windows.h>
#endif

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main()
{

#ifdef _WIN32
	FreeConsole(); //Hide console on Windows 
#endif

	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	//Opengl Loader
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ApplyTheme1();

	// Setup Platform/Renderer bindings
	const char* glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImFont* font = io.Fonts->AddFontFromFileTTF("res/font/bebas/Bebas-Regular.otf", 17.0f);
	IM_ASSERT(font != NULL);

	// Setup ImPlot context
	ImPlot::CreateContext();
	ImPlot::GetStyle().AntiAliasedLines = false;

	PerlinNoise1DVisualizer PerlinNoise1DVisualizer(500);
	PerlinNoise2DVisualizer PerlinNoise2DVisualizer(300, 50);

	int display_w = 0, display_h = 0;
	while (!glfwWindowShouldClose(window))
	{
		//inputs
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();



		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("Theme"))
		{
			if (ImGui::Button("Light"))
				ApplyTheme6();
			if (ImGui::Button("Dark"))
				ApplyTheme1();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		//Setup Window
		ImGui::SetNextWindowPos(ImVec2(0, 25));
		ImGui::SetNextWindowSize(ImVec2(display_w * .35f, display_h -25));
		ImGui::Begin("Setup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		PerlinNoise2DVisualizer.ShowSetup();
		ImGui::End(); 

		//Scene Window : texture, terrain etc...
		ImGui::SetNextWindowPos(ImVec2(display_w * .35f, 25));
		ImGui::SetNextWindowSize(ImVec2(display_w * .65f, display_h - 25));
		ImGui::Begin("Scene", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::BeginTabBar("SceneTabBar");
		if (ImGui::BeginTabItem("Texture"))
		{
			PerlinNoise2DVisualizer.ResponsiveImg(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
			PerlinNoise2DVisualizer.ShowTexture();
			PerlinNoise2DVisualizer.Update();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Terrain"))
		{
			ImGui::Text("Terrain *WIP*");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
		ImGui::End();


		// Rendering
		ImGui::Render();
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImPlot::DestroyContext();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}