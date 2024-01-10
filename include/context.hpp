// this file describes a context class and multiple shader classes
// a context class contains different shaders that are applied to a pipeline
// the vertex shader modifies the vertex data passed into a pipeline (vertex->VS->vertex)
// the geometry shader modifies triangles passed into a pipeline (triangle->GS->triangle)
// the pixel shader modifies the pixels within a triangle passed into a pipeline (pixel->PS->pixel)

#pragma once

#include "util.hpp"
#include "window.hpp"

namespace demo {

template <class Input, class Output>
class GShader {
public:
	typedef Input InputType;
	typedef Output OutputType;

	GShader(GWindow& w) : window(w) { }

	virtual Output operator()(const Input&) = 0;

	GWindow& window;
};

class GDefaultVertexShader : public GShader<IVertex, IVertex> {
public:
	GDefaultVertexShader(GWindow& win) : GShader(win) { }

	OutputType operator()(const InputType& v) {
		return v;
	}
};

class GDefaultGeometryShader : public GShader<GTriangle<IVertex>, GTriangle<IVertex>> {
public:
	GDefaultGeometryShader(GWindow& win) : GShader(win) { }
	
	OutputType operator()(const InputType& tri) {
		return tri;
	}
};

class GDefaultFragmentShader : public GShader<IVertex, GRgba> {
public:
	GDefaultFragmentShader(GWindow& win) : GShader(win) { }
	
	OutputType operator()(const InputType& v) {
		return OutputType{};
	}
};

template <class VertexShader, class GeometryShader, class FragmentShader>
class GContext {
public:
	GContext(GWindow& w) :
		vertex_shader(w),
		geometry_shader(w),
		fragment_shader(w) { }

	typedef typename VertexShader::InputType VInputType;
	typedef typename GeometryShader::InputType GInputType;
	typedef typename FragmentShader::InputType FInputType;
	typedef typename VertexShader::OutputType VOutputType;
	typedef typename GeometryShader::OutputType GOutputType;
	typedef typename FragmentShader::OutputType FOutputType;

	VertexShader vertex_shader;
	GeometryShader geometry_shader;
	FragmentShader fragment_shader;
};

typedef GContext< 
	GDefaultVertexShader, 
	GDefaultGeometryShader,
	GDefaultFragmentShader> GDefaultContext;

}