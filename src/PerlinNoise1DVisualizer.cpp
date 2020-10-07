#include "..\include\PerlinNoise1DVisualizer.h"

#include <imgui.h>
#include <implot/implot.h>
#include <string>

PerlinNoise1DVisualizer::PerlinNoise1DVisualizer(int n)
{
	nbPoints = n;
	pn.setSeed(seed);
	xvalues = new double[nbPoints];
}


PerlinNoise1DVisualizer::~PerlinNoise1DVisualizer()
{
	delete xvalues;
}

void PerlinNoise1DVisualizer::Show()
{
	ImGui::BeginChild("Perlin Noise 1D Graph", ImVec2(ImGui::GetWindowWidth() * .6f, 300));
	ImPlot::SetNextPlotLimits(0, 1, 0, 1, ImGuiCond_Always);
	ImPlot::BeginPlot("Perlin Noise 1D","", "", ImVec2(-1,0), ImPlotFlags_NoChild);
	if (update) //update graph when needed : avoid calculation of perlin noise every frame with params unchanged
	{
		//Reset arrays
		std::fill_n(xvalues, nbPoints, 0);
		for (double* e : yvalues)
			delete(e);
		yvalues.clear();

		for (int i = 0; i < nbPoints; i++)
		{
			std::vector<double> interValues; //values for each octave
			double n = pn.noise(offset + i / double(nbPoints) * frequency, octaves, persistence, interpolationMethod, &interValues);
			xvalues[i] = i / double(nbPoints);
			if (!onlyFinal) //Draw octaves
			{
				for (unsigned int k = 0; k < interValues.size(); k++)
				{
					if (i == 0) //First iteration
					{
						yvalues.push_back(new double[nbPoints]);
					}
					yvalues[k][i] = interValues[k];
				}
			}
			if (i == 0)
				yvalues.push_back(new double[nbPoints]);
			yvalues[yvalues.size() - 1][i] = n;
		}
		update = false;
	}
	for (unsigned int i = 0; i < yvalues.size() - 1; i++)
	{
		std::string graphTitle = "Octave " + std::to_string(i + 1);
		ImPlot::PlotLine(graphTitle.c_str(), xvalues, yvalues[i], nbPoints);

		if (ImPlot::IsLegendEntryHovered(graphTitle.c_str())) //Tooltip on legend to display frequency and amplitude for each octave
		{
			ImGui::BeginTooltip();
			ImGui::Text("Frequency : %.2f", frequency * std::pow(2, i));
			ImGui::Text("Amplitude : %.4f", 1 * std::pow(persistence, i));
			ImGui::EndTooltip();
		}
	}
	//Cool side view hill effect (shaded graph) for the final noise function 
	ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, 1), 1.0f);
	ImPlot::PlotLine("Final", xvalues, yvalues[yvalues.size() - 1], nbPoints); //Line
	ImPlot::SetNextFillStyle(ImVec4(0, 0, 0, 1), .25f);
	ImPlot::PlotShaded("Final", xvalues, yvalues[yvalues.size() - 1], nbPoints); //Shade
	ImPlot::EndPlot();
	ImGui::EndChild();  ImGui::SameLine();

	ImGui::BeginChild("Perlin Noise 1D Setup", ImVec2(ImGui::GetWindowWidth() * .45f, 300));
	if (ImGui::DragInt("seed", &seed, 1.0f, 0, 1000))
	{
		pn.setSeed(seed);
		update = true;
	}
	if (ImGui::SliderInt("Octaves", &octaves, 1, 8))
		update = true;
	if (ImGui::DragFloat("Persistence", &persistence, .01f, 0.0f, 10.0f))
		update = true;
	if (ImGui::DragFloat("Frequency", &frequency, 0.01, 0, 100))
		update = true;
	if (ImGui::DragFloat("offset", &offset, 0.1, -100, 100))
		update = true;
	if (ImGui::Checkbox("Final only", &onlyFinal))
		update = true;
	ImGui::Text("Interpolation : ");
	if (ImGui::Button("Linear"))
	{
		interpolationMethod = Linear;
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Cosine"))
	{
		interpolationMethod = Cosine;
		update = true;
	} ImGui::SameLine();
	if (ImGui::Button("Cubic"))
	{
		interpolationMethod = Cubic;
		update = true;
	}

	ImGui::EndChild();
}
