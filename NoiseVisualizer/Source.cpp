#define MAHI_GUI_NO_CONSOLE

#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>

using namespace mahi::gui;
using namespace mahi::util;

#include "src/opengl/Shader.h"


#include "src/Graph.h"
#include "src/Texture.h"

#include "src/Setup.h"
#include "src/ColorMapSelector.h"

#include "src/Functions.h"

#include "src/Terrain.h"

// Inherit from Application
class NoiseVisualizer : public Application {
public:
	
	NoiseVisualizer() : Application(800, 600, "NoiseVisualizer") 
	{
		ImGui::StyleColorsLight();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("res/font/Roboto-Regular.ttf", 18.0f);

		//Opengl Loader
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			
		}

		glEnable(GL_DEPTH_TEST);
		set_background(Colors::White);

		setupdata = std::make_shared<SetupData>();
		setupdata->seed = 123;
		setupdata->octaves = 1;
		setupdata->persistence = 1;
		setupdata->frequency = 10;
		setupdata->interpolationMethod = Cosine;
		setupdata->colors = ImGui::COLORMAP_DEFAULT;

		texture.Init(300, 50, setupdata);
		terrain.Init(250, 250, setupdata);
		terrain.loadShader("res/shaders/vertex.vert", "res/shaders/fragment.frag");
	}

	~NoiseVisualizer() {
		setupdata.reset();
	}

	// Override update (called once per frame)
	void update() override {
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::BeginMenu("Theme"))
			{
				if (ImGui::MenuItem("Light"))
					ImGui::StyleColorsLight();
				if (ImGui::MenuItem("Dark"))
					ImGui::StyleColorsMahiDark3();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Graph", NULL, &graph_window);
			ImGui::MenuItem("Texture", NULL, &texture_window);
			ImGui::MenuItem("Terrain", NULL, &terrain_window);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		ImGui::Begin("Setup");
			ShowSetupWindow(*setupdata);
			if (texture_window)
			{
				if (ImGui::CollapsingHeader("Texture Setup", ImGuiTreeNodeFlags_DefaultOpen))
					texture.ShowSetup();
			}
			if (terrain_window)
			{
				if (ImGui::CollapsingHeader("Terrain Setup", ImGuiTreeNodeFlags_DefaultOpen))
					terrain.ShowSetup();
			}
		ImGui::End();

		//Scene Window : texture, terrain etc...
		if (graph_window)
		{
			ImGui::Begin("Graph", &graph_window);
				graph.Show();
			ImGui::End();
		}

		if (terrain_window)
		{
			ImGui::Begin("Terrain", &terrain_window);
				terrain.ShowTerrain();
			ImGui::End();
		}

		if (texture_window)
		{
			ImGui::Begin("Texture", &texture_window, ImGuiWindowFlags_MenuBar);
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::BeginMenu("Save As"))
						{
							ImGui::MenuItem("pgm", NULL, &export_window);
							ImGui::EndMenu();
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}
				texture.ResponsiveImg(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
				texture.ShowTexture();
			ImGui::End();
		}

		if (export_window)
		{
			ImGui::Begin("Export", &export_window);
				ImGui::InputText("name", filename, IM_ARRAYSIZE(filename));
				if (ImGui::Button("Export")) save_image(filename, texture.GetPixels(), texture.GetPixelCount(), texture.GetPixelCount());
			ImGui::End();
		}

		
	}

	void draw() override {
		//======== Draw opengl stuff here ===============
		terrain.RenderTerrain();

		// !!! Don't forget to reset updated boolean to avoid infinite calculation !!!
		setupdata->updated = false;
	}


	bool graph_window = false, terrain_window = true, texture_window = true;

	bool export_window = false;
	char filename[128] = "";
	
	std::shared_ptr<SetupData> setupdata;
	Texture texture;
	Terrain terrain;
	Graph graph = Graph(500);

};

int main() {
	NoiseVisualizer app;
	app.run();
	return 0;
}