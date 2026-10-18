#pragma once

#include "particle.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

class SpatialGrid {
public:
  void configure(Vector2 origin, float width, float height, float cellSize) {
    cellSize = std::max(cellSize, 1.0f);

    const int newColumns =
        std::max(1, static_cast<int>(std::ceil(width / cellSize)));
    const int newRows =
        std::max(1, static_cast<int>(std::ceil(height / cellSize)));

    if (newColumns != columns || newRows != rows ||
        cellSize != this->cellSize || origin.x != this->origin.x ||
        origin.y != this->origin.y) {
      this->cellSize = cellSize;
      this->origin = origin;
      columns = newColumns;
      rows = newRows;
      cells.clear();
      cells.resize(static_cast<std::size_t>(columns) * rows);
    }
  }

  void rebuild(const std::vector<Particle> &particles) {
    for (auto &cell : cells) {
      cell.clear();
    }

    for (std::size_t i = 0; i < particles.size(); ++i) {
      const int x = cellX(particles[i].position.x);
      const int y = cellY(particles[i].position.y);

      cells[index(x, y)].push_back(i);
    }
  }

  template <typename Function>
  void forEachNearby(Vector2 position, Function &&function) const {
    const int centerX = cellX(position.x);
    const int centerY = cellY(position.y);

    for (int y = std::max(0, centerY - 1); y <= std::min(rows - 1, centerY + 1);
         ++y) {
      for (int x = std::max(0, centerX - 1);
           x <= std::min(columns - 1, centerX + 1); ++x) {
        for (std::size_t i : cells[index(x, y)]) {
          function(i);
        }
      }
    }
  }

private:
  float cellSize = 20.0f;
  int columns = 0;
  int rows = 0;
  std::vector<std::vector<std::size_t>> cells;
  Vector2 origin = {0.0f, 0.0f};

  int cellX(float x) const {
    return std::clamp(static_cast<int>(std::floor((x - origin.x) / cellSize)),
                      0, columns - 1);
  }

  int cellY(float y) const {
    return std::clamp(static_cast<int>(std::floor((y - origin.y) / cellSize)),
                      0, rows - 1);
  }

  std::size_t index(int x, int y) const {
    return static_cast<std::size_t>(y) * columns + x;
  }
};
