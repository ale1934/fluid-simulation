#pragma once

#include <raylib.h>
#include <raymath.h>

struct Particle {
  Vector2 position = {0.0f, 0.0f};
  Vector2 velocity = {0.0f, 0.0f};
  Vector2 acceleration = {0.0f, 0.0f};

  float density = 0.0f;
  float pressure = 0.0f;

  float mass = 1.0f;
};
