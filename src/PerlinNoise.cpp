#include "PerlinNoise.h"

#include <iostream>

PerlinNoise::PerlinNoise()
{
	p = new int[512];
	for (int x = 0; x < 512; x++) {
		p[x] = permutation[x % 256];
	}
}

PerlinNoise::PerlinNoise(unsigned int seed)
{
	p = new int[512];
	for (int x = 0; x < 512; x++) {
		p[x] = permutation[x % 256];
	}
	setSeed(seed);
}


PerlinNoise::~PerlinNoise()
{
	delete p;
}

void PerlinNoise::setSeed(unsigned int seed)
{
	m_seed = seed;
	generator.seed(m_seed);
	mask();
}

unsigned int PerlinNoise::getSeed() const
{
	return m_seed;
}

double PerlinNoise::noise(double x, Interpolation method)
{
	int floor = std::floor(x); //get int value
	float t = x - floor;

	int xMin = floor % m_mask.size();
	int xMax = (xMin + 1) % m_mask.size();

	if (method == Linear)
		return linear_interpolation(m_mask[xMin], m_mask[xMax], t);
	else if (method == Cosine)
		return cosine_interpolation(m_mask[xMin], m_mask[xMax], t);
	else if (method == Cubic)
	{
		int xMinminus1 = (xMin - 1) % m_mask.size();
		int xMaxplus1 = (xMax + 1) % m_mask.size();
		return cubic_interpolation(m_mask[xMinminus1], m_mask[xMin], m_mask[xMax], m_mask[xMaxplus1], t);
	}
	else
		return linear_interpolation(m_mask[xMin], m_mask[xMax], t); //default linear
}
double PerlinNoise::noise(double x, int octaves, double persistence, Interpolation method, std::vector<double>* intermediateValues) {
	double total = 0;
	double frequency = 1;
	double amplitude = 1;
	double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0

	if (intermediateValues != nullptr)
		intermediateValues->clear();

	for (int i = 0; i < octaves; i++) {
		double n = noise(x * frequency, method) * amplitude;
		total += n;

		if(intermediateValues != nullptr)
			intermediateValues->push_back(n/amplitude);

		maxValue += amplitude;

		amplitude *= persistence;
		frequency *= 2;
	}

	return total / maxValue;
}


double PerlinNoise::noise(double x, double y, Interpolation method)
{
	//Find relative position in unit cube
	double xf = x - (int)std::floor(x);
	double yf = y - (int)std::floor(y);

	//Used for interpolation 
	//A fade function can be applied to avoid some pattern with linear interpolation
	double tx = x - (int)std::floor(x);
	double ty = y - (int)std::floor(y);

	//Look in permutation table to grab a number (this number will be converted to a gradient vector later)
	int xi = (int)std::floor(x) & 255; //Map to 255 to over overflow of the permutation array                     
	int yi = (int)std::floor(y) & 255;                            
	int aa, ab, ba, bb;
	aa = p[p[xi] + yi];
	ab = p[p[xi] + yi+1];
	ba = p[p[xi+1] + yi];
	bb = p[p[xi+1] + yi+1];

	// 1. Applying dot product using hash values 
	// 2. Interpolation X -> Y -> Z according to the chosen method  
	double n0, n1, n2, n3, ix0, ix1, result;
	n0 = grad(aa, xf, yf);
	n1 = grad(ba, xf-1, yf);
	n2 = grad(ab, xf, yf-1);
	n3 = grad(bb, xf-1, yf-1);

	if (method == Linear)
	{
		ix0 = linear_interpolation(n0, n1, tx);
		ix1 = linear_interpolation(n2, n3, tx);
		return (linear_interpolation(ix0, ix1, ty) + 1) / 2.0f; //normalize to 0 - 1
	}
	else if (method == Cosine)
	{
		ix0 = cosine_interpolation(n0, n1, tx);
		ix1 = cosine_interpolation(n2, n3, tx);
		return ( cosine_interpolation(ix0, ix1, ty) + 1) / 2.0f; //normalize to 0 - 1
	}
	else if (method == Cubic)
	{
		// WIP
	}
}

double PerlinNoise::noise(double x, double y, int octaves, double persistence, Interpolation method, std::vector<double>* intermediateValues)
{
	double total = 0;
	double frequency = 1;
	double amplitude = 1;
	double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0

	if (intermediateValues != nullptr)
		intermediateValues->clear();

	for (int i = 0; i < octaves; i++) {
		double n = noise(x * frequency, y * frequency, method) * amplitude;
		total += n;

		if (intermediateValues != nullptr)
			intermediateValues->push_back(n / amplitude);

		maxValue += amplitude;

		amplitude *= persistence;
		frequency *= 2;
	}

	return total / maxValue;
}

float PerlinNoise::linear_interpolation(float a, float b, float t)
{
	return a + t * (b-a);
}

float PerlinNoise::cosine_interpolation(float a, float b, float t)
{
	//cosinus interpolation 
	double c = (1 - cos(t * 3.1415927)) * 0.5;

	return (1 - c) * a + c * b;
}

float PerlinNoise::cubic_interpolation(float y0, float y1, float y2, float y3, float t)
{
	double a0, a1, a2, a3, t2;

	t2 = t * t;
	a0 = y3 - y2 - y0 + y1;
	a1 = y0 - y1 - a0;
	a2 = y2 - y0;
	a3 = y1;

	return(a0*t*t2 + a1 * t2 + a2 * t + a3);
}

float PerlinNoise::random(float a, float b)
{
	std::uniform_real_distribution<float> fdis(a, b);
	return fdis(generator);
}

void PerlinNoise::mask()
{
	m_mask.clear();
	for (int i = 0; i < 200; i++) {
		m_mask.push_back( random(0.f, 1.f) );
	}
}

double PerlinNoise::grad(int hash, double x, double y)
{
	//Hash table : return one of the four vectors : (1,0) (0,1) (-1,0) (0,-1)
	return ((hash & 1) ? x : -x) + ((hash & 2) ? y : -y); 
}

double PerlinNoise::dotProduct(int hash, int ix, int iy, int x, int y)
{
	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx*grad(hash, ix, iy) + dy * grad(hash, ix, iy));
}
