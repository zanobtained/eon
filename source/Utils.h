#pragma once

#include "Entity.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <filesystem>
#include <memory>

//  /+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\
// | Utils are function to help with game logic.                                   |
// | * All utilities to be called are in a single file (this file).                |
// | * Functions are grouped based in what terms is it used for.                   |
// |                                                                               |
// | ^ I don't know if it's a good idea to put utility functions like this but     |
// |   this is the first time I did this.                                          |
//  \+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/

namespace utils
{
  namespace renderer
  {
    sf::View getLetterboxView(sf::Vector2u size, sf::View view);
  }

  namespace collider
  {
    bool checkPointVsRectangle(sf::Vector2f point, std::shared_ptr<Entity> entity);
    bool checkCircleVsCircle(std::shared_ptr<Entity> first_entity, std::shared_ptr<Entity> second_entity);
  }

  namespace calculator
  {
    sf::Angle angleBetween(sf::Vector2f from, sf::Vector2f to);
    sf::Vector2f directionBetween(sf::Vector2f from, sf::Vector2f to);
  }

  namespace locator
  {
    std::filesystem::path getAssetPath(const std::filesystem::path& file_path);
  }

  namespace generator
  {
    int generateRandomIndex(int min_range, int max_range);
    sf::Vector2f generateRandomDirection();
    sf::Vector2f generateRandomPosition(float x_min_range, float max_range,
                                        float y_min_range, float y_max_range,
                                        sf::Vector2f player_position, float player_range);
  }
}
