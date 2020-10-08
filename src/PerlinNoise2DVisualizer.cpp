#include "PerlinNoise2DVisualizer.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <implot/implot.h>
#include <iostream>

PerlinNoise2DVisualizer::PerlinNoise2DVisualizer(float imagesize, int pixelcount)
{
	pn.setSeed(seed);

	imageSize = imagesize;
	resizeImg(pixelcount);
	image.create(pixelCount, pixelCount);

	colors.push_back(new float[3]{ 0, 0, 0 });
	colors.push_back(new float[3]{ 1, 1, 1 });
}


PerlinNoise2DVisualizer::~PerlinNoise2DVisualizer()
{
	for (unsigned int i = 0; i < colors.size(); i++)
		delete colors[i];
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
					n = pn.noise(x + seed*100, y + seed*100, octaves, persistence, interpolationMethod);
					noise.push_back(n); //Update noise buffer
				}
				if (update == All || update == Color)
				{
					double r = 0;
					double g = 0;
					double b = 0;
					double currNoiseValue = noise[(i*pixelCount) + j];

					if (terrain == false) //Interpolation
					{
						double h = 1.0f / double(colors.size() - 1);
						double colorIndex = std::floor(currNoiseValue / h);

						double t = (currNoiseValue - colorIndex*h) / h;

						r = colors[colorIndex][0] + t * (colors[colorIndex + 1][0] - colors[colorIndex][0]);
						g = colors[colorIndex][1] + t * (colors[colorIndex + 1][1] - colors[colorIndex][1]);
						b = colors[colorIndex][2] + t * (colors[colorIndex + 1][2] - colors[colorIndex][2]);

						int index = (i + j * pixelCount) * 4;
						pixels[index + 0] = r * 255.0f;
						pixels[index + 1] = g * 255.0f;
						pixels[index + 2] = b * 255.0f;
					}
					else //No interpolation
					{
						double h = 1.0f / double(colors.size());
						double colorIndex = std::floor(currNoiseValue / h);

						r = colors[colorIndex][0]; g = colors[colorIndex][1]; b = colors[colorIndex][2];
					}
					
				}
			}
		}
		image.create(pixelCount, pixelCount, pixels); //Apply pixels
		texture.loadFromImage(image);
		update = None;
	}

	//Style the menu bar to fit the Theme6
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(.8f,.8f,.8f,1.0f));
	ImGui::BeginChild("2D Perlin Noise Menu Bar", ImVec2(0, 25), false, ImGuiWindowFlags_MenuBar);

	/* Menu Bar */
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
		//Display the Texture
		ImGui::Image(texture, sf::Vector2f(imageSize, imageSize), sf::FloatRect(0, 0, texture.getSize().x, texture.getSize().y)); 
		ImGui::SameLine();
		//Show a colormap diagram
		const int N = colors.size();
		ImVec4* a = new ImVec4[N];
		for (int i = 0; i < N; i++)
		{
			a[i] = ImVec4(colors[i][0], colors[i][1], colors[i][2], 1.0f);
		}
		ImPlot::PushColormap(a, N);
		ImPlot::ShowColormapScale(0, 1, imageSize);
		ImPlot::PopColormap();
		delete a;
		//Display all params as a string under the texture
		ImGui::Text("Res(%dx%d) ; Octaves(%d) ; Freq(%.2f) ; Persistence(%.2f)", pixelCount, pixelCount, octaves, frequency, persistence); 
	ImGui::EndChild(); ImGui::SameLine();

	ImGui::BeginChild("Perlin Noise 2D Setup", ImVec2(ImGui::GetWindowWidth() - (imageSize+70), imageSize + 200));
	if (ImGui::DragInt("seed", &seed, 1.0f, 0, 1000)) //Seed selection : not yet implemented for 2D Perlin !
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
	ImGui::Text("Interpolation : "); ImGui::SameLine();
	std::string s = "";
	if (interpolationMethod == Linear) s = "Linear";
	if (interpolationMethod == Cosine) s = "Cosine";
	if (interpolationMethod == Cubic) s = "Cubic";
	ImGui::Text(s.c_str());
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


	/* Resolution of the image/texture */
	ImGui::Text("Resolution : ");
	if (ImGui::Button("Very Low\n(50x50)"))
	{
		pixelCount = 50;
		resizeImg(pixelCount);
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("Low\n(100x100)"))
	{
		pixelCount = 100;
		resizeImg(pixelCount);
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("Medium\n(150x150)"))
	{
		pixelCount = 150;
		resizeImg(pixelCount);
		update = All;
	} ImGui::SameLine();
	if (ImGui::Button("High\n(300x300)"))
	{
		pixelCount = 300;
		resizeImg(pixelCount);
		update = All;
	} 


	/* Color palette configuration */
	ImGui::Text("Colormap : ");
	if (ImGui::Button("Default"))
	{
		update = Color;
		for (auto c : colors)
			delete c;
		colors.clear();
		colors.push_back(new float[3]{ 0, 0, 0 });
		colors.push_back(new float[3]{ 1, 1, 1 });
	} ImGui::SameLine();
	if (ImGui::Button("Terrain"))
	{
		update = Color;
		for (auto c : colors)
			delete c;
		colors.clear();
		colors.push_back(new float[3]{ 0, 0.23, 0.54 });
		colors.push_back(new float[3]{ 0.24, 0.90, 0.87 });
		colors.push_back(new float[3]{ 1, 1, 0.58 });
		colors.push_back(new float[3]{ 0.23, 0.73, 0.23 });
		colors.push_back(new float[3]{ 0.45, 0.45, 0.45 });
		colors.push_back(new float[3]{ 1, 1, 1 });
	} ImGui::SameLine();
	if (ImGui::Button("Lava"))
	{
		update = Color;
		for (auto c : colors)
			delete c;
		colors.clear();
		colors.push_back(new float[3]{ 1, 1, 1 });
		colors.push_back(new float[3]{ 1, 0.94, 0 });
		colors.push_back(new float[3]{ 0.75, 0.1, 0.1 });
		colors.push_back(new float[3]{ 0, 0, 0 });

	}
	for (unsigned int i = 0; i < colors.size(); i++)
	{
		if (ImGui::ColorEdit3(("##"+std::to_string(i)).c_str(), colors[i]))
			update = Color;
		ImGui::SameLine();
		if (ImGui::Button((" - ##" + std::to_string(i)).c_str())) // '-' button next to every color
		{
			update = Color;
			if(colors.size() > 2) //2 colors required
				colors.erase(colors.begin() + i);
		}
		if (i == colors.size() - 1) //Add a '+' button next to the last color
		{
			ImGui::SameLine();
			if (ImGui::Button(" + "))
			{
				update = Color;
				colors.push_back(new float[3]{ 0,0,0 });
			}
		}
	}
	
	ImGui::EndChild();
}

void PerlinNoise2DVisualizer::resizeImg(int pixelcount)
{
	pixelCount = pixelcount;
	nPixels = pixelCount * pixelCount * 4;
	pixels = new sf::Uint8[nPixels];
	for (int i = 0; i < nPixels; i++)
		pixels[i] = 255;
}