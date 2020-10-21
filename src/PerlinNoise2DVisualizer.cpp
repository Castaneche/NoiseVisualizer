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

	colors = ImGui::COLORMAP_DEFAULT;
}


PerlinNoise2DVisualizer::~PerlinNoise2DVisualizer()
{
	if (t)
	{
		t->join(); //Join thread to prevent accessing deleted variables
		delete t;
	}
	if(pixels)
		delete[] pixels;
}
void PerlinNoise2DVisualizer::ShowTexture()
{
	if (update == true)
	{
		std::lock_guard<std::mutex> guard(mutex);
		//Retreive isCalculating boolean
		bool b = true;
		b = isCalculating;
		if (!b) //Previous noise calculation is done. We can update with the new params
		{
			t = new std::thread(&PerlinNoise2DVisualizer::Calculate, this); 
			isCalculating = true;
			update = false;
		}
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	{
		std::lock_guard<std::mutex> guard(mutex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pixelCount, pixelCount, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	//Display the Texture
	ImGui::Image((void*)(intptr_t)1, ImVec2(imageSize, imageSize));
	//Display all params as a string under the texture
	ImGui::Text("Res(%dx%d) ; Octaves(%d) ; Freq(%.2f) ; Persistence(%.2f)", pixelCount, pixelCount, octaves, frequency, persistence);
}

void PerlinNoise2DVisualizer::ShowSetup()
{
	ImGui::Indent(10);
	if (ImGui::DragInt("seed", &seed, 1.0f, 0, 1000)) //Seed selection : not yet implemented for 2D Perlin !
	{
		pn.setSeed(seed);
		update = true;
	}
	if (ImGui::SliderInt("Octaves", &octaves, 1, 8))
		update = true;
	if (octaves > 1)
	{
		if (ImGui::DragFloat("Persistence", &persistence, .01f, 0.0f, 10.0f))
			update = true;
	}
	if (ImGui::DragFloat("Frequency", &frequency, 0.1, 0, 500))
		update = true;
	ImGui::Text("Interpolation : "); ImGui::SameLine();
	std::string s = "";
	if (interpolationMethod == Linear) s = "Linear";
	if (interpolationMethod == Cosine) s = "Cosine";
	if (interpolationMethod == Cubic) s = "Cubic";
	ImGui::Text(s.c_str());
	if (ImGui::Button("Linear"))
	{
		interpolationMethod = Linear;
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Cosine"))
	{
		interpolationMethod = Cosine;
		update = true;
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
		resizeImg(50);
		if (!isCalculating) update = true; //Resizing image while processing noise texture is not allowed
	} ImGui::SameLine();
	if (ImGui::Button("Low\n(100x100)"))
	{
		resizeImg(100);
		if (!isCalculating) update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Medium\n(150x150)"))
	{
		resizeImg(150);
		if (!isCalculating) update = true;
	} ImGui::SameLine();
	if (ImGui::Button("High\n(250x250)"))
	{
		resizeImg(250);
		if (!isCalculating) update = true;
	}

	//colormap.Show();
	if (ImGui::ColorMapSelector("Hello", colors))
		update = true;
}

void PerlinNoise2DVisualizer::ResponsiveImg(float window_w, float window_h)
{
	if(window_w <= window_h)
		imageSize = window_w - 100;
	if (window_w > window_h)
		imageSize = window_h - 100;
}

void PerlinNoise2DVisualizer::Calculate()
{
	uint8_t* pixelsCopy;
	int pixelCountCopy;
	{ //Lock data while copying
		std::lock_guard<std::mutex> guard(mutex);
		pixelsCopy = pixels; //copy pixel array
		pixelCountCopy = pixelCount;
	}
	//We will update a copy of the actual pixel array to avoid accessing the same data in different threads
	double n = 0;
	for (int i = 0; i < pixelCountCopy; i++)
	{
		for (int j = 0; j < pixelCountCopy; j++)
		{
			//Noise calculation
			double x = i / double(pixelCountCopy) * frequency;
			double y = j / double(pixelCountCopy) * frequency;
			n = pn.noise(x + seed * 100, y + seed * 100, octaves, persistence, interpolationMethod);

			//Pixel calculation
			int index = (i * pixelCountCopy + j) * 3;
			auto c = ImGui::ColorValue(colors, n);
			pixelsCopy[index + 0] = uint8_t(c.x * 255);
			pixelsCopy[index + 1] = uint8_t(c.y * 255);
			pixelsCopy[index + 2] = uint8_t(c.z * 255);
		}
	}
	{ 
		std::lock_guard<std::mutex> guard(mutex);
		std::swap(pixelsCopy, pixels); //paste pixels array
		isCalculating = false; //calculation done
	}

}


void PerlinNoise2DVisualizer::resizeImg(int pixelcount)
{
	if(!isCalculating)
	{ //lock data while updating the array
		std::lock_guard<std::mutex> guard(mutex);

		delete[] pixels; //Clear old array

		pixelCount = pixelcount;
		int nPixels = pixelCount * pixelCount * 3;
		pixels = new uint8_t[nPixels];
		for (int i = 0; i < nPixels; i++)
			pixels[i] = 255;
	}
}