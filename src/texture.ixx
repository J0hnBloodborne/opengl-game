module;

#include <glad/glad.h>
#include <iostream>

export module texture;

export class Texture {
public:
    unsigned int ID;
    unsigned int width, height; // Width and height of loaded image in pixels
    unsigned int Internal_Format; // Format of texture object
    unsigned int Image_Format; // Format of loaded image
    unsigned int Wrap_S; // Wrapping mode on S axis
    unsigned int Wrap_T; // Wrapping mode on T axis
    unsigned int Filter_Min; // Filtering mode if texture pixels < screen pixels
    unsigned int Filter_Max; // Filtering mode if texture pixels > screen pixels

    Texture() // Default constructor defaults to Nearest Neighbor for Pixel Art look
        : width(0), height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_NEAREST), Filter_Max(GL_NEAREST)
    {
        glGenTextures(1, &ID);
    }

    void Generate(unsigned int width, unsigned int height, unsigned char* data)
    {
        this->width = width;
        this->height = height;
        // Create Texture
        glBindTexture(GL_TEXTURE_2D, this->ID);
        glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
        // Set Texture wrap and filter modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Bind() const {
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};
