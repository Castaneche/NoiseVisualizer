#include "PerlinNoise2DVisualizer.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <implot/implot.h>
#include <iostream>

PerlinNoise2DVisualizer::PerlinNoise2DVisualizer(float imagesize, int pixelcount)
{
	pn.setSeed(seed);

	imageSize = imagesize;
	pixelCount = pixelcount;
	image.create(pixelCount, pixelCount);

	lowColor = new float[3] {0,0,0};
	highColor = new float[3]{ 1,1,1 };
}


PerlinNoise2DVisualizer::~PerlinNoise2DVisualizer()
{
	delete lowColor, highColor;
}

void PerlinNoise2DVisualizer::Show()
{
	if (update)
	{
		for (int i = 0; i < pixelCount; i++)
		{
			for (int j = 0; j < pixelCount; j++)
			{
				double n = pn.noise(i / double(pixelCount) * frequency, j / double(pixelCount) * frequency, octaves, persistence, interpolationMethod);
				double r = lowColor[0] + n * (highColor[0] - lowColor[0]);
				double g = lowColor[1] + n * (highColor[1] - lowColor[1]);
				double b = lowColor[2] + n * (highColor[2] - lowColor[2]);

				image.setPixel(i, j, sf::Color(r*255.0f,g*255.0f,b*255.0f));
			}
		}

		texture.loadFromImage(image);
		update = false;
	}
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(.8f,.8f,.8f,1.0f));
	ImGui::BeginChild("2D Perlin Noise Menu Bar", ImVec2(0, 25), false, ImGuiWindowFlags_MenuBar);

	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("Load");
		if (ImGui::BeginMenu("Save As"))
		{
			ImGui::MenuItem("Preset");
			ImGui::MenuItem("Image");
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::BeginMenu("Show"))
		{
			bool temp = false;
			ImGui::Checkbox("Octaves", &temp);
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	ImGui::EndChild();
	ImGui::PopStyleColor();

	ImGui::BeginChild("Perlin Noise 2D Texture", ImVec2(imageSize + 70,imageSize + 50));
		ImGui::Image(texture, sf::Vector2f(imageSize, imageSize), sf::FloatRect(0, 0, texture.getSize().x, texture.getSize().y));
		ImGui::SameLine();
		ImVec4 a[2] = { ImVec4(lowColor[0],lowColor[1],lowColor[2],1), ImVec4(highColor[0],highColor[1],highColor[2],1) };
		ImPlot::PushColormap(a, 2);
		ImPlot::ShowColormapScale(0, 1, imageSize);
		ImPlot::PopColormap();
		ImGui::Text("Res(%dx%d) ; Octaves(%d) ; Freq(%.2f) ; Persistence(%.2f)",pixelCount, pixelCount, octaves, frequency, persistence);
	ImGui::EndChild(); ImGui::SameLine();

	ImGui::BeginChild("Perlin Noise 2D Setup", ImVec2(ImGui::GetWindowWidth() - (imageSize+70), imageSize + 50));
	if (ImGui::DragInt("seed", &seed, 1.0f, 0, 1000))
	{
		pn.setSeed(seed);
		update = true;
	}
	if (ImGui::SliderInt("Octaves", &octaves, 1, 8))
		update = true;
	if (ImGui::DragFloat("Persistence", &persistence, .01f, 0.0f, 10.0f))
		update = true;
	if (ImGui::DragFloat("Frequency", &frequency, 0.1, 0, 500))
		update = true;
	ImGui::Text("Interpolation : ");
	if (ImGui::Button("Linear"))
	{
		interpolationMethod = Linear;
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Cosine"))
	{
		interpolationMethod = Cosine;
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Cubic"))
	{
		interpolationMethod = Cubic;
		update = true;
	}


	ImGui::Text("Resolution : ");
	if (ImGui::Button("Very Low\n(50x50)"))
	{
		pixelCount = 50;
		image.create(pixelCount, pixelCount);
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Low\n(100x100)"))
	{
		pixelCount = 100;
		image.create(pixelCount, pixelCount);
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Medium\n(200x200)"))
	{
		pixelCount = 200;
		image.create(pixelCount, pixelCount);
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("High\n(300x300)"))
	{
		pixelCount = 300;
		image.create(pixelCount, pixelCount);
		update = true;
	} 

	ImGui::Text("Colormap : ");
	if (ImGui::ColorEdit3("Low", lowColor))
		update = true;
	if (ImGui::ColorEdit3("High", highColor))
		update = true;
	
	ImGui::EndChild();
}
