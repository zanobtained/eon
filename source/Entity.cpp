#include "Entity.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <string>

// +===============================================================================+
// | Construct object with id and tag.                                             |
// +===============================================================================+
Entity::Entity(unsigned int id, const std::string& tag)
  : m_active(true)
  , m_id(id)
  , m_tag(tag)
{}

// +===============================================================================+
// | Checks if entity is alive or not.                                             |
// +===============================================================================+
bool Entity::isActive()
{
  return m_active;
}


// +===============================================================================+
// | Get entity ID.                                                                |
// |                                                                               |
// | ^ I don't even think this will ever be used somewhere.                        |
// +===============================================================================+
unsigned int Entity::getID() const
{
  return m_id;
}

// +===============================================================================+
// | Get enitity tag.                                                              |
// +===============================================================================+
const std::string& Entity::getTag() const
{
  return m_tag;
}


// +===============================================================================+
// | Remove entity (set to inactive).                                              |
// |                                                                               |
// | ^ Entity manager will later handle this.                                      |
// +===============================================================================+
void Entity::destroy()
{
  m_active = false;
}
