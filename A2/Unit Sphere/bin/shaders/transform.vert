// vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

// matrices
uniform mat4 model_matrix;
uniform mat4 view_projection_matrix;
uniform mat4	aspect_matrix;	// tricky 4x4 aspect-correction matrix
uniform uint tc_mode;

out vec3 norm;
out vec2 tc;
flat out uint tc_mode_fg;

void main()
{
	gl_Position = aspect_matrix * view_projection_matrix * model_matrix * vec4(position,1);
	norm = normal;
	tc = texcoord;
	tc_mode_fg = tc_mode;
}
