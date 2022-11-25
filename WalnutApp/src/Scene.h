#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material {
	glm::vec3 Albedo{ 1.0f };
	float roughness = 1.0f;
	float Metallic = 0.0f;
};

struct Sphere
{
	glm::vec3 Position{ 0.0f };
	float radius = 0.5f;
	int materialIndex = 0;

	// Max horizontal angle. Represents value within the range [0,1] (==[0, 2*PI] angle)
	float maxPhi = 1.0f; 

	float yMin = 0.0f, yMax = 1.0f;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> materials;
};