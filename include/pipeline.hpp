// this file describes the pipeline class
// the pipeline class describes the rendering pipeline for triangles

#pragma once

#include "util.hpp"
#include "window.hpp"
#include "context.hpp"

namespace demo {

using namespace glm;

template <class VertexShader, class GeometryShader, class FragmentShader>
class GContext;

/*
 * vertex transformer -> vertex shader -> triangle assembler -> 
 * triangle clipper -> geometry shader -> persp/screen transformer -> 
 * triangle rasterizer -> fragment shader -> put pixel
 */
template <class Context>
class GPipeline {
public:
	typedef typename Context::VInputType VInputType;
	typedef typename Context::GInputType GInputType;
	typedef typename Context::FInputType FInputType;
	typedef typename Context::VOutputType VOutputType;
	typedef typename Context::GOutputType GOutputType;
	typedef typename Context::FOutputType FOutputType;

	GPipeline(GWindow& win) : window(win), context(win) { }

	// start pipeline
	template <typename T>
	void process(GTriangleList<T> tri) {
		std::vector<VOutputType> tri_transformed;

		for(auto v : tri.vertices) {
			tri_transformed.emplace_back(context.vertex_shader(v));
		}

		assemble_triangles(tri_transformed, tri.indices);
	}

private:
	// build triangles, culls back facing triangles
	void assemble_triangles(std::vector<VOutputType>& vertices, std::vector<size_t>& indices) {
		for(size_t idx = 0; idx < indices.size(); idx += 3) {
			const VOutputType& v0 = vertices[indices[idx]],
				v1 = vertices[indices[idx + 1]],
				v2 = vertices[indices[idx + 2]];
		
			// vec4 -> vec3 for back-face culling
			vec3 vv0(v0.pos), vv1(v1.pos), vv2(v2.pos);

			// back-face culling
			if((dot(normalize(-vv0), cross((vv1 - vv0), (vv2 - vv0)))) >= 0)
				continue;

			// view frustum culling

			// frustum inequalities
			if((v0.pos.x > v0.pos.w && v1.pos.x > v1.pos.w && v2.pos.x > v2.pos.w) ||
				(v0.pos.x < -v0.pos.w && v1.pos.x < -v1.pos.w && v2.pos.x < -v2.pos.w)) continue;
			if((v0.pos.y > v0.pos.w && v1.pos.y > v1.pos.w && v2.pos.y > v2.pos.w) ||
				(v0.pos.y < -v0.pos.w && v1.pos.y < -v1.pos.w && v2.pos.y < -v2.pos.w)) continue;
			if((v0.pos.z > v0.pos.w && v1.pos.z > v1.pos.w && v2.pos.z > v2.pos.w) || 
				(v0.pos.z < -v0.pos.w && v1.pos.z < -v1.pos.w && v2.pos.z < -v2.pos.w)) continue;
			
			// behind camera
			if(v0.pos.z < 0 && v1.pos.z < 0 && v2.pos.z < 0) continue;

			clip_triangle(v0, v1, v2);
		}
	}

	// clip and generate triangles

	static float plane(int i, vec4 v) {
		switch(i) {
		case 0: return v.w + v.x; // left
		case 1: return v.w - v.x; // right
		case 2: return v.w - v.y; // top
		case 3: return v.w + v.y; // bottom
		case 4: return v.z; // near
		case 5: return v.w - v.z; // far
		default: return 0;
		}
	}

	static u8 out_code(vec4 v) {
		u8 code = 0;
		float pl;

		for(int i = 0; i < 6; i++) {
			if(plane(i, v) < 0)
				code |= (1 << i);
		}

		return code;
	}

	// modified cohen-sutherland
	void clip_triangle(const VOutputType& v0, const VOutputType& v1, const VOutputType& v2) {
		std::deque<VOutputType> in_vertices = { v1, v2, v0 };
		std::vector<VOutputType> out_vertices;

		VOutputType last;
		u8 new_code = 0, old_code = 0, mask = 0;
		float old_alpha = 0, new_alpha = 0, alpha = 0;
		float t_last = 0, t_current = 0;
		
		last = v0;
		old_code = out_code(last.pos);

		while(!in_vertices.empty()) {
			VOutputType current = *in_vertices.begin();

			in_vertices.pop_front();

			new_code = out_code(current.pos);
			mask = new_code | old_code;

			if(!(new_code & old_code)) {
				if(!mask) {
					// two points inside
					out_vertices.push_back(current);
				} else {
					// we have to clip some planes
					old_alpha = 0;
					new_alpha = 1;
					int i;
					for(i = 0; i < 6; i++) {
						if(mask & (1 << i)) {
							t_last = plane(i, last.pos);
							t_current = plane(i, current.pos);
							alpha = t_last / (t_last - t_current);
						
							if((old_code & mask) && old_alpha < alpha) old_alpha = alpha;
							else if(new_alpha > alpha) new_alpha = alpha;

							if(old_alpha > new_alpha) break;
						}
					}

					if(old_code) {
						VOutputType v;
						v.lerp(last, current, old_alpha);

						out_vertices.push_back(v);
					}

					if(new_code) {
						VOutputType v;
						v.lerp(last, current, new_alpha);
						
						out_vertices.push_back(v);
					} else {
						out_vertices.push_back(current);
					}
				}
			}

			old_code = new_code;
			last = current;
		}

		generate_triangles(out_vertices);
	}

	void generate_triangles(std::vector<VOutputType> vertices) {
		for(int i = 1; i < vertices.size(); i++) {
			if(vertices.size() - i < 2)
				break;

			VOutputType v0 = vertices[0], v1 = vertices[i], v2 = vertices[i+1];
			transform_triangle(context.geometry_shader(GTriangle(v0, v1, v2)));
		}
	}

	template <typename V>
	void transform(V& v) {
		float invw = 1 / v.pos.w;

		// perspective divide
		v = v * invw;
		v.pos.w = invw;

		// screen transform
		v.pos.x = ((v.pos.x + 1) * window.width) / 2;
		v.pos.y = ((-v.pos.y + 1) * window.height) / 2;
	}
	
	// perspective divide and screen transform
	// also transform vertex attributes
	void transform_triangle(GOutputType tri) {
		transform(tri.a);
		transform(tri.b);
		transform(tri.c);

		draw_triangle(tri);
	}

	// rasterize
	// triangle is already in screen space
	void draw_triangle(GOutputType& tri) {
		// get bounding box
		int bb_min_x = std::min(std::min(tri.a.pos.x, tri.b.pos.x), tri.c.pos.x),
			bb_min_y = std::min(std::min(tri.a.pos.y, tri.b.pos.y), tri.c.pos.y),
			bb_max_x = std::max(std::max(tri.a.pos.x, tri.b.pos.x), tri.c.pos.x),
			bb_max_y = std::max(std::max(tri.a.pos.y, tri.b.pos.y), tri.c.pos.y);

		// loop over bounding box
		for(int y = bb_min_y; y <= bb_max_y; y++) {
			for(int x = bb_min_x; x <= bb_max_x; x++) {
				vec2 point(x+0.5f, y+0.5f),
					ta(tri.a.pos), tb(tri.b.pos), tc(tri.c.pos);
				
				// screen space (2D) barycentric
				vec3 s_bary = barycentric(point, ta, tb, tc);
				
				// discard outside fragments
				if(s_bary.x < 0 || s_bary.y < 0 || s_bary.z < 0)
					continue;

				// find interpolated depth at point
				float in_w = 1 / b_interpolate(s_bary, vec3(tri.a.pos.w, tri.b.pos.w, tri.c.pos.w));

				// depth buffer test
				if(window.test_set_depth_buffer(x, y, in_w)) {
					// interpolate vertex attributes
					FInputType input;
					input.berp(s_bary, tri.a, tri.b, tri.c, in_w);

					window.put_pixel(x, y, context.fragment_shader(input));
				}
			}
		}
	}

private:
	GWindow& window;

public:
	Context context;
};


}