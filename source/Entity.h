#pragma once

#include "Components.h"

#include <memory>
#include <string>

//  /+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\
// | Entity is object that mainly is a container for components (data).            |
// | * For each entity components are alocated as pointer to heap alocated data,   |
// |   the value will be nullptr if one is not yet alocated.                       |
// |                                                                               |
// | ^ I know that this is very very very very very very bad idea, but I don't     |
// |   have enough knowledge yet to implement this the better way.                 |
//  \+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/

class Entity
{
  friend class EntityManager;

private:
  Entity(unsigned int id, const std::string& tag);

  bool m_active;

  unsigned int m_id;
  std::string m_tag;

public:
  std::shared_ptr<CTransform> transform;
  std::shared_ptr<CSprite> sprite;
  std::shared_ptr<CText> text;
  std::shared_ptr<CRectangleCollider> rectangle_collider;
  std::shared_ptr<CCircleCollider> circle_collider;
  std::shared_ptr<CScore> score;
  std::shared_ptr<CLifeSpan> life_span;
  std::shared_ptr<CInput> input;

  bool isActive();

  unsigned int getID() const;
  const std::string& getTag() const;

  void destroy();
};
