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

export enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_LOSE,
    GAME_LEVEL_SELECT,
    GAME_PAUSE
};

const glm::vec3 COLOR_BG(0.95f, 0.90f, 0.95f);
const glm::vec3 COLOR_UI_OUTLINE(0.4f, 0.6f, 0.8f);
const glm::vec3 COLOR_ACCENT(1.0f, 0.6f, 0.8f);
const glm::vec3 COLOR_HIGHLIGHT(0.6f, 0.9f, 0.9f);
const glm::vec3 COLOR_TEXT_NORMAL(0.3f, 0.3f, 0.4f);
const glm::vec3 COLOR_TEXT_SELECTED(0.9f, 0.4f, 0.6f);

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

const glm::vec2 PLAYER_SIZE(100.0f, 25.0f);
const float PLAYER_VELOCITY(500.0f);
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
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
    float                   ShakeTime;
    glm::vec2               ViewOffset;

    std::unique_ptr<SpriteRenderer>    Renderer;
    std::unique_ptr<GameObject>        Player;
    std::unique_ptr<BallObject>        Ball;
    std::unique_ptr<ParticleGenerator> Particles;
    std::unique_ptr<AudioEngine>       Audio;
    std::unique_ptr<TextRenderer>      Text;

    Game(unsigned int width, unsigned int height) 
        : State(GAME_MENU), Keys(), KeysProcessed(), MouseButtons(), MouseProcessed(), Width(width), Height(height), Level(0), Lives(3), MenuSelection(0), ShakeTime(0.0f), ViewOffset(0.0f)
    { 
    }
    
    void TriggerShake(float duration)
    {
        this->ShakeTime = duration;
    }
    
    void UpdateShake(float dt)
    {
        if (this->ShakeTime > 0.0f)
        {
            this->ShakeTime -= dt;
            float strength = 3.0f;
            this->ViewOffset.x = ((rand() % 100) / 100.0f - 0.5f) * 2.0f * strength;
            this->ViewOffset.y = ((rand() % 100) / 100.0f - 0.5f) * 2.0f * strength;
        }
        else
        {
            this->ViewOffset = glm::vec2(0.0f);
        }
    }
    
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
        ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
        ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");

        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
        ResourceManager::GetShader("sprite").use().setInt("image", 0);
        ResourceManager::GetShader("sprite").setMatrix4("projection", projection);
        ResourceManager::GetShader("particle").use().setInt("sprite", 0);
        ResourceManager::GetShader("particle").setMatrix4("projection", projection);

        ResourceManager::LoadTexture("assets/background_pastel.png", false, "background");
        ResourceManager::LoadTexture("assets/ball.png", true, "ball");
        ResourceManager::LoadTexture("assets/block_crystal.png", false, "block");
        ResourceManager::LoadTexture("assets/block_solid.png", false, "block_solid");
        ResourceManager::LoadTexture("assets/paddle.png", true, "paddle");
        ResourceManager::LoadTexture("assets/particle.png", true, "particle");
        ResourceManager::LoadTexture("assets/heart.png", true, "heart");
        ResourceManager::LoadTexture("assets/button_atlas.png", true, "button");
        ResourceManager::LoadTexture("assets/chibi_main.png", true, "chibi_main");
        ResourceManager::LoadTexture("assets/chibi_sad.png", true, "chibi_sad");

        Renderer = std::make_unique<SpriteRenderer>(ResourceManager::GetShader("sprite"));
        Particles = std::make_unique<ParticleGenerator>(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
        Text = std::make_unique<TextRenderer>(this->Width, this->Height);
        Text->Load("assets/fonts/arial.ttf", 24);
        Audio = std::make_unique<AudioEngine>();

        GameLevel one; one.Load("assets/levels/one.lvl", this->Width, this->Height / 2);
        GameLevel two; two.Load("assets/levels/two.lvl", this->Width, this->Height / 2);
        GameLevel three; three.Load("assets/levels/three.lvl", this->Width, this->Height / 2);
        GameLevel four; four.Load("assets/levels/four.lvl", this->Width, this->Height / 2);
        this->Levels.push_back(one);
        this->Levels.push_back(two);
        this->Levels.push_back(three);
        this->Levels.push_back(four);
        this->Level = 0;

        glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Player = std::make_unique<GameObject>(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
        
        glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
        Ball = std::make_unique<BallObject>(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("ball"));
        
        Audio->Play("assets/audio/bleep.wav");
    }

    void Update(float dt)
    {
        this->UpdateShake(dt);
        
        if (this->State != GAME_ACTIVE)
            return;
            
        Ball->Move(dt, this->Width);
        this->DoCollisions();
        Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
        if (Ball->Position.y >= this->Height)
        {
            --this->Lives;
            this->TriggerShake(0.3f);
            Audio->Play("assets/audio/lose.wav");
            if (this->Lives == 0)
            {
                this->ResetLevel();
                this->State = GAME_LOSE;
                this->MenuSelection = 0;
            }
            this->ResetPlayer();
        }

        // check win
        if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
        {
            Audio->Play("assets/audio/win.wav");
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
            if (this->Keys[GLFW_KEY_UP] && !this->KeysProcessed[GLFW_KEY_UP])
            {
                this->MenuSelection = (this->MenuSelection + 2) % 3;
                Audio->Play("assets/audio/ui_select.wav");
                this->KeysProcessed[GLFW_KEY_UP] = true;
            }
            if (this->Keys[GLFW_KEY_DOWN] && !this->KeysProcessed[GLFW_KEY_DOWN])
            {
                this->MenuSelection = (this->MenuSelection + 1) % 3;
                Audio->Play("assets/audio/ui_select.wav");
                this->KeysProcessed[GLFW_KEY_DOWN] = true;
            }
            if ((this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) || 
                (this->MouseButtons[0] && !this->MouseProcessed[0] && IsMouseOverButton(this->MenuSelection)))
            {
                Audio->Play("assets/audio/ui_confirm.wav");
                if (this->MenuSelection == 0)
                {
                    this->State = GAME_LEVEL_SELECT;
                    this->MenuSelection = 0;
                }
                else if (this->MenuSelection == 1)
                {
                    this->State = GAME_LEVEL_SELECT;
                    this->MenuSelection = 0;
                }
                else if (this->MenuSelection == 2)
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
                
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
                this->MouseProcessed[0] = true;
            }
            if (this->Keys[GLFW_KEY_ESCAPE] && !this->KeysProcessed[GLFW_KEY_ESCAPE])
            {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
                this->KeysProcessed[GLFW_KEY_ESCAPE] = true;
            }
            
            UpdateMenuHover();
        }
        if (this->State == GAME_LEVEL_SELECT)
        {
            if (this->Keys[GLFW_KEY_UP] && !this->KeysProcessed[GLFW_KEY_UP])
            {
                this->MenuSelection = (this->MenuSelection + 4) % 5;
                Audio->Play("assets/audio/ui_select.wav");
                this->KeysProcessed[GLFW_KEY_UP] = true;
            }
            if (this->Keys[GLFW_KEY_DOWN] && !this->KeysProcessed[GLFW_KEY_DOWN])
            {
                this->MenuSelection = (this->MenuSelection + 1) % 5;
                Audio->Play("assets/audio/ui_select.wav");
                this->KeysProcessed[GLFW_KEY_DOWN] = true;
            }
            if ((this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) ||
                (this->MouseButtons[0] && !this->MouseProcessed[0] && IsMouseOverLevelButton(this->MenuSelection)))
            {
                Audio->Play("assets/audio/ui_confirm.wav");
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
        if (this->State == GAME_PAUSE)
        {
            if (this->Keys[GLFW_KEY_ESCAPE] && !this->KeysProcessed[GLFW_KEY_ESCAPE])
            {
                this->State = GAME_ACTIVE;
                this->KeysProcessed[GLFW_KEY_ESCAPE] = true;
            }
            if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
            {
                this->State = GAME_MENU;
                this->MenuSelection = 0;
                this->KeysProcessed[GLFW_KEY_ENTER] = true;
            }
        }
        if (this->State == GAME_ACTIVE)
        {
            if (this->Keys[GLFW_KEY_ESCAPE] && !this->KeysProcessed[GLFW_KEY_ESCAPE])
            {
                this->State = GAME_PAUSE;
                this->KeysProcessed[GLFW_KEY_ESCAPE] = true;
            }
            
            float velocity = PLAYER_VELOCITY * dt;
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
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        if (this->State == GAME_ACTIVE || this->State == GAME_WIN || this->State == GAME_LOSE || this->State == GAME_PAUSE)
        {
            glm::vec2 shakeOffset = this->ViewOffset;
            
            glm::vec2 shadowOffset(3.0f, 3.0f);
            glm::vec3 shadowColor(0.1f, 0.1f, 0.1f);

            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (!box.Destroyed)
                    Renderer->DrawSprite(box.Sprite, box.Position + shadowOffset + shakeOffset, box.Size, box.Rotation, shadowColor);
            }
            
            Renderer->DrawSprite(Player->Sprite, Player->Position + shadowOffset + shakeOffset, Player->Size, Player->Rotation, shadowColor);
            
            Renderer->DrawSprite(Ball->Sprite, Ball->Position + shadowOffset + shakeOffset, glm::vec2(Ball->Radius * 2.0f), 0.0f, shadowColor);

            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (!box.Destroyed)
                    Renderer->DrawSprite(box.Sprite, box.Position + shakeOffset, box.Size, box.Rotation, box.Color);
            }
            Renderer->DrawSprite(Player->Sprite, Player->Position + shakeOffset, Player->Size, Player->Rotation, Player->Color);
            Particles->Draw();
            Renderer->DrawSprite(Ball->Sprite, Ball->Position + shakeOffset, glm::vec2(Ball->Radius * 2.0f), 0.0f, Ball->Color);
            
            if (this->State == GAME_ACTIVE || this->State == GAME_PAUSE) {
                 float heartSize = 28.0f;
                 for (unsigned int i = 0; i < this->Lives; i++)
                 {
                     Renderer->DrawSprite(ResourceManager::GetTexture("heart"), 
                         glm::vec2(5.0f + i * (heartSize + 5.0f), 5.0f), 
                         glm::vec2(heartSize), 0.0f);
                 }
                 Text->RenderText("Level: " + std::to_string(this->Level + 1), this->Width - 120.0f, 5.0f, 1.0f, COLOR_TEXT_NORMAL);
            }
        }
        
        if (this->State == GAME_MENU)
        {
            Renderer->DrawSprite(ResourceManager::GetTexture("chibi_main"), 
                glm::vec2(this->Width - 180.0f, this->Height / 2.0f - 80.0f), 
                glm::vec2(160.0f, 160.0f), 0.0f);
            
            Text->RenderText("BREAKOUT", this->Width / 2.0f - 100.0f, 120.0f, 2.0f, COLOR_ACCENT);

            float btnX = this->Width / 2.0f - 80.0f;
            float btnY = this->Height / 2.0f - 20.0f;
            float btnW = 160.0f;
            float btnH = 40.0f;
            float btnSpacing = 50.0f;
            
            for (int i = 0; i < 3; i++)
            {
                float y = btnY + i * btnSpacing;
                bool selected = (this->MenuSelection == (unsigned int)i);
                glm::vec2 uvMin = selected ? glm::vec2(0.0f, 0.5f) : glm::vec2(0.0f, 0.0f);
                glm::vec2 uvMax = selected ? glm::vec2(1.0f, 1.0f) : glm::vec2(1.0f, 0.5f);
                Renderer->DrawSpriteAtlas(ResourceManager::GetTexture("button"), 
                    glm::vec2(btnX - 15.0f, y - 5.0f), 
                    glm::vec2(btnW, btnH), uvMin, uvMax);
            }
            
            Text->RenderText("Start Game", btnX, btnY + 8.0f, 0.8f, this->MenuSelection == 0 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            Text->RenderText("Select Level", btnX, btnY + btnSpacing + 8.0f, 0.8f, this->MenuSelection == 1 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            Text->RenderText("Exit", btnX, btnY + btnSpacing * 2 + 8.0f, 0.8f, this->MenuSelection == 2 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            
            Text->RenderText("Arrow Keys / Mouse to navigate, Enter / Click to select", 120.0f, this->Height - 40.0f, 0.5f, COLOR_UI_OUTLINE);
        }

        if (this->State == GAME_LEVEL_SELECT)
        {
            Text->RenderText("SELECT LEVEL", this->Width / 2.0f - 120.0f, 100.0f, 1.5f, COLOR_ACCENT);
            
            float btnX = this->Width / 2.0f - 80.0f;
            float btnY = this->Height / 2.0f - 100.0f;
            float btnW = 160.0f;
            float btnH = 40.0f;
            float btnSpacing = 50.0f;
            
            for (int i = 0; i < 5; i++)
            {
                float y = btnY + i * btnSpacing;
                bool selected = (this->MenuSelection == (unsigned int)i);
                glm::vec2 uvMin = selected ? glm::vec2(0.0f, 0.5f) : glm::vec2(0.0f, 0.0f);
                glm::vec2 uvMax = selected ? glm::vec2(1.0f, 1.0f) : glm::vec2(1.0f, 0.5f);
                Renderer->DrawSpriteAtlas(ResourceManager::GetTexture("button"), 
                    glm::vec2(btnX - 15.0f, y - 5.0f), 
                    glm::vec2(btnW, btnH), uvMin, uvMax);
            }
            
            Text->RenderText("Level 1", btnX, btnY + 8.0f, 0.8f, this->MenuSelection == 0 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            Text->RenderText("Level 2", btnX, btnY + btnSpacing + 8.0f, 0.8f, this->MenuSelection == 1 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            Text->RenderText("Level 3", btnX, btnY + btnSpacing * 2 + 8.0f, 0.8f, this->MenuSelection == 2 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            Text->RenderText("Level 4", btnX, btnY + btnSpacing * 3 + 8.0f, 0.8f, this->MenuSelection == 3 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            Text->RenderText("Back", btnX, btnY + btnSpacing * 4 + 8.0f, 0.8f, this->MenuSelection == 4 ? COLOR_TEXT_SELECTED : COLOR_TEXT_NORMAL);
            
            Text->RenderText("ESC to go back", this->Width / 2.0f - 70.0f, this->Height - 40.0f, 0.5f, COLOR_UI_OUTLINE);
        }

        if (this->State == GAME_WIN)
        {
            Renderer->DrawSprite(ResourceManager::GetTexture("chibi_main"), 
                glm::vec2(this->Width / 2.0f - 60.0f, this->Height / 2.0f - 120.0f), 
                glm::vec2(120.0f, 120.0f), 0.0f);
            Text->RenderText("YOU WON!", this->Width / 2.0f - 80.0f, Height / 2.0f + 20.0f, 1.5f, COLOR_HIGHLIGHT);
            Text->RenderText("Press ENTER to return to Menu", 130.0f, Height / 2.0f + 70.0f, 1.0f, COLOR_TEXT_NORMAL);
        }
        
        if (this->State == GAME_LOSE)
        {
            Renderer->DrawSprite(ResourceManager::GetTexture("chibi_sad"), 
                glm::vec2(this->Width / 2.0f - 60.0f, this->Height / 2.0f - 120.0f), 
                glm::vec2(120.0f, 120.0f), 0.0f);
            Text->RenderText("GAME OVER", this->Width / 2.0f - 90.0f, this->Height / 2.0f + 20.0f, 1.5f, COLOR_ACCENT);
            Text->RenderText("Press ENTER to return to Menu", 130.0f, this->Height / 2.0f + 70.0f, 1.0f, COLOR_TEXT_NORMAL);
        }
        
        if (this->State == GAME_PAUSE)
        {
            Text->RenderText("PAUSED", this->Width / 2.0f - 70.0f, this->Height / 2.0f - 40.0f, 2.0f, COLOR_ACCENT);
            Text->RenderText("ESC to Resume", this->Width / 2.0f - 70.0f, this->Height / 2.0f + 20.0f, 0.8f, COLOR_TEXT_NORMAL);
            Text->RenderText("ENTER to Quit to Menu", this->Width / 2.0f - 100.0f, this->Height / 2.0f + 50.0f, 0.8f, COLOR_TEXT_NORMAL);
        }
    }

    void DoCollisions()
    {
        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if (!box.Destroyed)
            {
                Collision collision = CheckCollision(*Ball, box);
                if (std::get<0>(collision))
                {
                    if (!box.IsSolid) {
                        box.Destroyed = true;
                        Audio->Play("assets/audio/bleep.wav");
                    } else {
                        this->TriggerShake(0.05f);
                        Audio->Play("assets/audio/solid.wav");
                    }
                    
                    Direction dir = std::get<1>(collision);
                    glm::vec2 diff_vector = std::get<2>(collision);
                    if (dir == LEFT || dir == RIGHT)
                    {
                        Ball->Velocity.x = -Ball->Velocity.x;
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration;
                        else
                            Ball->Position.x -= penetration;
                    }
                    else
                    {
                        Ball->Velocity.y = -Ball->Velocity.y;
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration;
                        else
                            Ball->Position.y += penetration;
                    }				
                }
            }
        }
        
        Collision result = CheckCollision(*Ball, *Player);
        if (!Ball->Stuck && std::get<0>(result))
        {
            float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
            float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
            float percentage = distance / (Player->Size.x / 2.0f);
            float strength = 2.0f;
            glm::vec2 oldVelocity = Ball->Velocity;
            Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
            Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);
            Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);

            Audio->Play("assets/audio/paddle_hit.wav");
        }
    }

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
        Player->Size = PLAYER_SIZE;
        Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
    }

private:
    bool CheckCollision(GameObject &one, GameObject &two)
    {
        bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
            two.Position.x + two.Size.x >= one.Position.x;
        bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
            two.Position.y + two.Size.y >= one.Position.y;
        return collisionX && collisionY;
    }

    Collision CheckCollision(BallObject &one, GameObject &two)
    {
        glm::vec2 center(one.Position + one.Radius);
        glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
        glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
        glm::vec2 difference = center - aabb_center;
        glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
        glm::vec2 closest = aabb_center + clamped;
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
