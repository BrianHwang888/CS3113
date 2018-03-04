#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include "ShaderProgram.h"
#include "Matrix.h"


#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

class Entity {
	public:
		void Draw(int num_Tri, ShaderProgram &program);
		Entity(float x_pos, float y_pos, float z_pos, float wid, float hei, float x_sp, float y_sp);

		Matrix model_matrix;

		float x;
		float y;
		float z;
		float rotation;

		int textureID;

		float width;
		float height;

		float velocity_x;
		float velocity_y;
};

GLuint LoadTexture(const char *filepath);
void render_game();


SDL_Window* displayWindow;
SDL_Surface* surface;

ShaderProgram program;

Entity player_1(-2.0, 0.0, 0.0, 0.25, 1.0, 0.0, 20.0);
Entity player_2(2.0, 0.0, 0.0, 0.25, 1.0, 0.0, 20.0);
Entity ball(0.0, 0.0, 0.0, 0.5, 0.5, 2.0, 2.0);
Entity top_wall(0, 2, 0.0, 7.0, 0.5, 0.0, 0.0);
Entity bot_wall(0, -2, 0.0, 7.0, 0.5, 0.0, 0.0);
Entity l_wall(-3.5, 0.0, 0.0, 0.5, 4.0, 0.0, 0.0);
Entity r_wall(3.5, 0.0, 0.0, 0.5, 4.0, 0.0, 0.0);

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
				if (event.key.keysym.scancode == SDL_SCANCODE_W && player_1.y < top_wall.y - 0.75) {
					player_1.y += player_1.velocity_y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_S && player_1.y > bot_wall.y + 0.75) {
					player_1.y -= player_1.velocity_y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_UP && player_2.y < top_wall.y - 0.75) {
					player_2.y += player_2.velocity_y * elapsed;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN && player_2.y > bot_wall.y + 0.75) {
					player_2.y -= player_2.velocity_y * elapsed;
				}
			}
		}
		/*if(ball.x < r_wall.x && ball.x > l_wall.x)
			ball.x += sin(angle * 3.1415926 / 180) * elapsed * ball.velocity_x;
		*/
		if (ball.x >= r_wall.x || ball.x <= l_wall.x)
			done = true;

		if (ball.y + ball.height/2 >= top_wall.y - top_wall.height/2) {
			ball.velocity_y *= -1;
		}
		if (ball.y - ball.height/2 <= bot_wall.y + bot_wall.height/2) {
			ball.velocity_y *= -1;
		}
		if (player_1.x - player_1.width/2 <= ball.x + ball.width / 2
				&& player_1.x + player_1.width / 2 >= ball.x - ball.width/2
				&& player_1.y + player_1.height / 2 >= ball.y - ball.height / 2
				&& player_1.y - player_1.height / 2 <= ball.y + ball.height / 2)
				ball.velocity_x *= -1;
		

		if (ball.x + ball.width / 2 >= player_2.x - player_2.width / 2
			&& ball.x - ball.width / 2 <= player_2.x + player_2.width / 2
			&& ball.y + ball.height / 2 >= player_2.y - player_2.height / 2
			&& ball.y - ball.height / 2 <= player_2.y + player_2.height / 2){
			ball.velocity_x *= -1;
		}
		ball.y += sin(angle * 3.1415926 / 180) * elapsed * ball.velocity_y;
		ball.x += cos(angle * 3.1415926 / 180) * elapsed * ball.velocity_x;

		glClear(GL_COLOR_BUFFER_BIT);
		program.SetProjectionMatrix(projection_matrix);
		program.SetViewMatrix(view_matrix);
		
		render_game();
		
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

void Entity::Draw(int num_Tri, ShaderProgram &program) {
	float vertices[] = { 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f };
	model_matrix.Identity();
	model_matrix.Translate(x, y, z);
	model_matrix.Scale(width, height, 0.0);
	program.SetModelMatrix(model_matrix);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, num_Tri * 3);

	glDisableVertexAttribArray(program.positionAttribute);
}
Entity::Entity(float x_pos, float y_pos, float z_pos, float wid, float hei, float x_sp, float y_sp) {
	x = x_pos;
	y = y_pos;
	z = z_pos;
	velocity_x = x_sp;
	velocity_y = y_sp;
	width = wid;
	height = hei;

}
void render_game() {
	player_1.Draw(2, program);
	player_2.Draw(2, program);
	ball.Draw(2, program);
	top_wall.Draw(2, program);
	bot_wall.Draw(2, program);
	l_wall.Draw(2, program);
	r_wall.Draw(2, program);
}