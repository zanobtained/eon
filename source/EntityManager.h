#pragma once

#include "Entity.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

using EntityCollection = std::vector<std::shared_ptr<Entity>>;
using TaggedEntityCollection = std::map<std::string, EntityCollection>;

//  /+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\
// | Entity manager responsible of determining wether entity is on game or not.    |
// | * Entities stored in vector and map, stored as copy of shared pointer to both |
// |   container.                                                                  |
// |                                                                               |
// | ^ This is bad idea too and absolutely waste of performance, but I'm just too  |
// |   dumb to understand more complex stuff.                                      |
//  \+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/

class EntityManager
{
private:
  EntityCollection m_entities;
  EntityCollection m_entities_to_add;
  TaggedEntityCollection m_tagged_entities;

  unsigned int m_total_entities;

  void removeInactiveEntities(EntityCollection& entities);

public:
  EntityManager();

  const EntityCollection& getEntities() const;
  const EntityCollection& getEntities(const std::string& tag) const;

  std::shared_ptr<Entity> makeEntity(const std::string& tag);

  void update();
};
