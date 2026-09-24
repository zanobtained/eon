#include "Components.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

// +===============================================================================+
// | Represent spatial properties of entity including position, rotation,          |
// | direction, scale, and speed of the entity.                                    |
// +===============================================================================+
CTransform::CTransform(sf::Vector2f position)
  : position(position)
  , rotation(sf::degrees(0.0f))
  , direction(sf::Vector2f{0.0f, 0.0f})
  , scale(sf::Vector2f{1.0f, 1.0f})
  , speed(0.0f)
{}

// +===============================================================================+
// | Represent visual of entity as an image to be rendered.                        |
// +===============================================================================+
CSprite::CSprite(const sf::Texture& texture)
  : visual(texture)
{}

// +===============================================================================+
// | Represent visual of entity as text to be rendered.                            |
// +===============================================================================+
CText::CText(const sf::Font& font)
  : visual(font)
{}

// +===============================================================================+
// | Represent collideable area of entity in rectangle.                            |
// +===============================================================================+
CRectangleCollider::CRectangleCollider(sf::Vector2f size)
  : bounds(size)
{}

// +===============================================================================+
// | Represent collideable area of entity in circle.                               |
// +===============================================================================+
CCircleCollider::CCircleCollider(float radius)
  : bounds(radius)
{}

// +===============================================================================+
// | Represent entity's game world life time.                                      |
// +===============================================================================+
CLifeSpan::CLifeSpan(float value)
  : total(value)
  , remaining(value)
{}

// +===============================================================================+
// | Represent entity's score value to be added to game score.                     |
// +===============================================================================+
CScore::CScore(int value)
  : amount(value)
{}

// +===============================================================================+
// | Represent booleans state of user input for an entity.                         |
// +===============================================================================+
CInput::CInput()
  : left(false)
  , right(false)
  , up(false)
  , down(false)
  , shoot(false)
{}
