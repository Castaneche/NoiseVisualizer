#include "Terrain.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLFW/glfw3.h"

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
	if (update == true || setupdata->updated == true)
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	ImGui::Image((void*)(intptr_t)texColorBuffer, ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().x *0.75));
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
	model = glm::rotate(model, (float)glfwGetTime() * glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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
	if (ImGui::SliderFloat("Maximum Height", &maxHeight, 0, 50))
		update = true;
	ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0, 10);
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
			//Noise calculation
			double px = i / double(w+1) * setupdata->frequency;
			double py = j / double(h+1) * setupdata->frequency;
			float z = setupdata->pn.noise(px + setupdata->seed * 100, py + setupdata->seed * 100, setupdata->octaves, setupdata->persistence, setupdata->interpolationMethod) * maxHeight;
			int index = (i * (h+1) + j) * 3;
			vertices[index + 2] = z;
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
	vertices.resize((w + 1) * (h + 1) * 3, 0);
	indices.resize(w * h * 6, 0);


	std::cout << "Vertices" << std::endl;
	for (unsigned int i = 0; i < w + 1; i++) //+1 because width refers to number of cells ex: 2 cells = 3 vertices in y axis
	{
		float x = i - (w / 2.);
		for (unsigned int j = 0; j < h + 1; j++)
		{
			int index = (i * (h + 1) + j) * 3;

			float y = j - (h / 2.);
			vertices[index] = x;
			vertices[index + 1] = y;
			//Noise calculation
			double px = i / double(w+1) * setupdata->frequency;
			double py = j / double(h+1) * setupdata->frequency;
			float z = setupdata->pn.noise(px + setupdata->seed * 100, py + setupdata->seed * 100, setupdata->octaves, setupdata->persistence, setupdata->interpolationMethod) * maxHeight;
			vertices[index + 2] = z;

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

