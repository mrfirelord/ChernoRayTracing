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
	for (uint32_t i = 0; i < m_FinalImage->GetWidth() * m_FinalImage->GetHeight(); i++) {
		m_ImageData[i] = Walnut::Random::UInt();
		m_ImageData[i] |= 0xff000000;
	}

	m_FinalImage->SetData(m_ImageData);
}