#pragma once

#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings {
		bool accumulate = true;
	};
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() {
		frameIndex = 1;
	}

	Settings& GetSettings() {
		return setting;
	}
private:
	struct HitPayload {
		// Represents t in equation O + tD, where O - Ray Origin, D - Ray Direction
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	Settings setting;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* accumulationData = nullptr;

	uint32_t frameIndex = 1;
};