#include "particle.hpp"
#include "spatialgrid.hpp"
#include <vector>

class FluidSimulation {
private:
  std::vector<Particle> particles;
  Vector2 gravity = {0.0f, 0.0f};
  SpatialGrid spatialGrid;

  float width = 400.0f;
  float height = 400.0f;
  float particleRadius = 3.0f;
  float smoothingRadius = 15.0f;

  float targetDensity = 0.001f;
  float pressureStiffness = 6000.0f;

  float viscosity = 100.0f;

  float mouseRadius = 100.0f;
  float mouseStrength = 1500.0f;
  bool mouseGrab = false;

  bool showSmoothingRadius = false;
  bool showParticles = true;

  Color minPressureColor = BLUE;
  Color maxPressureColor = RED;

  float pressureDivisor = 50.0f;

  void calculateDensities();
  void calculatePressures();
  void calculatePressureForces();
  void calculateViscosityForces();

public:
  FluidSimulation(int particleCount, float boundaryWidth, float boundaryHeight);

  void update(double dt);
  void render() const;

  void handleMouse(Vector2 mousePosition, bool isDragging);

  // Parameters
  float getGravity() const;
  void setGravity(float value);

  float getTargetDensity() const;
  void setTargetDensity(float value);

  float getPressureStiffness() const;
  void setPressureStiffness(float value);

  float getSmoothingRadius() const;
  void setSmoothingRadius(float value);

  float getViscosity() const;
  void setViscosity(float value);

  float getMouseRadius() const;
  void setMouseRadius(float value);

  float getMouseStrength() const;
  void setMouseStrength(float value);

  bool getShowSmoothingRadius() const;
  void setShowSmoothingRadius(bool value);

  bool getShowParticles() const;
  void setShowParticles(bool value);

  Color getMinPressureColor() const;
  void setMinPressureColor(Color value);

  Color getMaxPressureColor() const;
  void setMaxPressureColor(Color value);

  float getPressureDivisor() const;
  void setPressureDivisor(float value);

  const std::vector<Particle> &getParticles() const { return particles; }

  float getParticleRadius() const;
};
