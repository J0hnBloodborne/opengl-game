module;

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module sprite_renderer;

import shader;
import texture;

export class SpriteRenderer
{
public:
    SpriteRenderer(Shader shader)
    {
        this->shader = shader;
        this->initRenderData();
    }
    
    ~SpriteRenderer()
    {
        glDeleteVertexArrays(1, &this->quadVAO);
        glDeleteVertexArrays(1, &this->atlasVAO);
        glDeleteBuffers(1, &this->atlasVBO);
    }
    
    void DrawSprite(const Texture& texture, glm::vec2 position, glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f))
    {
        this->shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));

        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
        model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

        model = glm::scale(model, glm::vec3(size, 1.0f));
  
        this->shader.setMatrix4("model", model);
        this->shader.setVector3f("spriteColor", color);
        glActiveTexture(GL_TEXTURE0);
        texture.Bind();

        glBindVertexArray(this->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    
    void DrawSpriteAtlas(const Texture& texture, glm::vec2 position, glm::vec2 size, 
                         glm::vec2 uvMin, glm::vec2 uvMax, glm::vec3 color = glm::vec3(1.0f))
    {
        this->shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));
        this->shader.setMatrix4("model", model);
        this->shader.setVector3f("spriteColor", color);
        
        float vertices[] = { 
            0.0f, 1.0f, uvMin.x, uvMax.y,
            1.0f, 0.0f, uvMax.x, uvMin.y,
            0.0f, 0.0f, uvMin.x, uvMin.y, 
        
            0.0f, 1.0f, uvMin.x, uvMax.y,
            1.0f, 1.0f, uvMax.x, uvMax.y,
            1.0f, 0.0f, uvMax.x, uvMin.y
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, this->atlasVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        
        glActiveTexture(GL_TEXTURE0);
        texture.Bind();

        glBindVertexArray(this->atlasVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

private:
    Shader shader; 
    unsigned int quadVAO;
    unsigned int atlasVAO;
    unsigned int atlasVBO;
    
    void initRenderData()
    {
        unsigned int VBO;
        float vertices[] = { 
            // pos    // tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 
        
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &this->quadVAO);
        glGenBuffers(1, &VBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(this->quadVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        glGenVertexArrays(1, &this->atlasVAO);
        glGenBuffers(1, &this->atlasVBO);
        
        glBindBuffer(GL_ARRAY_BUFFER, this->atlasVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        
        glBindVertexArray(this->atlasVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};
