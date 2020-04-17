#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec3 norm;
in vec2 tc;
flat in uint tc_mode_fg;

// the only output variable
out vec4 fragColor;

void main()
{
	if(tc_mode_fg == 0u) fragColor = vec4(tc.xy,0,1);
	else if(tc_mode_fg == 1u) fragColor = vec4(tc.xxx,1);
	else fragColor = vec4(tc.yyy,1);
}
