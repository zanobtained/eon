#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <unordered_map>
#include <vector>

//  /+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\
// | Debug overlay is object that shows in game data to be shown.                  |
// | * Class should be instantiated meaning there can be multiple debugger.        |
//  \+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/

class DebugOverlay
{
private:
  sf::Font m_font;

  std::unordered_map<std::string, sf::Text> m_text_entries;
  std::vector<std::string> m_key_order;

  sf::Vector2f m_draw_position;
  float m_text_gap;

public:
  DebugOverlay();

  void setDrawPosition(sf::Vector2f position);
  void setTextGap(float gap);

  void write(const std::string& key, const std::string& message);
  void draw(sf::RenderWindow& window);
};
