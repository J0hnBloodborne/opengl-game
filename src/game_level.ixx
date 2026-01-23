module;

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

export module game_level;

import game_object;
import sprite_renderer;
import resource_manager;

export class GameLevel
{
public:
    std::vector<GameObject> Bricks;

    GameLevel() { }

    void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight)
    {
        this->Bricks.clear();
        unsigned int tileCode;
        GameLevel level;
        std::string line;
        std::ifstream fstream(file);
        std::vector<std::vector<unsigned int>> tileData;
        if (fstream)
        {
            while (std::getline(fstream, line))
            {
                std::istringstream sstream(line);
                std::vector<unsigned int> row;
                while (sstream >> tileCode)
                    row.push_back(tileCode);
                tileData.push_back(row);
            }
            if (tileData.size() > 0)
                this->init(tileData, levelWidth, levelHeight);
        }
    }
    void Draw(SpriteRenderer& renderer)
    {
        for (GameObject& tile : this->Bricks)
            if (!tile.Destroyed)
                tile.Draw(renderer);
    }
    bool IsCompleted()
    {
        for (GameObject& tile : this->Bricks)
            if (!tile.IsSolid && !tile.Destroyed)
                return false;
        return true;
    }

private:
    void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
    {
        unsigned int height = tileData.size();
        unsigned int width = tileData[0].size();
        float unit_width = levelWidth / static_cast<float>(width);
        float unit_height = levelHeight / height; 
        
        for (unsigned int y = 0; y < height; ++y)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                if (tileData[y][x] == 1)
                {
                    glm::vec2 pos(unit_width * x, unit_height * y);
                    glm::vec2 size(unit_width, unit_height);
                    GameObject obj(pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
                    obj.IsSolid = true;
                    this->Bricks.push_back(obj);
                }
                else if (tileData[y][x] > 1)
                {
                    glm::vec3 color = glm::vec3(1.0f);
                    if (tileData[y][x] == 2)
                        color = glm::vec3(1.0f, 0.8f, 0.8f);
                    else if (tileData[y][x] == 3)
                        color = glm::vec3(0.6f, 0.9f, 0.7f);
                    else if (tileData[y][x] == 4)
                        color = glm::vec3(0.7f, 0.85f, 1.0f);
                    else if (tileData[y][x] == 5)
                        color = glm::vec3(1.0f, 0.9f, 0.6f);

                    glm::vec2 pos(unit_width * x, unit_height * y);
                    glm::vec2 size(unit_width, unit_height);
                    this->Bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("block"), color));
                }
            }
        }
    }
};
