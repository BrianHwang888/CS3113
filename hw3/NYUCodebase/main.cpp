#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "ShaderProgram.h"
#include "Matrix.h"


#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define MAX_BULLETS 10
#define MAX_ALIENS 10


class Entity {
	public:
		void Draw(int num_Tri, ShaderProgram &program);
		Entity();
		Entity(GLuint textID, float x_pos, float y_pos, float z_pos, float u_tex, float v_tex, float wid, float hei, float x_sp, float y_sp);
		Matrix model_matrix;

		float x;
		float y;
		float z;
		float rotation;

		GLuint textureID;

		float u;
		float v;
		float width;
		float height;
		float size;
		float aspect;

		float velocity_x;
		float velocity_y;
};
class game_state {
public:
	Entity player_1;
	Entity aliens[MAX_ALIENS];
	Entity bullets[MAX_BULLETS];

	void set_entity(Entity &entity, GLuint textID, float x_pos, float y_pos, float z_pos, float u_tex, float v_tex, float wid, float hei, float siz, float x_sp, float y_sp);
};

void render();
void update(float elapsed);
void process_input(float elapsed);

GLuint LoadTexture(const char *filepath);
void draw_text(ShaderProgram *program, GLuint font_text, std::string, float size, float spacing, float x, float y);
void update_player(game_state &game, SDL_Event &event, float elapsed);

void game_setup(game_state &game);
void process_game(game_state &game, float elapsed);
void render_game(game_state &game);
void update_game(game_state &game, float elapsed);

void process_mainmenu();
void render_mainmenu();

void shootBullet();

int bulletsIndex = 0;
bool done = false;
enum GameMode
{
	STATE_MAIN_MENU,
	STATE_GAME_LEVEL,
	STATE_GAME_OVER
};

SDL_Window* displayWindow;
SDL_Surface* surface;
ShaderProgram program;
GameMode mode = STATE_MAIN_MENU;
game_state game;

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

	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	projection_matrix.SetOrthoProjection(-7.1f, 7.1f, -4.0f, 4.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	float lastframeticks = 0.0f;
	game_setup(game);
	
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastframeticks;
		lastframeticks = ticks;
		process_input(elapsed);
		update(elapsed);
		glClear(GL_COLOR_BUFFER_BIT);
		program.SetProjectionMatrix(projection_matrix);
		program.SetViewMatrix(view_matrix);
		render();
		
		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}

void Entity::Draw(int num_Tri, ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	float aspect = width / height;
	float vertices[] = { -0.5f * aspect * size, -0.5f * aspect * size, 
		0.5f * aspect * size,-0.5f * aspect * size,
		0.5f * aspect * size, 0.5f * aspect * size, 
		0.5f * aspect * size, 0.5f * aspect * size, 
		-0.5f * aspect * size, 0.5f * aspect * size, 
		-0.5f * aspect * size, -0.5f * aspect * size };
	GLfloat texCoord[] = { 
		u, v + height,
		u + width, v + height,
		u + width, v,

		u + width, v,
		u, v,
		u, v + height,
		};
	model_matrix.Identity();
	model_matrix.Translate(x, y, z);
	program.SetModelMatrix(model_matrix);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, num_Tri * 3);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}
Entity::Entity() {
	x = -2000.f;
	y = 0;
	u = 0;
	v = 0;
	velocity_x = 0;
	velocity_y = 0;
	width = 0;
	height = 0;
}
Entity::Entity(GLuint textID, float x_pos, float y_pos, float z_pos, float u_tex, float v_tex, float wid, float hei, float x_sp, float y_sp) {
	textureID = textID;
	x = x_pos;
	y = y_pos;
	z = z_pos;
	u = u_tex;
	v = v_tex;
	width = wid;
	height = hei;
	velocity_x = x_sp;
	velocity_y = y_sp;
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

void game_setup(game_state &game) {
	game.set_entity(game.player_1, LoadTexture(RESOURCE_FOLDER"sheet.png"),  0.0, -3.0, 0.0, 224.0f/1024.0f , 832.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.5f, 50.0f, 0.0f);
	for (int i = 0; i <MAX_BULLETS; i++) {
		game.bullets[i].height = 37.0f / 1024.0f;
		game.bullets[i].width = 13.0f / 1024.0f;
		game.bullets[i].velocity_y = 2.0f;
		game.bullets[i].textureID = LoadTexture(RESOURCE_FOLDER"sheet.png");
		game.bullets[i].u = 841.0f / 1024.0f;
		game.bullets[i].v = 647.0f / 1024.0f;
		game.bullets[i].size = 0.25f;
		game.bullets[i].aspect = game.bullets[i].width / game.bullets[i].height;
	}
	int y_pos = 0;
	int x_pos = 0;
	for (int i = 0; i < MAX_ALIENS; i++) {
		game.aliens[i].textureID = LoadTexture(RESOURCE_FOLDER"sheet.png");
		game.aliens[i].x = -2.0f + x_pos;// *game.aliens[i].width * game.aliens[i].aspect;
		game.aliens[i].y = 3 - y_pos;
		if (-2.0f + x_pos >= 3.0f) {
			x_pos = 0;
			y_pos++;
		}
		game.aliens[i].height = 84.0f /1024.0f;
		game.aliens[i].width = 93.0f /1024.0f;
		game.aliens[i].velocity_x = 2.0f;
		game.aliens[i].velocity_y = 50.0f;
		game.aliens[i].u = 423.0f / 1024.0f;
		game.aliens[i].v = 728.0f / 1024.0f;
		game.aliens[i].size = 0.5f;
		game.aliens[i].aspect = game.aliens[i].width / game.aliens[i].height;
		x_pos++;
	}
}
void game_state::set_entity(Entity &entity, GLuint textID, float x_pos, float y_pos, float z_pos, float u_tex, float v_tex, float wid, float hei, float siz, float x_sp, float y_sp) {
	entity.x = x_pos;
	entity.y = y_pos;
	entity.z = z_pos;
	entity.velocity_x = x_sp;
	entity.velocity_y = y_sp;
	entity.u = u_tex;
	entity.v = v_tex;
	entity.width = wid;
	entity.height = hei;
	entity.textureID = textID;
	entity.size = siz;
	entity.aspect = entity.width / entity.height;
}
void render_game(game_state &game) {
	game.player_1.Draw(2, program);
	for (int i = 0; i < MAX_BULLETS; i++) {
		game.bullets[i].Draw(2, program);
	}
	for (int i = 0; i < MAX_ALIENS; i++) {
		game.aliens[i].Draw(2, program);
	}
}
void process_game(game_state &game, float elapsed){
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) done = true;
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_D) {
				game.player_1.x += game.player_1.velocity_x * elapsed;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
				game.player_1.x -= game.player_1.velocity_x * elapsed;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				shootBullet();
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
				done = true;
			}
		}
	}
}
void update_player(game_state &game, SDL_Event &event, float elapsed) {
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) done = true;
		else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_D) {
					game.player_1.x += game.player_1.velocity_x * elapsed;
			}
				if (event.key.keysym.scancode == SDL_SCANCODE_A) {
					game.player_1.x -= game.player_1.velocity_x * elapsed;
			}
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					shootBullet();
			}
		}
	}
}
void update_game(game_state &game, float elapsed) {
	for (int i = 0; i < MAX_ALIENS; i++) {
		if (game.aliens[i].x != 4.0f && game.aliens[i].x != -4.0f) {
			game.aliens[i].x += game.aliens[i].velocity_x * elapsed;
		}
		if (game.aliens[i].x >= 4.0f || game.aliens[i].x <= -4.0f) {
			game.aliens[i].velocity_x *= -1.0f;
			game.aliens[i].x += game.aliens[i].velocity_x * elapsed;
			game.aliens[i].y -= game.aliens[i].velocity_y * elapsed;
		}
		if (game.player_1.x + 0.5 * game.player_1.aspect / 2 >= game.aliens[i].x - 0.5 * game.aliens[i].aspect * game.aliens[i].size / 2
			&& game.player_1.x - 0.5 * game.player_1.aspect / 2 <= game.aliens[i].x + 0.5 * game.aliens[i].aspect * game.aliens[i].size / 2
			&& game.player_1.y + 0.5 * game.player_1.aspect / 2 >= game.aliens[i].y - 0.5 * game.aliens[i].aspect * game.aliens[i].size / 2
			&& game.player_1.y - 0.5 * game.player_1.aspect / 2 <= game.aliens[i].y + 0.5 * game.aliens[i].aspect * game.aliens[i].size / 2) {
			done = true;
		}

	}
	for (int i = 0; i < MAX_BULLETS; i++) {
		game.bullets[i].y += game.bullets[i].velocity_y* elapsed;
		for (int j = 0; j < MAX_ALIENS; j++) {
			if (game.bullets[i].x + game.bullets[i].width * game.bullets[i].aspect / 2 >= game.aliens[j].x - 0.5 * game.aliens[j].aspect * game.aliens[j].size / 2
				&& game.bullets[i].x - game.bullets[i].width * game.bullets[i].aspect / 2 <= game.aliens[j].x + 0.5 * game.aliens[j].aspect * game.aliens[j].size / 2
				&& game.bullets[i].y + game.bullets[i].height * game.bullets[i].aspect / 2 >= game.aliens[j].y - 0.5 * game.aliens[j].aspect * game.aliens[j].size / 2
				&& game.bullets[i].y - game.bullets[i].height * game.bullets[i].aspect / 2 <= game.aliens[j].y + 0.5 * game.aliens[j].aspect * game.aliens[j].size / 2) {
				game.aliens[j].velocity_x = 0;
				game.aliens[j].velocity_y = 0;
				game.aliens[j].x = -1000.0f;
				game.bullets[i].x = -2000.f;
			}
		}
	}
}
void shootBullet() {
	//int count = MAX_BULLETS;
	game.bullets[bulletsIndex].x = game.player_1.x;
	game.bullets[bulletsIndex].y = game.player_1.y + 1;//game.player_1.height;
	bulletsIndex++;
	if (bulletsIndex >= MAX_BULLETS) {
		bulletsIndex = 0;
	}
}

void render_mainmenu() {
	GLuint text = LoadTexture(RESOURCE_FOLDER"font1.png");
	draw_text(&program, text, "Space Invaders", 0.5f, 0.0f, -3.05f, 3.0f);
	draw_text(&program, text, "Press P to play", 0.25f, 0.0f, -2.0f, 2.0f);
	draw_text(&program, text, "Press Q to quit", 0.25f, 0.0f, -2.0f, 1.0f);
	draw_text(&program, text, "A to move left", 0.15f, 0.0f, -1.5f, 0.5f);
	draw_text(&program, text, "D to move right", 0.15f, 0.0f, -1.5f, 0.0f);
	draw_text(&program, text, "Space to shoot", 0.15f, 0.0f, -1.5f, -0.5f);
}
void process_mainmenu() {
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) done = true;
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_P) {
				mode = STATE_GAME_LEVEL;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
				done = true;
			}
		}
	}
}

void render() {
	switch (mode)
	{
	case STATE_MAIN_MENU:
		render_mainmenu();
		break;
	case STATE_GAME_LEVEL:
		render_game(game);
		break;
	}
}	 
void update(float elapsed) {
	switch (mode)
	{
	case STATE_MAIN_MENU:
		process_mainmenu();
		break;
	case STATE_GAME_LEVEL:
		update_game(game, elapsed);
		break;
	}
}
void process_input(float elapsed) {
	switch (mode)
	{
	case STATE_MAIN_MENU:
		process_mainmenu();
		break;
	case STATE_GAME_LEVEL:
		process_game(game, elapsed);
		break;
	}
}
void draw_text(ShaderProgram *program, GLuint font_text, std::string text, float size, float spacing, float x, float y) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertex_data;
	std::vector<float> texCoordData;
	std::vector<Entity> sentence;
	for (int i = 0; i < text.size(); i++) {
		texture_size = 1.0 / 16.0f;
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;
		if (spriteIndex % 16 == 0) {
			texture_x = 0.015f;
		}
		if (spriteIndex % 16 == 15) {
			texture_x -= 0.02f;
		}
		vertex_data.insert(vertex_data.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
			});
	}
	for (int i = 0; i < text.size(); i++) {
		glBindTexture(GL_TEXTURE_2D, font_text);
		sentence.insert(sentence.end(), Entity(font_text, x, y, 0, 1, 1, 1, 1, 0, 0));
		sentence[i].model_matrix.Identity();
		sentence[i].model_matrix.Translate(sentence[i].x, sentence[i].y, 0);
		program->SetModelMatrix(sentence[i].model_matrix);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertex_data.data());
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
}
