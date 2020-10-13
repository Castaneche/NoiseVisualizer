#pragma once

#include <vector>

class ColorMapSelector
{
public:
	ColorMapSelector();
	~ColorMapSelector();

	void Show();
	void ShowWidget(float height);

	bool updated = false;
	std::vector<float*> colors;
};

