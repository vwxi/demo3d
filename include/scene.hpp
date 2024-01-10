// this file describes a scene class
// the scene class is an abstract wrapper for the window class. it controls the window's
// rendering behavior

#pragma once

#include "util.hpp"

namespace demo {

class GWindow;

class GScene {
public:
	GScene() { }

	virtual void process(const SDL_Event&) { }
	virtual void draw() { }
};

}