#include "../external/rlImGui/rlImGui.h"
#include <imgui.h>

#include "fluidsimulation.hpp"
#include "particlerenderer.hpp"
#include <algorithm>
#include <omp.h>
#include <raylib.h>

int main() {
  const int screenWidth = 780;
  const int screenHeight = 640;

  InitWindow(screenWidth, screenHeight, "Particle Simulation");
  SetExitKey(KEY_ESCAPE);
  SetTargetFPS(0);

  omp_set_num_threads(4);

  ParticleRenderer particleRenderer;

  rlImGuiSetup(true);

  FluidSimulation simulation(2000, screenWidth, screenHeight + 20);

  bool showUI = false;
  bool paused = false;

  double tUpdate = 0.0, tDraw = 0.0;

  TraceLog(LOG_INFO, "Max Threads: %d", omp_get_max_threads());

  while (!WindowShouldClose()) {
    // Toggle the UI with Tab.
    if (IsKeyPressed(KEY_TAB)) {
      showUI = !showUI;
    }

    double t0 = GetTime();

    // Update the simulation unless paused.
    if (!paused) {
      float dt = std::min(GetFrameTime(), 1.0f / GetFPS());

      Vector2 mousePosition = GetMousePosition();

      // Around the simulation step:
      simulation.handleMouse(mousePosition,
                             IsMouseButtonDown(MOUSE_BUTTON_LEFT));
      simulation.update(dt);
      tUpdate = GetTime() - t0;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    t0 = GetTime();
    if (simulation.getShowParticles()) {
      particleRenderer.draw(
          simulation.getParticles(), static_cast<float>(screenWidth),
          static_cast<float>(screenHeight), simulation.getParticleRadius());
    }
    tDraw = GetTime() - t0;

    TraceLog(LOG_INFO, "update: %.2f ms  draw: %.2f ms", tUpdate * 1000.0,
             tDraw * 1000.0);

    simulation.render();

    DrawFPS(10, 10);

    rlImGuiBegin();

    if (showUI) {
      ImGui::Begin("SPH Simulation Controls", &showUI);

      ImGui::Text("Simulation");
      ImGui::Separator();

      ImGui::Checkbox("Pause simulation", &paused);

      ImGui::Spacing();
      ImGui::Text("Physics Parameters");

      float gravity = simulation.getGravity();
      if (ImGui::SliderFloat("Gravity", &gravity, 0.0f, 2000.0f)) {
        simulation.setGravity(gravity);
      }

      float targetDensity = simulation.getTargetDensity();
      if (ImGui::SliderFloat("Target Density", &targetDensity, 0.001f, 0.15f,
                             "%.4f")) {
        simulation.setTargetDensity(targetDensity);
      }

      float stiffness = simulation.getPressureStiffness();
      if (ImGui::SliderFloat("Pressure Stiffness", &stiffness, 0.0f,
                             10000.0f)) {
        simulation.setPressureStiffness(stiffness);
      }

      float smoothingRadius = simulation.getSmoothingRadius();
      if (ImGui::SliderFloat("Smoothing Radius", &smoothingRadius, 5.0f,
                             50.0f)) {
        simulation.setSmoothingRadius(smoothingRadius);
      }

      float viscosity = simulation.getViscosity();
      if (ImGui::SliderFloat("Viscosity", &viscosity, 0.0f, 1000.0f, "%.1f")) {
        simulation.setViscosity(viscosity);
      }

      float radius = simulation.getMouseRadius();
      if (ImGui::SliderFloat("Mouse Radius", &radius, 20.0f, 250.0f)) {
        simulation.setMouseRadius(radius);
      }

      float strength = simulation.getMouseStrength();
      if (ImGui::SliderFloat("Mouse Strength", &strength, 100.0f, 5000.0f)) {
        simulation.setMouseStrength(strength);
      }

      bool showSmoothingRadius = simulation.getShowSmoothingRadius();
      if (ImGui::Checkbox("Show Smoothing Radius", &showSmoothingRadius)) {
        simulation.setShowSmoothingRadius(showSmoothingRadius);
      }

      bool showParticles = simulation.getShowParticles();
      if (ImGui::Checkbox("Show Particles", &showParticles)) {
        simulation.setShowParticles(showParticles);
      }

      ImGui::End();
    }

    rlImGuiEnd();

    EndDrawing();
  }

  rlImGuiShutdown();
  CloseWindow();

  return 0;
}
