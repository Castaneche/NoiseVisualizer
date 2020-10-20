/*
ColorMapSelector::ColorMapSelector()
{
}


ColorMapSelector::~ColorMapSelector()
{
	for (unsigned int i = 0; i < colors.size(); i++)
		delete colors[i];
}

void ColorMapSelector::Show()
{
	ImGui::Text("Colormap : ");
	if (ImGui::Button("Default"))
	{
		updated = true;
		for (auto c : colors)
			delete c;
		colors.clear();
		colors.push_back(new float[3]{ 0, 0, 0 });
		colors.push_back(new float[3]{ 1, 1, 1 });
	} ImGui::SameLine();
	if (ImGui::Button("Terrain"))
	{
		updated = true;
		for (auto c : colors)
			delete c;
		colors.clear();
		colors.push_back(new float[3]{ 0, 0.23, 0.54 });
		colors.push_back(new float[3]{ 0.24, 0.90, 0.87 });
		colors.push_back(new float[3]{ 1, 1, 0.58 });
		colors.push_back(new float[3]{ 0.23, 0.73, 0.23 });
		colors.push_back(new float[3]{ 0.45, 0.45, 0.45 });
		colors.push_back(new float[3]{ 1, 1, 1 });
	} ImGui::SameLine();
	if (ImGui::Button("Lava"))
	{
		updated = true;
		for (auto c : colors)
			delete c;
		colors.clear();
		colors.push_back(new float[3]{ 1, 1, 1 });
		colors.push_back(new float[3]{ 1, 0.94, 0 });
		colors.push_back(new float[3]{ 0.75, 0.1, 0.1 });
		colors.push_back(new float[3]{ 0, 0, 0 });

	}
	for (unsigned int i = 0; i < colors.size(); i++)
	{
		if (ImGui::ColorEdit3(("##" + std::to_string(i)).c_str(), colors[i]))
			updated = true;
		ImGui::SameLine();
		if (ImGui::Button((" - ##" + std::to_string(i)).c_str())) // '-' button next to every color
		{
			updated = true;
			if (colors.size() > 2) //2 colors required
				colors.erase(colors.begin() + i);
		}
		if (i == colors.size() - 1) //Add a '+' button next to the last color
		{
			ImGui::SameLine();
			if (ImGui::Button(" + "))
			{
				updated = true;
				colors.push_back(new float[3]{ 0,0,0 });
			}
		}
	}
}

void ColorMapSelector::ShowWidget(float height)
{
	//Show a colormap diagram
	const int N = colors.size();
	ImVec4* a = new ImVec4[N];
	for (int i = 0; i < N; i++)
	{
		a[i] = ImVec4(colors[i][0], colors[i][1], colors[i][2], 1.0f);
	}
	ImPlot::PushColormap(a, N);
	ImPlot::ShowColormapScale(0, 1, height);
	ImPlot::PopColormap();
	delete a;
}
*/