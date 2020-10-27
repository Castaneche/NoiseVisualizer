#include "Terrain.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLFW/glfw3.h"

#include "ColorMapSelector.h"

Terrain::Terrain(int w, int h, std::shared_ptr<SetupData> setupdata)
{
	this->setupdata = setupdata;

	width = w;
	height = h;

	//Init buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//FrameBuffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//Texture
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1440, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	//RBO
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1440);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Terrain::~Terrain()
{
	if (t != nullptr && t->joinable())
	{
		t->join(); //Join thread to prevent accessing deleted variables
		delete t;
	}

	delete shader;
	
	//Clean buffers
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteFramebuffers(1, &framebuffer);
}

void Terrain::loadShader(std::string vertexFile, std::string fragmentFile)
{
	shader = new Shader(vertexFile.c_str(), fragmentFile.c_str());
}

void Terrain::ShowTerrain()
{
	//Setupdata is reset to false at the end of the frame in order to keep track of user inputs every frames
	// So we store state of setupdata->updated in order to update the terrain with modified values
	if (setupdata->updated) update = true; 

	if (update == true)
	{
		std::lock_guard<std::mutex> guard(mutex);
		//Retreive isCalculating boolean
		bool b = true;
		b = isCalculating;
		if (!b) //Previous noise calculation is done. We can update with the new params
		{
			if(generate == true)
				t = new std::thread(&Terrain::Generate, this);
			else
				t = new std::thread(&Terrain::Update, this);

			isCalculating = true;
			update = false;//Local variable can be reset here but setupdata->updated will be reset in Source.cpp at the end of the frame 
		}
	}

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	ImGui::Image((void*)(intptr_t)texColorBuffer, ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().x *0.75));

	/* Code for Overlay : Need to be outside of the function (outside of any imgui window)
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
	ImGuiViewport* viewport = ImGui::GetWindowViewport();
	ImVec2 pos = viewport->GetWorkPos();
	pos.y += (ImGui::GetScrollY() / ImGui::GetScrollMaxY()) * ImGui::GetWindowSize().x *0.75f / 2.0;
	std::cout << pos.y << std::endl;
	bool open = true;
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
	if (ImGui::Begin("Overlay", &open, window_flags))
	{
		ImGui::Text("%d x %d", width, height);
		ImGui::End();
	}*/ 
}

void Terrain::RenderTerrain()
{
	// Projection, View  matrices
	glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(90.0f), (float)800 / (float)600, 0.1f, 1000.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -width - 5.0f));
	// pass transformation matrices to the shader
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);
	// calculate the model matrix for each object and pass it to shader before drawing
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, 0));
	model = glm::rotate(model, glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation += glm::radians(0.5f) * (rotationSpeed);
	model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	shader->setMat4("model", model);
	shader->setFloat("maxH", maxHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, 1920, 1440);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	shader->use();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Terrain::ShowSetup()
{
	/* Resolution of the image/texture */
	ImGui::Text("Resolution : ");
	if (ImGui::Button("Very Low\n(50x50)##Terrain"))
	{
		if (!isCalculating)//Resizing terrain while processing noise data is not allowed
		{
			width = 50; height = 50;
			generate = true;
			update = true;
		}
	} ImGui::SameLine();
	if (ImGui::Button("Low\n(100x100)##Terrain"))
	{
		if (!isCalculating)
		{
			width = 100; height = 100;
			generate = true;
			update = true;
		}
	}ImGui::SameLine();
	if (ImGui::Button("Medium\n(150x150)##Terrain"))
	{
		if (!isCalculating)
		{
			width = 150; height = 150;
			generate = true;
			update = true;
		}
	} ImGui::SameLine();
	if (ImGui::Button("High\n(250x250)##Terrain"))
	{
		if (!isCalculating) //Resizing terrain while processing noise data is not allowed
		{
			width = 250; height = 250;
			generate = true;
			update = true;
		}
	}
	if (ImGui::SliderFloat("Maximum Height", &maxHeight, 0, 100))
		update = true;
	ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0, 1);
}

void Terrain::Update()
{
	int w, h;
	{ //Lock data while copying
		std::lock_guard<std::mutex> guard(mutex);
		w = width; 
		h = height;
	}

	for (unsigned int i = 0; i < w + 1; i++)//+1 because width refers to number of cells ex: 2 cells = 3 vertices in y axis
	{
		for (unsigned int j = 0; j < h + 1; j++)
		{
			int index = (i * (h + 1) + j) * 6;

			//Noise calculation
			double px = i / double(w+1) * setupdata->frequency;
			double py = j / double(h+1) * setupdata->frequency;
			float n = setupdata->pn.noise(px + setupdata->seed * 100, py + setupdata->seed * 100, setupdata->octaves, setupdata->persistence, setupdata->interpolationMethod);

			float z = n * maxHeight;
			vertices[index + 2] = z;

			auto c = ImGui::ColorValue(setupdata->colors, n);
			vertices[index + 3] = c.x;
			vertices[index + 4] = c.y;
			vertices[index + 5] = c.z;
		}
	}

	{
		std::lock_guard<std::mutex> guard(mutex);
		isCalculating = false; //calculation done
	}
}

void Terrain::Generate()
{
	int w, h;
	{ //Lock data while copying
		std::lock_guard<std::mutex> guard(mutex);
		w = width; 
		h = height;
	}

	vertices.clear();
	indices.clear();
	vertices.resize((w + 1) * (h + 1) * 6, 0);
	indices.resize(w * h * 6, 0);


	std::cout << "Vertices" << std::endl;
	for (unsigned int i = 0; i < w + 1; i++) //+1 because width refers to number of cells ex: 2 cells = 3 vertices in y axis
	{
		float x = i - (w / 2.);
		for (unsigned int j = 0; j < h + 1; j++)
		{
			int index = (i * (h + 1) + j) * 6;

			float y = j - (h / 2.);
			vertices[index] = x;
			vertices[index + 1] = y;

			//Noise calculation
			double px = i / double(w+1) * setupdata->frequency;
			double py = j / double(h+1) * setupdata->frequency;
			float n = setupdata->pn.noise(px + setupdata->seed * 100, py + setupdata->seed * 100, setupdata->octaves, setupdata->persistence, setupdata->interpolationMethod);

			float z = n * maxHeight;
			vertices[index + 2] = z;

			auto c = ImGui::ColorValue(setupdata->colors, n);
			vertices[index + 3] = c.x;
			vertices[index + 4] = c.y;
			vertices[index + 5] = c.z;

			//std::cout << x << " " << y << " " << z << std::endl;
		}
	}

	//std::cout << std::endl << "Indices" << std::endl;
	int index = 0;
	for (unsigned int i = 0; i < w; i++)
	{
		for (unsigned int j = 0; j < h; j++)
		{
			int index = (i * h + j) * 6;

			int i1 = i * (h + 1) + j;
			int i2 = i1 + 1;
			int i3 = (i + 1) * (h + 1) + j;
			indices[index] = i1;
			indices[index + 1] = i2;
			indices[index + 2] = i3;

			int i4 = i3 + 1;
			indices[index + 3] = i2;
			indices[index + 4] = i4;
			indices[index + 5] = i3;

			//std::cout << i1 << " " << i2 << " " << i3 << "   " << i2 << " " << i3 << " " << i4 << std::endl;
		}
	}

	{
		std::lock_guard<std::mutex> guard(mutex);
		isCalculating = false; //calculation done
		generate = false;
	}
}

