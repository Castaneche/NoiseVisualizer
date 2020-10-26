#pragma once

#include "opengl/Shader.h"
#include "Setup.h"

#include <string>
#include <vector>
#include <mutex>

class Terrain
{
public:
	Terrain(int w, int h, std::shared_ptr<SetupData> setupdata);
	~Terrain();

	void loadShader(std::string vertexFile, std::string fragmentFile); //Shader is needed in order to display a terrain

	void ShowTerrain();
	void RenderTerrain();
	void ShowSetup();

private:
	void Update(); //Update z coordinates of vertices (fast)
	void Generate(); //Generate all vertices and indices (slow) : used when w or h changed

	std::thread* t; //Calculation thread
	std::mutex mutex;
	bool isCalculating; // variable to know when a calculation thread is running to avoid initializing the threads infinitely (1 thread max)
	bool generate = true; //if true : Generate() is called, false : Update()

	bool update = true; //True is a setup param changed

	Shader* shader;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	unsigned int VBO, VAO, EBO; //Terrain
	unsigned int framebuffer, texColorBuffer, rbo; //Terrain is rendered in a texture in order to be displayed by imgui

	//params
	int width, height;
	float maxHeight = 10;
	float rotationSpeed = 5;
	std::shared_ptr<SetupData> setupdata; //pointer to the global setup data
};

