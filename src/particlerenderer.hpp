#pragma once

#include "particle.hpp"
#include <raylib.h>
#include <vector>

class ParticleRenderer {
public:
  void draw(const std::vector<Particle> &particles, float width, float height,
            float radius);
};
