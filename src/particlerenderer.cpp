#include "particlerenderer.hpp"

void ParticleRenderer::draw(const std::vector<Particle> &particles, float width,
                            float height, float radius, FluidSimulation sim) {
  if (particles.empty())
    return;

  for (const Particle &p : particles) {
    const float normalized =
        Clamp(p.pressure / sim.getPressureDivisor(), 0.0f, 1.0f);
    const Color color = ColorLerp(sim.getMinPressureColor(),
                                  sim.getMaxPressureColor(), normalized);
    DrawCircleV(p.position, radius, color);
  }
}
