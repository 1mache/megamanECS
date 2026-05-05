#include "Texture.h"

#include <SDL3_image/SDL_image.h>

#include <cassert>
#include <iostream>

Texture::Texture()
    : _texture(nullptr), _size{}
{
}

Texture::~Texture()
{
    if (_texture)
        SDL_DestroyTexture(_texture);
}

bool Texture::loadFromFile(const std::string& path, SDL_Renderer* renderer)
{
    assert(renderer != nullptr);
    assert(!path.empty());

    _texture = IMG_LoadTexture(renderer, path.c_str());
    if (!_texture)
    {
        std::cerr << "IMG_LoadTexture failed for " << path << ": " << SDL_GetError() << "\n";
        return false;
    }

    float w{}, h{};
    SDL_GetTextureSize(_texture, &w, &h);
    _size = { static_cast<int>(w), static_cast<int>(h) };

    return true;
}
