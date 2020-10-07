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
	sf::RenderWindow window(sf::VideoMode(700, 600), "Noise Visualizer");
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);
	ImPlot::CreateContext();
	ImPlot::GetStyle().AntiAliasedLines = false;

	bool pause = true;
	bool focus = true;

	ApplyTheme6();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF("res/font/bebas/Bebas-Regular.ttf", 17.0f);
	ImGui::SFML::UpdateFontTexture();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1);

	if (font == nullptr) std::cout << "Error";

	// Create a new render-texture
	sf::RenderTexture renderTexture;
	if (!renderTexture.create(600, 600))
		return -1;
	sf::View view;
	view.setCenter(300, 300);
	view.setSize(300, 600);

	float SceneWindowWidth = 0;
	float SceneWindowHeight = 0;

	sf::Texture texture;
	if (!texture.loadFromFile("res/sprite.png"))
		std::cout << "Error loadfromfile sprite.png" << std::endl;

	sf::Sprite sprite(texture);
	sprite.setPosition(0, 0);
	sprite.setScale(4, 4);

	sf::Color bgColor;

	float color[3] = { 0.f, 0.f, 0.f };

	//Layout
	float Separator1 = 0.2f * window.getSize().x;
	float Separator2 = 0.7f * window.getSize().x;
	bool resizeLayout = true; //Alternative to ImGuiCond_Once to allow sfml window resizing -> see sf::Event::resized

	bool displayMousePosition = false;

	sf::Vector2i MousePos;
	sf::Vector2f MouseWorld;

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

			if (event.type == sf::Event::MouseWheelMoved)
			{
				view.zoom(1.0f + (-0.1f * event.mouseWheel.delta));
			}

			if (event.type == sf::Event::Resized)
			{

				//Convert layout from old window to new window 
				float percent1 = Separator1 / window.getView().getSize().x;
				float percent2 = Separator2 / window.getView().getSize().x;

				Separator1 = percent1 * event.size.width;
				Separator2 = percent2 * event.size.width;

				resizeLayout = true;


				// update the view to the new size of the window
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window.setView(sf::View(visibleArea));
			}
		}

		if (!focus)
			continue;

		delta = deltaClock.restart();

		float speed = 2.5;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			view.move(-speed, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			view.move(speed, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			view.move(0, -speed);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			view.move(0, speed);

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



		/*
		if (displayMousePosition)
		{
			ImGui::BeginTooltip();
			ImGui::SetTooltip("%d - %d\n%.0f - %.0f", MousePos.x, MousePos.y, MouseWorld.x, MouseWorld.y);
			ImGui::EndTooltip();
		}

		// =============== MAINMENU ==============
		if (ImGui::BeginMainMenuBar())
		{
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
				if (ImGui::MenuItem("Settings")) {}
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
			ImGui::EndMainMenuBar();
		}


		ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));
		// =============== GLOBAL ==============
		ImGui::Begin("Global", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus); // begin window
		ImGui::SetWindowPos(ImVec2(0, 27));
		if (resizeLayout)
			ImGui::SetWindowSize(ImVec2(Separator1, window.getSize().y));
		Separator1 = ImGui::GetWindowWidth();


		if (ImGui::CollapsingHeader("Setup", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Background color edit
			if (ImGui::ColorEdit3("Background color", color)) {
				// this code gets called if color value changes, so
				// the background color is upgraded automatically!
				bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
				bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
				bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
			}
			ImGui::Checkbox("Display Mouse Position", &displayMousePosition);
		}
		if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Scene Area");
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 80.f);
			ImGui::Text("x = %.0f\ny = %.0f", Separator1, 27.f);
			ImGui::NextColumn();
			ImGui::Text("w = %.0f\nh = %.0f", SceneWindowWidth, SceneWindowHeight);

			ImGui::Columns(1);
			ImGui::Text("\nView Bounds");
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 80.f);
			sf::FloatRect viewbounds = sf::FloatRect(view.getCenter().x - view.getSize().x / 2.,
				view.getCenter().y - view.getSize().y / 2,
				view.getSize().x, view.getSize().y);
			ImGui::Text("x = %.0f\ny = %.0f", viewbounds.left, viewbounds.top);
			ImGui::NextColumn();
			ImGui::Text("w = %.0f\nh = %.0f", viewbounds.width, viewbounds.height);

			ImGui::Columns(1);
			ImGui::Text("\nViewport");
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 80.f);
			sf::FloatRect viewport = view.getViewport();
			ImGui::Text("x = %.0f\ny = %.0f", viewport.left, viewport.top);
			ImGui::NextColumn();
			ImGui::Text("w = %.0f\nh = %.0f", viewport.width, viewport.height);
		}
		ImGui::End(); // end window


		ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));
		// =============== TOOLS ==============
		ImGui::Begin("Tools", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus); // begin window
		if (resizeLayout)
			ImGui::SetWindowPos(ImVec2(Separator2, 27));
		ImGui::SetWindowSize(ImVec2(window.getSize().x - Separator2, window.getSize().y));
		Separator2 = ImGui::GetWindowPos().x;

		if (ImGui::CollapsingHeader("Perf Graph", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.8, 0.4f, 1.0f));
			ImGui::SliderInt("Delay (ms)", &fpsLogRate, 50, 1000);
			ImGui::PlotHistogram("", fpsvalues, 50, 0, "FPS", 0, 80, ImVec2(ImGui::GetWindowWidth() - 20, 100.f));
			ImGui::PopStyleColor();
			ImGui::Text("Current FPS : %.0f", fpsvalues[49]);
		}
		if (ImGui::CollapsingHeader("Perlin Noise", ImGuiTreeNodeFlags_DefaultOpen))
		{

		}

		ImGui::End();
		

		renderTexture.clear(bgColor);
		renderTexture.setView(view);
		renderTexture.display();


		SceneWindowWidth = Separator2 - Separator1 - 15;
		SceneWindowHeight = window.getSize().y - 70.f;
		// =============== SCENE ==============
		ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::SetWindowPos(ImVec2(Separator1, 27));
		ImGui::SetWindowSize(ImVec2(Separator2 - Separator1, window.getSize().y));
		ImGui::ImageRT(renderTexture, sf::Vector2f(SceneWindowWidth, SceneWindowHeight), sf::FloatRect(0, 0, renderTexture.getSize().x, renderTexture.getSize().y), sf::Color::White, sf::Color::Green);
		view.setSize(view.getSize().x, SceneWindowHeight / SceneWindowWidth * view.getSize().x);
		//ImGui::Image(renderTexture.getTexture());

		if (ImGui::IsWindowFocused() && ImGui::IsItemHovered())
		{
			sf::Vector2i MousePosOffsetRM = MousePos - sf::Vector2i(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
			sf::Vector2i MousePosInsideScene = sf::Vector2i((float)MousePosOffsetRM.x / SceneWindowWidth * window.getSize().x,
				(float)MousePosOffsetRM.y / SceneWindowHeight * window.getView().getSize().y);
			MouseWorld = window.mapPixelToCoords(MousePosInsideScene, view);
			pause = false;
		}
		else
		{
			pause = true;
		}
		if (pause)
		{
			ImGui::SetCursorPos(ImVec2(15, 40));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
			ImGui::Text("PAUSED");
			ImGui::PopStyleColor();
		}
		ImGui::End();
		*/

		ImGui::PopFont();

		resizeLayout = false; //Only resize once

		window.clear(sf::Color::Black); // fill background with color
		ImGui::SFML::Render(window);
		window.display();
	}

	ImPlot::DestroyContext();
	ImGui::SFML::Shutdown();

	return 0;
}