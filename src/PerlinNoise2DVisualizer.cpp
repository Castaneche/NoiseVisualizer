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
	colors.push_back(lowColor);
	colors.push_back(highColor);
}


PerlinNoise2DVisualizer::~PerlinNoise2DVisualizer()
{
	delete lowColor, highColor;
}

void PerlinNoise2DVisualizer::Show()
{
	if (update != None)
	{
		if (update == All) //Clear noise buffer
				noise.clear();

		double n = 0;
		for (int i = 0; i < pixelCount; i++)
		{
			for (int j = 0; j < pixelCount; j++)
			{
				double x = i / double(pixelCount) * frequency;
				double y = j / double(pixelCount) * frequency;

				if (update == All)
				{
					n = pn.noise(x, y, octaves, persistence, interpolationMethod);

					noise.push_back(n);
				}
				if (update == All || update == Color)
				{
					double r = 0;
					double g = 0;
					double b = 0;
					double currNoiseValue = noise[(i*pixelCount) + j % pixelCount];
					if (terrain == false)
					{
						double h = 1.0f / double(colors.size() - 1);
						double colorIndex = std::floor(currNoiseValue / h);

						double t = (currNoiseValue - colorIndex*h) / h;

						r = colors[colorIndex][0] + t * (colors[colorIndex + 1][0] - colors[colorIndex][0]);
						g = colors[colorIndex][1] + t * (colors[colorIndex + 1][1] - colors[colorIndex][1]);
						b = colors[colorIndex][2] + t * (colors[colorIndex + 1][2] - colors[colorIndex][2]);
					}
					else
					{
						double h = 1.0f / double(colors.size());
						double colorIndex = std::floor(currNoiseValue / h);

						r = colors[colorIndex][0]; g = colors[colorIndex][1]; b = colors[colorIndex][2];
					}
					
					image.setPixel(i, j, sf::Color(r*255.0f, g*255.0f, b*255.0f));
				}
			}
		}

		texture.loadFromImage(image);
		update = None;
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
		if (ImGui::MenuItem("Texture"))
		{
			update = Color;
			terrain = false;
		}
		if (ImGui::MenuItem("Terrain"))
		{
			update = Color;
			terrain = true;
		}
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	ImGui::EndChild();
	ImGui::PopStyleColor();

	ImGui::BeginChild("Perlin Noise 2D Texture", ImVec2(imageSize + 70, imageSize + 200));
		ImGui::Image(texture, sf::Vector2f(imageSize, imageSize), sf::FloatRect(0, 0, texture.getSize().x, texture.getSize().y));
		ImGui::SameLine();
		const int N = colors.size();
		ImVec4* a = new ImVec4[N];
		for (int i = 0; i < N; i++)
		{
			a[i] = ImVec4(colors[i][0], colors[i][1], colors[i][2], 1.0f);
		}
		ImPlot::PushColormap(a, N);
		ImPlot::ShowColormapScale(0, 1, imageSize/1.7);
		ImPlot::PopColormap();
		ImGui::Text("Res(%dx%d) ; Octaves(%d) ; Freq(%.2f) ; Persistence(%.2f)", pixelCount, pixelCount, octaves, frequency, persistence);
	ImGui::EndChild(); ImGui::SameLine();

	ImGui::BeginChild("Perlin Noise 2D Setup", ImVec2(ImGui::GetWindowWidth() - (imageSize+70), imageSize + 50));
	if (ImGui::DragInt("seed", &seed, 1.0f, 0, 1000))
	{
		pn.setSeed(seed);
		update = All;
	}
	if (ImGui::SliderInt("Octaves", &octaves, 1, 8))
		update = All;
	if (ImGui::DragFloat("Persistence", &persistence, .01f, 0.0f, 10.0f))
		update = All;
	if (ImGui::DragFloat("Frequency", &frequency, 0.1, 0, 500))
		update = All;
	ImGui::Text("Interpolation : ");
	if (ImGui::Button("Linear"))
	{
		interpolationMethod = Linear;
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("Cosine"))
	{
		interpolationMethod = Cosine;
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("Cubic"))
	{
		interpolationMethod = Cubic;
		update = All;
	}


	ImGui::Text("Resolution : ");
	if (ImGui::Button("Very Low\n(50x50)"))
	{
		pixelCount = 50;
		image.create(pixelCount, pixelCount);
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("Low\n(100x100)"))
	{
		pixelCount = 100;
		image.create(pixelCount, pixelCount);
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("Medium\n(200x200)"))
	{
		pixelCount = 200;
		image.create(pixelCount, pixelCount);
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("High\n(300x300)"))
	{
		pixelCount = 300;
		image.create(pixelCount, pixelCount);
		update = All;
	} 

	ImGui::Text("Colormap : ");
	for (int i = 0; i < colors.size(); i++)
	{
		if (ImGui::ColorEdit3(("##"+std::to_string(i)).c_str(), colors[i]))
			update = Color;
		ImGui::SameLine();
		if (ImGui::Button((" - ##" + std::to_string(i)).c_str()))
		{
			colors.erase(colors.begin() + i);
		}
		if (i == colors.size() - 1)
		{
			ImGui::SameLine();
			if (ImGui::Button(" + "))
			{
				colors.push_back(new float[3]{ 0,0,0 });
			}
		}
	}
	
	ImGui::EndChild();
}
