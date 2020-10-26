#include "Setup.h"

void ShowSetupWindow(SetupData& setupdata)
{
	ImGui::Indent(10);
	if (ImGui::DragInt("seed", &setupdata.seed, 1.0f, 0, 1000))
	{
		setupdata.pn.setSeed(setupdata.seed);
		setupdata.updated = true;
	}
	if (ImGui::SliderInt("Octaves", &setupdata.octaves, 1, 8))
		setupdata.updated = true;
	if (setupdata.octaves > 1)
	{
		if (ImGui::DragFloat("Persistence", &setupdata.persistence, .01f, 0.0f, 10.0f))
			setupdata.updated = true;
	}
	if (ImGui::DragFloat("Frequency", &setupdata.frequency, 0.1, 0, 500))
		setupdata.updated = true;
	ImGui::Text("Interpolation : "); ImGui::SameLine();
	std::string s = "";
	if (setupdata.interpolationMethod == Linear) s = "Linear";
	if (setupdata.interpolationMethod == Cosine) s = "Cosine";
	if (setupdata.interpolationMethod == Cubic) s = "Cubic";
	ImGui::Text(s.c_str());
	if (ImGui::Button("Linear"))
	{
		setupdata.interpolationMethod = Linear;
		setupdata.updated = true;
	} ImGui::SameLine();
	if (ImGui::Button("Cosine"))
	{
		setupdata.interpolationMethod = Cosine;
		setupdata.updated = true;
	}
	/* WIP
	if (ImGui::Button("Cubic"))
	{
		interpolationMethod = Cubic;
		update = All;
	}*/


	/*if (ImGui::ColorMapSelector("Hello", setupdata.colors))
		setupdata.updated = true;*/
}
