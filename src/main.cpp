#include "gfx.hpp"

using namespace demo;

struct Light {
	vec3 pos;
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
	vec3 color;
	float shinyness;
} static light{
	{ 0, 0, 10 }, 
	{1.f, 1.f, 1.f}, 
	{0.1f, 0.1f, 0.1f}, 
	{1.f, 1.f, 1.f}, 
	{1.f, 1.f, 1.f}, 
	100.f
};

struct Camera {
	vec3 eye;
	vec3 angle;
	vec3 up;

	float pitch;
	float yaw;
	
	float speed = 1.f;

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

class GouraudVertShader : public GShader<GObjVertex, GObjVertex> {
public:
	GouraudVertShader(GWindow& win) :
		GShader(win),
		aspect_ratio((float)win.width / win.height),
		fov(45),
		far(10000.0f), near(0.1f) { 
		update();
	}

	OutputType operator()(const InputType& v) {
		vec3 pos = model_view * v.pos;
		vec3 light_pos = model_view * vec4(light.pos, 1);

		vec3 N = normalize(normal_matrix * v.normal);
		vec3 L = normalize(light_pos - pos);
		vec3 V = normalize(-pos);
		vec3 H = normalize(L + V);

		vec3 ambient = light.ambient;

		vec3 diffuse = max(dot(L, N), 0.0f) * light.diffuse;
		vec3 specular = pow(max(dot(N, H), 0.0f), light.shinyness) * light.specular;

		vec3 color = saturate(light.color * (ambient + diffuse + specular));

		return OutputType(projection * model_view * v.pos, v.uv, N, color);
	}

	void update() {
		projection = perspective(fov, aspect_ratio, near, far);
		model_view = lookAt(camera.eye, camera.eye + camera.angle, camera.up);
		normal_matrix = mat3x3(transpose(inverse(model_view)));
	}

	float aspect_ratio;
	float fov;
	float far;
	float near;

private:
	mat4x4 projection;
	mat4x4 model_view;
	mat3x3 normal_matrix;
};

class GeoShader : public GShader<GTriangle<GObjVertex>, GTriangle<GObjVertex>> {
public:
	GeoShader(GWindow& win) : GShader(win) { }
	
	OutputType operator()(const InputType& tri) {
		return tri;
	}

	void update() { }
};

class ColorFragShader : public GShader<GObjVertex, GRgba> {
public:
	ColorFragShader(GWindow& win) : 
		GShader(win) { }

	GRgba operator()(const GObjVertex& v) {
		return GRgba{ 
			(u8)(v.color.x * 255), 
			(u8)(v.color.y * 255), 
			(u8)(v.color.z * 255), 
			255
		};
	}

	void update() { }
};

class ExampleScene : public GScene {
public:
	using EContext = GContext<
		GouraudVertShader, 
		GeoShader, 
		ColorFragShader>;

	ExampleScene(GWindow& win) :
		pipeline(win),
		window(win),
		object("../assets/dragon.obj"),
		object2("../assets/suzanne.obj") {
		win.register_scene(this);
		
		// init camera
		camera.up = { 0, 1, 0 };
		camera.eye = { 0, 2, 5 };
		camera.angle = { 0, 0, -1 };
		camera.yaw = -90.0f;
		camera.pitch = 0.0f;

		camera.update();
		pipeline.context.vertex_shader.update();

		mesh = object.get_triangle_list();
		mesh2 = object2.get_triangle_list();

		for(auto& e : mesh2.vertices) {
			e.pos.y += 10;
		}
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

		const float t = 0.01;
		mat3x3 angle(
			vec3(cos(t), 0, sin(t)),
			vec3(0, 1, 0),
			vec3(-sin(t), 0, cos(t))
		);

		// for(auto& tri : mesh.vertices) {
		// 	vec3 pos(tri.pos);
		// 	pos = angle * pos;
		// 	tri.pos = vec4(pos, 1);
		// }

		light.pos = angle * light.pos;
		
		{
			std::stringstream ss;
			ss << to_string(camera.eye);
			window.print(0, 20, ss.str());
		}
		pipeline.process(mesh);
		pipeline.process(mesh2);
	}

	GPipeline<EContext> pipeline;
	GWindow& window;
	
	GObj object;
	GObj object2;
	GMesh<GObjVertex> mesh;
	GMesh<GObjVertex> mesh2;
};

int main() {
	GWindow window("hello", 800, 600, 0);
	ExampleScene es(window);

	window.run();

	return 0;
}
