#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <PerlinNoise.h>

class PerlinNoise2DVisualizer
{
public:
	PerlinNoise2DVisualizer(float imagesize, int pixelcount);
	~PerlinNoise2DVisualizer();

	void Show();


private: 
	void resizeImg(int pixelcount);

	PerlinNoise pn;
	sf::Image image;
	sf::Texture texture;
	sf::Uint8* pixels;
	int nPixels;

	enum Update { None, All, Color };
	Update update = All;

	std::vector<double> noise;

	//params
	int maxOctave = 8;
	float imageSize; 
	int pixelCount;
	int seed = 123; //Seed change the noise values by applying an offset to the x and y coordinates of the PerlinNoise::noise function
	int octaves = 1;
	float persistence = 1.0f;
	float frequency = 10;
	Interpolation interpolationMethod = Cosine; 
	std::vector<float*> colors;
	bool terrain = false;
};