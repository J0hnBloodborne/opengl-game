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
    // resource storage
    static std::map<std::string, Shader> Shaders;
    static std::map<std::string, Texture> Textures;
    
    // loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
    static Shader LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name)
    {
        Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
        return Shaders[name];
    }
    
    // retrieves a stored sader
    static Shader GetShader(std::string name)
    {
        return Shaders[name];
    }
    
    // loads (and generates) a texture from file
    static Texture LoadTexture(const char* file, bool alpha, std::string name)
    {
        Textures[name] = loadTextureFromFile(file, alpha);
        return Textures[name];
    }
    
    // retrieves a stored texture
    static Texture GetTexture(std::string name)
    {
        return Textures[name];
    }
    
    // properly de-allocates all loaded resources
    static void Clear()
    {
        // (properly delete all shaders/textures)
        for (auto iter : Shaders)
            glDeleteProgram(iter.second.ID);
        for (auto iter : Textures)
            glDeleteTextures(1, &iter.second.ID);
    }

private:
    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager() { }
    
    // loads and generates a shader from file
    static Shader loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr)
    {
        // The Shader class constructor already handles file loading, 
        // but existing tutorial code creates Shader from strings usually.
        // However, our current Shader class (in shader.ixx) takes paths!
        // So we can simply delegate to the Shader constructor.
        
        Shader shader(vShaderFile, fShaderFile, gShaderFile);
        return shader;

        // Note: The tutorial splits logic: read file -> compile. 
        // Our Shader constructor does read file -> compile.
        // This is fine.
    }
    
    // loads a single texture from file
    static Texture loadTextureFromFile(const char* file, bool alpha)
    {
        // create texture object
        Texture texture;
        if (alpha)
        {
            texture.Internal_Format = GL_RGBA;
            texture.Image_Format = GL_RGBA;
        }
        
        // load image
        int width, height, nrChannels;
        unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
        // now generate texture
        texture.Generate(width, height, data);
        // and finally free image data
        stbi_image_free(data);
        return texture;
    }
};

// Define static members
std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture> ResourceManager::Textures;
