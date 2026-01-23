module;

#include <glad/glad.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module resource_manager;

import shader;
import texture;

export class ResourceManager
{
public:
    static std::map<std::string, Shader> Shaders;
    static std::map<std::string, Texture> Textures;
    
    static Shader LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name)
    {
        Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
        return Shaders[name];
    }
    
    static Shader GetShader(std::string name)
    {
        return Shaders[name];
    }
    
    static Texture LoadTexture(const char* file, bool alpha, std::string name)
    {
        Textures[name] = loadTextureFromFile(file, alpha);
        return Textures[name];
    }
    
    static Texture GetTexture(std::string name)
    {
        return Textures[name];
    }
    
    static void Clear()
    {
        for (auto iter : Shaders)
            glDeleteProgram(iter.second.ID);
        for (auto iter : Textures)
            glDeleteTextures(1, &iter.second.ID);
    }

private:
    ResourceManager() { }
    
    static Shader loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr)
    {
        Shader shader(vShaderFile, fShaderFile, gShaderFile);
        return shader;
    }
    
    static Texture loadTextureFromFile(const char* file, bool alpha)
    {
        Texture texture;
        
        int width, height, nrChannels;
        unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
        
        if (data)
        {
            if (nrChannels == 4)
            {
                texture.Internal_Format = GL_RGBA;
                texture.Image_Format = GL_RGBA;
            }
            else if (nrChannels == 3)
            {
                texture.Internal_Format = GL_RGB;
                texture.Image_Format = GL_RGB;
            }
            else if (nrChannels == 1)
            {
                texture.Internal_Format = GL_RED;
                texture.Image_Format = GL_RED;
            }
            
            texture.Generate(width, height, data);
        }
        else
        {
            std::cout << "Failed to load texture: " << file << std::endl;
        }
        
        stbi_image_free(data);
        return texture;
    }
};

std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture> ResourceManager::Textures;
