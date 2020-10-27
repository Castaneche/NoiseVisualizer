#include "Texture.h"

#include "imgui.h"
#include "implot.h"
#include <iostream>

#include "ColorMapSelector.h"

Texture::Texture(float imagesize, int pixelcount, std::shared_ptr<SetupData> setupdata)
{
	this->setupdata = setupdata;

	imageSize = imagesize;
	resizeImg(pixelcount); //generate array of pixels

	//Init Texture
	glGenTextures(1, &texture); //Generate texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Color Interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //Color Interpolation
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Set pixel storage mode to Byte-Alignment
	glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::~Texture()
{
	if (t != nullptr && t->joinable())
	{
		t->join(); //Join thread to prevent accessing deleted variables
		delete t;
	}
	if(pixels)
		delete[] pixels;
}
void Texture::ShowTexture()
{
	//Setupdata is reset to false at the end of the frame in order to keep track of user inputs every frames
	// So we store state of setupdata->updated in order to update the texture with modified values
	if (setupdata->updated) update = true;

	if (update == true || setupdata->updated == true)
	{
		std::lock_guard<std::mutex> guard(mutex);
		//Retreive isCalculating boolean
		bool b = true;
		b = isCalculating;
		if (!b) //Previous noise calculation is done. We can update with the new params
		{
			t = new std::thread(&Texture::Calculate, this);
			isCalculating = true;
			update = false; //Local variable can be reset here but setupdata->updated will be reset in Source.cpp at the end of the frame 
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
	ImGui::Text("Res(%dx%d) ; Octaves(%d) ; Freq(%.2f) ; Persistence(%.2f)", pixelCount, pixelCount, setupdata->octaves, setupdata->frequency, setupdata->persistence);
}

void Texture::ShowSetup()
{
	/* Resolution of the image/texture */
	ImGui::Text("Resolution : ");
	if (ImGui::Button("Very Low\n(50x50)##Texture"))
	{
		resizeImg(50);
		if (!isCalculating) update = true; //Resizing image while processing noise texture is not allowed
	} ImGui::SameLine();
	if (ImGui::Button("Low\n(100x100)##Texture"))
	{
		resizeImg(100);
		if (!isCalculating) update = true;
	}ImGui::SameLine();
	if (ImGui::Button("Medium\n(150x150)##Texture"))
	{
		resizeImg(150);
		if (!isCalculating) update = true;
	} ImGui::SameLine();
	if (ImGui::Button("High\n(250x250)##Texture"))
	{
		resizeImg(250);
		if (!isCalculating) update = true;
	}
}

void Texture::ResponsiveImg(float window_w, float window_h)
{
	if(window_w <= window_h)
		imageSize = window_w - 100;
	if (window_w > window_h)
		imageSize = window_h - 100;
}

uint8_t * Texture::GetPixels()
{
	if (t->joinable())
		t->join();
	return &*pixels;
}

int Texture::GetPixelCount()
{
	return pixelCount;
}

void Texture::Calculate()
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
			double x = i / double(pixelCountCopy) * setupdata->frequency;
			double y = j / double(pixelCountCopy) * setupdata->frequency;
			n = setupdata->pn.noise(x + setupdata->seed * 100, y + setupdata->seed * 100, setupdata->octaves, setupdata->persistence, setupdata->interpolationMethod);

			//Pixel calculation
			int index = (i * pixelCountCopy + j) * 3;
			auto c = ImGui::ColorValue(setupdata->colors, n);
			//ImVec4 c(0,0,0,0);
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
void Texture::resizeImg(int pixelcount)
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