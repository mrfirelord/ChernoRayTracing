#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material {
	glm::vec3 Albedo{ 1.0f };
	float roughness = 1.0f;
	float Metallic = 0.0f;
};

enum ShapeType {
	UNKNOWN, SPHERE, CYLINDER
};

struct Shape {
	ShapeType type = UNKNOWN;

	glm::vec3 Position{ 0.0f };
	int materialIndex = 0;
};

struct Sphere : public Shape {
	Sphere() {
		type = SPHERE;
	};

	float radius = 0.5f;

	// Max horizontal angle. Represents value within the range [0,1] (==[0, 2*PI] angle)
	float maxPhi = 1.0f;

	float yMin = 0.0f, yMax = 1.0f;
};

struct Cylinder : public Shape {
	Cylinder() {
		type = CYLINDER;
	}

	glm::vec3 Position{ 0.0f };
	float radius = 0.5f;

	float yMin = -0.3f, yMax = 0.3f;
};

struct Scene {
	std::vector<Sphere> Spheres;
	std::vector<Cylinder> cylinders;
	std::vector<Material> materials;
	std::vector<Shape*> shapes;
};