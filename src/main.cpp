/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
#define PI 3.1415926
float rot_angle = 0;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -3*ftime;
		else if(d==1)
			yangle = 3*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog,psky,prog2;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;
    GLuint VertexArrayID2;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox, InstanceBuffer;
    GLuint VertexBufferID2, VertexNormDBox2, VertexTexBox2, IndexBufferIDBox2, InstanceBuffer2;

	//texture data
	GLuint Texture;
	GLuint Texture2;
    GLuint Texture3;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
//		double posX, posY;
//		float newPt[2];
//		if (action == GLFW_PRESS)
//		{
//			glfwGetCursorPos(window, &posX, &posY);
//			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;
//
//			//change this to be the points converted to WORLD
//			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
//			newPt[0] = 0;
//			newPt[1] = 0;
//
//			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
//			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
//			//update the vertex array with the updated points
//			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
//			glBindBuffer(GL_ARRAY_BUFFER, 0);
//		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
    
//    vector<vec3> ranV;
    
    float frand() {return rand()/(float)RAND_MAX;}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{

		string resourceDirectory = "../../resources";
		// Initialize mesh.
		shape = make_shared<Shape>();
		//shape->loadMesh(resourceDirectory + "/t800.obj");
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			cube_vertices[i] *= 0.8;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,

		};
		glGenBuffers(1, &VertexNormDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


		//generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
		glm::vec4 *positions = new glm::vec4[10000];
        
        for (int i = 0; i < 10000; i++){
            float r = frand()* 200.0f;
            float ax = frand() * 2 * PI;
            float ay = frand() * PI;
            mat3 Rx = rotate(mat4(1), ax, vec3(1.0f,0,0));
            mat3 Ry = rotate(mat4(1), ay, vec3(0,1.0f,0));
            vec3 v = vec3 (0,0,r);
            v = Rx * v;
            v = Ry * v;
            v.y = v.y / 2.5;
            positions[i] = vec4(v,0);
        }
        
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(glm::vec4), positions, GL_STATIC_DRAW);
		int position_loc = glGetAttribLocation(prog->pid, "InstancePos");
		
		// Set up the vertex attribute
		glVertexAttribPointer(position_loc , 4, GL_FLOAT, GL_FALSE,  sizeof(vec4),  (void *)(sizeof(vec4))); 
		glEnableVertexAttribArray(position_loc);
		// Make it instanced
		glVertexAttribDivisor(position_loc, 1);

		glBindVertexArray(0);
        
        //generate the VAO
        glGenVertexArrays(1, &VertexArrayID2);
        glBindVertexArray(VertexArrayID2);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID2);

        GLfloat cube_vertices2[] = {
            // front
            -1.0, -1.0,  1.0,//LD
            1.0, -1.0,  1.0,//RD
            1.0,  1.0,  1.0,//RU
            -1.0,  1.0,  1.0,//LU
        };
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices2), cube_vertices2, GL_DYNAMIC_DRAW);

        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        GLfloat cube_norm2[] = {
            // front colors
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,

        };
        glGenBuffers(1, &VertexNormDBox2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm2), cube_norm2, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        glm::vec2 cube_tex2[] = {
            // front colors
            glm::vec2(0.0, 1.0),
            glm::vec2(1.0, 1.0),
            glm::vec2(1.0, 0.0),
            glm::vec2(0.0, 0.0),

        };
        glGenBuffers(1, &VertexTexBox2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex2), cube_tex2, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &IndexBufferIDBox2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox2);
        GLushort cube_elements2[] = {

            // front
            0, 1, 2,
            2, 3, 0,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements2), cube_elements2, GL_STATIC_DRAW);
        
        //generate vertex buffer to hand off to OGL ###########################
        glGenBuffers(1, &InstanceBuffer2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer2);
        glm::vec4 *positions2 = new glm::vec4[25];
//        for (int i = 0; i<500; i++)
//            positions[i] = glm::vec4(-250 + i, 0, -10, 0);
        
        for (int i = 0; i < 25; i++){
            float r = (frand() + 0.5)* 200.0f;
            float ax = frand() * 2 * PI;
            float ay = frand() * PI;
            mat3 Rx = rotate(mat4(1), ax, vec3(1.0f,0,0));
            mat3 Ry = rotate(mat4(1), ay, vec3(0,1.0f,0));
            vec3 v = vec3 (0,0,r);
            v = Rx * v;
            v = Ry * v;
            v.y = v.y /2;
            positions2[i] = vec4(v,0);
        }
        
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, 25 * sizeof(glm::vec4), positions2, GL_STATIC_DRAW);
        int position_loc2 = glGetAttribLocation(prog2->pid, "InstancePos");
        
        // Set up the vertex attribute
        glVertexAttribPointer(position_loc2 , 4, GL_FLOAT, GL_FALSE,  sizeof(vec4),  (void *)(sizeof(vec4)));
        glEnableVertexAttribArray(position_loc2);
        // Make it instanced
        glVertexAttribDivisor(position_loc2, 1);

        glBindVertexArray(0);

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/Blue_Giant.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/galaxy.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
        
        // texture 3
        str = resourceDirectory + "/gas.png";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture3);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Texture3);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
        
         Tex1Location = glGetUniformLocation(prog2->pid, "tex");//tex, tex2... sampler in the fragment shader
         Tex2Location = glGetUniformLocation(prog2->pid, "tex2");
        // Then bind the uniform samplers to texture units:
        glUseProgram(prog2->pid);
        glUniform1i(Tex1Location, 0);
        glUniform1i(Tex2Location, 1);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
        prog->addUniform("rot_angle");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addAttribute("InstancePos");
        
        prog2 = std::make_shared<Program>();
        prog2->setVerbose(true);
        prog2->setShaderNames(resourceDirectory + "/gas_vertex.glsl", resourceDirectory + "/gas_frag.glsl");
        if (!prog2->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        prog2->addUniform("P");
        prog2->addUniform("V");
        prog2->addUniform("M");
        prog2->addUniform("campos");
        prog2->addAttribute("vertPos");
        prog2->addAttribute("vertNor");
        prog2->addAttribute("vertTex");
        prog2->addAttribute("InstancePos");

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/

    
    
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

        rot_angle += 0.01;
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the box using GLSL.
		psky->bind();

		
		//send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, false);
		glEnable(GL_DEPTH_TEST);
	
		psky->unbind();

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926/2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		M =  TransZ * RotateY * RotateX * S;

		// Draw the box using GLSL.
		prog->bind();

		
		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(prog->getUniform("rot_angle"),rot_angle);

		
	
		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		mat4 Vi = glm::transpose(V);
		Vi[0][3] = 0;
		Vi[1][3] = 0;
		Vi[2][3] = 0;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		M = TransZ * S * Vi;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        
//        sort(ranV.begin(), ranV.end(), compareZ);
		
		glDisable(GL_DEPTH_TEST);
        
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0,10000);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);

		
		prog->unbind();
        
        prog2->bind();

        
        //send the matrices to the shaders
        glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog2->getUniform("campos"), 1, &mycam.pos[0]);

        
    
        glBindVertexArray(VertexArrayID2);
        //actually draw from vertex 0, 3 vertices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox2);
        //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
        Vi = glm::transpose(V);
        Vi[0][3] = 0;
        Vi[1][3] = 0;
        Vi[2][3] = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture3);
        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -300.0f));
        S = scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
        M = TransZ * S* Vi;
        glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        
//        sort(ranV.begin(), ranV.end(), compareZ);
        
        glDisable(GL_DEPTH_TEST);
        
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0,25);
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(0);

        
        prog2->unbind();


	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
