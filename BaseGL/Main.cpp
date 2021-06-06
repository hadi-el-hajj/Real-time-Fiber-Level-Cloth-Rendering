// ----------------------------------------------
// Computer Graphics 
// Base code for practical assignment
// Introduction to interactive 3D graphics 
// application development with modern OpenGL.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define SOLUTION

using namespace std;

// Window parameters
static GLFWwindow * window = nullptr;

// camera control variables
float g_meshScale = 1.0; // to update based on the mesh size, so that navigation runs at scale
bool g_rotatingP = false;
bool g_panningP = false;
bool g_zoomingP = false;
double g_baseX = 0.0, g_baseY = 0.0;
glm::vec3 g_baseTrans(0.0);
glm::vec3 g_baseRot(0.0);

static std::vector<float> vertexPositions; // All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
static std::vector< float > vertexNormals;
static std::vector<unsigned int> vertexIndices; // All triangle indices packed in one array[v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
static std::vector< unsigned int > normalIndices;

// GPU objects
static GLuint program; // A GPU program contains at least a vertex shader and a fragment shader
static GLuint posVbo; // Identifier of the vertex buffer storing the list of vertex positions as an array in GPU memory
static GLuint ibo; // Identifier of the index buffer storing the connectivity of the mesh as an array in GPU memory 
static GLuint vao; // Identifier of a single wrapper linking vbos and ibos to form an complete GPU mesh, ready to draw
static GLuint normalVbo; // Vertex buffer storing list of normals 
static GLuint normalibo; // Index buffer storing normals index 

// Basic camera model
class Camera {
public:
	inline float getFov () const { return m_fov; }
	inline void setFoV (float f) { m_fov = f; }
	inline float getAspectRatio () const { return m_aspectRatio; }
	inline void setAspectRatio (float a) { m_aspectRatio = a; }
	inline float getNear () const { return m_near; }
	inline void setNear (float n) { m_near = n; }
	inline float getFar () const { return m_far; }
	inline void setFar (float n) { m_far = n; }
	const glm::vec3& getPosition() const { return _pos; }
  	void setPosition(const glm::vec3 &t) { _pos = t; }
  	const glm::vec3& getRotation() const { return _rotation; }
  	void setRotation(const glm::vec3 &r) { _rotation = r; }
	
	// Returns the view matrix: to express the world w.r.t. the camera, 
	// we use the inverse of the camera own transform, to all scene's entities.
	glm::mat4 computeViewMatrix() const {
		glm::mat4 rot = glm::rotate(glm::mat4(1.0), _rotation[0], glm::vec3(1.0, 0.0, 0.0));
		rot = glm::rotate(rot, _rotation[1], glm::vec3(0.0, 1.0, 0.0));
		rot = glm::rotate(rot, _rotation[2], glm::vec3(0.0, 0.0, 1.0));
		const glm::mat4 trn = glm::translate(glm::mat4(1.0), _pos);
		return glm::inverse(rot*trn);
	}

	// Returns the projection matrix stemming from the camera intrinsic parameter.
	glm::mat4 computeProjectionMatrix() const {
		return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
	}

private:
	glm::vec3 _pos = glm::vec3(0, 0, 10);
	glm::vec3 _rotation = glm::vec3(0, 0, 0);

	float m_fov = 45.f; // Field of view, in degrees
	float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
	float m_near = 0.1f; // Distance before which geometry is excluded fromt he rasterization process
	float m_far = 1000.f; // Distance after which the geometry is excluded fromt he rasterization process
};

static Camera g_cam;

vector<float> g_parameters = vector<float>();; 

//function to initialise the radius data. Contains 21 random numbers in the range [0,1], one for each fiber. 
void computeParamsVector(){
	for(int i=0; i<64 ; i++){
		g_parameters.push_back(rand() / double(RAND_MAX)); 
	}
}

//texture that contains radius information 
void set1DTexture() {
	computeParamsVector() ; 

	GLuint texID; // OpenGL texture identifier
	glGenTextures(1, &texID); // generate an OpenGL texture container
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_1D, texID); // activate the texture
	// The following lines setup the texture filtering option and repeat mode; check www.opengl.org for details.
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// fills the GPU texture with the data stored in the CPU image
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, g_parameters.size(), 0, GL_RED, GL_FLOAT, &g_parameters[0]);
}  

// LOADING MESHES FUNCTIONS ############################################################

//function to load a .obj file into buffers (cf: http://www.opengl-tutorial.org/fr/beginners-tutorials/tutorial-7-model-loading/)
bool loadOBJ(const char * filename){

	FILE * file = fopen(filename, "r");
	if( file == NULL ){
    	printf("Impossible to open the file !\n");
    	return false;
	}
	while( 1 ){
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
		else{
			if ( strcmp( lineHeader, "v" ) == 0){
				float vertexx; 
				float vertexy; 
				float vertexz; 
				fscanf(file, "%f %f %f\n", &vertexx, &vertexy, &vertexz );
				vertexPositions.push_back(vertexx);
				vertexPositions.push_back(vertexy);
				vertexPositions.push_back(vertexz);
			}else if ( strcmp( lineHeader, "vn" ) == 0 ){
				float normalx;
				float normaly;
				float normalz;
				fscanf(file, "%f %f %f\n", &normalx, &normaly, &normalz );
				vertexNormals.push_back(normalx);
				vertexNormals.push_back(normaly); 
				vertexNormals.push_back(normalz); 
			}else if ( strcmp( lineHeader, "f" ) == 0 ){
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
				if (matches != 6){
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return false;
				}
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
		}
	}
}

//Another method could be loading a BCC model (Binary Curve Collection) (cf http://www.cemyuksel.com/cyCodeBase/soln/using_bcc_files.html)

//END ############################################################


// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window. 
void windowSizeCallback (GLFWwindow* window, int width, int height) {
	g_cam.setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Function called every time the mouse is moved
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  const float normalizer = static_cast<float>((width + height)/2);
  const float dx = static_cast<float>((g_baseX - xpos) / normalizer);
  const float dy = static_cast<float>((ypos - g_baseY) / normalizer);
  if(g_rotatingP) {
    const glm::vec3 dRot(-dy*M_PI, dx*M_PI, 0.0);
    g_cam.setRotation(g_baseRot + dRot);
  } else if(g_panningP) {
    g_cam.setPosition(g_baseTrans + g_meshScale*glm::vec3(10*dx, 10*dy, 0.0));
  } else if(g_zoomingP) {
    g_cam.setPosition(g_baseTrans + g_meshScale*glm::vec3(0.0, 0.0, 10*dy));
  }
}

// Called each time a mouse button is pressed
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if(!g_rotatingP) {
      g_rotatingP = true;
      glfwGetCursorPos(window, &g_baseX, &g_baseY);
      g_baseRot = g_cam.getRotation();
    }
  } else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    g_rotatingP = false;
  } else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    if(!g_panningP) {
      g_panningP = true;
      glfwGetCursorPos(window, &g_baseX, &g_baseY);
      g_baseTrans = g_cam.getPosition();
    }
  } else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    g_panningP = false;
  } else if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    if(!g_zoomingP) {
      g_zoomingP = true;
      glfwGetCursorPos(window, &g_baseX, &g_baseY);
      g_baseTrans = g_cam.getPosition();
    }
  } else if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    g_zoomingP = false;
  }
}

// Executed each time a key is entered.
void keyCallback (GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_F2) {
    	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose (window, true); // Closes the application if the escape key is pressed
	}
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	window = glfwCreateWindow (1024, 768, "Base GL - Computer Graphics - Practical Assignment - OpenGL Intro.", nullptr, nullptr);
	if (!window) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (window);
	glfwSetWindowSizeCallback (window, windowSizeCallback);
	glfwSetKeyCallback (window, keyCallback);

	//add mouse position callback function
	glfwSetCursorPosCallback(window, mouse_move_callback); 

	//add mouse button function
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void initOpenGL () {
	glfwMakeContextCurrent (window);
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
		glfwTerminate();
		std::exit (EXIT_FAILURE);
	}
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (1.f, 1.f, 1.f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String (const std::string & filename) {
	std::ifstream t (filename.c_str ());
	std::stringstream buffer;
	buffer << t.rdbuf ();
	return buffer.str ();
}

// Loads and compile a shader, before attaching it to a program
void loadShader (GLuint program, GLenum type, const std::string & shaderFilename) {
	GLuint shader = glCreateShader (type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh

	//check if shader was created successfully
	if(shader == 0) {
		std::cout<<"ERROR IN CREATING SHADER ." << std::endl ; 
	}

	std::string shaderSourceString = file2String (shaderFilename); // Loads the shader source from a file to a C++ string
	//print out shader source to check if parsed correctly
	// std::cout<< shaderSourceString <<std::endl ; 

	const GLchar * shaderSource = (const GLchar *)shaderSourceString.c_str (); // Interface the C++ string through a C pointer
	glShaderSource (shader, 1, &shaderSource, NULL); // Load the vertex shader source code
	glCompileShader (shader);  // THe GPU driver compile the shader

	//check if shader was compiled successfully
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled) ; 
	if(isCompiled == GL_TRUE){
		std::cout<<"SHADER COMPILED SUCCESSFULLY"<< std::endl ; 
	}
	else{
		std::cout<<"SHADER DID NOT COMPILE" <<std::endl ; 

		//get error message
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		for(int i =0 ; i< errorLog.size(); i++){
			std::cout<<errorLog[i] ; 
		}
	}
	glAttachShader (program, shader); // Set the vertex shader as the one ot be used with the program/pipeline
	glDeleteShader (shader);
}

void initGPUProgram () {
	program = glCreateProgram (); // Create a GPU program i.e., a graphics pipeline
	loadShader (program, GL_VERTEX_SHADER, "VertexShader.glsl");
	loadShader (program, GL_FRAGMENT_SHADER, "FragmentShader.glsl");
	loadShader (program, GL_TESS_CONTROL_SHADER, "TesselationControlShader.glsl"); 
	loadShader (program, GL_TESS_EVALUATION_SHADER, "TessellationEvaluationShader.glsl");
	// loadShader (program, GL_GEOMETRY_SHADER, "GeometryShader.glsl"); 
	glLinkProgram (program); // The main GPU program is ready to be handle streams of polygons
} //

// Change this code to crear a different geometry
void initCPUGeometry () {
	// vertexPositions = { // The array of vertex positions [x0, y0, z0, x1, y1, z1, ...]
	// 					3.0f, 0.0f, 0.0f,
	// 					4.0f, 0.0f, 0.0f,
	// 					5.0f, 0.0f, 0.0f,
	// 					6.0f, 0.0f, 0.0f
	// 				  };

	loadOBJ("./Models/openwork_trellis_pattern.obj"); 
	
	// vertexIndices = { 0, 1, 2 , 3 };
	// vertexNormals  = {
	// 					0.f , 1.f, 0.f, 
	// 					0.f , 1.f, 0.f, 
	// 					0.f , 1.f, 0.f, 
	// 					0.f , 1.f, 0.f}; 
	// normalIndices = { 0, 1, 2 , 3  };

}


void initGPUGeometry () {
	// vertex positions
	glCreateBuffers (1, &posVbo); // Generate a GPU buffer to store the positions of the vertices
	size_t vertexBufferSize = sizeof (float) * vertexPositions.size (); // Gather the size of the buffer from the CPU-side vector
	glNamedBufferStorage (posVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT); // Creta a data store on the GPU
	glNamedBufferSubData (posVbo, 0, vertexBufferSize, vertexPositions.data ()); // Fill the data store from a CPU array

	// normals
	glCreateBuffers(1, &normalVbo); 
	size_t normalsBufferSize = sizeof(float)*vertexNormals.size(); 
	glNamedBufferStorage(normalVbo, normalsBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(normalVbo, 0, normalsBufferSize, vertexNormals.data());


	// vertex indices 
	glCreateBuffers (1, &ibo); // Same for the index buffer, that stores the list of indices of the triangles forming the mesh
	size_t indexBufferSize = sizeof (unsigned int) * vertexIndices.size ();
	glNamedBufferStorage (ibo, indexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (ibo, 0, indexBufferSize, vertexIndices.data ());

	// normal indices 
	size_t normalindexBufferSize = sizeof(unsigned int)*normalIndices.size();
	glCreateBuffers(1, &normalibo);
	glNamedBufferStorage(normalibo, normalindexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(normalibo, 0, normalindexBufferSize, normalIndices.data());
	

	glCreateVertexArrays (1, &vao); // Create a single hangle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, normalibo); 
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time. 
}

void initCamera () {
	int width, height;
	glfwGetWindowSize (window, &width, &height);
	g_cam.setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
}

void init () {
	initGLFW ();
	initOpenGL ();
	initCPUGeometry ();
	initGPUProgram ();
	initGPUGeometry ();
	initCamera ();

	//set 1D texture for now containing fiber radius information
	set1DTexture(); 
}

void clear () {
	glDeleteProgram (program);
	glDeleteVertexArrays (1, &vao);
	glDeleteBuffers (1, &posVbo);
	glDeleteBuffers (1, &ibo);
	glDeleteBuffers (1, &normalVbo);
	glDeleteBuffers (1, &normalibo); 
	glfwDestroyWindow (window);
	glfwTerminate ();
}

// The main rendering call
void render () {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	glm::mat4 projectionMatrix = g_cam.computeProjectionMatrix ();
	glm::mat4 viewMatrix = g_cam.computeViewMatrix ();
	glUseProgram (program); // Activate the program to be used for upcoming primitive

	//1D texture
	glUniform1i(glGetUniformLocation( program, "Texture" ), 0); 

	glUniformMatrix4fv (glGetUniformLocation (program, "projectionMat"), 1, GL_FALSE, glm::value_ptr (projectionMatrix)); // Pass it to the GPU program
	glUniformMatrix4fv (glGetUniformLocation (program, "modelViewMat"), 1, GL_FALSE, glm::value_ptr (viewMatrix)); 
	glBindVertexArray (vao); // Activate the VAO storing geometry data
	glPatchParameteri(GL_PATCH_VERTICES,4);  
	glDrawElements (GL_PATCHES, vertexIndices.size (), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
	// glDrawElements (GL_TRIANGLES, vertexIndices.size (), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program

}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->
}

int main (int argc, char ** argv) {
	init (); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
	while (!glfwWindowShouldClose (window)) {
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (window);
		glfwPollEvents ();
	}
	clear ();
	return EXIT_SUCCESS;
}

