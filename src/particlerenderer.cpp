#include "particlerenderer.hpp"

void ParticleRenderer::draw(const std::vector<Particle> &particles, float width,
                            float height, float radius) {
  if (particles.empty())
    return;

  for (const Particle &p : particles) {
    const float normalized = Clamp(p.pressure / 70.0f, 0.0f, 1.0f);
    const Color color = ColorLerp(BLUE, RED, normalized);
    DrawCircleV(p.position, radius, color);
  }
}
