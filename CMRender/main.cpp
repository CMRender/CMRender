/* OpenGL example code - Geometry Shader and Blending
*
* Uses a geometry shader to expand points to billboard quads.
* The billboards are then blended while drawing to create a galaxy
* made of particles.
*https://github.com/progschj/OpenGL-Examples/blob/master/07geometry_shader_blending.cpp
* Autor: Jakob Progsch
*/

#include <GL/glew.h> // Mark -- Originally it was glxw
#include <GLFW/glfw3.h>

//glm is used to create perspective and transform matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
//#include <string>
#include <vector>
#include <cstdlib>

#include "BasicShaders.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"




// Forward decleration for cleanliness (Ignore)
void APIENTRY debugCallbackARB(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, GLvoid *);

// helper to check and display for shader compiler errors
bool check_shader_compile_status(GLuint obj) {
	GLint status;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
		std::vector<char> log(length);
		glGetShaderInfoLog(obj, length, &length, &log[0]);
		std::cerr << &log[0];
		return false;
	}
	return true;
}

// helper to check and display for shader linker error
bool check_program_link_status(GLuint obj) {
	GLint status;
	glGetProgramiv(obj, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
		std::vector<char> log(length);
		glGetProgramInfoLog(obj, length, &length, &log[0]);
		std::cerr << &log[0];
		return false;
	}
	return true;
}

int main() {


	//------------------------------------------------------------------------------------------------
	//                                     Window, opengl, and extension stuff
	//------------------------------------------------------------------------------------------------
	
	int width = 640;
	int height = 480;

	if (glfwInit() == GL_FALSE) {
		std::cerr << "failed to init GLFW" << std::endl;
		return 1;
	}

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// select opengl version
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// create a window
	GLFWwindow *window;
	if ((window = glfwCreateWindow(width, height, "07geometry_shader_blending", 0, 0)) == 0) {
		std::cout << "failed to open window" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Make the g_window's context is current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(window);

	//Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	if (glewInit()) { // Mark -- Originally it was glxwInit()
		std::cout << "failed to init GLEW" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	// Print out our OpenGL verisions
	std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << std::endl;

	// Enable GL_ARB_debug_output if available. Not nessesary, just helpful
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// This allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Set the up callback
		glDebugMessageCallbackARB(debugCallbackARB, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		std::cout << "GL_ARB_debug_output callback installed" << std::endl;
	}
	else {
		std::cout << "GL_ARB_debug_output not available. No worries." << std::endl;
	}

	
	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	//------------------------------------------------------------------------------------------------
	//                                     Shader Stuff
	//------------------------------------------------------------------------------------------------

	// program and shader handles
	GLuint shader_program, vertex_shader, geometry_shader, fragment_shader;

	// we need these to properly pass the strings
	const char *source;
	int length;

	// create and compiler vertex shader
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	source = vertex_source.c_str();
	length = vertex_source.size();
	glShaderSource(vertex_shader, 1, &source, &length);
	glCompileShader(vertex_shader);
	if (!check_shader_compile_status(vertex_shader)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	// create and compiler geometry shader
	geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
	source = geometry_source.c_str();
	length = geometry_source.size();
	glShaderSource(geometry_shader, 1, &source, &length);
	glCompileShader(geometry_shader);
	if (!check_shader_compile_status(geometry_shader)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	// create and compiler fragment shader
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	source = fragment_source.c_str();
	length = fragment_source.size();
	glShaderSource(fragment_shader, 1, &source, &length);
	glCompileShader(fragment_shader);
	if (!check_shader_compile_status(fragment_shader)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	// create program
	shader_program = glCreateProgram();

	// attach shaders
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, geometry_shader);
	glAttachShader(shader_program, fragment_shader);

	// link the program and check for errors
	glLinkProgram(shader_program);
	check_program_link_status(shader_program);

	//------------------------------------------------------------------------------------------------
	//                                     Initialize Geometry/Material/Lights
	//------------------------------------------------------------------------------------------------

	// obtain location of projection uniform
	GLint View_location = glGetUniformLocation(shader_program, "View");
	GLint Projection_location = glGetUniformLocation(shader_program, "Projection");

	// vao and vbo handle
	GLuint vao, vbo;

	// generate and bind the vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// generate and bind the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// create a galaxylike distribution of points
	const int particles = 128 * 1024;
	std::vector<GLfloat> vertexData(particles * 3);
	for (int i = 0; i<particles; ++i)
	{
		int arm = 3 * (std::rand() / float(RAND_MAX));
		float alpha = 1 / (0.1f + std::pow(std::rand() / float(RAND_MAX), 0.7f)) - 1 / 1.1f;
		float r = 4.0f*alpha;
		alpha += arm*2.0f*3.1416f / 3.0f;

		vertexData[3 * i + 0] = r*std::sin(alpha);
		vertexData[3 * i + 1] = 0;
		vertexData[3 * i + 2] = r*std::cos(alpha);

		vertexData[3 * i + 0] += (4.0f - 0.2*alpha)*(2 - (std::rand() / float(RAND_MAX) + std::rand() / float(RAND_MAX) +
			std::rand() / float(RAND_MAX) + std::rand() / float(RAND_MAX)));
		vertexData[3 * i + 1] += (2.0f - 0.1*alpha)*(2 - (std::rand() / float(RAND_MAX) + std::rand() / float(RAND_MAX) +
			std::rand() / float(RAND_MAX) + std::rand() / float(RAND_MAX)));
		vertexData[3 * i + 2] += (4.0f - 0.2*alpha)*(2 - (std::rand() / float(RAND_MAX) + std::rand() / float(RAND_MAX) +
			std::rand() / float(RAND_MAX) + std::rand() / float(RAND_MAX)));
	}

	// fill with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertexData.size(), &vertexData[0], GL_STATIC_DRAW);


	// set up generic attrib pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (char*)0 + 0 * sizeof(GLfloat));

	// we are blending so no depth testing
	glDisable(GL_DEPTH_TEST);

	// enable blending
	glEnable(GL_BLEND);
	//  and set the blend function to result = 1*source + 1*destination
	glBlendFunc(GL_ONE, GL_ONE);


	//------------------------------------------------------------------------------------------------
	//                                     Main Loop
	//------------------------------------------------------------------------------------------------

	while (!glfwWindowShouldClose(window)) {
		
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();


		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		
		static float speed = 0.5f;
		{
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("Speed", &speed, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			if (ImGui::Button("Test Window")) show_test_window ^= 1;
			if (ImGui::Button("Another Window")) show_another_window ^= 1;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}


		// get the time in seconds
		float t = glfwGetTime();
		t *= speed;

		// clear first
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use the shader program
		glUseProgram(shader_program);

		// calculate ViewProjection matrix
		glm::mat4 Projection = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.f);

		// translate the world/view position
		glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -50.0f));

		// make the camera rotate around the origin
		View = glm::rotate(View, 30.0f*std::sin(0.1f*t), glm::vec3(1.0f, 0.0f, 0.0f));
		View = glm::rotate(View, -22.5f*t, glm::vec3(0.0f, 1.0f, 0.0f));


		// set the uniform
		glUniformMatrix4fv(View_location, 1, GL_FALSE, glm::value_ptr(View));
		glUniformMatrix4fv(Projection_location, 1, GL_FALSE, glm::value_ptr(Projection));

		// bind the vao
		glBindVertexArray(vao);

		// draw
		glDrawArrays(GL_POINTS, 0, particles);

		// check for errors -- this breaks the code when context is set to opengl 3.1+
		/*GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << error << std::endl;
			break;
		}*/

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);

		// finally swap buffers
		ImGui::Render();
		glfwSwapBuffers(window);
	}

	//------------------------------------------------------------------------------------------------
	//                                     Clean-up
	//------------------------------------------------------------------------------------------------

	// delete the created objects
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, geometry_shader);
	glDetachShader(shader_program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(geometry_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(shader_program);

	ImGui_ImplGlfwGL3_Shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


//--------------------------------------------------------------------------
// Fancy debug stuff
//--------------------------------------------------------------------------

// function to translate source to string
std::string getStringForSource(GLenum source) {

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return ("API");
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return ("Window System");
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return ("Shader Compiler");
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return ("Third Party");
	case GL_DEBUG_SOURCE_APPLICATION:
		return ("Application");
	case GL_DEBUG_SOURCE_OTHER:
		return ("Other");
	default:
		return ("n/a");
	}
}

// function to translate severity to string
std::string getStringForSeverity(GLenum severity) {

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return ("HIGH!");
	case GL_DEBUG_SEVERITY_MEDIUM:
		return ("Medium");
	case GL_DEBUG_SEVERITY_LOW:
		return ("Low");
	default:
		return ("n/a");
	}
}

// function to translate type to string
std::string getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return ("Error");
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return ("Deprecated Behaviour");
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return ("Undefined Behaviour");
	case GL_DEBUG_TYPE_PORTABILITY:
		return ("Portability Issue");
	case GL_DEBUG_TYPE_PERFORMANCE:
		return ("Performance Issue");
	case GL_DEBUG_TYPE_OTHER:
		return ("Other");
	default:
		return ("n/a");
	}
}

// actually define the function
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar *message,
	GLvoid *) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) return;

	std::cerr << std::endl; // extra space

	std::cerr << "Type: " <<
		getStringForType(type) << "; Source: " <<
		getStringForSource(source) << "; ID: " << id << "; Severity: " <<
		getStringForSeverity(severity) << std::endl;

	std::cerr << message << std::endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw std::runtime_error("");
}