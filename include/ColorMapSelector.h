#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>


/* ColorMap widget
 First call ColorMapSelector() then retreive the color with ColorValue() 
 Huge perf issues -> code need to be reworked */
namespace ImGui {

	ImVec4 ColorValue(std::vector<ImVec4>& colors, float x) //x [0 - 1]
	{
		std::vector<ImVec4> colorsTemp(colors.size());
		std::copy(colors.begin(), colors.end(), colorsTemp.begin());
		std::sort(colorsTemp.begin(), colorsTemp.end(), [](ImVec4 a, ImVec4 b) { return a.w < b.w; });

		unsigned int colorIndex = 0;
		float r, g, b;
		if (x <= colorsTemp[0].w)
		{
			r = colorsTemp[0].x;
			g = colorsTemp[0].y;
			b = colorsTemp[0].z;
		}
		else if (x >= colorsTemp[colorsTemp.size() - 1].w)
		{
			r = colorsTemp[colorsTemp.size() - 1].x;
			g = colorsTemp[colorsTemp.size() - 1].y;
			b = colorsTemp[colorsTemp.size() - 1].z;
		}
		else
		{
			for (unsigned int i = colorsTemp.size() - 2; i >= 0; i--)
			{
				colorIndex = i;
				if (x > colorsTemp[i].w) break;
			}
			float t = (x - colorsTemp[colorIndex].w) / (colorsTemp[colorIndex + 1].w - colorsTemp[colorIndex].w);
			r = float((colorsTemp[colorIndex].x + t * (colorsTemp[colorIndex + 1].x - colorsTemp[colorIndex].x)));
			g = float((colorsTemp[colorIndex].y + t * (colorsTemp[colorIndex + 1].y - colorsTemp[colorIndex].y)));
			b = float((colorsTemp[colorIndex].z + t * (colorsTemp[colorIndex + 1].z - colorsTemp[colorIndex].z)));
		}
		return ImVec4(r,g,b,1.0f);
	}

	//Imvec4 colors = {r, g, b, x in range[0 - 1]} x refer to position in colormap 
	int ColorMapSelector(const char* label, std::vector<ImVec4>& colors)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		bool changed = false;

		// prepare canvas
		const float dim = GetContentRegionAvail().x - 100;
		ImVec2 Canvas(dim, 35);
		Dummy(ImVec2(0, 5));

		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + Canvas);
		ItemSize(bb);
		if (!ItemAdd(bb, NULL))
			return false;

		RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, style.FrameRounding);

		//sort
		std::vector<ImVec4> colorsTemp(colors.size());
		std::copy(colors.begin(), colors.end(), colorsTemp.begin());
		std::sort(colorsTemp.begin(), colorsTemp.end(), [](ImVec4 a, ImVec4 b) { return a.w < b.w; });


		float offsetx = bb.Min.x;
		float width = bb.Max.x - bb.Min.x;
		int N = colors.size();
		if (N >= 1)
		{
			//Begin 
			float posx = colorsTemp[0].w;
			ImVec4 tempColor = ImVec4(colorsTemp[0].x, colorsTemp[0].y, colorsTemp[0].z, 1.0f);
			ImU32 c = ImGui::GetColorU32(tempColor);
			DrawList->AddRectFilledMultiColor(ImVec2(offsetx, bb.Min.y), ImVec2(offsetx + posx * width, bb.Min.y + 25), c, c, c, c);

			//End 
			posx = colorsTemp[N-1].w;
			tempColor = ImVec4(colorsTemp[N - 1].x, colorsTemp[N - 1].y, colorsTemp[N - 1].z, 1.0f);
			c = ImGui::GetColorU32(tempColor);
			DrawList->AddRectFilledMultiColor(ImVec2(offsetx + posx * width, bb.Min.y), ImVec2(bb.Max.x, bb.Min.y + 25), c, c, c, c);

			//Middle
			for (int i = 0; i < N - 1; i++)
			{
				float posx1 = colorsTemp[i].w;
				float posx2 = colorsTemp[i + 1].w;
				ImVec4 tempColor1 = ImVec4(colorsTemp[i].x, colorsTemp[i].y, colorsTemp[i].z, 1.0f);
				ImVec4 tempColor2 = ImVec4(colorsTemp[i + 1].x, colorsTemp[i + 1].y, colorsTemp[i + 1].z, 1.0f);
				ImU32 c1 = ImGui::GetColorU32(tempColor1);
				ImU32 c2 = ImGui::GetColorU32(tempColor2);

				DrawList->AddRectFilledMultiColor(ImVec2(offsetx + posx1 * width, bb.Min.y), ImVec2(offsetx + posx2 * width, bb.Min.y + 25), c1, c2, c2, c1);
			}
		}   

		int hovered = IsItemActive() || IsItemHovered();

		hovered |= 0 != ItemHoverable(bb, id);
		

		std::vector<float> pos;
		for (int i = 0; i < colorsTemp.size(); i++)
		{
			pos.push_back(colors[i].w);
		}
		float grab_w = 5;

		for (int i = 0; i < pos.size(); i++)
		{
			ImRect grab_bb(bb.Min.x + (pos[i] * Canvas.x - grab_w/2), bb.Min.y + 25, bb.Min.x + (pos[i] * Canvas.x + grab_w /2), bb.Max.y);
			// Render grab
			if (grab_bb.Max.x > grab_bb.Min.x)
				window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, ImGui::GetColorU32(ImVec4(colors[i].x, colors[i].y, colors[i].z, 1)), style.GrabRounding);

		}

		for (unsigned int i = 0; i < colors.size(); i++)
		{
			ImGui::PushItemWidth(bb.Max.x - bb.Min.x);
			if (ImGui::SliderFloat(("##ColorMap_Position" + std::to_string(i)).c_str(), &colors[i].w, 0, 1))
				changed = true;
			ImGui::SameLine();
			if (ImGui::ColorEdit4(("##ColorMap_Color" + std::to_string(i)).c_str(), &colors[i].x, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs))
				changed = true;
			ImGui::SameLine();
			if (colors.size() > 1)
			{
				if (ImGui::Button((" - ##ColorMap" + std::to_string(i)).c_str()))
				{
					changed = true;
					colors.erase(colors.begin() + i);
				}
			}
			if (i == colors.size() - 1 && colors.size() <= 10)
			{
				ImGui::SameLine();
				if (ImGui::Button(" + ##ColorMap"))
				{
					colors.push_back(ImVec4(0, 0, 0, 1));
					changed = true;
				}
			}
		}

		Dummy(ImVec2(0, 5));

		return changed;
	}
}