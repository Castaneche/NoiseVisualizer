#pragma once

#include "PerlinNoise.h"
#include "imgui.h"


#include <vector>
#include <string>

struct SetupData
{
	int seed;
	int octaves;
	float persistence;
	float frequency;
	Interpolation interpolationMethod;
	PerlinNoise pn;
	std::vector<ImVec4> colors;
	bool updated; 
};

void ShowSetupWindow(SetupData& setupdata);