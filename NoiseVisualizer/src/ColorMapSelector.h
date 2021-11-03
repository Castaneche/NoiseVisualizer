#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>


/* ColorMap widget
 First call ColorMapSelector() then retreive the color with ColorValue() 
 Huge perf issues -> code need to be reworked */
namespace ImGui {
	static std::vector<ImVec4> COLORMAP_DEFAULT = {
		ImVec4(0, 0, 0, 0),
		ImVec4(1, 1, 1, 1)
	};
	static std::vector<ImVec4> COLORMAP_TERRAIN = {
		ImVec4(0, 0.23, 0.54, 0),
		ImVec4(0.24, 0.90, 0.87, 0.2),
		ImVec4(1, 1, 0.58, 0.4),
		ImVec4(0.23, 0.73, 0.23, 0.6),
		ImVec4(0.45, 0.45, 0.45, 0.8),
		ImVec4(1, 1, 1, 1)
	};
	static std::vector<ImVec4> COLORMAP_LAVA = {
		ImVec4(1, 1, 1, 0),
		ImVec4(1, 0.94, 0, 0.3),
		ImVec4(0.75, 0.1, 0.1, 0.6),
		ImVec4(0, 0, 0, 1)
	};

	ImVec4 ColorValue(std::vector<ImVec4>& colors, float x); //x [0 - 1]

	//Imvec4 colors = {r, g, b, x in range[0 - 1]} x refer to position in colormap 
	int ColorMapSelector(const char* label, std::vector<ImVec4>& colors);
}