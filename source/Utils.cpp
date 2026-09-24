#include "Utils.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cmath>
#include <filesystem>
#include <random>
#include <utility>

#if defined (_WIN32)
  #include <Windows.h>
#endif

#if defined (__linux__)
  #include <string>

  #include <limits.h>
  #include <unistd.h>
#endif

#if defined (__APPLE__)
  #include <cstdint>
  #include <memory>

  #include <mach-o/dyld.h>
#endif

// +===============================================================================+
// | Get the letterbox view from a render target.                                  |
// | * The ratio will always be constant, only scaling and positioning render      |
// |   target's view to always be centered.                                        |
// +===============================================================================+
sf::View utils::renderer::getLetterboxView(sf::Vector2u size, sf::View view)
{
  float size_ratio {static_cast<float>(size.x) / static_cast<float>(size.y)};
  float view_ratio {view.getSize().x / view.getSize().y};
  float width_scale {1.0f};
  float height_scale {1.0f};
  float x_position_map {0.0f};
  float y_position_map {0.0f};

  if (size_ratio < view_ratio)
  {
    height_scale = size_ratio / view_ratio;
    y_position_map = (1.0f - height_scale) / 2.0f;
  }
  else
  {
    width_scale = view_ratio / size_ratio;
    x_position_map = (1.0f - width_scale) / 2.0f;
  }

  view.setViewport(sf::FloatRect{sf::Vector2f{x_position_map, y_position_map},
                                 sf::Vector2f{width_scale, height_scale}});

  return view;
}

// +===============================================================================+
// | Checks if a point is colliding with a rectangle.                              |
// +===============================================================================+
bool utils::collider::checkPointVsRectangle(sf::Vector2f point, std::shared_ptr<Entity> entity)
{
  float rectangle_left {entity->rectangle_collider->bounds.getPosition().x - entity->rectangle_collider->bounds.getSize().x / 2};
  float rectangle_right {entity->rectangle_collider->bounds.getPosition().x + entity->rectangle_collider->bounds.getSize().x / 2};
  float rectangle_top {entity->rectangle_collider->bounds.getPosition().y - entity->rectangle_collider->bounds.getSize().y / 2};
  float rectangle_bottom {entity->rectangle_collider->bounds.getPosition().y + entity->rectangle_collider->bounds.getSize().y / 2};

  return (point.x >= rectangle_left &&
          point.x <= rectangle_right &&
          point.y >= rectangle_top &&
          point.y <= rectangle_bottom);
}

// +===============================================================================+
// | Checks if a circle and circle is colliding.                                   |
// +===============================================================================+
bool utils::collider::checkCircleVsCircle(std::shared_ptr<Entity> first_entity, std::shared_ptr<Entity> second_entity)
{
  const float x_distance {second_entity->transform->position.x - first_entity->transform->position.x};
  const float y_distance {second_entity->transform->position.y - first_entity->transform->position.y};
  const float squared_distance {(x_distance * x_distance) + (y_distance * y_distance)};

  const float first_entity_radius {first_entity->circle_collider->bounds.getRadius()};
  const float second_entity_radius {second_entity->circle_collider->bounds.getRadius()};
  const float squared_radius {(first_entity_radius + second_entity_radius) * (first_entity_radius + second_entity_radius)};

  return squared_distance <= squared_radius;
}

// +===============================================================================+
// | Returns angle between two positios.                                           |
// +===============================================================================+
sf::Angle utils::calculator::angleBetween(sf::Vector2f from, sf::Vector2f to)
{
  return (to - from).angle();
}

// +===============================================================================+
// | Returns direction between two positions.                                      |
// +===============================================================================+
sf::Vector2f utils::calculator::directionBetween(sf::Vector2f from, sf::Vector2f to)
{
  return (to - from).normalized();
}

// +===============================================================================+
// | Returns a path to game assets.                                                |
// | * This is platform specific because there's no way to detect the current      |
// |   executable path automatically.                                              |
// |                                                                               |
// | ^ I don't really know about the code because it makes sense to me and not at  |
// |   the same time.                                                              |
// | ^ The code taking reference from some random guy on the internet and a bit    |
// |   reading even though i don't really know what is going on under the hood.    |
// | ^ For MacOS, I don't know if it works or not, i have tested it on Windows and |
// |   Linux, both should work.                                                    |
// +===============================================================================+
std::filesystem::path utils::locator::getAssetPath(const std::filesystem::path& file_path)
{
  #if defined(_WIN32)
    wchar_t path[MAX_PATH] {0};
    DWORD len {GetModuleFileNameW(nullptr, path, MAX_PATH)};

    if (len == 0 || len == MAX_PATH) {
      return {};
    }

    return std::filesystem::path(path).parent_path() / ".." / "assets" / file_path;
  #endif

  #if defined(__linux__)
    char result[PATH_MAX] {0};
    ssize_t count {readlink("/proc/self/exe", result, PATH_MAX)};

    if (count <= 0) {
      return {};
    }

    return std::filesystem::path(std::string(result, count)).parent_path() / ".." / "assets" / file_path;
  #endif

  #if defined(__APPLE__)
    uint32_t size {0};
    _NSGetExecutablePath(nullptr, &size);
    std::unique_ptr<char[]> buffer {std::make_unique<char[]>(size)};

    if (_NSGetExecutablePath(buffer.get(), &size) != 0) {
        return {};
    }

    return std::filesystem::path(buffer.get()).parent_path() / ".." / "assets" / file_path;
  #endif
}

// +===============================================================================+
// | Generate random number (index) between specified range.                       |
// |                                                                               |
// | ^ Bad naming, I know.                                                         |
// | ^ Honestly, I don't know what the heck is going on in every RNG function, I   |
// |   found this when I implementing RNG in more C way using system clock and     |
// |   random function, but I found this on the internet they say it's better.     |
// +===============================================================================+
int utils::generator::generateRandomIndex(int min_range, int max_range)
{
  if (min_range > max_range)
  {
    std::swap(min_range, max_range);
  }

  static std::mt19937 generator {std::random_device{}()};
  std::uniform_int_distribution<int> distribution {min_range, max_range};

  return distribution(generator);
}

// +===============================================================================+
// | Generate random direction between 360 degrees (circular).                     |
// +===============================================================================+
sf::Vector2f utils::generator::generateRandomDirection()
{
  sf::Angle angle_range {sf::degrees(360)};

  static std::mt19937 generator {std::random_device{}()};
  std::uniform_real_distribution<float> angle_distribution {0.0f, angle_range.asRadians()};

  float angle {angle_distribution(generator)};
  float x {std::cos(angle)}; 
  float y {std::sin(angle)}; 

  return sf::Vector2f{x, y};
}

// +===============================================================================+
// | Generate random position in game world.                                       |
// |                                                                               |
// | ^ This is specific to this game and not generalized, because the enemy can't  |
// |   just spawn in player's position and broke the game.                         |
// +===============================================================================+
sf::Vector2f utils::generator::generateRandomPosition(float x_min_range, float x_max_range,
                                                      float y_min_range, float y_max_range,
                                                      sf::Vector2f player_position, float player_range)
{
  if (x_min_range > x_max_range)
  {
    std::swap(x_min_range, x_max_range);
  }

  if (y_min_range > y_max_range)
  {
    std::swap(y_min_range, y_max_range);
  }

  static std::mt19937 generator {std::random_device{}()};
  std::uniform_real_distribution<float> x_distribution {x_min_range, x_max_range};
  std::uniform_real_distribution<float> y_distribution {y_min_range, y_max_range};

  sf::Vector2f random_position {0.0f, 0.0f};
  int attempts {0};

  do
  {
    random_position.x = x_distribution(generator);
    random_position.y = y_distribution(generator);

    float x_distance {random_position.x - player_position.x};
    float y_distance {random_position.y - player_position.y};
    float squared_distance {(x_distance * x_distance) + (y_distance * y_distance)};

    if (squared_distance >= (player_range * player_range) || (++attempts > 100))
    {
      break;
    }
  }
  while (true);

  return random_position;
}
