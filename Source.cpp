#include <imgui.h>
#include <imgui-SFML.h>
#include <theme.h>
#include <wrappers.h>
#include <implot/implot.h>

#include <SFML/Graphics.hpp>

#include <iostream>

#include <PerlinNoise1DVisualizer.h>
#include <PerlinNoise2DVisualizer.h>

int main()
{
	sf::RenderWindow window(sf::VideoMode(700, 600), "Noise Visualizer", sf::Style::Default);
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);
	ImPlot::CreateContext();
	ImPlot::GetStyle().AntiAliasedLines = false;

	bool focus = true;

	ApplyTheme6();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF("res/font/bebas/Bebas-Regular.ttf", 17.0f);
	ImGui::SFML::UpdateFontTexture();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1);

	if (font == nullptr) std::cout << "Error";

	window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.
	sf::Clock deltaClock;
	sf::Clock fpsClock;
	sf::Time delta;
	int fpsLogRate = 200;
	float fpsvalues[50] = { 0 };

	PerlinNoise1DVisualizer PN1DVisualizer(500);
	PerlinNoise2DVisualizer PN2DVisualizer(300, 200);

	while (window.isOpen()) {
		sf::Event event;

		MousePos = sf::Mouse::getPosition(window);

		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();

			if (event.type == sf::Event::LostFocus)
				focus = false;
			if (event.type == sf::Event::GainedFocus)
				focus = true;

			if (!focus)
				break;
		}

		if (!focus)
			continue;

		delta = deltaClock.restart();

		// ===== FPS CALCULATION ====
		int fps = 1.0f / delta.asSeconds();

		if (fpsClock.getElapsedTime().asMilliseconds() >= fpsLogRate) {
			for (int i = 0; i < 49; i++)
			{
				fpsvalues[i] = fpsvalues[i + 1];
			}
			fpsvalues[49] = fps;
			fpsClock.restart();
		}


		ImGui::SFML::Update(window, delta);
		ImGui::PushFont(font);

		ImGui::Begin("Perlin Noise", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(window.getSize().x, window.getSize().y));

		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open")) {}
			if (ImGui::MenuItem("Open Recent")) {}
			if (ImGui::MenuItem("Save")) {}
			if (ImGui::MenuItem("Save As")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::Checkbox("Anti Aliasing", &ImPlot::GetStyle().AntiAliasedLines);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Layout"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				Separator1 = 0.2f * window.getSize().x;
				Separator2 = 0.7f * window.getSize().x;
				resizeLayout = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::Button("About"))
		{
		}
		ImGui::EndMenuBar();

		if (ImGui::CollapsingHeader("[1D] Perlin Noise"))
		{
			PN1DVisualizer.Show();
		}
		if (ImGui::CollapsingHeader("[2D] Perlin Noise : Texture, Terrain", ImGuiTreeNodeFlags_DefaultOpen))
		{
			PN2DVisualizer.Show();
		}
		ImGui::End();

		ImGui::PopFont();

		window.clear(sf::Color::Black); // fill background with color
		ImGui::SFML::Render(window);
		window.display();
	}

	ImPlot::DestroyContext();
	ImGui::SFML::Shutdown();

	return 0;
}