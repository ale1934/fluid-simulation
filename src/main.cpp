#include "../external/rlImGui/rlImGui.h"
#include "../external/rlImGui/rlImGuiColors.h"
#include <imgui.h>

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

  omp_set_num_threads(8);

  ParticleRenderer particleRenderer;

  rlImGuiSetup(true);

  FluidSimulation simulation(7000, screenWidth, screenHeight + 20);

  bool showUI = false;
  bool paused = false;

  bool showFps = false;

  while (!WindowShouldClose()) {
    // Toggle the UI with Tab.
    if (IsKeyPressed(KEY_TAB)) {
      showUI = !showUI;
    }

    rlImGuiBegin();

    const bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;

    // Update the simulation unless paused.
    if (!paused) {
      float dt = std::min(GetFrameTime(), 1.0f / GetFPS());

      if (!imguiWantsMouse) {
        Vector2 mousePosition = GetMousePosition();

        // Around the simulation step:
        simulation.handleMouse(mousePosition,
                               IsMouseButtonDown(MOUSE_BUTTON_LEFT));

      } else {
        simulation.handleMouse(GetMousePosition(), false);
      }

      simulation.update(dt);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if (simulation.getShowParticles()) {
      particleRenderer.draw(simulation.getParticles(),
                            static_cast<float>(screenWidth),
                            static_cast<float>(screenHeight),
                            simulation.getParticleRadius(), simulation);
    }

    simulation.render();

    if (showFps)
      DrawFPS(10, 10);

    if (showUI) {
      ImGui::Begin("SPH Simulation Controls", &showUI);

      ImGui::Text("Simulation");
      ImGui::Separator();

      ImGui::Checkbox("Pause simulation", &paused);

      ImGui::Spacing();
      ImGui::Text("Physics Parameters");

      float gravity = simulation.getGravity();
      if (ImGui::SliderFloat("Gravity", &gravity, 0.0f, 200.0f)) {
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

      float pressureDivisor = simulation.getPressureDivisor();
      if (ImGui::SliderFloat("Pressure Divisor", &pressureDivisor, 50.f,
                             300.0f)) {
        simulation.setPressureDivisor(pressureDivisor);
      }

      bool showSmoothingRadius = simulation.getShowSmoothingRadius();
      if (ImGui::Checkbox("Show Smoothing Radius", &showSmoothingRadius)) {
        simulation.setShowSmoothingRadius(showSmoothingRadius);
      }

      bool showParticles = simulation.getShowParticles();
      if (ImGui::Checkbox("Show Particles", &showParticles)) {
        simulation.setShowParticles(showParticles);
      }

      ImGui::Checkbox("Show Fps", &showFps);

      auto editColor = [](const char *label, Color color, auto setter) {
        ImVec4 c = rlImGuiColors::Convert(color);
        float col[4] = {c.x, c.y, c.z, c.w};

        if (ImGui::ColorEdit4(label, col)) {
          setter(Color{static_cast<unsigned char>(col[0] * 255.0f),
                       static_cast<unsigned char>(col[1] * 255.0f),
                       static_cast<unsigned char>(col[2] * 255.0f),
                       static_cast<unsigned char>(col[3] * 255.0f)});
        }
      };

      editColor("Minimum Pressure Color", simulation.getMinPressureColor(),
                [&](Color c) { simulation.setMinPressureColor(c); });

      editColor("Maximum Pressure Color", simulation.getMaxPressureColor(),
                [&](Color c) { simulation.setMaxPressureColor(c); });

      ImGui::End();
    }

    rlImGuiEnd();

    EndDrawing();
  }

  rlImGuiShutdown();
  CloseWindow();

  return 0;
}
