module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <tuple>
#include <string>

export module game;

import resource_manager;
import sprite_renderer;
import game_object;
import ball_object;
import particle_generator;
import game_level;
import audio_engine;
import texture;
import shader;
import text_renderer;

// Game state
export enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_LOSE,
    GAME_LEVEL_SELECT
};

// Represents the four possible (collision) directions
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// Defines a Tuple of <collision?, what direction?, difference vector center - closest point>
typedef std::tuple<bool, Direction, glm::vec2> Collision;

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;

export class Game
{
public:
    GameState               State;
    bool                    Keys[1024];
    bool                    KeysProcessed[1024];
    bool                    MouseButtons[3]; // Left, Right, Middle
    bool                    MouseProcessed[3];
    glm::vec2               MousePos;
    unsigned int            Width, Height;
    std::vector<GameLevel>  Levels;
    unsigned int            Level;
    unsigned int            Lives;
    unsigned int            MenuSelection;

    // Game-related objects
    std::unique_ptr<SpriteRenderer>    Renderer;
    std::unique_ptr<GameObject>        Player;
    std::unique_ptr<BallObject>        Ball;
    std::unique_ptr<ParticleGenerator> Particles;
    std::unique_ptr<AudioEngine>       Audio;
    std::unique_ptr<TextRenderer>      Text;

    Game(unsigned int width, unsigned int height) 
        : State(GAME_MENU), Keys(), KeysProcessed(), MouseButtons(), MouseProcessed(), Width(width), Height(height), Level(0), Lives(3), MenuSelection(0)
    { 
    }
    
    // Button hit detection helpers
    bool IsMouseOverButton(unsigned int buttonIndex)
    {
        float btnX = this->Width / 2.0f - 100.0f;
        float btnY = this->Height / 2.0f + buttonIndex * 50.0f;
        float btnW = 200.0f;
        float btnH = 40.0f;
        return MousePos.x >= btnX && MousePos.x <= btnX + btnW &&
               MousePos.y >= btnY && MousePos.y <= btnY + btnH;
    }
    
    bool IsMouseOverLevelButton(unsigned int buttonIndex)
    {
        float btnX = this->Width / 2.0f - 100.0f;
        float btnY = this->Height / 2.0f - 60.0f + buttonIndex * 50.0f;
        float btnW = 200.0f;
        float btnH = 40.0f;
        return MousePos.x >= btnX && MousePos.x <= btnX + btnW &&
               MousePos.y >= btnY && MousePos.y <= btnY + btnH;
    }
    
    void UpdateMenuHover()
    {
        for (unsigned int i = 0; i < 3; i++)
        {
            if (IsMouseOverButton(i))
            {
                this->MenuSelection = i;
                break;
            }
        }
    }
    
    void UpdateLevelSelectHover()
    {
        for (unsigned int i = 0; i < 5; i++)
        {
            if (IsMouseOverLevelButton(i))
            {
                this->MenuSelection = i;
                break;
            }
        }
    }

    ~Game()
    {
    }

    void Init()
    {
        // load shaders
        ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
        ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");

        // configure shaders
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
        ResourceManager::GetShader("sprite").use().setInt("image", 0);
        ResourceManager::GetShader("sprite").setMatrix4("projection", projection);
        ResourceManager::GetShader("particle").use().setInt("sprite", 0);
        ResourceManager::GetShader("particle").setMatrix4("projection", projection);

        // load textures
        ResourceManager::LoadTexture("assets/background.jpg", false, "background");
        ResourceManager::LoadTexture("assets/face.png", true, "face");
        ResourceManager::LoadTexture("assets/block.png", false, "block");
        ResourceManager::LoadTexture("assets/block_solid.png", false, "block_solid");
        ResourceManager::LoadTexture("assets/paddle.png", true, "paddle");
        ResourceManager::LoadTexture("assets/particle.png", true, "particle");

        // set render-specific controls
        Renderer = std::make_unique<SpriteRenderer>(ResourceManager::GetShader("sprite"));
        Particles = std::make_unique<ParticleGenerator>(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
        Text = std::make_unique<TextRenderer>(this->Width, this->Height);
        Text->Load("assets/fonts/arial.ttf", 24);
        Audio = std::make_unique<AudioEngine>();

        // load levels
        GameLevel one; one.Load("assets/levels/one.lvl", this->Width, this->Height / 2);
        GameLevel two; two.Load("assets/levels/two.lvl", this->Width, this->Height / 2);
        GameLevel three; three.Load("assets/levels/three.lvl", this->Width, this->Height / 2);
        GameLevel four; four.Load("assets/levels/four.lvl", this->Width, this->Height / 2);
        this->Levels.push_back(one);
        this->Levels.push_back(two);
        this->Levels.push_back(three);
        this->Levels.push_back(four);
        this->Level = 0;

        // configure game objects
        glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Player = std::make_unique<GameObject>(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
        
        glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
        Ball = std::make_unique<BallObject>(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
        
        // play simple startup sound
        Audio->Play("assets/audio/bleep.wav");
    }

    void Update(float dt)
    {
        // update objects
        Ball->Move(dt, this->Width);
        // check for collisions
        this->DoCollisions();
        // update particles
        Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
        if (Ball->Position.y >= this->Height)
        {
            --this->Lives;
            if (this->Lives == 0)
            {
                this->ResetLevel();
                this->State = GAME_LOSE;
                this->MenuSelection = 0; // Reset menu selection for Game Over screen
            }
            this->ResetPlayer();
        }

        // check win condition
        if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
        {
            this->Level = (this->Level + 1) % this->Levels.size();
            this->ResetLevel();
            this->ResetPlayer();
            this->State = GAME_ACTIVE;
        }
    }

    void ProcessInput(float dt)
    {
        if (this->State == GAME_MENU)
        {
            // Keyboard navigation
            if (this->Keys[GLFW_KEY_UP] && !this->KeysProcessed[GLFW_KEY_UP])
            {
                this->MenuSelection = (this->MenuSelection + 2) % 3;
                this->KeysProcessed[GLFW_KEY_UP] = true;
            }
            if (this->Keys[GLFW_KEY_DOWN] && !this->KeysProcessed[GLFW_KEY_DOWN])
            {
                this->MenuSelection = (this->MenuSelection + 1) % 3;
                this->KeysProcessed[GLFW_KEY_DOWN] = true;
            }
            if ((this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) || 
                (this->MouseButtons[0] && !this->MouseProcessed[0] && IsMouseOverButton(this->MenuSelection)))
            {
                if (this->MenuSelection == 0) // Start Game
                {
                    this->State = GAME_LEVEL_SELECT;
                    this->MenuSelection = 0;
                }
                else if (this->MenuSelection == 1) // Level Select
                {
                    this->State = GAME_LEVEL_SELECT;
                    this->MenuSelection = 0;
                }
                else if (this->MenuSelection == 2) // Exit
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
                
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
                this->MouseProcessed[0] = true;
            }
            if (this->Keys[GLFW_KEY_ESCAPE] && !this->KeysProcessed[GLFW_KEY_ESCAPE])
            {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
                this->KeysProcessed[GLFW_KEY_ESCAPE] = true;
            }
            
            // Mouse hover detection for menu
            UpdateMenuHover();
        }
        if (this->State == GAME_LEVEL_SELECT)
        {
            // Keyboard navigation
            if (this->Keys[GLFW_KEY_UP] && !this->KeysProcessed[GLFW_KEY_UP])
            {
                this->MenuSelection = (this->MenuSelection + 4) % 5;
                this->KeysProcessed[GLFW_KEY_UP] = true;
            }
            if (this->Keys[GLFW_KEY_DOWN] && !this->KeysProcessed[GLFW_KEY_DOWN])
            {
                this->MenuSelection = (this->MenuSelection + 1) % 5;
                this->KeysProcessed[GLFW_KEY_DOWN] = true;
            }
            if ((this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) ||
                (this->MouseButtons[0] && !this->MouseProcessed[0] && IsMouseOverLevelButton(this->MenuSelection)))
            {
                if (MenuSelection < 4) {
                    this->Level = MenuSelection;
                    this->ResetLevel();
                    this->ResetPlayer();
                    this->State = GAME_ACTIVE;
                    this->Lives = 3;
                } else {
                    this->State = GAME_MENU;
                    this->MenuSelection = 0;
                }
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
                this->MouseProcessed[0] = true;
            }
            if (this->Keys[GLFW_KEY_ESCAPE] && !this->KeysProcessed[GLFW_KEY_ESCAPE])
            {
                this->State = GAME_MENU;
                this->MenuSelection = 0;
                this->KeysProcessed[GLFW_KEY_ESCAPE] = true;
            }
            
            // Mouse hover detection
            UpdateLevelSelectHover();
        }
        if (this->State == GAME_WIN || this->State == GAME_LOSE)
        {
            if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
            {
                this->ResetLevel();
                this->State = GAME_MENU;
                this->MenuSelection = 0;
                this->Lives = 3;
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
            }
        }
        if (this->State == GAME_ACTIVE)
        {
            // ESC pauses/returns to menu
            if (this->Keys[GLFW_KEY_ESCAPE] && !this->KeysProcessed[GLFW_KEY_ESCAPE])
            {
                this->State = GAME_MENU;
                this->MenuSelection = 0;
                this->KeysProcessed[GLFW_KEY_ESCAPE] = true;
            }
            
            float velocity = PLAYER_VELOCITY * dt;
            // move playerboard
            if (this->Keys[GLFW_KEY_A] || this->Keys[GLFW_KEY_LEFT])
            {
                if (Player->Position.x >= 0.0f)
                {
                    Player->Position.x -= velocity;
                    if (Ball->Stuck)
                        Ball->Position.x -= velocity;
                }
            }
            if (this->Keys[GLFW_KEY_D] || this->Keys[GLFW_KEY_RIGHT])
            {
                if (Player->Position.x <= this->Width - Player->Size.x)
                {
                    Player->Position.x += velocity;
                    if (Ball->Stuck)
                        Ball->Position.x += velocity;
                }
            }
            if (this->Keys[GLFW_KEY_SPACE])
                Ball->Stuck = false;
        }
    }

    void Render()
    {
        // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        if (this->State == GAME_ACTIVE || this->State == GAME_WIN || this->State == GAME_LOSE)
        {
            // Draw Shadows (Fake 3D Depth) - Pass 1
            glm::vec2 shadowOffset(3.0f, 3.0f);
            glm::vec3 shadowColor(0.1f, 0.1f, 0.1f); // Dark Grey/Blackish

            // Shadows for Level Bricks
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (!box.Destroyed)
                    Renderer->DrawSprite(box.Sprite, box.Position + shadowOffset, box.Size, box.Rotation, shadowColor);
            }
            
            // Shadow for Player
            Renderer->DrawSprite(Player->Sprite, Player->Position + shadowOffset, Player->Size, Player->Rotation, shadowColor);
            
            // Shadow for Ball
            Renderer->DrawSprite(Ball->Sprite, Ball->Position + shadowOffset, glm::vec2(Ball->Radius * 2.0f), 0.0f, shadowColor);

            // Draw Main Objects - Pass 2
            this->Levels[this->Level].Draw(*Renderer);
            Player->Draw(*Renderer);
            Particles->Draw();
            Ball->Draw(*Renderer);
            
            // UI
            if (this->State == GAME_ACTIVE) {
                 Text->RenderText("Lives: " + std::to_string(this->Lives), 5.0f, 5.0f, 1.0f);
                 Text->RenderText("Level: " + std::to_string(this->Level + 1), this->Width - 120.0f, 5.0f, 1.0f);
            }
        }
        
        if (this->State == GAME_MENU)
        {
            // Title
            Text->RenderText("BREAKOUT", this->Width / 2.0f - 100.0f, 120.0f, 2.0f, glm::vec3(1.0f, 0.41f, 0.71f)); // Hot Pink

            // Button styling - Cyber Kawaii
            glm::vec3 selectedColor(0.0f, 1.0f, 1.0f);  // Cyan
            glm::vec3 normalColor(1.0f, 1.0f, 1.0f);    // White
            glm::vec3 hoverBg(0.2f, 0.2f, 0.3f);        // Dark blue-ish
            
            float btnX = this->Width / 2.0f - 100.0f;
            float btnY = this->Height / 2.0f;
            
            // Draw button backgrounds
            for (int i = 0; i < 3; i++)
            {
                float y = btnY + i * 50.0f;
                if (this->MenuSelection == (unsigned int)i)
                    Renderer->DrawSprite(ResourceManager::GetTexture("block"), glm::vec2(btnX - 10.0f, y - 5.0f), glm::vec2(220.0f, 35.0f), 0.0f, glm::vec3(0.3f, 0.1f, 0.4f));
            }
            
            Text->RenderText("> Start Game", btnX, btnY, 1.0f, this->MenuSelection == 0 ? selectedColor : normalColor);
            Text->RenderText("> Select Level", btnX, btnY + 50.0f, 1.0f, this->MenuSelection == 1 ? selectedColor : normalColor);
            Text->RenderText("> Exit", btnX, btnY + 100.0f, 1.0f, this->MenuSelection == 2 ? selectedColor : normalColor);
            
            // Controls hint
            Text->RenderText("Arrow Keys / Mouse to navigate, Enter / Click to select", 120.0f, this->Height - 40.0f, 0.5f, glm::vec3(0.6f, 0.6f, 0.6f));
        }

        if (this->State == GAME_LEVEL_SELECT)
        {
            Text->RenderText("SELECT LEVEL", this->Width / 2.0f - 120.0f, 100.0f, 1.5f, glm::vec3(1.0f, 0.41f, 0.71f));
            
            glm::vec3 selectedColor(0.0f, 1.0f, 1.0f);  // Cyan
            glm::vec3 normalColor(1.0f, 1.0f, 1.0f);    // White
            
            float btnX = this->Width / 2.0f - 100.0f;
            float btnY = this->Height / 2.0f - 60.0f;
            
            // Draw button backgrounds
            for (int i = 0; i < 5; i++)
            {
                float y = btnY + i * 50.0f;
                if (this->MenuSelection == (unsigned int)i)
                    Renderer->DrawSprite(ResourceManager::GetTexture("block"), glm::vec2(btnX - 10.0f, y - 5.0f), glm::vec2(220.0f, 35.0f), 0.0f, glm::vec3(0.3f, 0.1f, 0.4f));
            }
            
            Text->RenderText("> Level 1", btnX, btnY, 1.0f, this->MenuSelection == 0 ? selectedColor : normalColor);
            Text->RenderText("> Level 2", btnX, btnY + 50.0f, 1.0f, this->MenuSelection == 1 ? selectedColor : normalColor);
            Text->RenderText("> Level 3", btnX, btnY + 100.0f, 1.0f, this->MenuSelection == 2 ? selectedColor : normalColor);
            Text->RenderText("> Level 4", btnX, btnY + 150.0f, 1.0f, this->MenuSelection == 3 ? selectedColor : normalColor);
            Text->RenderText("> Back", btnX, btnY + 200.0f, 1.0f, this->MenuSelection == 4 ? selectedColor : normalColor);
            
            Text->RenderText("ESC to go back", this->Width / 2.0f - 70.0f, this->Height - 40.0f, 0.5f, glm::vec3(0.6f, 0.6f, 0.6f));
        }

        if (this->State == GAME_WIN)
        {
            Text->RenderText("You WON!!!", 320.0f, Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            Text->RenderText("Press ENTER to return to Menu", 130.0f, Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        }
        
        if (this->State == GAME_LOSE)
        {
            Text->RenderText("GAME OVER", this->Width / 2.0f - 90.0f, this->Height / 2.0f - 20.0f, 1.5f, glm::vec3(1.0f, 0.0f, 0.0f));
            Text->RenderText("Press ENTER to return to Menu", 130.0f, this->Height / 2.0f + 40.0f, 1.0f, glm::vec3(1.0f));
        }
    }

    void DoCollisions()
    {
        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if (!box.Destroyed)
            {
                Collision collision = CheckCollision(*Ball, box);
                if (std::get<0>(collision)) // if collision is true
                {
                    // destroy block if not solid
                    if (!box.IsSolid) {
                        box.Destroyed = true;
                        Audio->Play("assets/audio/bleep.wav");
                    } else {
                        // Audio->Play("assets/audio/solid.wav");
                    }
                    
                    // collision resolution
                    Direction dir = std::get<1>(collision);
                    glm::vec2 diff_vector = std::get<2>(collision);
                    if (dir == LEFT || dir == RIGHT) // horizontal collision
                    {
                        Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration; // move ball to right
                        else
                            Ball->Position.x -= penetration; // move ball to left;
                    }
                    else // vertical collision
                    {
                        Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration; // move ball back up
                        else
                            Ball->Position.y += penetration; // move ball back down
                    }				
                }
            }
        }
        
        // Check collision for player pad
        Collision result = CheckCollision(*Ball, *Player);
        if (!Ball->Stuck && std::get<0>(result))
        {
            // check where it hit the board, and change velocity based on where it hit the board
            float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
            float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
            float percentage = distance / (Player->Size.x / 2.0f);
            // then move accordingly
            float strength = 2.0f;
            glm::vec2 oldVelocity = Ball->Velocity;
            Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
            Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);
            Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);

            Audio->Play("assets/audio/pop.wav"); // Pop sound on paddle
        }
    }

    // Reset
    void ResetLevel()
    {
        if (this->Level == 0)
            this->Levels[0].Load("assets/levels/one.lvl", this->Width, this->Height / 2);
        else if (this->Level == 1)
            this->Levels[1].Load("assets/levels/two.lvl", this->Width, this->Height / 2);
        else if (this->Level == 2)
            this->Levels[2].Load("assets/levels/three.lvl", this->Width, this->Height / 2);
        else if (this->Level == 3)
            this->Levels[3].Load("assets/levels/four.lvl", this->Width, this->Height / 2);
    }

    void ResetPlayer()
    {
        // reset player/ball stats
        Player->Size = PLAYER_SIZE;
        Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
    }

private:
    // collision detection
    bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
    {
        // collision x-axis?
        bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
            two.Position.x + two.Size.x >= one.Position.x;
        // collision y-axis?
        bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
            two.Position.y + two.Size.y >= one.Position.y;
        // collision only if on both axes
        return collisionX && collisionY;
    }

    Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
    {
        // get center point circle first 
        glm::vec2 center(one.Position + one.Radius);
        // calculate AABB info (center, half-extents)
        glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
        glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
        // get difference vector between both centers
        glm::vec2 difference = center - aabb_center;
        glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
        // now that we know the clamped values, add this to AABB_center and we get the value of box closest to circle
        glm::vec2 closest = aabb_center + clamped;
        // now retrieve vector between center circle and closest point AABB and check if length < radius
        difference = closest - center;
        
        if (glm::length(difference) < one.Radius)
            return std::make_tuple(true, VectorDirection(difference), difference);
        else
            return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
    }

    Direction VectorDirection(glm::vec2 target)
    {
        glm::vec2 compass[] = {
            glm::vec2(0.0f, 1.0f),	// up
            glm::vec2(1.0f, 0.0f),	// right
            glm::vec2(0.0f, -1.0f),	// down
            glm::vec2(-1.0f, 0.0f)	// left
        };
        float max = 0.0f;
        unsigned int best_match = -1;
        for (unsigned int i = 0; i < 4; i++)
        {
            float dot_product = glm::dot(glm::normalize(target), compass[i]);
            if (dot_product > max)
            {
                max = dot_product;
                best_match = i;
            }
        }
        return (Direction)best_match;
    }
};
