#pragma once

#include <cmath>
#include <raylib.h>

namespace SPHKernels {

inline float poly6(float distanceSquared, float h) {
  if (distanceSquared < 0.0f || distanceSquared >= h * h)
    return 0.0f;

  const float h2 = h * h;
  const float h8 = h2 * h2 * h2 * h2;

  const float difference = h2 - distanceSquared;

  return (4.0f / (PI * h8)) * difference * difference * difference;
}

inline Vector2 spikyGradient(Vector2 delta, float h) {
  const float distanceSquared = delta.x * delta.x + delta.y * delta.y;

  const float h2 = h * h;

  if (distanceSquared <= 1e-12f || distanceSquared >= h2) {
    return {0.0f, 0.0f};
  }

  const float distance = std::sqrt(distanceSquared);
  const float difference = h - distance;

  // Gradient of the normalized 2D Spiky kernel.
  const float scale =
      -30.0f * difference * difference / (PI * std::pow(h, 5.0f) * distance);

  return {delta.x * scale, delta.y * scale};
}

inline float viscosityLaplacian(float distance, float h) {
  if (distance < 0.0f || distance >= h) {
    return 0.0f;
  }

  return (40.0f / (PI * std::pow(h, 5.0f))) * (h - distance);
}

} // namespace SPHKernels
