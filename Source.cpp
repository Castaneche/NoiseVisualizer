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

		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();
		}

		delta = deltaClock.restart();

		// ===== FPS CALCULATION ====
		float fps = 1.0f / delta.asSeconds();

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

		ImGui::Begin("Perlin Noise", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(float(window.getSize().x), float(window.getSize().y)));

		if (ImGui::CollapsingHeader("[1D] Perlin Noise : Graph, Hills"))
		{
			PN1DVisualizer.Show();
		}
		if (ImGui::CollapsingHeader("[2D] Perlin Noise : Texture, Terrain"))
		{
			PN2DVisualizer.Show();
		}

		ImGui::SetCursorPosX(280);
		ImGui::Text("Click on headers to open them");
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