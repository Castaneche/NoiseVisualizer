#pragma once

#include "glad/glad.h"

#include "Setup.h"

#include <string>
#include <mutex>

class Texture
{
public:
	Texture(float imagesize, int pixelcount, std::shared_ptr<SetupData> setupdata);
	~Texture();

	void ShowTexture(); //Draw imgui items related to the texture
	void ShowSetup(); //draw imgui items related to the setup

	void ResponsiveImg(float window_w, float window_h); //Resize imgui texture according to w and h values

	uint8_t* GetPixels();
	int GetPixelCount();
private: 
	void resizeImg(int pixelcount);
	void Calculate();

	std::thread* t; //Calculation thread
	std::mutex mutex; 
	bool isCalculating; // variable to know when a calculation thread is running to avoid initializing the threads infinitely (1 thread max)

	GLuint texture;
	uint8_t* pixels; //unsigned char (0-255) array (pixelCount * pixelCount * 3) that store pixel colors
	int pixelCount;

	bool update = true;

	//params
	float imageSize; 
	std::shared_ptr<SetupData> setupdata;
};