#include "Renderer.h"

#include "Walnut/Random.h"

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

static float PI = 3.14159265358979323846264338327950288;
static float TWO_PI = 2 * PI;

void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (m_FinalImage) {
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else {
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] accumulationData;
	accumulationData = new glm::vec4[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (frameIndex == 1)
		memset(accumulationData, 0, sizeof(glm::vec4) * m_FinalImage->GetWidth() * m_FinalImage->GetHeight());

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			glm::vec4 color = PerPixel(x, y);
			accumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = accumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)frameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}

	m_FinalImage->SetData(m_ImageData);

	if (setting.accumulate)
		frameIndex++;
	else
		frameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0;

	int bounces = 5;
	for (int i = 0; i < bounces; i++) {
		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f) {
			glm::vec3 skyColor(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(angle)

		Shape* shape = m_ActiveScene->shapes[payload.ObjectIndex];
		const Material& material = m_ActiveScene->materials[shape->materialIndex];

		glm::vec3 sphereColor = material.Albedo;
		sphereColor *= lightIntensity;
		color += sphereColor * multiplier;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.000001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

		multiplier = 0.5;
	}

	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {
	Renderer::HitPayload closestHitPayload;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_ActiveScene->shapes.size(); i++) {
		Shape* shape = m_ActiveScene->shapes[i];
		Renderer::HitPayload hitPayload;
		if (shape->type == CYLINDER) {
			const Cylinder& cylinder = *((Cylinder*)shape);
			hitPayload = findRayParameterFromHit(ray, cylinder, i);
			if (hitPayload.HitDistance == -1)
				continue;
		}
		else if (shape->type == SPHERE) {
			const Sphere& sphere = *((Sphere*)shape);
			hitPayload = findRayParameterFromHit(ray, sphere, i);
			if (hitPayload.HitDistance == -1)
				continue;
		}

		if (hitPayload.HitDistance > 0 && hitPayload.HitDistance < hitDistance) {
			hitDistance = hitPayload.HitDistance;
			closestHitPayload = hitPayload;
		}
	}

	if (closestHitPayload.HitDistance < 0)
		return Miss(ray);

	return closestHitPayload;
}

Renderer::HitPayload Renderer::findRayParameterFromHit(const Ray& ray, const Cylinder& cylinder, size_t objectIndex) {
	// ray.Origin represents world coordinate system. To find a hit we need to convert ray coordinates to object coordinate
	// origin is the ray origin but in object coordinate system
	glm::vec3 origin = ray.Origin - cylinder.Position;

	float a = ray.Direction.x * ray.Direction.x + ray.Direction.z * ray.Direction.z;
	float b = 2.0f * (ray.Direction.x * ray.Origin.x + ray.Direction.z * ray.Origin.z);
	float c = ray.Origin.x * ray.Origin.x + ray.Origin.z * ray.Origin.z - cylinder.radius * cylinder.radius;

	// Quadratic forumula discriminant:
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
		return Renderer::HitPayload();

	// Quadratic formula:
	// (-b +- sqrt(discriminant)) / 2a
	float hitDistance = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
	bool rayTouchesTheSide = true;

	Renderer::HitPayload payload;
	payload.ObjectIndex = objectIndex;
	if (hitPoint.y > cylinder.yMax) {
		float t1 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		glm::vec3 hitPointAtT1 = origin + ray.Direction * t1;
		// we reached the top 
		if (hitPointAtT1.y < cylinder.yMax) {
			hitDistance = (cylinder.yMax - ray.Origin.y) / ray.Direction.y;
			payload.WorldNormal = glm::vec3(0.0f, 1.0f, 0.0f);
			rayTouchesTheSide = false;
		}
		else {
			return payload;
		}
	}
	else if (hitPoint.y < cylinder.yMin) {
		float t1 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		glm::vec3 hitPointAtT1 = origin + ray.Direction * t1;
		// we reached the bottom 
		if (hitPointAtT1.y > cylinder.yMin) {
			hitDistance = (cylinder.yMin - ray.Origin.y) / ray.Direction.y;
			payload.WorldNormal = glm::vec3(0.0f, -1.0f, 0.0f);
			rayTouchesTheSide = false;
		}
		else {
			return payload;
		}
	}

	hitPoint = origin + ray.Direction * hitDistance;

	payload.HitDistance = hitDistance;

	if (rayTouchesTheSide)
		payload.WorldNormal = glm::normalize(glm::vec3(hitPoint.x, 0, hitPoint.z));
	payload.WorldPosition = hitPoint + cylinder.Position;

	return payload;
}

Renderer::HitPayload Renderer::findRayParameterFromHit(const Ray& ray, const Sphere& sphere, size_t objectIndex) {
	// ray.Origin represents world coordinate system. To find a hit we need to convert ray coordinates to object coordinate
	// origin is the ray origin but in object coordinate system
	glm::vec3 origin = ray.Origin - sphere.Position;

	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(origin, ray.Direction);
	float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

	float discriminant = b * b - 4.0f * a * c;
	float hitDistance = discriminant >= 0.0f ? (-b - glm::sqrt(discriminant)) / (2.0f * a) : -1.0f;

	Renderer::HitPayload payload;
	if (hitDistance < 0) {
		Renderer::HitPayload payload;
		payload.HitDistance = hitDistance;
		return payload;
	}

	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;

	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;
	payload.WorldNormal = glm::normalize(hitPoint);
	payload.WorldPosition = hitPoint + sphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) {
	Shape* shape = m_ActiveScene->shapes[objectIndex];
	// origin in object coornidate system
	glm::vec3 origin = ray.Origin - shape->Position;

	// hit point in sphere coornidate system
	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;

	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	payload.WorldNormal = glm::normalize(hitPoint);
	payload.WorldPosition = hitPoint + shape->Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray) {
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}