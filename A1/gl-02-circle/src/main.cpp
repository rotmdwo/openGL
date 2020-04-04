#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "circle.h"		// circle class definition

//*************************************
// global constants
static const char*	window_name = "cgbase - circle";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_TESS = 7;		// initial tessellation factor of the circle as a polygon
static const uint TESS = 64;

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(720, 480);

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
bool	b_solid_color = true;			// use circle's color?
bool	b_index_buffer = true;			// use index buffering?
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif
auto	circles = std::move(create_circles());
struct { bool add=false, sub=false; operator bool() const { return add||sub; } } b; // flags of keys for smooth changes

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_circle_vertices;	// host-side vertices

//*************************************
void update()
{
	// update global simulation parameter
	t = float(glfwGetTime())*0.4f;

	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix = 
	{
		min(1/aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "b_solid_color" );	if(uloc>-1) glUniform1i( uloc, b_solid_color );
	uloc = glGetUniformLocation( program, "aspect_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( vertex_array );

	// 충돌을 고려하기 위해 다음 공들의 다음 프레임 위치와 반지름을 circle_t::update()에 전달함
	
	for (int j = 0; j < 21; j++) {
		circles.at(j).center.x = circles.at(j).center.x + circles.at(j).speed * cos(circles.at(j).theta);
		circles.at(j).center.y = circles.at(j).center.y + circles.at(j).speed * sin(circles.at(j).theta);
	}
	for (int j = 0; j < 21; j++) {
		
		for (int i = 0; i < 21; i++) {
			float distance = sqrt(pow(circles.at(j).center.x - circles.at(i).center.x, 2) + pow(circles.at(j).center.y - circles.at(i).center.y, 2));
			float move = circles.at(j).radius + circles.at(i).radius - distance;

			if (distance == 0) continue;
			if (distance <= circles.at(j).radius + circles.at(i).radius ) { // 충돌하는 상황
				//float original_theta_j = circles.at(j).theta;
				//float original_theta_i = circles.at(i).theta;
				float phi;
				if (circles.at(j).center.x - circles.at(i).center.x == 0) phi = PI / 2;
				else phi = atan(abs(circles.at(j).center.y - circles.at(i).center.y) / abs(circles.at(j).center.x - circles.at(i).center.x));
				float v1x = circles.at(i).speed * (float)cos(circles.at(i).theta - phi) * (float)cos(phi) + circles.at(j).speed * (float)sin(circles.at(j).theta - phi) * (float)cos(phi + PI / 2);
				float v1y = circles.at(i).speed * (float)cos(circles.at(i).theta - phi) * (float)sin(phi) + circles.at(j).speed * (float)sin(circles.at(j).theta - phi) * (float)sin(phi + PI / 2);
				
				
				float v2x = circles.at(j).speed * (float)cos(circles.at(j).theta - phi) * (float)cos(phi) + circles.at(i).speed * (float)sin(circles.at(i).theta - phi) * (float)cos(phi + PI / 2);
				float v2y = circles.at(j).speed * (float)cos(circles.at(j).theta - phi) * (float)sin(phi) + circles.at(i).speed * (float)sin(circles.at(i).theta - phi) * (float)sin(phi + PI / 2);
				/*
				float normalX = (circles.at(i).center.x - circles.at(j).center.x) / distance;
				float normalY = (circles.at(i).center.y - circles.at(j).center.y) / distance;
				
				float delta_vx = circles.at(j).speed * cos(circles.at(j).theta) - circles.at(i).speed * cos(circles.at(i).theta);
				float delta_vy = circles.at(j).speed * sin(circles.at(j).theta) - circles.at(i).speed * sin(circles.at(i).theta);

				float v1x = circles.at(j).speed * cos(circles.at(j).theta) - (normalX * delta_vx + normalY * delta_vy) * normalX;
				float v1y = circles.at(j).speed * sin(circles.at(j).theta) - (normalX * delta_vx + normalY * delta_vy) * normalY;

				float v2x = circles.at(i).speed * cos(circles.at(i).theta) + (normalX * delta_vx + normalY * delta_vy) * normalX;
				float v2y = circles.at(i).speed * sin(circles.at(i).theta) + (normalX * delta_vx + normalY * delta_vy) * normalY;
				*/
				// 아크코사인 쓰면 안 됨!! 파이 이하 값이 나와 방향이 상향 됨
				circles.at(j).speed = sqrt(pow(v1x, 2) + pow(v1y, 2));  // 성분벡터의 합
				if (v1y >= 0) {
					circles.at(j).theta = acos(v1x / circles.at(j).speed);  // (1,0)과 내적을 이용
				}
				else {
					circles.at(j).theta = -acos(v1x / circles.at(j).speed);
				}
				//circles.at(j).theta = acos(v1x / circles.at(j).speed);  // (1,0)과 내적을 이용
				
				circles.at(i).speed = sqrt(pow(v2x, 2) + pow(v2y, 2));
				if (v2y >= 0) {
					circles.at(i).theta = acos(v2x / circles.at(i).speed);  // (1,0)과 내적을 이용
				}
				else {
					circles.at(i).theta = -acos(v2x / circles.at(i).speed);
				}
				//circles.at(i).theta = acos(v2x / circles.at(i).speed);
				
				//circles.at(j).center.x += circles.at(j).speed * cos(original_theta_j) * 2;
				//circles.at(j).center.y += circles.at(j).speed * sin(original_theta_j) * 2;

				//circles.at(i).center.x += circles.at(i).speed * cos(original_theta_i) * 2;
				//circles.at(i).center.y += circles.at(i).speed * sin(original_theta_i) * 2;

				//circles.at(j).center.x += move * cos(circles.at(j).theta) * 3;
				//circles.at(j).center.y += move * sin(circles.at(j).theta) * 3;

				//circles.at(i).center.x += move * cos(circles.at(i).theta) * 3;
				//circles.at(i).center.y += move * sin(circles.at(i).theta) * 3;

				circles.at(j).center.x += move / 2 * (circles.at(j).center.x - circles.at(i).center.x) / distance;
				circles.at(j).center.y += move / 2 * (circles.at(j).center.y - circles.at(i).center.y) / distance;

				circles.at(i).center.x -= move / 2 * (circles.at(j).center.x - circles.at(i).center.x) / distance;
				circles.at(i).center.y -= move / 2 * (circles.at(j).center.y - circles.at(i).center.y) / distance;
			}
		}
		// per-circle update
		//c.update(t);
		circles.at(j).update();

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, circles.at(j).color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, circles.at(j).model_matrix);

		// per-circle draw calls
		if (b_index_buffer)	glDrawElements(GL_TRIANGLES, TESS * 3, GL_UNSIGNED_INT, nullptr);
		else				glDrawArrays(GL_TRIANGLES, 0, TESS * 3); // TESS = N
	}
	// render two circles: trigger shader program to process vertex data
	/*
	for( auto& c : circles )
	{
		



		// per-circle update
		//c.update(t);
		c.update(next_center, radius_array,theta_array, speed_array);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, c.color);	// pointer version
		uloc = glGetUniformLocation( program, "model_matrix" );		if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, c.model_matrix );

		// per-circle draw calls
		if(b_index_buffer)	glDrawElements( GL_TRIANGLES, TESS*3, GL_UNSIGNED_INT, nullptr );
		else				glDrawArrays( GL_TRIANGLES, 0, TESS*3 ); // TESS = N
	}*/

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
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
	printf( "- press 'i' to toggle between index buffering and simple vertex buffering\n" );
#ifndef GL_ES_VERSION_2_0
	printf( "- press 'w' to toggle wireframe\n" );
#endif
	printf( "\n" );
}

std::vector<vertex> create_circle_vertices( uint N )
{
	std::vector<vertex> v = {{ vec3(0), vec3(0,0,-1.0f), vec2(0.5f) }}; // origin
	for( uint k=0; k <= N; k++ )
	{
		float t=PI*2.0f*k/float(N), c=cos(t), s=sin(t);
		v.push_back( { vec3(c,s,0), vec3(0,0,-1.0f), vec2(c,s)*0.5f+0.5f } );
	}
	return v;
}

void update_vertex_buffer( const std::vector<vertex>& vertices, uint N )
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	// create buffers
	if(b_index_buffer)
	{
		std::vector<uint> indices;
		for( uint k=0; k < N; k++ )
		{
			indices.push_back(0);	// the origin
			indices.push_back(k+1);
			indices.push_back(k+2);
		}

		// generation of vertex buffer: use vertices as it is
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers( 1, &index_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*indices.size(), &indices[0], GL_STATIC_DRAW );
	}
	else
	{
		std::vector<vertex> v; // triangle vertices
		for( uint k=0; k < N; k++ )
		{
			v.push_back(vertices.front());	// the origin
			v.push_back(vertices[k+1]);
			v.push_back(vertices[k+2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertices
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*v.size(), &v[0], GL_STATIC_DRAW );
	}

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
	vertex_array = cg_create_vertex_array( vertex_buffer, index_buffer );
	if(!vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return; }
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = true;
		else if(key==GLFW_KEY_I)
		{
			b_index_buffer = !b_index_buffer;
			update_vertex_buffer( unit_circle_vertices,TESS );
			printf( "> using %s buffering\n", b_index_buffer?"index":"vertex" );
		}
#ifndef GL_ES_VERSION_2_0
		else if(key==GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode( GL_FRONT_AND_BACK, b_wireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", b_wireframe ? "wireframe" : "solid" );
		}
#endif
	}
	else if(action==GLFW_RELEASE)
	{
		if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = false;
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = false;
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
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color  배경색 설정
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	
	// define the position of four corner vertices
	unit_circle_vertices = std::move(create_circle_vertices( TESS ));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( unit_circle_vertices, TESS );

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

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
