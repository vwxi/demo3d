// this file describes a wrapper for textures

#pragma once

#include "util.hpp"

namespace demo {

class GWindow;

class GTexture {
public:
    GTexture(SDL_PixelFormat* p_fmt, std::string filename) {
        SDL_Surface* t_surface = IMG_Load(filename.c_str());

        std::cout << IMG_GetError() << std::endl;

        if(!t_surface)
            throw std::runtime_error("could not load texture");

        surface = SDL_ConvertSurface(t_surface, p_fmt, 0);
        if(!surface) {
            SDL_FreeSurface(t_surface);
            throw std::runtime_error("could not convert texture surface");
        }

        SDL_FreeSurface(t_surface);

        width = surface->w;
        height = surface->h;
    }

    GTexture(GTexture&&) = default;
    GTexture& operator=(GTexture&&) = default;
    GTexture(const GTexture&) = delete;
    GTexture& operator=(const GTexture&) = delete;

    ~GTexture() {
        if(surface)
            SDL_FreeSurface(surface);
    }

    GRgba pixel(int x, int y) {
        std::uint8_t* ptr = (std::uint8_t*)surface->pixels + 
            y * surface->pitch + x * surface->format->BytesPerPixel;
        GRgba col { 0 };

        SDL_GetRGBA(*(std::uint32_t*)ptr, surface->format, &col.r, &col.g, &col.b, &col.a);

        return col;
    }

    SDL_Surface* get_surface_ptr() const {
        return surface;
    }

    int width;
    int height;

private:
    SDL_Surface* surface;
};

}