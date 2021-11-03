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
	std::vector<ImVec4> colors; //colors for texture and terrain 
	bool updated; //keep track of parameters state
};

void ShowSetupWindow(SetupData& setupdata);