#include "Renderer.h"


namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | r;;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (m_FinalImage && m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
		return;

	if (m_FinalImage)
		m_FinalImage->Resize(width, height);
	else
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render() {
	uint32_t height = m_FinalImage->GetHeight();
	uint32_t width = m_FinalImage->GetWidth();
	glm::vec3 lightDir(-1, -1, -1);

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			glm::vec2 coord = { (float)x / width, (float)y / height };
			coord = coord * 2.0f - 1.0f; // [0,1] -> [-1,1]
			glm::vec4 color = PerPixel(coord, lightDir);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[y * width + x] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord, const glm::vec3& lightDir) {
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f); // ray origin. 
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f); // ray direction
	rayDirection = glm::normalize(rayDirection);
	float radius = 0.5f;

	// RD = Ray Direction. RDx, RDy, RDz - x,y and z components of ray direction respectively
	// RO - Ray Origin. ROx, ROy, ROz - x,y and z components of ray origin respectively
	// Next calculations made with an assumption that sphere is located at origin (0,0,0)
	// (RDx^2+RDy^2+RDz^2)t^2+2*(ROxRDx+ROyRDy+ROzRDz)t+(RDx^2+RDy^2+RDz^2-r^2)=0
	// r = radius
	// t = hit distance

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Quadratic formula discriminant:
	// b^2-4ac

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
		return glm::vec4(0, 0, 0, 1);

	// Quadratic formula:
	// (-b +- sqrt(discriminant)) / 2a

	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // Second hit distance (currently unused)

	glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;
	glm::vec3 normal = glm::normalize(hitPoint);

	float lightIntensity = glm::dot(normal, -lightDir); // == cos(angle)

	glm::vec3 sphereColor(1, 0, 1);
	sphereColor *= lightIntensity;
	return glm::vec4(sphereColor, 1.0f);
}
