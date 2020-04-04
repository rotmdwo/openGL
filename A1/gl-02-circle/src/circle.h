#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__
#include<ctime>
#include<cstdlib>
struct circle_t
{
	vec2	center;		        // 2D position for translation
	float	radius;		        // radius
	float	theta;			    // moving angle
	vec4	color;				// RGBA color in [0,1]
	float   speed;              // moving speed
	mat4	model_matrix;		// modeling transformation
	// public functions
	void	update();
};

inline std::vector<circle_t> create_circles()
{
	std::vector<circle_t> circles;

	// 랜덤 시드 설정
	time_t current_time;
	time(&current_time);
	srand((unsigned int)current_time);
	
	int i = 0;
	vec2 centers[21];
	float radii[21];
	while (i < 21) {
		circle_t c;

		// 0~1 사이로 랜덤 수 설정
		float random_center_x = (float)rand() / 32767;
		float random_center_y = (float)rand() / 32767;
		float random_radius = 0.05f + (float)rand() / 32767 / 10;
		for (int j = 0; j < i; j++) {
			if (sqrt(pow(random_center_x - centers[j].x, 2) + pow(random_center_y - centers[j].y, 2)) < random_radius + radii[j]) continue;
		}
		float random_theta = (float)rand() / 32767 * PI;
		float random_color_r = (float)rand() / 32767;
		float random_color_g = (float)rand() / 32767;
		float random_color_b = (float)rand() / 32767;
		float random_speed = (float)rand() / 32767 / 20;

		// 랜덤하게 부호 결정
		if (rand() % 2 == 0) random_center_x *= -1;
		if (rand() % 2 == 0) random_center_y *= -1;
		if (rand() % 2 == 0) random_theta *= -1;
		
		c = { vec2(random_center_x, random_center_y), random_radius, random_theta, vec4(random_color_r, random_color_g, random_color_b, 1.0f), random_speed };
		circles.emplace_back(c);

		i++;
	}

	/*
	circle_t c;
	c = { vec2(-1.0f,0.0f),0.2f,0,vec4(1.0f,0,0,1.0f),0.005f };
	circles.emplace_back(c);
	c = { vec2(1.0f,0.0f),0.2f,PI,vec4(0,1.0f,0,1.0f),0.005f };
	circles.emplace_back(c);
	*/
	return circles;
}

inline void circle_t::update( )
{
	// 벽과 충돌
	if (center.x + radius >= 1.5f) {
		center.x = 1.5f - radius;
		theta = PI - theta;
	}
	else if (center.x - radius <= -1.5f) {
		center.x = -1.5f + radius;
		theta = PI - theta;
	}

	if (center.y + radius >= 1.0f) {
		center.y = 1.0f - radius;
		theta = 2*PI - theta;
	}
	else if (center.y - radius <= -1.0f) {
		center.y = -1.0f + radius;
		theta = 2*PI - theta;
	}

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		//c,-s, 0, 0,
		//s, c, 0, 0,
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

#endif
