#pragma once

#include <PerlinNoise.h>
#include <vector>

class PerlinNoise1DVisualizer
{
public:
	PerlinNoise1DVisualizer(int n);
	~PerlinNoise1DVisualizer();

	void Show();

private:
	PerlinNoise pn;
	//1D Perlin params
	int seed = 123;
	int octaves = 1;
	float persistence = 1.0f;
	float frequency = 10;
	float offset = 0;
	int nbPoints = 500;
	double* xvalues;
	std::vector<double*> yvalues; //Array to store octaves data
	bool update = true;
	bool onlyFinal = false; //Show only the final noise in graph
	bool resetView = true;
	Interpolation interpolationMethod = Cosine;
};

