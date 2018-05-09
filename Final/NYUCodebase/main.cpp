#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "Entity.h"
#include "ShaderProgram.h"
#include "Matrix.h"
#include "FlareMap.h"
#include "SDL_mixer.h"
using namespace std;

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

void render();
void update(float elapsed);
void process_input(float elapsed);

void render_mainmenu();
void process_mainmenu();

void render_game(game_state &game);
void process_game(game_state &game, float elapsed);
void update_game(game_state &game, float elapsed);

void process_gameover();
void render_gameover();

void draw_text(ShaderProgram *program, GLuint font_text, std::string text, float size, float spacing, float x, float y);
GLuint LoadTexture(const char *filepath);
void PlaceEntity(game_state &game, string type, float x, float y,int &enemy_index,int &red_index);
void read_map(FlareMap &map, std::vector<Entity> &tiles);

SDL_Window* displayWindow;
SDL_Surface* surface;
ShaderProgram program;

float gravity = -5.0f;
bool done = false;

game_state level_1, level_2, level_3;
game_state *curr_level;
GameMode mode;
Matrix view_matrix, model_matrix, projection_matrix;
GLuint tileText, rage;
Mix_Chunk *jump;
Mix_Chunk *spotted;
Mix_Chunk *death; 
Mix_Chunk *key_get;
Mix_Chunk *rage_sound;
Mix_Chunk *e_death;
Mix_Music *superbg;


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

	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	projection_matrix.SetOrthoProjection(-7.1f, 7.1f, -4.0f, 4.0f, -1.0f, 1.0f);
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tileText = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
	rage = LoadTexture(RESOURCE_FOLDER"pure_rage.png");
	level_1.map.Load("levelmap.txt");
	level_2.map.Load("level2map.txt");
	level_3.map.Load("level3map.txt");
	superbg = Mix_LoadMUS("Super Mario Bros. - Overworld Theme.wav");
	jump = Mix_LoadWAV("jump.wav");
	spotted = Mix_LoadWAV("Metal Gear Solid Alert (!).wav");
	death = Mix_LoadWAV("Roblox Death Sound Effect.wav");
	e_death = Mix_LoadWAV("Minecraft Oof.wav");
	key_get = Mix_LoadWAV("key_get.wav");
	rage_sound = Mix_LoadWAV("rage.wav");

	//Mix_VolumeChunk(spotted, 10);
	int enemy_index = 0, red_index = 0;
	read_map(level_1.map, level_1.tiles);
	for (int i = 0; i < level_1.map.entities.size(); i++) {
		PlaceEntity(level_1, level_1.map.entities[i].type, level_1.map.entities[i].x * TILE_SIZE, level_1.map.entities[i].y * -TILE_SIZE, enemy_index, red_index);
	}
	enemy_index = 0, red_index = 0;
	read_map(level_2.map, level_2.tiles);
	for (int i = 0; i < level_2.map.entities.size(); i++) {
		PlaceEntity(level_2, level_2.map.entities[i].type, level_2.map.entities[i].x * TILE_SIZE, level_2.map.entities[i].y * -TILE_SIZE, enemy_index, red_index);
	}
	enemy_index = 0, red_index = 0;
	read_map(level_3.map, level_3.tiles);
	for (int i = 0; i < level_3.map.entities.size(); i++) {
		PlaceEntity(level_3, level_3.map.entities[i].type, level_3.map.entities[i].x * TILE_SIZE, level_3.map.entities[i].y * -TILE_SIZE, enemy_index, red_index);
	}
	curr_level = &level_1;
	mode = STATE_GAME_MENU;


	float lastframeticks = 0.0f;
	float accumulator = 0.0f;
	program.SetProjectionMatrix(projection_matrix);
	program.SetViewMatrix(view_matrix);
	Mix_PlayMusic(superbg, -1);
	glClearColor(0.0f, 0.53f, 1.0f,  1.0f);	
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastframeticks;
		lastframeticks = ticks;
		/*elapsed += accumulator;
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}
		while (elapsed >= FIXED_TIMESTEP) {
			process_input(FIXED_TIMESTEP);
			update(FIXED_TIMESTEP);
			render();
			elapsed -= FIXED_TIMESTEP;

		}
		accumulator = elapsed;*/
		process_input(elapsed);
		update(elapsed);
		render();
		SDL_GL_SwapWindow(displayWindow);
	}
	Mix_FreeMusic(superbg);
	Mix_FreeChunk(e_death);
	Mix_FreeChunk(jump);
	Mix_FreeChunk(death);
	Mix_FreeChunk(spotted);
	Mix_FreeChunk(key_get);
	Mix_FreeChunk(rage_sound);
	SDL_Quit();
	return 0;
}

void read_map(FlareMap &map, std::vector<Entity> &tiles) {
	tiles.clear();
	map.vertexData.clear();
	map.texCoordData.clear();
	for (int y = 0; y < map.mapHeight; y++) {
		for (int x = 0; x < map.mapWidth; x++) {
			float u = (float)(((int)map.mapData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v = (float)(((int)map.mapData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
			if (map.mapData[y][x] != 11){
				float x_pos = (x * TILE_SIZE) + (x * TILE_SIZE + TILE_SIZE) / 2;
				float y_pos = (-TILE_SIZE * y) + ((-TILE_SIZE * y) - TILE_SIZE) / 2;
				if (map.mapData[y][x] == 99 || map.mapData[y][x] == 100)
					tiles.push_back(Entity(sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x * TILE_SIZE, y * -TILE_SIZE, 0.0f, TILE_SIZE, TILE_SIZE, true, "spike"));
				else
					tiles.push_back(Entity(sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x * TILE_SIZE, y * -TILE_SIZE, 0.0f, TILE_SIZE, TILE_SIZE, true, "tile"));
			}
				map.vertexData.insert(map.vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
					});
				map.texCoordData.insert(map.texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
					});
				
		}
	}
}
void PlaceEntity(game_state &game, string type, float x, float y, int &enemy_index, int &red_index) {
	float u; 
	float v;
	if (type == "Player") {
		u = (float)(((int) 80) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
		v = (float)(((int) 80) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
		
		game.set_entity(game.player_1, sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x, y, 0, TILE_SIZE, TILE_SIZE, false, type);
		game.player_1.anger = sheetsprite(rage, 0.0f, 0.0f,0.5f, 1.0f, TILE_SIZE);
	}
	if (type == "red") {
		u = (float)(((int)64) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)64) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

		game.red_Enemy.insert(game.red_Enemy.end(), red_enemy(sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x, y, 0, (float)TILE_SIZE, (float)TILE_SIZE, false, type));
		

		u = (float)(((int)99) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)99) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		game.red_Enemy[red_index].torso = sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE);

		u = (float)(((int)83) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)83) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		game.red_Enemy[red_index].head = sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE);
		game.red_Enemy[red_index].velocity.x = 2.0f;
		game.red_Enemy[red_index].e_front = true;
		red_index++;
	}
	if (type == "Key_1") {
		u = (float)(((int)86) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)86) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

		game.set_entity(game.key_1, sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x, y, 0, true, TILE_SIZE, TILE_SIZE, type);
	}
	if (type == "Key_2") {
		u = (float)(((int)86) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)86) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

		game.set_entity(game.key_2, sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x, y, 0, true, TILE_SIZE, TILE_SIZE, type);
	}
	if (type == "Key") {
		u = (float)(((int)86) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)86) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

		game.set_entity(game.key, sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x, y, 0, true, TILE_SIZE, TILE_SIZE, type);
	}
	if (type == "legs") {
		u = (float)(((int)64) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)64) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
	
		game.Enemy.insert(game.Enemy.end(), enemy(sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE), x, y, 0, TILE_SIZE, TILE_SIZE, false, type));
		
		u = (float)(((int)98) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)98) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		game.Enemy[enemy_index].torso = sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE);

		u = (float)(((int)82) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)(((int)82) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		game.Enemy[enemy_index].head = sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE);
		game.Enemy[enemy_index].velocity.x = 2.0f;
		enemy_index++;
	}
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

void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (mode)
	{
	case STATE_GAME_MENU:
		render_mainmenu();
		break;
	case STATE_GAME_LEVEL:
		render_game(level_1);
		break;
	case STATE_GAME_LEVEL_2:
		curr_level = &level_2;
		render_game(level_2);
		break;
	case STATE_GAME_LEVEL_3:
		curr_level = &level_3;
		render_game(level_3);
		break;
	case STATE_GAME_OVER:
		Mix_HaltMusic();
		render_gameover();
		break;
	}
}
void update(float elapsed) {
	switch (mode)
	{
	case STATE_GAME_LEVEL:
		update_game(level_1, elapsed);
		break;
	case STATE_GAME_LEVEL_2:
		update_game(level_2, elapsed);
		break;
	case STATE_GAME_LEVEL_3:
		update_game(level_3, elapsed);
		break;
	case STATE_GAME_OVER:
		view_matrix.Inverse();
		break;
	}
}
void process_input(float elapsed) {
	switch (mode)
	{
	case STATE_GAME_MENU:
		process_mainmenu();
		break;
	case STATE_GAME_LEVEL:
		process_game(level_1, elapsed);
		break;
	case STATE_GAME_LEVEL_2:
		process_game(level_2, elapsed);
		break;
	case STATE_GAME_LEVEL_3:
		process_game(level_3, elapsed);
		break;
	case STATE_GAME_OVER:
		process_gameover();
		break;
	}
}

void update_game(game_state &game, float elapsed) {
	view_matrix.Identity();
	view_matrix.Scale(0.25f, 0.25f, 0);
	view_matrix.Translate(-1 * game.player_1.position.x, -1 * game.player_1.position.y, -1 * game.player_1.position.z);
	
	program.SetViewMatrix(view_matrix);

	
	for (int i = 0; i < game.Enemy.size(); i++) {
		game.Enemy[i].update(game, elapsed);
	}
	for (int i = 0; i < game.red_Enemy.size(); i++) {
		game.red_Enemy[i].update(game, elapsed);
	}
	game.player_1.update(game, elapsed);
}

void renderlevel(FlareMap &map) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, map.vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, map.texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);
	
	model_matrix.Identity();
	program.SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, tileText);
	glDrawArrays(GL_TRIANGLES, 0, 6 * map.mapHeight * map.mapWidth);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}
void render_game(game_state &game) {
	renderlevel(game.map);
	for (int i = 0; i < game.Enemy.size(); i++) {
		game.Enemy[i].render();
	}
	for (int i = 0; i < game.red_Enemy.size(); i++) {
		game.red_Enemy[i].render();
	}
	game.player_1.render();
	
	game.key_1.render();
	game.key.render();
	game.key_2.render();
}
void process_game(game_state &game, float elapsed) {
	static float pl_ani = 1.0f / SPRITE_COUNT_X;
	static float pl_ang = 0.5f;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) done = true;
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
				if (game.player_1.kill_mode) {
					game.player_1.anger.u += pl_ang;
					pl_ang *= -1;
				}
				else if (!game.player_1.kill_mode){
					if((game.player_1.sprite.u += pl_ani) < 0)
						game.player_1.sprite.u = (float)(((int)80) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
					pl_ani *= -1;
				}
				game.player_1.acceleration.x = 25.0f;
				
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
				if (game.player_1.kill_mode){
					game.player_1.anger.u += pl_ang;
					pl_ang *= -1;
				}
				else if (!game.player_1.kill_mode) {
					if((game.player_1.sprite.u += pl_ani) < 0)
						game.player_1.sprite.u = (float)(((int) 80) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
					pl_ani *= -1;
				}
				game.player_1.acceleration.x = -25.0f;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && game.player_1.collidedBot) {
				Mix_PlayChannel(-1, jump, 0);
				game.player_1.velocity.y += 8.0f;
				game.player_1.collidedBot = false;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
				mode = STATE_GAME_OVER;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Z) {
				Mix_PlayChannel(-1, rage_sound, 0);
				game.player_1.kill_mode = true;
				game.player_1.anger.invertx = game.player_1.sprite.invertx;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
				game.player_1.position.x = 4;
				game.player_1.position.y = 8;
			}
		}
	}
}

void process_mainmenu() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
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
void render_mainmenu() {
	GLuint text = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
	draw_text(&program, text, "Brown Thingy", 0.5f, 0.0f, -3.05f, 3.0f);
	draw_text(&program, text, "Press P to play", 0.25f, 0.0f, -2.0f, 2.0f);
	draw_text(&program, text, "Press Q to quit", 0.25f, 0.0f, -2.0f, 1.0f);
	draw_text(&program, text, "Left arrow to move left", 0.25f, 0.0f, -1.5f, 0.5f);
	draw_text(&program, text, "Right arrow to move right", 0.25f, 0.0f, -1.5f, 0.0f);
	draw_text(&program, text, "Space to jump", 0.25f, 0.0f, -1.5f, -1.0f);
	draw_text(&program, text, "Press z for kill_mode", 0.25f, 0.0f, -1.5f, -2.0f);
}

void process_gameover() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) done = true;
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_R) {
				mode = STATE_GAME_LEVEL;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
				done = true;
			}
		}
	}
}
void render_gameover() {
	GLuint text = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
	draw_text(&program, text, "GAME OVER", 5.0f, 0.0f, curr_level->player_1.position.x - 20.0f, curr_level->player_1.position.y + 10.0f);
	draw_text(&program, text, "Press Q to quit", 1.0f, 0.0f, curr_level->player_1.position.x - 5.0f, curr_level->player_1.position.y);
}

void draw_text(ShaderProgram *program, GLuint font_text, std::string text, float size, float spacing, float x, float y) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertex_data;
	std::vector<float> texCoordData;
	std::vector<Entity> sentence;
	sheetsprite font(font_text);
	for (int i = 0; i < text.size(); i++) {
		texture_size = 1.0 / 16.0f;
		int spriteIndex = (int)text[i];
		float texture_x = (float)(((int)spriteIndex) % 16) / 16.0f;
		float texture_y = (float)(((int)spriteIndex) / 16) / 16.0f;
		
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
		sentence.insert(sentence.end(), Entity(font, x, y, 0.0f)); 
		sentence[i].sprite.modelMatrix.Identity();
		sentence[i].sprite.modelMatrix.Translate(sentence[i].position.x, sentence[i].position.y, 0);
		program->SetModelMatrix(sentence[i].sprite.modelMatrix);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertex_data.data());
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
}