#pragma once

#include "glad/glad.h"

#include "imgui.h"

#include <PerlinNoise.h>
#include <string>
#include <mutex>

class PerlinNoise2DVisualizer
{
public:
	PerlinNoise2DVisualizer(float imagesize, int pixelcount);
	~PerlinNoise2DVisualizer();

	void ShowTexture(); //Draw imgui items related to the texture
	void ShowSetup(); //draw imgui items related to the setup

	void ResponsiveImg(float window_w, float window_h); //Resize imgui texture according to w and h values

private: 
	void resizeImg(int pixelcount);
	void Calculate();

	std::thread t; //Calculation thread
	std::mutex mutex; 
	bool isCalculating; // variable to know when a calculation thread is running to avoid initializing the threads infinitely (1 thread max)

	PerlinNoise pn;
	GLuint texture;
	uint8_t* pixels; //unsigned char (0-255) array (pixelCount * pixelCount * 3) that store pixel colors
	int pixelCount;

	bool update = true;

	//params
	int maxOctave = 8;
	float imageSize; 
	int seed = 123; //Seed change the noise values by applying an offset to the x and y coordinates of the PerlinNoise::noise function
	int octaves = 1;
	float persistence = 1.0f;
	float frequency = 10;
	Interpolation interpolationMethod = Cosine; 

	std::vector<ImVec4> colors;

};