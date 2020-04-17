#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name = "cgsphere - unit sphere";
static const char*	vert_shader_path = "../bin/shaders/transform.vert";
static const char*	frag_shader_path = "../bin/shaders/transform.frag";
static const bool	b_index_buffer = true; // always use index buffer
static const uint MAX_TC_MODE = 2;

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1280, 720); // initial window size

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;		// index of rendering frames
uint	tc_mode = 0;	// To toggle colors
bool	b_rotation = false;	// where rotating
float	rotation_time_elapsed = 0.0f;	// only count the time of rotating
float	time_checkpoint = 0.0f;	// starting point of elapsed time
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif

// holder of vertices and indices of a unit sphere
std::vector<vertex>	unit_sphere_vertices;
std::vector<uint> indices;

//*************************************
void update()
{
	float aspect = window_size.x / float(window_size.y);
	mat4 aspect_matrix =
	{
		min(1 / aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// update uniform variables in vertex/fragment shaders
	mat4 view_projection_matrix = { 0,1,0,0,0,0,1,0,-1,0,0,1,0,0,0,1 };
	GLint uloc = glGetUniformLocation(program, "view_projection_matrix");
	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);

	// update aspect matrix
	uloc = glGetUniformLocation(program, "aspect_matrix");
	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, aspect_matrix);

	// set the texcoord mode : (tc.xy,0) or (tc.xxx) or (tc.yyy)
	uloc = glGetUniformLocation(program, "tc_mode");
	if (uloc > -1) glUniform1ui(uloc, tc_mode);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );
	
	// bind vertex array object
	glBindVertexArray(vertex_array);

	// render vertices: trigger shader programs to process vertex data
	// configure transformation parameters
	if (b_rotation) rotation_time_elapsed += float(glfwGetTime()) - time_checkpoint;
	time_checkpoint = float(glfwGetTime());
	float theta = rotation_time_elapsed * 0.5f;

	// build the model matrix
	mat4 model_matrix;

	mat4 scale_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		cos(theta), -sin(theta), 0, 0,
		sin(theta), cos(theta), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix * rotation_matrix * scale_matrix;

	// update the uniform model matrix and render
	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void update_vertex_buffer(const std::vector<vertex>& vertices, uint N)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	// create buffers
	if (b_index_buffer)
	{
		// generation of vertex buffer: use vertices as it is
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);
	}

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
#ifndef GL_ES_VERSION_2_0
	printf("- press 'w' to toggle wireframe\n");
#endif
	printf("- press 'd' to toggle (tc.xy,0) > (tc.xxx) > (tc.yyy)\n");
	printf("- press 'r' to rotate the sphere\n");
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
#ifndef GL_ES_VERSION_2_0
		else if (key == GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
#endif
		else if (key == GLFW_KEY_D) { // toggle (tc.xy,0) > (tc.xxx) > (tc.yyy)
			if (tc_mode == MAX_TC_MODE) tc_mode = 0;
			else tc_mode++;
		}
		else if (key == GLFW_KEY_R) { // rotation
			if (b_rotation) b_rotation = false;
			else b_rotation = true;
		}
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer(unit_sphere_vertices,0);

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	float phi = 0.0f;
	float theta = PI;
	vertex vertex;

	// creating vertices
	for (int i = 0; i <= 72; i++) {
		for (int j = 0; j <= 36; j++) {
			//printf("phi: %f , theta: %f\n", phi, theta);
			vertex.pos = vec3(sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta));
			vertex.norm = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vertex.tex = vec2(phi/2/PI, 1-theta/PI);
			unit_sphere_vertices.emplace_back(vertex);
			theta -= PI / 36;
		}
		theta = PI;
		phi += 2 * PI / 72;
	}

	// creating indices
	for (int i = 0; i < 72; i++) {
		for (int j = 0; j < 36; j++) {
			indices.emplace_back(37 * i + j);
			indices.emplace_back(37 * (i + 1) + j);
			indices.emplace_back(37 * (i + 1) + (j + 1));

			indices.emplace_back(37 * i + j);
			indices.emplace_back(37 * (i + 1) + (j + 1));
			indices.emplace_back(37 * i + (j + 1));
		}
	}

	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
