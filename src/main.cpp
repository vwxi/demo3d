#include "gfx.hpp"

using namespace demo;

struct Camera {
	vec3 eye;
	vec3 angle;
	vec3 up;

	float pitch;
	float yaw;
	
	float speed = 1.f;

	vec3 light_pos{0.f, 0.f, 5.f};
	vec3 light_diffuse{1.f, 1.f, 1.f};
	vec3 light_ambient{0.1f, 0.1f, 0.1f};
	vec3 light_color{1.0f, 1.0f, 1.0f};

	Camera() :
		eye(0, 0, 0),
		angle(0, 0, -1),
		up(0, 1, 0) { }

	void update() {
		float pitch_rad = radians(pitch), 
            yaw_rad = radians(yaw);

        angle = normalize(vec3(
            cos(yaw_rad) * cos(pitch_rad), 
            sin(pitch_rad), 
            sin(yaw_rad) * cos(pitch_rad)));
	}
} static camera;

class VertShader : public GShader<GObjVertex, GObjVertex> {
public:
	VertShader(GWindow& win) :
		GShader(win),
		aspect_ratio((float)win.width / win.height),
		fov(45),
		far(10000.0f), near(0.1f) { 
		update();
	}

	OutputType operator()(const InputType& v) {
		vec3 diffuse = camera.light_color * 
			std::max(0.0f, 
				dot(normalize(v.normal), 
					normalize(camera.light_pos - vec3(v.pos))));
		vec3 col = saturate((camera.light_ambient + diffuse) * camera.light_color);

		return OutputType(matrix * v.pos, v.uv, vec3(matrix * vec4(v.normal, 0.0f)), col);
	}

	void update() {
		matrix = perspective(fov, aspect_ratio, near, far) *
			lookAt(camera.eye, camera.eye + camera.angle, camera.up);
	}

	float aspect_ratio;
	float fov;
	float far;
	float near;

private:
	mat4x4 matrix;
};

class GeoShader : public GShader<GTriangle<GObjVertex>, GTriangle<GObjVertex>> {
public:
	GeoShader(GWindow& win) : GShader(win) { }
	
	OutputType operator()(const InputType& tri) {
		return tri;
	}
};

class FragShader : public GShader<GObjVertex, GRgba> {
public:
	FragShader(GWindow& win) : 
		GShader(win) { }

	GRgba operator()(const GObjVertex& v) {
		return GRgba{ 
			(u8)(v.color.x * 255), 
			(u8)(v.color.y * 255), 
			(u8)(v.color.z * 255), 
			255
		};
	}
};

class ExampleScene : public GScene {
public:
	using EContext = GContext<
		VertShader, 
		GeoShader, 
		FragShader>;

	ExampleScene(GWindow& win) :
		pipeline(win),
		window(win),
		object("../assets/teapot_out.obj") {
		win.register_scene(this);
		
		// init camera
		camera.up = { 0, 1, 0 };
		camera.eye = { 0, 2, 5 };
		camera.angle = { 0, 0, -1 };
		camera.yaw = -90.0f;
		camera.pitch = 0.0f;

		camera.update();

		triangles = object.get_triangle_list();
	}

	void process(const SDL_Event& event) {
		switch(event.type) {
		case SDL_QUIT: window.quit = true; break;
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
			case SDLK_w: // move forward
				camera.eye += camera.angle * camera.speed; 
				break;
			case SDLK_s: // move backward
				camera.eye -= camera.angle * camera.speed; 
				break;
			case SDLK_a: // move left
				camera.eye -= normalize(cross(camera.angle, camera.up)) * camera.speed;
				break;
			case SDLK_d: // move right
				camera.eye += normalize(cross(camera.angle, camera.up)) * camera.speed;
				break;
			case SDLK_RIGHT: // turn right
				camera.yaw += 4;
				if(camera.yaw > 359)
					camera.yaw -= 360;
				break;
			case SDLK_LEFT: // turn left
				camera.yaw -= 4;
				if(camera.yaw < 0)
					camera.yaw += 360;
				break;
			case SDLK_UP: // look up
				camera.pitch += 4;
				if(camera.pitch > 359)
					camera.pitch -= 360;
				break;
			case SDLK_DOWN: // look down
				camera.pitch -= 4;
				if(camera.pitch < 0)
					camera.pitch += 360;
				break;
			}

			switch(event.key.keysym.sym) {
			case SDLK_w: case SDLK_a: case SDLK_s: case SDLK_d:
			case SDLK_RIGHT: case SDLK_LEFT: case SDLK_UP: case SDLK_DOWN: {
				camera.update();
				pipeline.context.vertex_shader.update();
				break;
			}
		}
		}
	}

	void draw() {
		window.clear();
		{
			std::stringstream ss;
			ss << to_string(camera.eye);
			window.print(0, 20, ss.str());
		}
		pipeline.process(triangles);
	}

	GPipeline<EContext> pipeline;
	GWindow& window;

	GObj object;
	GTriangleList<GObjVertex> triangles;
};

int main() {
	GWindow window("hello", 800, 600, 0);
	ExampleScene es(window);

	window.run();

	return 0;
}