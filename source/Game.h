#pragma once

#include "DebugOverlay.h"
#include "Entity.h"
#include "EntityManager.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <string>

enum class GameState
{
  MAIN_MENU,
  PAUSE,
  PLAY,
  WIN,
  LOSE
};

//  /+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\
// | Game is implemented in single class to be run.                                |
// | * Every systems and playing with entities data are implemented in functions.  |
// |                                                                               |
// | ^ This, this is where everything being utilized and where the whole game is   |
// |   implemented.                                                                |
//  \+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/

class Game
{
private:
  std::string m_title;

  sf::Clock m_delta_clock;
  sf::Clock m_global_clock;
  float m_delta_time;
  float m_current_time;

  bool m_running;

  GameState m_state;

  sf::Vector2u m_resolution;
  sf::Vector2u m_min_resolution;
  sf::RenderWindow m_render_window;
  sf::RenderTexture m_render_texture;
  sf::View m_view;
  bool m_fullscreen;

  DebugOverlay debug;

  sf::Font m_game_fonts[1];
  sf::Texture m_banner_textures[4];
  sf::Texture m_player_textures[1];
  sf::Texture m_bullet_textures[1];
  sf::Texture m_enemy_textures[3];

  EntityManager m_entity_manager;

  sf::Vector2f m_mouse_position;

  std::shared_ptr<Entity> m_player;
  sf::Vector2f m_player_position_default;
  bool m_player_collided;

  float m_points;

  void init();
  void update();
  void reset();

  void loadMainMenu();
  void loadPause();
  void loadWin();
  void loadLose();
  void spawnPlayer();
  void spawnBullet();
  void spawnEnemy();
  void spawnSmallEnemies(std::shared_ptr<Entity> enemy);

  void handleMainMenu();
  void handlePause();
  void handleEndGame();
  void handleMovement();
  void handlePlayerShooting(float delay);
  void handleEnemySpawnTime(float interval, int max_enemies);
  void handleCollision();
  void handleTransform();
  void handleLifeSpan();
  void handleEvent();
  void handleRendering();

public:
  Game(const std::string& title, sf::Vector2u resolution, bool full_screen);

  void run();
};
