#include "../external/rlImGui/rlImGui.h"
#include "../external/rlImGui/rlImGuiColors.h"
#include <imgui.h>

#include "particlerenderer.hpp"
#include <algorithm>
#include <omp.h>
#include <raylib.h>

int main() {
  const int screenWidth = 1000;
  const int screenHeight = 640;

  InitWindow(screenWidth, screenHeight, "Particle Simulation");
  SetExitKey(KEY_ESCAPE);
  SetTargetFPS(0);

  omp_set_num_threads(8);

  ParticleRenderer particleRenderer;

  rlImGuiSetup(true);

  std::vector<FluidSimulation> simulations;

  FluidSimulation *selectedSimulation = nullptr;
  int selectedSimulationIndex = -1;

  bool showUI = false;
  bool showCreateSim = false;

  Vector2 createSimPosition = {0.0f, 0.0f};

  int createWidth = 300;
  int createHeight = 300;
  int createMaxParticles = 800;

  bool showFps = false;

  while (!WindowShouldClose()) {
    // Toggle the UI with Tab.
    if (IsKeyPressed(KEY_TAB)) {
      showUI = !showUI;
    }

    rlImGuiBegin();

    const bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;

    // Update the simulation unless paused.
    float dt = std::min(GetFrameTime(), 1.0f / GetFPS());

    if (!imguiWantsMouse) {
      Vector2 mousePosition = GetMousePosition();

      // Right-click once to select a simulation or open the create menu.
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        bool clickedSimulation = false;

        for (size_t i = 0; i < simulations.size(); ++i) {
          if (simulations[i].checkIfMouseOnSim(mousePosition)) {
            selectedSimulation = &simulations[i];
            selectedSimulationIndex = static_cast<int>(i);
            clickedSimulation = true;
            showCreateSim = false;
            break;
          }
        }

        if (!clickedSimulation) {
          createSimPosition = mousePosition;
          showCreateSim = true;
        }
      }

      for (auto &s : simulations) {
        s.handleMouse(mousePosition, IsMouseButtonDown(MOUSE_BUTTON_LEFT),
                      IsMouseButtonDown(MOUSE_BUTTON_MIDDLE));
      }
    } else {
      for (auto &s : simulations) {
        s.handleMouse(GetMousePosition(), false, false);
      }
    }

    for (auto &s : simulations)
      s.update(dt);

    BeginDrawing();
    ClearBackground(BLACK);

    for (auto &s : simulations) {
      if (s.getShowParticles()) {
        particleRenderer.draw(s.getParticles(), static_cast<float>(screenWidth),
                              static_cast<float>(screenHeight),
                              s.getParticleRadius(), s);
      }

      s.render();
    }

    if (showFps)
      DrawFPS(10, 10);

    if (showUI && selectedSimulation != nullptr) {
      ImGui::Begin("SPH Simulation Controls", &showUI);

      ImGui::Text("Simulation %d", selectedSimulationIndex);
      ImGui::Separator();

      bool paused = selectedSimulation->getPaused();
      if (ImGui::Checkbox("Pause simulation", &paused)) {
        selectedSimulation->setPaused(paused);
      }

      ImGui::Spacing();
      ImGui::Text("Physics Parameters");

      float gravity = selectedSimulation->getGravity();
      if (ImGui::SliderFloat("Gravity", &gravity, 0.0f, 200.0f)) {
        selectedSimulation->setGravity(gravity);
      }

      float targetDensity = selectedSimulation->getTargetDensity();
      if (ImGui::SliderFloat("Target Density", &targetDensity, 0.001f, 0.15f,
                             "%.4f")) {
        selectedSimulation->setTargetDensity(targetDensity);
      }

      float stiffness = selectedSimulation->getPressureStiffness();
      if (ImGui::SliderFloat("Pressure Stiffness", &stiffness, 0.0f,
                             10000.0f)) {
        selectedSimulation->setPressureStiffness(stiffness);
      }

      float smoothingRadius = selectedSimulation->getSmoothingRadius();
      if (ImGui::SliderFloat("Smoothing Radius", &smoothingRadius, 5.0f,
                             50.0f)) {
        selectedSimulation->setSmoothingRadius(smoothingRadius);
      }

      float viscosity = selectedSimulation->getViscosity();
      if (ImGui::SliderFloat("Viscosity", &viscosity, 0.0f, 1000.0f, "%.1f")) {
        selectedSimulation->setViscosity(viscosity);
      }

      float radius = selectedSimulation->getMouseRadius();
      if (ImGui::SliderFloat("Mouse Radius", &radius, 20.0f, 250.0f)) {
        selectedSimulation->setMouseRadius(radius);
      }

      float strength = selectedSimulation->getMouseStrength();
      if (ImGui::SliderFloat("Mouse Strength", &strength, 100.0f, 5000.0f)) {
        selectedSimulation->setMouseStrength(strength);
      }

      float pressureDivisor = selectedSimulation->getPressureDivisor();
      if (ImGui::SliderFloat("Pressure Divisor", &pressureDivisor, 50.f,
                             300.0f)) {
        selectedSimulation->setPressureDivisor(pressureDivisor);
      }

      bool showSmoothingRadius = selectedSimulation->getShowSmoothingRadius();
      if (ImGui::Checkbox("Show Smoothing Radius", &showSmoothingRadius)) {
        selectedSimulation->setShowSmoothingRadius(showSmoothingRadius);
      }

      bool showParticles = selectedSimulation->getShowParticles();
      if (ImGui::Checkbox("Show Particles", &showParticles)) {
        selectedSimulation->setShowParticles(showParticles);
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

      editColor("Minimum Pressure Color",
                selectedSimulation->getMinPressureColor(),
                [&](Color c) { selectedSimulation->setMinPressureColor(c); });

      editColor("Maximum Pressure Color",
                selectedSimulation->getMaxPressureColor(),
                [&](Color c) { selectedSimulation->setMaxPressureColor(c); });

      ImGui::Spacing();
      ImGui::Separator();

      if (ImGui::Button("Delete Simulation", ImVec2(-1.0f, 0.0f))) {
        simulations.erase(simulations.begin() + selectedSimulationIndex);

        selectedSimulation = nullptr;
        selectedSimulationIndex = -1;
        showUI = false;
      }

      ImGui::End();
    } else if (showUI && selectedSimulation == nullptr) {
      ImGui::Begin("SPH Simulation Controls", &showUI);

      ImGui::Text("No Simulation Selected!");
      ImGui::End();
    }

    if (showCreateSim) {
      ImGui::SetNextWindowPos({createSimPosition.x, createSimPosition.y},
                              ImGuiCond_Appearing);

      ImGui::SetNextWindowSize({260.0f, 0.0f}, ImGuiCond_Appearing);

      if (ImGui::Begin("Create Simulation", &showCreateSim)) {
        ImGui::InputInt("Width", &createWidth);
        ImGui::InputInt("Height", &createHeight);
        ImGui::InputInt("Max Particles", &createMaxParticles);

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(-1.0f, 0.0f))) {
          if (createWidth > 0 && createHeight > 0 && createMaxParticles > 0) {

            simulations.emplace_back(createMaxParticles, createSimPosition,
                                     static_cast<float>(createWidth),
                                     static_cast<float>(createHeight));

            // Adding to the vector can invalidate existing pointers.
            selectedSimulationIndex = static_cast<int>(simulations.size()) - 1;
            selectedSimulation = &simulations.back();

            showUI = true;
            showCreateSim = false;
          }
        }

        ImGui::End();
      } else {
        ImGui::End();
      }
    }

    rlImGuiEnd();

    EndDrawing();
  }

  rlImGuiShutdown();
  CloseWindow();

  return 0;
}
