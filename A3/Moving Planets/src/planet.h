#pragma once

#define NUM_OF_PLANETS 8

struct planet_t
{
	vec3	center;		        // 3D position for translation
	float	radius;		        // radius
	float	rotation_theta;		// self-rotating angle
	float	revolution_theta;	// around-rotating angle
	float   rotation_speed;     // self-rotating speed
	float   revolution_speed;   // around-rotating speed
	mat4	model_matrix;		// modeling transformation
	// public functions
	void	update();
};

inline std::vector<planet_t> create_planets()
{
	std::vector<planet_t> planets;
	planet_t planet;
	
	// Set Sun and 7 planets
	planet = { vec3(0.0f,0.0f,0.0f), 8.0f, 0.0f, 0.0f, 0.5f, 0.0f }; // Sun
	planets.emplace_back(planet);

	planet = { vec3(10.0f,sqrt(14.4f*14.4f - 10.0f*10.0f),0.0f), 1.4f, 0.0f, 0.0f, 1.0f, 1.0f };
	planets.emplace_back(planet);
	
	planet = { vec3(2.0f,-sqrt(20.8f * 20.8f - 2.0f * 2.0f),0.0f), 2.5f, 0.0f, 0.0f, 0.8f, 0.9f };
	planets.emplace_back(planet);

	planet = { vec3(-5.0f,sqrt(28.3f * 28.3f - 5.0f * 5.0f),0.0f), 1.7f, 0.0f, 0.0f, 0.4f, 0.8f };
	planets.emplace_back(planet);

	planet = { vec3(-15.0f,-sqrt(35.0f * 35.0f - 15.0f * 15.0f),0.0f), 1.8f, 0.0f, 0.0f, 0.5f, 0.7f };
	planets.emplace_back(planet);

	planet = { vec3(30.0f,sqrt(41.8f * 41.8f - 30.0f * 30.0f),0.0f), 1.5f, 0.0f, 0.0f, 0.3f, 0.6f };
	planets.emplace_back(planet);

	planet = { vec3(-35.0f,-sqrt(48.3f * 48.3f - 35.0f * 35.0f),0.0f), 1.2f, 0.0f, 0.0f, 0.8f, 0.5f };
	planets.emplace_back(planet);

	planet = { vec3(17.0f,sqrt(54.5f * 54.5f - 17.0f * 17.0f),0.0f), 0.7f, 0.0f, 0.0f, 0.8f, 0.4f };
	planets.emplace_back(planet);

	return planets;
}

inline void planet_t::update()
{

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		cos(rotation_theta), -sin(rotation_theta), 0, 0,
		sin(rotation_theta), cos(rotation_theta), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	// Revolve around Sun
	mat4 revolution_matrix =
	{
		cos(revolution_theta), -sin(revolution_theta), 0, 0,
		sin(revolution_theta), cos(revolution_theta), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	model_matrix = revolution_matrix * translate_matrix * rotation_matrix * scale_matrix;
}
