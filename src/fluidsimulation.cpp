#include "fluidsimulation.hpp"
#include "SPHKernels.hpp"

void FluidSimulation::calculateDensities() {
#pragma omp parallel for schedule(static)
  for (std::size_t i = 0; i < particles.size(); ++i) {
    Particle &p = particles[i];
    p.density = 0.0f;

    spatialGrid.forEachNearby(p.position, [&](std::size_t j) {
      const Particle &other = particles[j];
      const Vector2 delta = Vector2Subtract(p.position, other.position);
      const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
      if (distanceSquared < smoothingRadius * smoothingRadius) {
        p.density +=
            other.mass * SPHKernels::poly6(distanceSquared, smoothingRadius);
      }
    });
  }
}

void FluidSimulation::calculatePressures() {
  for (Particle &p : particles) {
    p.pressure = pressureStiffness * (p.density - targetDensity);

    // Prevent negative pressure for now.
    p.pressure = std::max(p.pressure, 0.0f);
  }
}

void FluidSimulation::calculatePressureForces() {
#pragma omp parallel for schedule(static)
  // Start with gravity for every particle.
  for (std::size_t i = 0; i < particles.size(); ++i) {
    particles[i].acceleration = gravity;
  }

#pragma omp parallel for schedule(static)
  for (std::size_t i = 0; i < particles.size(); ++i) {
    Particle &p = particles[i];

    spatialGrid.forEachNearby(p.position, [&](std::size_t j) {
      const Particle &other = particles[j];

      if (&p == &other) {
        return;
      }

      Vector2 delta = Vector2Subtract(p.position, other.position);

      Vector2 gradient = SPHKernels::spikyGradient(delta, smoothingRadius);

      const float densityI = std::max(p.density, 1e-6f);
      const float densityJ = std::max(other.density, 1e-6f);

      const float pressureTerm = p.pressure / (densityI * densityI) +
                                 other.pressure / (densityJ * densityJ);

      // Pressure acceleration contribution.
      p.acceleration.x -= other.mass * pressureTerm * gradient.x;

      p.acceleration.y -= other.mass * pressureTerm * gradient.y;
    });
  }
}

void FluidSimulation::calculateViscosityForces() {
#pragma omp parallel for schedule(static)
  for (std::size_t i = 0; i < particles.size(); ++i) {
    Particle &p = particles[i];
    const float densityI = std::max(p.density, 1e-6f);

    spatialGrid.forEachNearby(p.position, [&](std::size_t j) {
      const Particle &other = particles[j];

      if (&p == &other) {
        return;
      }

      Vector2 delta = Vector2Subtract(p.position, other.position);

      float distanceSquared = delta.x * delta.x + delta.y * delta.y;

      if (distanceSquared >= smoothingRadius * smoothingRadius) {
        return;
      }

      float distance = std::sqrt(distanceSquared);

      float laplacian =
          SPHKernels::viscosityLaplacian(distance, smoothingRadius);

      const float densityJ = std::max(other.density, 1e-6f);

      const float scale = viscosity * other.mass * laplacian / densityJ;

      p.acceleration.x += scale * (other.velocity.x - p.velocity.x);

      p.acceleration.y += scale * (other.velocity.y - p.velocity.y);
    });
  }
}

FluidSimulation::FluidSimulation(int particleCount, float boundaryWidth,
                                 float boundaryHeight) {
  particles.reserve(particleCount);
  width = boundaryWidth;
  height = boundaryHeight;

  const int columns = 80;
  const float spacing = particleRadius * 2.0f;

  for (int i = 0; i < particleCount; ++i) {
    Particle p;

    p.position = {50 + (i % columns) * spacing,
                  50 + ((float)i / columns) * spacing};

    particles.push_back(p);
  }
}

void FluidSimulation::update(double dt) {
  spatialGrid.configure(width, height, smoothingRadius);
  spatialGrid.rebuild(particles);

  calculateDensities();
  calculatePressures();
  calculatePressureForces();
  calculateViscosityForces();

  for (Particle &p : particles) {
    // Semi-implicit Euler integration.
    p.velocity.x += p.acceleration.x * dt;
    p.velocity.y += p.acceleration.y * dt;

    p.position.x += p.velocity.x * dt;
    p.position.y += p.velocity.y * dt;

    // Left and right walls.
    if (p.position.x < particleRadius) {
      p.position.x = particleRadius;
      p.velocity.x *= -0.7f;
    }

    if (p.position.x > width - particleRadius) {
      p.position.x = width - particleRadius;
      p.velocity.x *= -0.7f;
    }

    // Top and bottom walls.
    if (p.position.y < particleRadius) {
      p.position.y = particleRadius;
      p.velocity.y *= -0.7f;
    }

    if (p.position.y + particleRadius > height) {
      p.position.y = height - particleRadius;

      if (p.velocity.y > 0.0f) {
        p.velocity.y *= -0.8f;
      }
    }
  }
}

void FluidSimulation::render() const {
  if (showSmoothingRadius) {
    for (const Particle &p : particles) {
      const float normalized = Clamp(p.pressure / 60.0f, 0.0f, 1.0f);
      const Color color = ColorLerp(BLUE, RED, normalized);

      DrawCircleLines(static_cast<int>(p.position.x),
                      static_cast<int>(p.position.y), smoothingRadius,
                      Fade(color, 0.15f));
    }
  }

  DrawRectangleLines(0, 0, static_cast<int>(width), static_cast<int>(height),
                     WHITE);
}

void FluidSimulation::handleMouse(Vector2 mousePosition, bool isDragging) {
  mouseGrab = isDragging;

  if (!mouseGrab) {
    return;
  }

  for (Particle &p : particles) {
    Vector2 delta = {mousePosition.x - p.position.x,
                     mousePosition.y - p.position.y};

    float distanceSquared = delta.x * delta.x + delta.y * delta.y;

    if (distanceSquared >= mouseRadius * mouseRadius ||
        distanceSquared < 0.001f) {
      continue;
    }

    float distance = sqrtf(distanceSquared);

    // Strongest near the mouse, fading toward the edge.
    float falloff = 1.0f - distance / mouseRadius;

    float force = mouseStrength * falloff;

    p.velocity.x += (delta.x / distance) * force * GetFrameTime();
    p.velocity.y += (delta.y / distance) * force * GetFrameTime();
  }
}

// Setters -- Getters for UI -------------------------------------------------
float FluidSimulation::getGravity() const { return gravity.y; }

void FluidSimulation::setGravity(float value) { gravity.y = value; }

float FluidSimulation::getTargetDensity() const { return targetDensity; }

void FluidSimulation::setTargetDensity(float value) { targetDensity = value; }

float FluidSimulation::getPressureStiffness() const {
  return pressureStiffness;
}

void FluidSimulation::setPressureStiffness(float value) {
  pressureStiffness = value;
}

float FluidSimulation::getSmoothingRadius() const { return smoothingRadius; }

void FluidSimulation::setSmoothingRadius(float value) {
  smoothingRadius = value;
}

float FluidSimulation::getViscosity() const { return viscosity; }

void FluidSimulation::setViscosity(float value) { viscosity = value; }

float FluidSimulation::getMouseRadius() const { return mouseRadius; }
void FluidSimulation::setMouseRadius(float value) { mouseRadius = value; }

float FluidSimulation::getMouseStrength() const { return mouseStrength; }
void FluidSimulation::setMouseStrength(float value) { mouseStrength = value; }

bool FluidSimulation::getShowSmoothingRadius() const {
  return showSmoothingRadius;
}
void FluidSimulation::setShowSmoothingRadius(bool value) {
  showSmoothingRadius = value;
}

bool FluidSimulation::getShowParticles() const { return showParticles; }
void FluidSimulation::setShowParticles(bool value) { showParticles = value; }

Color FluidSimulation::getMinPressureColor() const { return minPressureColor; }
void FluidSimulation::setMinPressureColor(Color value) {
  minPressureColor = value;
}

Color FluidSimulation::getMaxPressureColor() const { return maxPressureColor; }
void FluidSimulation::setMaxPressureColor(Color value) {
  maxPressureColor = value;
}

float FluidSimulation::getPressureDivisor() const { return pressureDivisor; }
void FluidSimulation::setPressureDivisor(float value) {
  pressureDivisor = value;
}

float FluidSimulation::getParticleRadius() const { return particleRadius; }

// ----------------------------------------------------------------------------
