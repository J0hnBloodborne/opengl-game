module;

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

export module particle_generator;

import shader;
import texture;
import game_object;

struct Particle {
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    float     Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

export class ParticleGenerator
{
public:
    ParticleGenerator(Shader shader, Texture texture, unsigned int amount)
        : shader(shader), texture(texture), amount(amount)
    {
        this->init();
    }
    
    void Update(float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f))
    {
        for (unsigned int i = 0; i < newParticles; ++i)
        {
            int unusedParticle = this->firstUnusedParticle();
            this->respawnParticle(this->particles[unusedParticle], object, offset);
        }
        for (unsigned int i = 0; i < this->amount; ++i)
        {
            Particle &p = this->particles[i];
            p.Life -= dt;
            if (p.Life > 0.0f)
            {
                p.Position -= p.Velocity * dt; 
                p.Color.a -= dt * 2.5f;
            }
        }
    }
    
    void Draw()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        this->shader.use();
        for (Particle particle : this->particles)
        {
            if (particle.Life > 0.0f)
            {
                this->shader.setVector2f("offset", particle.Position);
                this->shader.setVector4f("color", particle.Color);
                this->texture.Bind();
                glBindVertexArray(this->VAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            }
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

private:
    std::vector<Particle> particles;
    unsigned int amount;
    Shader shader;
    Texture texture;
    unsigned int VAO;
    unsigned int lastUsedParticle = 0;

    void init()
    {
        unsigned int VBO;
        float particle_quad[] = {
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
    
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        }; 
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    
        for (unsigned int i = 0; i < this->amount; ++i)
            this->particles.push_back(Particle());
    }

    unsigned int firstUnusedParticle()
    {
        for (unsigned int i = lastUsedParticle; i < this->amount; ++i){
            if (this->particles[i].Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
        for (unsigned int i = 0; i < lastUsedParticle; ++i){
            if (this->particles[i].Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
        lastUsedParticle = 0;
        return 0;
    }

    void respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f))
    {
        float random = ((rand() % 100) - 50) / 10.0f;
        float rColor = 0.5f + ((rand() % 100) / 100.0f);
        particle.Position = object.Position + random + offset;
        
        glm::vec3 colors[] = {
            glm::vec3(1.0f, 0.41f, 0.71f),
            glm::vec3(0.0f, 1.0f, 1.0f),
            glm::vec3(0.2f, 1.0f, 0.2f)
        };
        int index = rand() % 3;
        particle.Color = glm::vec4(colors[index], 1.0f);
        
        particle.Life = 1.0f;
        particle.Velocity = object.Velocity * 0.1f;
    }
};
