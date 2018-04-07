#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h> 
#include <stdio.h>
#include <vector>
#include <utility>
#include "SatCollision.h"
#include "ShaderProgram.h"
#include "Matrix.h"


#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

enum Entity_type
{
	ENTITY_BALL, ENTITY_PLAYER
};

class Vector {
	public:
		Vector();
		Vector(float x, float y, float z);

		float x;
		float y;
		float z;

		friend Vector operator * (Matrix const &m , Vector const &v);
};

class shape {
public:
	std::vector<Vector> points;
};

class Entity {
	public:
		void Draw(int num_Tri, ShaderProgram &program);
		Entity(float x_pos, float y_pos, float z_pos, float wid, float hei, float x_sp, float y_sp, float angle, Entity_type type);

		Matrix model_matrix;

		Vector position;
		Vector velocity;
		shape poly;

		float angle;

		int textureID;
		Entity_type type; 
		float width;
		float height;
		
		bool collidesWith(Entity &entity, std::pair<float, float> &penetration);
		void update(float elapsed);
};

GLuint LoadTexture(const char *filepath);
void render_game();


SDL_Window* displayWindow;
SDL_Surface* surface;

ShaderProgram program;

Entity player_1(-5.0, 0.0, 0.0, 0.25, 1.0, 0.0, 20.0, 45.0f, ENTITY_PLAYER);
Entity player_2(5.0, 0.0, 0.0, 0.25, 1.0, 0.0, 20.0, -20.0f, ENTITY_PLAYER);

Entity ball(0.0, 0.0, 0.0, 0.5, 0.5, 2.0, 0.0f, 50.0f, ENTITY_BALL);

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	surface = SDL_GetWindowSurface(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 1280, 720);

	Matrix projection_matrix;
	Matrix view_matrix;

	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	projection_matrix.SetOrthoProjection(-7.1f, 7.1f, -4.0f, 4.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;

	float lastframeticks = 0.0f;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	float angle = 75.0;
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastframeticks;
		lastframeticks = ticks;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_W) {
					player_1.position.y += player_1.velocity.y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_S) {
					player_1.position.y -= player_1.velocity.y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_UP ) {
					player_2.position.y += player_2.velocity.y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					player_2.position.y -= player_2.velocity.y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_Q)
					done = true;
			}
		}
		/*if(ball.x < r_wall.x && ball.x > l_wall.x)
			ball.x += sin(angle * 3.1415926 / 180) * elapsed * ball.velocity_x;
		*/

		glClear(GL_COLOR_BUFFER_BIT);
		program.SetProjectionMatrix(projection_matrix);
		program.SetViewMatrix(view_matrix);
		
		render_game();
		ball.update(elapsed);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

GLuint LoadTexture(const char *filepath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Check path\n";
		assert(false);
	}

	GLuint reTexture;
	glGenTextures(1, &reTexture);
	glBindTexture(GL_TEXTURE_2D, reTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return reTexture;
}
bool Entity::collidesWith(Entity &entity, std::pair<float, float> &penetration) {
	
	std::vector<std::pair<float, float>> e1Points;
	std::vector<std::pair<float, float>> e2Points;

	for (int i = 0; i < poly.points.size(); i++) {
		Vector point = model_matrix * poly.points[i];
		e1Points.push_back(std::make_pair(point.x, point.y));
	}

	for (int i = 0; i < entity.poly.points.size(); i++) {
		Vector point = entity.model_matrix * entity.poly.points[i];
		e2Points.push_back(std::make_pair(point.x, point.y));
	}
	return CheckSATCollision(e1Points, e2Points, penetration);
	
}
void Entity::Draw(int num_Tri, ShaderProgram &program) {
	float vertices[] = { 0.5f, 0.5f, 
		-0.5f, 0.5f, 
		-0.5f, -0.5f, 
		-0.5f, -0.5f, 
		0.5f, -0.5f, 
		0.5f, 0.5f };
	model_matrix.Identity();
	model_matrix.Translate(position.x, position.y, 0);
	Matrix t = model_matrix;
	
	model_matrix.Rotate(angle * 3.1416 / 180);
	model_matrix.Scale(width, height, 0.0);
	poly.points.clear();
	/*poly.points.push_back(model_matrix * Vector(width, height, 0.0f));
	poly.points.push_back(model_matrix * Vector(-width, height, 0.0f));
	poly.points.push_back(model_matrix * Vector(-width, -height, 0.0f));
	poly.points.push_back(model_matrix * Vector(width, -height, 0.0f));
*/
	poly.points.push_back(Vector(0.5f, 0.5f, 0.0f));
	poly.points.push_back(Vector(-0.5f, 0.5f, 0.0f));
	poly.points.push_back(Vector(-0.5f, -0.5f, 0.0f));
	poly.points.push_back(Vector(0.5f, -0.5f, 0.0f));
	

	float vertices2[] = { poly.points[0].x, poly.points[0].y,
		poly.points[1].x, poly.points[1].y,
		poly.points[2].x, poly.points[2].y,
		poly.points[2].x, poly.points[2].y,
		poly.points[3].x, poly.points[3].y,
		poly.points[0].x, poly.points[0].y };

	program.SetModelMatrix(model_matrix);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
	glEnableVertexAttribArray(program.positionAttribute);

	glDrawArrays(GL_TRIANGLES, 0, num_Tri * 3);
	glDisableVertexAttribArray(program.positionAttribute);
}

Entity::Entity(float x_pos, float y_pos, float z_pos, float wid, float hei, float x_sp, float y_sp, float rota, Entity_type ty) {
	position.x = x_pos;
	position.y = y_pos;

	velocity.x = x_sp;
	velocity.y = y_sp;
	
	type = ty;

	width = wid;
	height = hei;
	angle = rota;
}
void Entity::update(float elapsed) {
	if (type == ENTITY_BALL) {
		std::pair<float, float> penetration;

			if (collidesWith(player_1, penetration)) {
				position.x += penetration.first;
				velocity.x *= -1;
			}

			if(collidesWith(player_2, penetration)) {
				position.x -= penetration.first;
				velocity.x *= -1;
			}
			
		position.x += elapsed * velocity.x;
		position.y += elapsed * velocity.y;

		
	}
}

Vector operator * (Matrix const &m, Vector const &v) {
	Vector vec;
	vec.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	vec.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	vec.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
	return vec;
}
Vector::Vector() {
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}
Vector::Vector(float x_val, float y_val, float z_val) {
	x = x_val;
	y = y_val;
	z = z_val;
}

void render_game() {
	player_1.Draw(2, program);
	player_2.Draw(2, program);
	ball.Draw(2, program);
}