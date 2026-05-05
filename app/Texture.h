#pragma once

#include <SDL3/SDL.h>

#include <string>

class Texture final
{
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = delete;

    bool loadFromFile(const std::string& path, SDL_Renderer* renderer);
    SDL_Point getSize() const { return _size; }

    operator SDL_Texture*() { return _texture; }

private:
    SDL_Texture* _texture;
    SDL_Point    _size;
};
