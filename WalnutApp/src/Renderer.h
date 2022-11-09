#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Ray.h"
#include "Camera.h"
#include "Walnut/Image.h"

class Renderer {
public:
	Renderer() = default;

	void Render(const Camera& camera);

	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }

private:
	glm::vec4 TraceRay(const Ray& ray, const glm::vec3& lightDir);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};

