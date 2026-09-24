#include "Components.h"
#include "Entity.h"
#include "Game.h"
#include "Utils.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <cmath>
#include <iostream>
#include <optional>
#include <string>

// +===============================================================================+
// | Construct object window title, game resolution, fullscreen option.            |
// +===============================================================================+
Game::Game(const std::string& title, sf::Vector2u resolution, bool fullscreen)
  : m_title(title)
  , m_current_time(0.0f)
  , m_delta_time(0.0f)
  , m_running(true)
  , m_resolution(resolution)
  , m_min_resolution(sf::Vector2u{640, 480})
  , m_fullscreen(fullscreen)
{
  m_resolution.x < m_min_resolution.x ? m_resolution.x = m_min_resolution.x : m_resolution.x = resolution.x ;
  m_resolution.y < m_min_resolution.y ? m_resolution.y = m_min_resolution.y : m_resolution.y = resolution.y ;

  init();
}

// +===============================================================================+
// | Initialize necessary stuff.                                                   |
// +===============================================================================+
void Game::init()
{
  m_state = GameState::MAIN_MENU;

  sf::VideoMode current_desktop_mode {sf::VideoMode::getDesktopMode()};

  if (m_fullscreen)
  {
    sf::Vector2u fullscreen_size {current_desktop_mode.size};

    m_render_window.create(sf::VideoMode{fullscreen_size}, m_title, sf::Style::None, sf::State::Fullscreen);
    m_render_window.setPosition(sf::Vector2i{0, 0});
  }
  else
  {
    sf::Vector2u windowed_size {sf::Vector2u{current_desktop_mode.size.x / 2, current_desktop_mode.size.y / 2}};

    m_render_window.create(sf::VideoMode{windowed_size}, m_title, sf::Style::Default, sf::State::Windowed);
    m_render_window.setPosition(sf::Vector2i{static_cast<int>((current_desktop_mode.size.x / 2) - (windowed_size.x / 2)),
                                             static_cast<int>((current_desktop_mode.size.y / 2) - (windowed_size.y / 2))});
  }
  m_render_window.setFramerateLimit(60u);

  if (!m_render_texture.resize(m_resolution))
  {
    std::cerr << "Failed to resize render texture\n";
  }

  m_view.setSize(static_cast<sf::Vector2f>(m_render_texture.getSize()));
  m_view.setCenter(m_view.getSize() / 2.0f);
  m_view = utils::renderer::getLetterboxView(m_render_window.getSize(), m_view);

  debug.setDrawPosition(sf::Vector2f{5.0f, 5.0f});
  debug.setTextGap(15.0f);
  
  if (!m_game_fonts[0].openFromFile(utils::locator::getAssetPath("fonts/Dogica.ttf")))
  {
    std::cerr << "Failed to load Dogica.ttf";
    exit(1);
  }
  m_game_fonts[0].setSmooth(false);

  if (!m_banner_textures[0].loadFromFile(utils::locator::getAssetPath("textures/MainBanner.png")))
  {
    std::cerr << "Failed to load MainBanner.png\n";
    exit(1);
  }

  if (!m_banner_textures[1].loadFromFile(utils::locator::getAssetPath("textures/PauseBanner.png")))
  {
    std::cerr << "Failed to load PauseBanner.png\n";
    exit(1);
  }

  if (!m_banner_textures[2].loadFromFile(utils::locator::getAssetPath("textures/WinBanner.png")))
  {
    std::cerr << "Failed to load WinBanner.png\n";
    exit(1);
  }

  if (!m_banner_textures[3].loadFromFile(utils::locator::getAssetPath("textures/LoseBanner.png")))
  {
    std::cerr << "Failed to load LoseBanner.png\n";
    exit(1);
  }

  if (!m_player_textures[0].loadFromFile(utils::locator::getAssetPath("textures/Player.png")))
  {
    std::cerr << "Failed to load Player.png\n";
    exit(1);
  }

  if (!m_bullet_textures[0].loadFromFile(utils::locator::getAssetPath("textures/Bullet.png")))
  {
    std::cerr << "Failed to load Bullet.png\n";
    exit(1);
  }

  if (! m_enemy_textures[0].loadFromFile(utils::locator::getAssetPath("textures/Enemy1.png")))
  {
    std::cerr << "Failed to load Enemy1.png\n";
    exit(1);
  }

  if (! m_enemy_textures[1].loadFromFile(utils::locator::getAssetPath("textures/Enemy2.png")))
  {
    std::cerr << "Failed to load Enemy2.png\n";
    exit(1);
  }

  if (! m_enemy_textures[2].loadFromFile(utils::locator::getAssetPath("textures/Enemy3.png")))
  {
    std::cerr << "Failed to load Enemy3.png\n";
    exit(1);
  }

  m_mouse_position = m_render_window.mapPixelToCoords(sf::Mouse::getPosition(m_render_window), m_view);

  m_player_position_default = static_cast<sf::Vector2f>(m_render_texture.getSize()) / 2.0f;
  m_player_collided = false;

  m_points = 0.0f;

  loadMainMenu();
  loadPause();
  loadWin();
  loadLose();
  spawnPlayer();
}

// +===============================================================================+
// | Updates everything every frame                                                |
// |                                                                               |
// | ^ To be honest, I think it's bad, but I don't know what it is.                |
// +===============================================================================+
void Game::update()
{
  debug.write("points", std::to_string(m_points));
  debug.write("  posx", std::to_string(m_player->transform->position.x));
  debug.write("  posy", std::to_string(m_player->transform->position.y));
  debug.write("  time", std::to_string(m_current_time));
  debug.write("   fps", std::to_string(1.0f / m_delta_time));

  if (m_render_window.hasFocus())
  {
    handleMainMenu();
    handlePause();
    handleEndGame();
    handleMovement();
    handlePlayerShooting(0.3f);
    handleEnemySpawnTime(0.6f, 10);
    handleCollision();
    handleTransform();
    handleLifeSpan();
  }

  handleEvent();
  handleRendering();
}

// +===============================================================================+
// | Resets the game world to initial condition.                                   |
// +===============================================================================+
void Game::reset()
{
  m_points = 0.0f;

  for (auto entity : m_entity_manager.getEntities())
  {
    if (entity->getTag() == "Bullet" ||
        entity->getTag() == "Enemy" ||
        entity->getTag() == "SmallEnemy")
    {
      entity->destroy();
    }
  }

  m_player->transform->position = m_player_position_default;

  m_player->input->left = false;
  m_player->input->right = false;
  m_player->input->up = false;
  m_player->input->down = false;
  m_player->input->shoot = false;

}

// +===============================================================================+
// | Load every main menus object.                                                 |
// +===============================================================================+
void Game::loadMainMenu()
{
  sf::Vector2f mid_world {static_cast<float>(m_render_texture.getSize().x) / 2.0f,
                          static_cast<float>(m_render_texture.getSize().y) / 2.0f};

  auto main_banner {m_entity_manager.makeEntity("MainBanner")};

  main_banner->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y - 96.0f});

  main_banner->sprite = std::make_shared<CSprite>(m_banner_textures[0]);

  auto play_indicator {m_entity_manager.makeEntity("MainIndicator")};

  play_indicator->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y + 64.0f});

  play_indicator->text = std::make_shared<CText>(m_game_fonts[0]);
  play_indicator->text->visual.setString("Press ENTER to play!");
  play_indicator->text->visual.setCharacterSize(12u);
  play_indicator->text->visual.setLetterSpacing(0.5);

  play_indicator->rectangle_collider = std::make_shared<CRectangleCollider>(play_indicator->text->visual.getGlobalBounds().size);
  play_indicator->rectangle_collider->bounds.setFillColor(sf::Color::Transparent);
  play_indicator->rectangle_collider->bounds.setOutlineColor(sf::Color::Magenta);
  play_indicator->rectangle_collider->bounds.setOutlineThickness(1.0f);
}

// +===============================================================================+
// | Load every pause state object.                                                |
// +===============================================================================+
void Game::loadPause()
{
  sf::Vector2f mid_world {static_cast<float>(m_render_texture.getSize().x) / 2.0f,
                          static_cast<float>(m_render_texture.getSize().y) / 2.0f};

  auto pause_banner {m_entity_manager.makeEntity("PauseBanner")};

  pause_banner->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y - 32.0f});

  pause_banner->sprite = std::make_shared<CSprite>(m_banner_textures[1]);

  auto pause_indicator {m_entity_manager.makeEntity("PauseIndicator")};

  pause_indicator->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y + 64.0f});

  pause_indicator->text = std::make_shared<CText>(m_game_fonts[0]);
  pause_indicator->text->visual.setString("Press SPACE to resume!");
  pause_indicator->text->visual.setCharacterSize(12u);
  pause_indicator->text->visual.setLetterSpacing(0.5);

  pause_indicator->rectangle_collider = std::make_shared<CRectangleCollider>(pause_indicator->text->visual.getGlobalBounds().size);
  pause_indicator->rectangle_collider->bounds.setFillColor(sf::Color::Transparent);
  pause_indicator->rectangle_collider->bounds.setOutlineColor(sf::Color::Magenta);
  pause_indicator->rectangle_collider->bounds.setOutlineThickness(1.0f);
}

// +===============================================================================+
// | Load every win state object.                                                  |
// +===============================================================================+
void Game::loadWin()
{
  sf::Vector2f mid_world {static_cast<float>(m_render_texture.getSize().x) / 2.0f,
                          static_cast<float>(m_render_texture.getSize().y) / 2.0f};

  auto win_banner {m_entity_manager.makeEntity("WinBanner")};

  win_banner->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y - 32.0f});

  win_banner->sprite = std::make_shared<CSprite>(m_banner_textures[2]);

  auto win_indicator {m_entity_manager.makeEntity("WinIndicator")};

  win_indicator->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y + 64.0f});

  win_indicator->text = std::make_shared<CText>(m_game_fonts[0]);
  win_indicator->text->visual.setString("Press SPACE to continue!");
  win_indicator->text->visual.setCharacterSize(12u);
  win_indicator->text->visual.setLetterSpacing(0.5);

  win_indicator->rectangle_collider = std::make_shared<CRectangleCollider>(win_indicator->text->visual.getGlobalBounds().size);
  win_indicator->rectangle_collider->bounds.setFillColor(sf::Color::Transparent);
  win_indicator->rectangle_collider->bounds.setOutlineColor(sf::Color::Magenta);
  win_indicator->rectangle_collider->bounds.setOutlineThickness(1.0f);
}

// +===============================================================================+
// | Load every lose state object.                                                 |
// +===============================================================================+
void Game::loadLose()
{
  sf::Vector2f mid_world {static_cast<float>(m_render_texture.getSize().x) / 2.0f,
                          static_cast<float>(m_render_texture.getSize().y) / 2.0f};

  auto lose_banner {m_entity_manager.makeEntity("LoseBanner")};

  lose_banner->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y - 32.0f});

  lose_banner->sprite = std::make_shared<CSprite>(m_banner_textures[3]);

  auto lose_indicator {m_entity_manager.makeEntity("LoseIndicator")};

  lose_indicator->transform = std::make_shared<CTransform>(sf::Vector2f{mid_world.x,
                                                                     mid_world.y + 64.0f});

  lose_indicator->text = std::make_shared<CText>(m_game_fonts[0]);
  lose_indicator->text->visual.setString("Press SPACE to continue!");
  lose_indicator->text->visual.setCharacterSize(12u);
  lose_indicator->text->visual.setLetterSpacing(0.5);

  lose_indicator->rectangle_collider = std::make_shared<CRectangleCollider>(lose_indicator->text->visual.getGlobalBounds().size);
  lose_indicator->rectangle_collider->bounds.setFillColor(sf::Color::Transparent);
  lose_indicator->rectangle_collider->bounds.setOutlineColor(sf::Color::Magenta);
  lose_indicator->rectangle_collider->bounds.setOutlineThickness(1.0f);
}

// +===============================================================================+
// | Spawn player.                                                                 |
// +===============================================================================+
void Game::spawnPlayer()
{
  auto player {m_entity_manager.makeEntity("Player")};

  player->transform = std::make_shared<CTransform>(m_player_position_default);
  player->transform->speed = 200.0f;

  player->sprite = std::make_shared<CSprite>(m_player_textures[0]);

  player->circle_collider = std::make_shared<CCircleCollider>(5.0f);
  player->circle_collider->bounds.setFillColor(sf::Color::Transparent);
  player->circle_collider->bounds.setOutlineColor(sf::Color::Green);
  player->circle_collider->bounds.setOutlineThickness(1.0f);

  player->input = std::make_shared<CInput>();

  m_player = player;
}

// +===============================================================================+
// | Spawn bullet from player.                                                     |
// +===============================================================================+
void Game::spawnBullet()
{
  auto bullet {m_entity_manager.makeEntity("Bullet")};

  bullet->transform = std::make_shared<CTransform>(m_player->transform->position);
  bullet->transform->rotation = utils::calculator::angleBetween(bullet->transform->position, m_mouse_position);
  bullet->transform->direction = utils::calculator::directionBetween(bullet->transform->position, m_mouse_position);
  bullet->transform->speed = 500.0f;

  bullet->sprite = std::make_shared<CSprite>(m_bullet_textures[0]);

  bullet->circle_collider = std::make_shared<CCircleCollider>(1.0f);
  bullet->circle_collider->bounds.setFillColor(sf::Color::Transparent);
  bullet->circle_collider->bounds.setOutlineColor(sf::Color::Yellow);
  bullet->circle_collider->bounds.setOutlineThickness(1.0f);

  bullet->life_span = std::make_shared<CLifeSpan>(1.0f);
}

// +===============================================================================+
// | Spawn enemy in random position and type.                                      |
// +===============================================================================+
void Game::spawnEnemy()
{
  int random_type {utils::generator::generateRandomIndex(1, 3)};
  float x_min {96.0f};
  float x_max {m_render_texture.getSize().x - 96.0f};
  float y_min {96.0f};
  float y_max {m_render_texture.getSize().y - 96.0f};
  float player_range {96.0f};
  sf::Vector2f random_position {utils::generator::generateRandomPosition(x_min, x_max, y_min, y_max,
                                                                         m_player->transform->position,
                                                                         player_range)};
  sf::Vector2f random_direction {utils::generator::generateRandomDirection()};

  auto enemy {m_entity_manager.makeEntity("Enemy")};

  switch (random_type)
  {
    case 1:
    {
      enemy->transform = std::make_shared<CTransform>(random_position);
      enemy->transform->direction = random_direction;
      enemy->transform->speed = 70.0f;

      enemy->sprite = std::make_shared<CSprite>(m_enemy_textures[0]);

      enemy->circle_collider = std::make_shared<CCircleCollider>(20.0f);
      enemy->circle_collider->bounds.setFillColor(sf::Color::Transparent);
      enemy->circle_collider->bounds.setOutlineColor(sf::Color::Red);
      enemy->circle_collider->bounds.setOutlineThickness(1.0f);
      
      enemy->score = std::make_shared<CScore>(5.0f);
    }
    break;

    case 2:
    {
      enemy->transform = std::make_shared<CTransform>(random_position);
      enemy->transform->direction = random_direction;
      enemy->transform->speed = 60.0f;

      enemy->sprite = std::make_shared<CSprite>(m_enemy_textures[1]);

      enemy->circle_collider = std::make_shared<CCircleCollider>(28.0f);
      enemy->circle_collider->bounds.setFillColor(sf::Color::Transparent);
      enemy->circle_collider->bounds.setOutlineColor(sf::Color::Red);
      enemy->circle_collider->bounds.setOutlineThickness(1.0f);

      enemy->score = std::make_shared<CScore>(10.0f);
    }
    break;

    case 3:
    {
      enemy->transform = std::make_shared<CTransform>(random_position);
      enemy->transform->direction = random_direction;
      enemy->transform->speed = 50.0f;

      enemy->sprite = std::make_shared<CSprite>(m_enemy_textures[2]);

      enemy->circle_collider = std::make_shared<CCircleCollider>(36.0f);
      enemy->circle_collider->bounds.setFillColor(sf::Color::Transparent);
      enemy->circle_collider->bounds.setOutlineColor(sf::Color::Red);
      enemy->circle_collider->bounds.setOutlineThickness(1.0f);

      enemy->score = std::make_shared<CScore>(15.0f);
    }
    break;

    default:
      ;
    break;
  }
}

// +===============================================================================+
// | Spawn small enemies after original enemy dead.                                |
// +===============================================================================+
void Game::spawnSmallEnemies(std::shared_ptr<Entity> enemy)
{
  int type {0};
  int object_division {0};

  if  (&enemy->sprite->visual.getTexture() == &m_enemy_textures[0])
  {
    type = 1;
    object_division = 3;
  }
  else if (&enemy->sprite->visual.getTexture() == &m_enemy_textures[1])
  {
    type = 2;
    object_division = 4;
  }
  else if (&enemy->sprite->visual.getTexture() == &m_enemy_textures[2])
  {
    type = 3;
    object_division = 5;
  }

  switch (type)
  {
    case 1:
    {
      for (int i = 0; i < object_division; i++)
      {
        sf::Angle circular_angle {(sf::degrees(360) / object_division) * i};
        sf::Vector2f direction {std::cos(circular_angle.asRadians()), std::sin(circular_angle.asRadians())};

        auto small_enemy {m_entity_manager.makeEntity("SmallEnemy")};

        small_enemy->transform = std::make_shared<CTransform>(enemy->transform->position);
        small_enemy->transform->direction = direction;
        small_enemy->transform->scale = enemy->transform->scale / 2.0f;
        small_enemy->transform->speed = enemy->transform->speed * 2.0f;

        small_enemy->sprite = std::make_shared<CSprite>(m_enemy_textures[0]);

        small_enemy->circle_collider = std::make_shared<CCircleCollider>(enemy->circle_collider->bounds.getRadius() / 2.0f);
        small_enemy->circle_collider->bounds.setFillColor(sf::Color::Transparent);
        small_enemy->circle_collider->bounds.setOutlineColor(sf::Color::Red);
        small_enemy->circle_collider->bounds.setOutlineThickness(1.0f);

        small_enemy->life_span = std::make_shared<CLifeSpan>(0.8f);

        small_enemy->score = std::make_shared<CScore>(1.0f);
      }
    }
    break;

    case 2:
    {
      for (int i = 0; i < object_division; i++)
      {
        sf::Angle circular_angle {(sf::degrees(360) / object_division) * i};
        sf::Vector2f direction {std::cos(circular_angle.asRadians()), std::sin(circular_angle.asRadians())};

        auto small_enemy {m_entity_manager.makeEntity("SmallEnemy")};

        small_enemy->transform = std::make_shared<CTransform>(enemy->transform->position);
        small_enemy->transform->direction = direction;
        small_enemy->transform->scale = enemy->transform->scale / 2.0f;
        small_enemy->transform->speed = enemy->transform->speed * 2.0f;

        small_enemy->sprite = std::make_shared<CSprite>(m_enemy_textures[1]);

        small_enemy->circle_collider = std::make_shared<CCircleCollider>(enemy->circle_collider->bounds.getRadius() / 2.0f);
        small_enemy->circle_collider->bounds.setFillColor(sf::Color::Transparent);
        small_enemy->circle_collider->bounds.setOutlineColor(sf::Color::Red);
        small_enemy->circle_collider->bounds.setOutlineThickness(1.0f);

        small_enemy->life_span = std::make_shared<CLifeSpan>(0.8f);

        small_enemy->score = std::make_shared<CScore>(2.0f);
      }
    }
    break;

    case 3:
    {
      for (int i = 0; i < object_division; i++)
      {
        sf::Angle circular_angle {(sf::degrees(360) / object_division) * i};
        sf::Vector2f direction {std::cos(circular_angle.asRadians()), std::sin(circular_angle.asRadians())};

        auto small_enemy {m_entity_manager.makeEntity("SmallEnemy")};

        small_enemy->transform = std::make_shared<CTransform>(enemy->transform->position);
        small_enemy->transform->direction = direction;
        small_enemy->transform->scale = enemy->transform->scale / 2.0f;
        small_enemy->transform->speed = enemy->transform->speed * 2.0f;

        small_enemy->sprite = std::make_shared<CSprite>(m_enemy_textures[2]);

        small_enemy->circle_collider = std::make_shared<CCircleCollider>(enemy->circle_collider->bounds.getRadius() / 2.0f);
        small_enemy->circle_collider->bounds.setFillColor(sf::Color::Transparent);
        small_enemy->circle_collider->bounds.setOutlineColor(sf::Color::Red);
        small_enemy->circle_collider->bounds.setOutlineThickness(1.0f);

        small_enemy->life_span = std::make_shared<CLifeSpan>(0.8f);

        small_enemy->score = std::make_shared<CScore>(3.0f);
      }
    }
    break;

    default:
      ;
    break;
  }
}

// +===============================================================================+
// | Handle events in each state.                                                  |
// +===============================================================================+
void Game::handleEvent()
{
  while(std::optional event = m_render_window.pollEvent())
  {
    if (event->is<sf::Event::Closed>())
    {
      m_running = false;
    }
    else if (const auto* resized = event->getIf<sf::Event::Resized>())
    {
      m_view = utils::renderer::getLetterboxView(resized->size, m_view);
    }
    else if (const auto* mouse_moved = event->getIf<sf::Event::MouseMoved>())
    {
      m_mouse_position = m_render_window.mapPixelToCoords(sf::Mouse::getPosition(m_render_window), m_view);
    }

    switch (m_state)
    {
      case GameState::MAIN_MENU:
      {
        if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>())
        {
          switch (key_pressed->scancode)
          {
            case sf::Keyboard::Scancode::Enter:
              m_state = GameState::PLAY;
              reset();
            break;

            default:
              ;
            break;
          }
        }
      }
      break;

      case GameState::PAUSE:
        ;
      break;

      case GameState::PLAY:
      {
        if (const auto* mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
          switch (mouse_pressed->button)
          {
            case sf::Mouse::Button::Left:
              m_player->input->shoot = true;
            break;

            default:
              ;
            break;
          }
        }
        else if (const auto* mouse_released = event->getIf<sf::Event::MouseButtonReleased>())
        {
          switch (mouse_released->button)
          {
            case sf::Mouse::Button::Left:
              m_player->input->shoot = false;
            break;

            default:
              ;
            break;
          }
        }
        else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>())
        {
          switch (key_pressed->scancode)
          {
            case sf::Keyboard::Scancode::Left:
            case sf::Keyboard::Scancode::A:
              m_player->input->left = true;
            break;

            case sf::Keyboard::Scancode::Right:
            case sf::Keyboard::Scancode::D:
              m_player->input->right = true;
            break;

            case sf::Keyboard::Scancode::Up:
            case sf::Keyboard::Scancode::W:
              m_player->input->up = true;
            break;

            case sf::Keyboard::Scancode::Down:
            case sf::Keyboard::Scancode::S:
              m_player->input->down = true;
            break;

            default:
              ;
            break;
          }
        }
        else if (const auto* key_released = event->getIf<sf::Event::KeyReleased>())
        {
          switch (key_released->scancode)
          {
            case sf::Keyboard::Scancode::Left:
            case sf::Keyboard::Scancode::A:
              m_player->input->left = false;
            break;

            case sf::Keyboard::Scancode::Right:
            case sf::Keyboard::Scancode::D:
              m_player->input->right = false;
            break;

            case sf::Keyboard::Scancode::Up:
            case sf::Keyboard::Scancode::W:
              m_player->input->up = false;
            break;

            case sf::Keyboard::Scancode::Down:
            case sf::Keyboard::Scancode::S:
              m_player->input->down = false;
            break;

            default:
              ;
            break;
          }
        }
      }
      break;

      case GameState::WIN:
      {
        if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>())
        {
          switch (key_pressed->scancode)
          {
            case sf::Keyboard::Scancode::Space:
              m_state = GameState::MAIN_MENU;
            break;

            default:
              ;
            break;
          }
        }
      }
      break;

      case GameState::LOSE:
      {
        if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>())
        {
          switch (key_pressed->scancode)
          {
            case sf::Keyboard::Scancode::Space:
              m_state = GameState::MAIN_MENU;
            break;

            default:
              ;
            break;
          }
        }
      }
      break;

      default:
        ;
      break;
    }
  }
}

// +===============================================================================+
// | Handle rendering in each state.                                               |
// +===============================================================================+
void Game::handleRendering()
{
  switch (m_state)
  {
    case GameState::MAIN_MENU:
    {
      m_render_texture.clear(sf::Color::Black);

      for (auto entity : m_entity_manager.getEntities())
      {
        if (entity->transform &&
           (entity->getTag() == "MainBanner" ||
            entity->getTag() == "MainIndicator"))
        {
          if (entity->sprite)
          {
            m_render_texture.draw(entity->sprite->visual);
          }

          if (entity->text)
          {
            m_render_texture.draw(entity->text->visual);
          }

          if (entity->rectangle_collider)
          {
            m_render_texture.draw(entity->rectangle_collider->bounds);
          }
        }
      }

      m_render_texture.display();

      sf::Sprite render_sprite {m_render_texture.getTexture()};

      m_render_window.clear(sf::Color::White);
      m_render_window.setView(m_view);
      m_render_window.draw(render_sprite);
      m_render_window.display();
    }
    break;

    case GameState::PAUSE:
    {
      m_render_texture.clear(sf::Color::Black);

      for (auto entity : m_entity_manager.getEntities())
      {
        if (entity->transform && entity->sprite &&
           (entity->getTag() == "Player" ||
            entity->getTag() == "Bullet" ||
            entity->getTag() == "Enemy" ||
            entity->getTag() == "SmallEnemy"))
        {
          if (entity->sprite)
          {
            m_render_texture.draw(entity->sprite->visual);
          }

          if (entity->circle_collider)
          {
            m_render_texture.draw(entity->circle_collider->bounds);
          }

          if (entity->life_span)
          {
            float ratio {entity->life_span->remaining / entity->life_span->total};
            sf::Color color_transparent_change {entity->sprite->visual.getColor()};

            color_transparent_change.a = 255 * ratio;

            entity->sprite->visual.setColor(color_transparent_change);
          }
        }
      }

      for (auto entity : m_entity_manager.getEntities())
      {
        if (entity->transform &&
           (entity->getTag() == "PauseBanner" ||
            entity->getTag() == "PauseIndicator"))
        {
          if (entity->sprite)
          {
            m_render_texture.draw(entity->sprite->visual);
          }

          if (entity->text)
          {
            m_render_texture.draw(entity->text->visual);
          }

          if (entity->rectangle_collider)
          {
            m_render_texture.draw(entity->rectangle_collider->bounds);
          }
        }
      }

      m_render_texture.display();

      sf::Sprite render_sprite {m_render_texture.getTexture()};

      m_render_window.clear(sf::Color::White);
      m_render_window.setView(m_view);
      m_render_window.draw(render_sprite);
      debug.draw(m_render_window);
      m_render_window.display();
    }
    break;

    case GameState::PLAY:
    {
      m_render_texture.clear(sf::Color::Black);

      for (auto entity : m_entity_manager.getEntities())
      {
        if (entity->transform && entity->sprite &&
           (entity->getTag() == "Player" ||
            entity->getTag() == "Bullet" ||
            entity->getTag() == "Enemy" ||
            entity->getTag() == "SmallEnemy"))
        {
          m_render_texture.draw(entity->sprite->visual);

          if (entity->circle_collider)
          {
            m_render_texture.draw(entity->circle_collider->bounds);
          }

          if (entity->life_span)
          {
            float ratio {entity->life_span->remaining / entity->life_span->total};
            sf::Color color_transparent_change {entity->sprite->visual.getColor()};

            color_transparent_change.a = 255 * ratio;

            entity->sprite->visual.setColor(color_transparent_change);
          }
        }
      }

      m_render_texture.display();

      sf::Sprite render_sprite {m_render_texture.getTexture()};

      m_render_window.clear(sf::Color::White);
      m_render_window.setView(m_view);
      m_render_window.draw(render_sprite);
      debug.draw(m_render_window);
      m_render_window.display();
    }
    break;

    case GameState::WIN:
    {
      m_render_texture.clear(sf::Color::Black);

      for (auto entity : m_entity_manager.getEntities())
      {
        if (entity->transform &&
           (entity->getTag() == "WinBanner" ||
            entity->getTag() == "WinIndicator"))
        {
          if (entity->sprite)
          {
            m_render_texture.draw(entity->sprite->visual);
          }

          if (entity->text)
          {
            m_render_texture.draw(entity->text->visual);

          }
          if (entity->rectangle_collider)
          {
            m_render_texture.draw(entity->rectangle_collider->bounds);
          }
        }
      }

      m_render_texture.display();

      sf::Sprite render_sprite {m_render_texture.getTexture()};

      m_render_window.clear(sf::Color::White);
      m_render_window.setView(m_view);
      m_render_window.draw(render_sprite);
      m_render_window.display();
    }
    break;

    case GameState::LOSE:
    {
      m_render_texture.clear(sf::Color::Black);

      for (auto entity : m_entity_manager.getEntities())
      {
        if (entity->transform &&
           (entity->getTag() == "LoseBanner" ||
            entity->getTag() == "LoseIndicator"))
        {
          if (entity->sprite)
          {
            m_render_texture.draw(entity->sprite->visual);
          }

          if (entity->text)
          {
            m_render_texture.draw(entity->text->visual);

          }
          if (entity->rectangle_collider)
          {
            m_render_texture.draw(entity->rectangle_collider->bounds);
          }
        }
      }

      m_render_texture.display();

      sf::Sprite render_sprite {m_render_texture.getTexture()};

      m_render_window.clear(sf::Color::White);
      m_render_window.setView(m_view);
      m_render_window.draw(render_sprite);
      m_render_window.display();
    }
    break;

    default:
    {
      m_render_window.clear(sf::Color::Black);
      m_render_window.setView(m_view);
      m_render_window.display();
    }
    break;
  }
}

// +===============================================================================+
// | Manages main menu state.                                                      |
// +===============================================================================+
void Game::handleMainMenu()
{
  if (m_state == GameState::MAIN_MENU)
  {
    m_player->input->left = false;
    m_player->input->right = false;
    m_player->input->up = false;
    m_player->input->down = false;
    m_player->input->shoot = false;

    for (auto entity : m_entity_manager.getEntities("MainIndicator"))
    {
      if (utils::collider::checkPointVsRectangle(m_mouse_position, entity))
      {
        entity->transform->scale = sf::Vector2f{1.1f, 1.1f};

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
          entity->text->visual.setString("I said press the ENTER!");
        }
      }
      else
      {
        entity->transform->scale = sf::Vector2f{1.0f, 1.0f};

        entity->text->visual.setString("Press ENTER to play!");
      }
    }
  }
}

// +===============================================================================+
// | Manages pause state.                                                          |
// +===============================================================================+
void Game::handlePause()
{
  static bool space_pressed {false};

  if (m_state == GameState::PLAY)
  {
    if (!space_pressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space))
    {
      space_pressed = true;
      m_state = GameState::PAUSE;
    }
    else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space))
    {
      space_pressed = false;
    }
  }
  else if (m_state == GameState::PAUSE)
  {
    m_player->input->left = false;
    m_player->input->right = false;
    m_player->input->up = false;
    m_player->input->down = false;
    m_player->input->shoot = false;

    if (!space_pressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space))
    {
      space_pressed = true;
      m_state = GameState::PLAY;
    }
    else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space))
    {
      space_pressed = false;
    }

    for (auto entity : m_entity_manager.getEntities("PauseIndicator"))
    {
      if (utils::collider::checkPointVsRectangle(m_mouse_position, entity))
      {
        entity->transform->scale = sf::Vector2f{1.1f, 1.1f};

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
          entity->text->visual.setString("I said press the SPACE!");
        }
      }
      else
      {
        entity->transform->scale = sf::Vector2f{1.0f, 1.0f};

        entity->text->visual.setString("Press SPACE to resume!");
      }
    }
  }
}

// +===============================================================================+
// | Manages end game state (win/lose).                                            |
// +===============================================================================+
void Game::handleEndGame()
{
  if (m_state == GameState::PLAY)
  {
    if (m_points >= 100.0f)
    {
      m_state = GameState::WIN;

      reset();
    }
    else if (m_player_collided)
    {
      m_player_collided = false;
      m_state = GameState::LOSE;

      reset();
    }
  }
  else if (m_state == GameState::WIN)
  {
    for (auto entity : m_entity_manager.getEntities("WinIndicator"))
    {
      if (utils::collider::checkPointVsRectangle(m_mouse_position, entity))
      {
        entity->transform->scale = sf::Vector2f{1.1f, 1.1f};

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
          entity->text->visual.setString("I said press the SPACE!");
        }
      }
      else
      {
        entity->transform->scale = sf::Vector2f{1.0f, 1.0f};

        entity->text->visual.setString("Press SPACE to continue!");
      }
    }
  }
  else if (m_state == GameState::LOSE)
  {
    for (auto entity : m_entity_manager.getEntities("LoseIndicator"))
    {
      if (utils::collider::checkPointVsRectangle(m_mouse_position, entity))
      {
        entity->transform->scale = sf::Vector2f{1.1f, 1.1f};

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
          entity->text->visual.setString("I said press the SPACE!");
        }
      }
      else
      {
        entity->transform->scale = sf::Vector2f{1.0f, 1.0f};

        entity->text->visual.setString("Press SPACE to continue!");
      }
    }
  }
}

// +===============================================================================+
// | Implement every game object movement in game world.                           |
// +===============================================================================+
void Game::handleMovement()
{
  if (m_state == GameState::PLAY)
  {
    int player_move_x {static_cast<int>(m_player->input->right) - static_cast<int>(m_player->input->left)};
    int player_move_y {static_cast<int>(m_player->input->down) - static_cast<int>(m_player->input->up)};

    m_player->transform->rotation = utils::calculator::angleBetween(m_player->transform->position, m_mouse_position);

    if (player_move_x || player_move_y)
    {
      m_player->transform->direction = sf::Vector2f{static_cast<float>(player_move_x), static_cast<float>(player_move_y)}.normalized();
    }
    else
    {
      m_player->transform->direction = sf::Vector2f{0.0f, 0.0f};
    }

    for (auto entity : m_entity_manager.getEntities())
    {
      entity->transform->position += entity->transform->direction * entity->transform->speed * m_delta_time;
    }
  }
}

// +===============================================================================+
// | Handle shooting mechanism for player.                                         |
// +===============================================================================+
void Game::handlePlayerShooting(float delay)
{
  if (m_state == GameState::PLAY)
  {
    static float last_shot_time {m_current_time};

    if (m_player->input->shoot && ((m_current_time - last_shot_time) >= delay))
    {
      spawnBullet();

      last_shot_time = m_current_time;
    }
  }
}

// +===============================================================================+
// | Handle enemy spawn timer.                                                     |
// +===============================================================================+
void Game::handleEnemySpawnTime(float interval, int max_enemies)
{
  if (m_state == GameState::PLAY)
  {
    static float last_spawn_time {m_current_time};

    if ((m_current_time - last_spawn_time) >= interval)
    {
      if (m_entity_manager.getEntities("Enemy").size() <= max_enemies)
      {
        spawnEnemy();
      }
      last_spawn_time = m_current_time;
    }
  }
}

// +===============================================================================+
// | Handle collisions between entities in game world.                             |
// +===============================================================================+
void Game::handleCollision()
{
  if (m_state == GameState::PLAY)
  {
    float player_left {m_player->transform->position.x - m_player->circle_collider->bounds.getRadius()};
    float player_right {m_player->transform->position.x + m_player->circle_collider->bounds.getRadius()};
    float player_top {m_player->transform->position.y - m_player->circle_collider->bounds.getRadius()};
    float player_bottom {m_player->transform->position.y + m_player->circle_collider->bounds.getRadius()};

    if (player_left <= 0.0f)
    {
      m_player->transform->position.x = 0.0f + m_player->circle_collider->bounds.getRadius();
    }
    if (player_right >= m_render_texture.getSize().x)
    {
      m_player->transform->position.x = m_render_texture.getSize().x - m_player->circle_collider->bounds.getRadius();
    }
    if (player_top <= 0.0f)
    {
      m_player->transform->position.y = 0.0f + m_player->circle_collider->bounds.getRadius();
    }
    if (player_bottom >= m_render_texture.getSize().y)
    {
      m_player->transform->position.y = m_render_texture.getSize().y - m_player->circle_collider->bounds.getRadius();
    }

    for (auto enemy : m_entity_manager.getEntities("Enemy"))
    {
      float enemy_left {enemy->transform->position.x - enemy->circle_collider->bounds.getRadius()};
      float enemy_right {enemy->transform->position.x + enemy->circle_collider->bounds.getRadius()};
      float enemy_top {enemy->transform->position.y - enemy->circle_collider->bounds.getRadius()};
      float enemy_bottom {enemy->transform->position.y + enemy->circle_collider->bounds.getRadius()};

      if (enemy_left <= 0 || enemy_right >= m_render_texture.getSize().x)
      {
        enemy->transform->direction.x = -enemy->transform->direction.x;
      }
      if (enemy_top <= 0 || enemy_bottom >= m_render_texture.getSize().y)
      {
        enemy->transform->direction.y = -enemy->transform->direction.y;
      }

      if (utils::collider::checkCircleVsCircle(m_player, enemy))
      {
        m_player_collided = true;
      }

      for (auto bullet : m_entity_manager.getEntities("Bullet"))
      {
        if (utils::collider::checkCircleVsCircle(bullet, enemy))
        {
          spawnSmallEnemies(enemy);

          enemy->destroy();
          bullet->destroy();

          m_points += enemy->score->amount;
        }
      }
    }

    for (auto small_enemy : m_entity_manager.getEntities("SmallEnemy"))
    {
      float small_enemy_left {small_enemy->transform->position.x - small_enemy->circle_collider->bounds.getRadius()};
      float small_enemy_right {small_enemy->transform->position.x + small_enemy->circle_collider->bounds.getRadius()};
      float small_enemy_top {small_enemy->transform->position.y - small_enemy->circle_collider->bounds.getRadius()};
      float small_enemy_bottom {small_enemy->transform->position.y + small_enemy->circle_collider->bounds.getRadius()};

      if (small_enemy_left <= 0 || small_enemy_right >= m_render_texture.getSize().x)
      {
        small_enemy->transform->direction.x = -small_enemy->transform->direction.x;
      }
      if (small_enemy_top <= 0 || small_enemy_bottom >= m_render_texture.getSize().y)
      {
        small_enemy->transform->direction.y = -small_enemy->transform->direction.y;
      }

      if (utils::collider::checkCircleVsCircle(m_player, small_enemy))
      {
        m_player_collided = true;
      }

      for (auto bullet : m_entity_manager.getEntities("Bullet"))
      {
        if (utils::collider::checkCircleVsCircle(bullet, small_enemy))
        {
          small_enemy->destroy();
          bullet->destroy();

          m_points += small_enemy->score->amount;
        }
      }
    }
  }
}

// +===============================================================================+
// | Handle entity's transform components.                                         |
// +===============================================================================+
void Game::handleTransform()
{
  for (auto entity : m_entity_manager.getEntities())
  {
    if (entity->transform)
    {
      if (entity->sprite)
      {
        entity->sprite->visual.setPosition(entity->transform->position);
        entity->sprite->visual.setRotation(entity->transform->rotation);
        entity->sprite->visual.setScale(entity->transform->scale);
        entity->sprite->visual.setOrigin(sf::Vector2f{static_cast<float>(entity->sprite->visual.getTexture().getSize().x) * entity->transform->scale.x,
                                                      static_cast<float>(entity->sprite->visual.getTexture().getSize().y) * entity->transform->scale.y} / 2.0f);
      }

      if (entity->text)
      {
        entity->text->visual.setPosition(entity->transform->position);
        entity->text->visual.setRotation(entity->transform->rotation);
        entity->text->visual.setScale(entity->transform->scale);
        entity->text->visual.setOrigin(sf::Vector2f{entity->text->visual.getLocalBounds().getCenter().x,
                                                    entity->text->visual.getLocalBounds().getCenter().y});
      }

      if (entity->rectangle_collider)
      {
        entity->rectangle_collider->bounds.setPosition(entity->transform->position);
        entity->rectangle_collider->bounds.setRotation(entity->transform->rotation);
        entity->rectangle_collider->bounds.setOrigin(sf::Vector2f{entity->rectangle_collider->bounds.getSize().x,
                                                                  entity->rectangle_collider->bounds.getSize().y} / 2.0f);
      }

      if (entity->circle_collider)
      {
        entity->circle_collider->bounds.setPosition(entity->transform->position);
        entity->circle_collider->bounds.setRotation(entity->transform->rotation);
        entity->circle_collider->bounds.setOrigin(sf::Vector2f{entity->circle_collider->bounds.getRadius() * entity->transform->scale.x,
            entity->circle_collider->bounds.getRadius() * entity->transform->scale.y});
      }
    }
  }
}

// +===============================================================================+
// | Handle entity's life span components.                                         |
// +===============================================================================+
void Game::handleLifeSpan()
{
  if (m_state == GameState::PLAY)
  {
    for (auto entity : m_entity_manager.getEntities())
    {
      if (entity->life_span)
      {
        entity->life_span->remaining -= m_delta_time;

        if (entity->life_span->remaining <= 0.0f)
        {
          entity->destroy();
        }
      }
    }
  }
}

// +===============================================================================+
// | Actually run the game.                                                        |
// +===============================================================================+
void Game::run()
{
  m_delta_clock.restart();
  m_global_clock.restart();

  while (m_running)
  {
    m_delta_time = m_delta_clock.restart().asSeconds();
    m_current_time = m_global_clock.getElapsedTime().asSeconds();

    update();
    m_entity_manager.update();
  }

  m_render_window.close();
}

// +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
// | The functions don't have good description because there's too many flaws in   |
// | the implementation, as you can see, iteration throught vector is everywhere   |
// | EVERY FRAME. WHICH IS BAD!! *cough. And there's more but i don't know how     |
// | to explain it so yeah. One day I feel like wanna quit C++ cause this language |
// | is very hard to learn, I even looked on how to render graphics from scratch   |
// | in OpenGL with GLFW and GLAD and GOD DAMN! I can't even tell how did someone  |
// | manage to do such complex computing graphics implementation, Its not just C++ |
// | being language, that's when I realized that libraries are also language.      |
// |                                                                               |
// | I hope I'll learn more.                                                       |
// +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
