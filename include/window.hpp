// this file describes the window class
// the window class is a wrapper for the SDL library and is controlled by a scene class

#pragma once

#include "util.hpp"
#include "scene.hpp"
#include "texture.hpp"

namespace demo {

class GFont {
public:
	void init(SDL_Renderer* rend, std::string filename, int pt) {
		texture = NULL;
		renderer = rend;

		if(!renderer)
			throw std::runtime_error("fed font bad renderer ptr");

		font = TTF_OpenFont(filename.c_str(), pt);
		if(!font)
			throw std::runtime_error("failed to initialize font");
	}

	void destroy() {
		if(texture) {
			SDL_DestroyTexture(texture);
			texture = NULL;
		}

		if(font) {
			TTF_CloseFont(font);
			font = NULL;
		}
	}

	void draw_to_screen(int x, int y, SDL_Color color, std::string text) {
		if(texture) {
			SDL_DestroyTexture(texture);
			texture = NULL;
		}

		if(!font)
			throw std::runtime_error("font is not loaded");

		SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
		
		if(!surface)
			throw std::runtime_error("could not render text");

		texture = SDL_CreateTextureFromSurface(renderer, surface);
		if(!texture)
			throw std::runtime_error("could not create text surface");

		SDL_Rect rect = {
			.x = x,
			.y = y,
			.w = surface->w,
			.h = surface->h
		};

		SDL_FreeSurface(surface);

		SDL_RenderCopy(renderer, texture, NULL, &rect);
	}

private:
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	TTF_Font* font;
};

class GWindow {
public:
	GWindow(std::string title, int W, int H, int flags) : 
		width(W), height(H), quit(false) {
		SDL_Init(SDL_INIT_EVERYTHING);

		window = SDL_CreateWindow(
			title.c_str(), 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			width, 
			height, 
			flags
		);

		if(!window)
			throw std::runtime_error("could not create SDL window");

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);

		if(!renderer)
			throw std::runtime_error("could not create SDL renderer");

		if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
			throw std::runtime_error("could not initialize SDL_image");

		if(TTF_Init() == -1)
			throw std::runtime_error("could not initialize SDL_ttf");

		depth_buffer = new float[width*height];

		if(!depth_buffer)
			throw std::runtime_error("could not allocate depth buffer");

		font.init(renderer, "../assets/Hack-Bold.ttf", 18);
	}

	~GWindow() {
		font.destroy();

		if(depth_buffer != NULL)
			delete[] depth_buffer;

		if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0) {
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);

			TTF_Quit();
			IMG_Quit();
			SDL_Quit();
		}
	}

	void register_scene(GScene* scene_) {
		scene = scene_;
	}

	void put_pixel(int x, int y, GRgba c) {
		SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
		SDL_RenderDrawPoint(renderer, x, y);
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		clear_depth_buffer();
	}

	void print(int x, int y, std::string text) {
		font.draw_to_screen(x, y, font_color, text);
	}

	void run() {
		clear();

		while(!quit) {
			u32 first = SDL_GetTicks(), delta;

			while(SDL_PollEvent(&event)) {
				scene->process(event);
			}

			scene->draw();

			delta = SDL_GetTicks() - first;

			{
				std::stringstream ss;
				ss << (int)(1000.0f / delta);
				print(0, 0, ss.str());
			}

			SDL_RenderPresent(renderer);
		}
	}

	SDL_PixelFormat* get_window_pixel_format() {
		return SDL_GetWindowSurface(window)->format;
	}

	float* get_depth_buffer() {
		return depth_buffer;
	}

	void clear_depth_buffer() {
		if(depth_buffer != NULL) {
			for(int i = 0; i < width * height - 1; i++)
				depth_buffer[i] = INFINITY;
		}
	}

	bool test_set_depth_buffer(int x, int y, float depth) {
		if(x < 0 || y < 0 || x >= width || y >= height)
			return false;

		float* d = &depth_buffer[width * y + x];
		
		if(depth < *d) {
			*d = depth;
			return true;
		}
		
		return false;
	}

public:
	int width;
	int height;
	bool quit;

	SDL_Color font_color{255,255,255,255};

private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;
	GFont font;

	GScene* scene;
	float* depth_buffer;
};

}