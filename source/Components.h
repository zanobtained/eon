#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//  /+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\
// | Components are data to be attached to entity.                                 |
// | * All components to be attached are in a single file (this file).             |
// | * Every components represent general structure of data that can be allocated. |
//  \+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/

struct CTransform
{
  sf::Vector2f position;
  sf::Angle rotation;
  sf::Vector2f direction;
  sf::Vector2f scale;
  float speed;

  CTransform(sf::Vector2f position);
};

struct CSprite
{
  sf::Sprite visual;

  CSprite(const sf::Texture& texture);
};

struct CText
{
  sf::Text visual;

  CText(const sf::Font& font);
};

struct CRectangleCollider
{
  sf::RectangleShape bounds;

  CRectangleCollider(sf::Vector2f size);
};

struct CCircleCollider
{
  sf::CircleShape bounds;

  CCircleCollider(float radius);
};

struct CLifeSpan
{
  float total;
  float remaining;

  CLifeSpan(float value);
};

struct CScore
{
  int amount;

  CScore(int value);
};

struct CInput
{
  bool left;
  bool right;
  bool up;
  bool down;
  bool shoot;

  CInput();
};
