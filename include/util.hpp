#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <cstring>
#include <optional>
#include <cstdint>
#include <deque>
#include <sstream>
#include <fstream>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_PRECISION_LOWP_FLOAT

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/compatibility.hpp>

namespace demo {

using namespace glm;

template <typename Out>
static void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

static std::vector<std::string> split_string(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

template <typename T>
vec3 barycentric(const T& p, const T& a, const T& b, const T& c) {
	T v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = dot(v0, v0),
		d01 = dot(v0, v1),
		d11 = dot(v1, v1),
		d20 = dot(v2, v0),
		d21 = dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;

	float v = (d11 * d20 - d01 * d21) / denom,
		w = (d00 * d21 - d01 * d20) / denom;
	
	return vec3(1.0f - v - w, v, w);
}

static float b_interpolate(vec3 bary, vec3 values) {
	return bary.x * values.x + bary.y * values.y + bary.z * values.z;
}

template <typename T>
static T l_interpolate(const T& a, const T& b, float alpha) {
	return (1 - alpha) * a + alpha * b;
}

struct GPlane {
	float a, b, c, d;
};

class GFrustum {
public:
	GFrustum() { }

	void update(const mat4x4& mat) {
		// left
		planes[0].a = mat[3][0] + mat[0][0];
		planes[0].b = mat[3][1] + mat[0][1];
		planes[0].c = mat[3][2] + mat[0][2];
		planes[0].d = mat[3][3] + mat[0][3];

		// right
		planes[1].a = mat[3][0] - mat[0][0];
		planes[1].b = mat[3][1] - mat[0][1];
		planes[1].c = mat[3][2] - mat[0][2];
		planes[1].d = mat[3][3] - mat[0][3];
		
		// top
		planes[2].a = mat[3][0] - mat[1][0];
		planes[2].b = mat[3][1] - mat[1][1];
		planes[2].c = mat[3][2] - mat[1][2];
		planes[2].d = mat[3][3] - mat[1][3];
		
		// bottom
		planes[3].a = mat[3][0] + mat[1][0];
		planes[3].b = mat[3][1] + mat[1][1];
		planes[3].c = mat[3][2] + mat[1][2];
		planes[3].d = mat[3][3] + mat[1][3];
		
		// near
		planes[4].a = mat[3][0] + mat[2][0];
		planes[4].b = mat[3][1] + mat[2][1];
		planes[4].c = mat[3][2] + mat[2][2];
		planes[4].d = mat[3][3] + mat[2][3];
		
		// far
		planes[5].a = mat[3][0] - mat[2][0];
		planes[5].b = mat[3][1] - mat[2][1];
		planes[5].c = mat[3][2] - mat[2][2];
		planes[5].d = mat[3][3] - mat[2][3];
	}

	GPlane planes[6];
};

struct IVertex {
	IVertex() { }

	// interpolate attributes with barycentric coordinates
	IVertex(vec3, IVertex, IVertex, IVertex, float);
	IVertex operator*(float);
	IVertex operator/(float);

	std::string to_string();

	// barycentric interpolate attributes 
	void berp(vec3, IVertex, IVertex, IVertex);
	
	// linear interpolate attributes
	void lerp(IVertex, IVertex, float);
};

template <typename T>
struct GMesh {
	GMesh() { }

	GMesh(std::vector<T> vs, std::vector<size_t> is) :
		vertices(vs), 
		indices(is) { 
		assert(vertices.size() > 2);
		assert(indices.size() % 3 == 0);
	}

	std::vector<T> vertices;
	std::vector<size_t> indices;
};

template <typename T>
struct GTriangle {
	GTriangle(T x, T y, T z) : a(x), b(y), c(z) { }
	T a, b, c;
};

struct GRgba {
	std::uint8_t r, g, b, a;
};

}
