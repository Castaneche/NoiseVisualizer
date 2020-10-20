#include "PerlinNoise2DVisualizer.h"

#include "imgui.h"
#include "implot.h"
#include "ColorMapSelector.h"
#include <iostream>

PerlinNoise2DVisualizer::PerlinNoise2DVisualizer(float imagesize, int pixelcount)
{
	pn.setSeed(seed);

	imageSize = imagesize;
	resizeImg(pixelcount); //generate array of pixels

	//Init Texture
	glGenTextures(1, &texture); //Generate texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Color Interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //Color Interpolation
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Set pixel storage mode to Byte-Alignment
	glBindTexture(GL_TEXTURE_2D, 0);

	//colormap.colors.push_back(new float[3]{ 0, 0, 0 });
	//colormap.colors.push_back(new float[3]{ 1, 1, 1 });

	colors.push_back(ImVec4(0, 0, 0, 0));
	colors.push_back(ImVec4(1, 1, 1, 0.5));
	colors.push_back(ImVec4(1, 0, 0, .8));
	colors.push_back(ImVec4(1, 0, 1, 0));
}


PerlinNoise2DVisualizer::~PerlinNoise2DVisualizer()
{
	delete[] pixels;
}

void PerlinNoise2DVisualizer::ShowTexture()
{
	//Display the Texture
	ImGui::Image((void*)(intptr_t)1, ImVec2(imageSize, imageSize));
	//ImGui::SameLine();
	//Show ColorMap widget
	//colormap.ShowWidget(imageSize);
	//Display all params as a string under the texture
	ImGui::Text("Res(%dx%d) ; Octaves(%d) ; Freq(%.2f) ; Persistence(%.2f)", pixelCount, pixelCount, octaves, frequency, persistence);
}

void PerlinNoise2DVisualizer::ShowSetup()
{
	ImGui::Indent(10);
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
	}
	/* WIP
	if (ImGui::Button("Cubic"))
	{
		interpolationMethod = Cubic;
		update = All;
	}*/


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
	if (ImGui::Button("High\n(250x250)"))
	{
		pixelCount = 250;
		resizeImg(pixelCount);
		update = All;
	}

	//colormap.Show();
	if (ImGui::ColorMapSelector("Hello", colors))
		update = Color;
}

void PerlinNoise2DVisualizer::Update()
{
	/*if (colormap.updated)
	{
		colormap.updated = false;
		update = Color;
	}*/

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
					n = pn.noise(x + seed * 100, y + seed * 100, octaves, persistence, interpolationMethod);
					noise.push_back(n); //Update noise buffer
				}
				if (update == All || update == Color)
				{
					uint8_t r = 0;
					uint8_t g = 0;
					uint8_t b = 0;
					double currNoiseValue = noise[(i*pixelCount) + j];
					
					//float h = 1.0f / double(colormap.colors.size() - 1);
					float h = 1;
					float colorIndex = std::floor(currNoiseValue / h);

					float t = (currNoiseValue - colorIndex * h) / h;

					//r = uint8_t(std::floor((colormap.colors[colorIndex][0] + t * (colormap.colors[colorIndex + 1][0] - colormap.colors[colorIndex][0])) * 255));
					//g = uint8_t(std::floor((colormap.colors[colorIndex][1] + t * (colormap.colors[colorIndex + 1][1] - colormap.colors[colorIndex][1])) * 255));
					//b = uint8_t(std::floor((colormap.colors[colorIndex][2] + t * (colormap.colors[colorIndex + 1][2] - colormap.colors[colorIndex][2])) * 255));

					int index = (i * pixelCount + j) * 3;
					auto c = ImGui::ColorValue(colors, currNoiseValue);
					pixels[index + 0] = uint8_t(c.x * 255);
					pixels[index + 1] = uint8_t(c.y * 255);
					pixels[index + 2] = uint8_t(c.z * 255);
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pixelCount, pixelCount, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		update = None;
	}
}

void PerlinNoise2DVisualizer::ResponsiveImg(float window_w, float window_h)
{
	if(window_w <= window_h)
		imageSize = window_w - 100;
	if (window_w > window_h)
		imageSize = window_h - 100;
}

void PerlinNoise2DVisualizer::resizeImg(int pixelcount)
{
	delete[] pixels; //Clear old array

	pixelCount = pixelcount;
	int nPixels = pixelCount * pixelCount * 3;
	pixels = new uint8_t[nPixels];
	for (int i = 0; i < nPixels; i++)
		pixels[i] = 255;
}