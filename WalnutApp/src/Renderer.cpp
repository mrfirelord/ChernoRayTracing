#include "Renderer.h"


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

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			glm::vec2 coord = { (float)x / width, (float)y / height };
			coord = coord * 2.0f - 1.0f; // [0,1] -> [-1,1]
			m_ImageData[y * width + x] = PerPixel(coord);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord) {
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	glm::vec3 ro(0.0f, 0.0f, 2.00f); // ray origin. 
	glm::vec3 rd(coord.x, coord.y, -1.0f); // ray direction
	rd = glm::normalize(rd);
	float radius = 1.0f;

	// RD = Ray Direction. RDx, RDy, RDz - x,y and z components of ray direction respectively
	// RO - Ray Origin. ROx, ROy, ROz - x,y and z components of ray origin respectively
	// Next calculations made with an assumption that sphere is located at origin (0,0,0)
	// (RDx^2+RDy^2+RDz^2)t^2+2*(ROxRDx+ROyRDy+ROzRDz)t+(RDx^2+RDy^2+RDz^2-r^2)=0
	// r = radius
	// t = hit distance

	float a = glm::dot(rd, rd);
	float b = 2.0f * glm::dot(ro, rd);
	float c = glm::dot(ro, ro) - radius * radius;

	// Quadratic formula discriminant:
	// b^2-4ac

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant >= 0.0f)
		return 0xffff00ff;

	return 0xff000000;
}
