#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "FlareMap.h"

using namespace std;

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define TILE_SIZE 2.0f
#define	LEVEL_HEIGHT 32
#define LEVEL_WIDTH 128
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

enum EntitiyType { ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_KEY };
class Vector3 {
public:
	Vector3(float x, float y, float z);
	Vector3();

	float x;
	float y;
	float z;
};
class sheetsprite {
public:
	sheetsprite();
	sheetsprite(unsigned int textureID, float u, float v, float width, float height, float size, int index);
	
	void Draw(float x, float y, float z);

	float size;
	float u;
	float v;
	float width;
	float height;
	GLuint textureID;
	int index;
};
class Entity {
public:
	Entity();
	Entity(sheetsprite entity, float x_pos, float y_pos, float z_pos, bool stat);

	void update(float elapsed);
	void render();
	bool collideswith(const Entity &entity);

	sheetsprite sprite;

	Vector3 position;
	Vector3 size;
	Vector3 velocity;
	Vector3 acceleration;

	bool isStatic;
	EntitiyType entityType;

	bool collidedTop;
	bool collidedBot;
	bool collidedLeft;
	bool collidedRight;

	void set_sprite(sheetsprite &sheet, GLuint text, float u_pos, float v_pos, float wid, float hei, float siz);
};
class game_state {
public:
	Entity player_1;
	Entity key;
	Entity tile[24];

	void set_entity(Entity &entity, sheetsprite &sprite, float x_pos, float y_pos, float z_pos, bool static, string type);
};

void render();
void update(float elapsed);
void process_input(float elapsed);

GLuint LoadTexture(const char *filepath);
void PlaceEntity(game_state &game, string type, float x, float y);
void render_game(game_state &game);

float gravity = 0.025f;
bool done = false;

SDL_Window* displayWindow;
SDL_Surface* surface;
ShaderProgram program;
game_state game;
Matrix view_matrix, model_matrix, projection_matrix;
FlareMap map;
GLuint tileText;

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
	projection_matrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -0.5f, 0.5f);

	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	tileText = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
	map.Load("levelmap.txt");
	for (int i = 0; i < map.entities.size(); i++) {
		PlaceEntity(game, map.entities[i].type, map.entities[i].x * TILE_SIZE, map.entities[i].y * -TILE_SIZE);
	}
	SDL_Event event;
	float lastframeticks = 0.0f;
	float accumulator = 0.0f;
	program.SetProjectionMatrix(projection_matrix);
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastframeticks;
		lastframeticks = ticks;
		elapsed += accumulator;
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}
		while (elapsed >= FIXED_TIMESTEP) {
			process_input(FIXED_TIMESTEP);
			update(FIXED_TIMESTEP);
			elapsed -= FIXED_TIMESTEP;
		}
		accumulator = elapsed;
		render();
		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

void PlaceEntity(game_state &game, string type, float x, float y) {
	static int index = 0;
	float u; 
	float v;
	if (type == "Player") {
		u = (float)(80 % SPRITE_COUNT_X) / SPRITE_COUNT_X;
		v = 80.0f / SPRITE_COUNT_X / SPRITE_COUNT_Y;
		game.set_entity(game.player_1, sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE, 80), x, y, 0, false, type);
		game.player_1.velocity.y = 2.0f;
	}
	if (type == "Key") {
		u = (float)(86 % SPRITE_COUNT_X) / SPRITE_COUNT_X;
		v = 86.0f / SPRITE_COUNT_X / SPRITE_COUNT_Y;

		game.set_entity(game.key, sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE, 86), 28.0f, y, 0, true, type);
	}
	if (type == "dirt") {
		u = (float)(1 % SPRITE_COUNT_X) / SPRITE_COUNT_X;
		v = 1.0f / SPRITE_COUNT_X / SPRITE_COUNT_Y;

		game.tile[index] = Entity(sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE, 1), x, y, 0, true);
		index++;
	}
	if (type == "dirt_2") {
		u = (float)(18 % SPRITE_COUNT_X) / SPRITE_COUNT_X;
		v = 18.0f / SPRITE_COUNT_X / SPRITE_COUNT_Y;

		game.tile[index] = Entity(sheetsprite(tileText, u, v, 1.0f / (float)SPRITE_COUNT_X, 1.0f / (float)SPRITE_COUNT_Y, TILE_SIZE, 18), x, y, 0, true);
		index++;
	}
	
}
void set_position(Vector3 &val, float x_pos, float y_pos, float z_pos) {
	val.x = x_pos;
	val.y = y_pos;
	val.z = z_pos;
}
void game_state::set_entity(Entity &entity, sheetsprite &sprite, float x_pos, float y_pos, float z_pos, bool stat, string type) {
	entity.sprite = sprite;
	set_position(entity.position, x_pos, y_pos, z_pos);
	entity.isStatic = stat;
	if (type == "Player")
		entity.entityType = ENTITY_PLAYER;
	if (type == "Key")
		entity.entityType = ENTITY_KEY;
	
}
void sheetsprite::Draw(float x, float y, float z) {
	glBindTexture(GL_TEXTURE_2D, tileText);
	float aspect = width / height;
	float vertices[] = {
		-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f
		};
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
	model_matrix.Scale(TILE_SIZE, TILE_SIZE, 1.0f);
	program.SetModelMatrix(model_matrix);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

sheetsprite::sheetsprite() {
	textureID = tileText;
	size = 0.1f;
	u = 0;
	v = 0;
	width = 1.0f / 16.0f;
	height = 1.0f / 8.0f;
}
sheetsprite::sheetsprite(unsigned int textID, float u_cor, float v_cor, float wid, float hei, float siz, int i) {
	size = siz;
	textureID = textID;
	u = u_cor;
	v = v_cor;
	width = wid;
	height = hei;
	index = i;
}
Vector3::Vector3(float x_val, float y_val, float z_val) {
	x = x_val;
	y = y_val;
	z = z_val;
}
Vector3::Vector3() {
	x = 0;
	y = 0;
	z = 0;
}

Entity::Entity() {
	position = Vector3(0.0f, 0.0f, 0.0f);
	size = Vector3(0.0f, 0.0f, 0.0f);
	velocity = Vector3(5.0f, 0.0f, 0.0f);
	acceleration = Vector3(0.0f, 0.0f, 0.0f);
	isStatic = false;
}
Entity::Entity(sheetsprite entity, float x_pos, float y_pos, float z_pos, bool stat) {
	sprite = entity;
	position = Vector3(x_pos, y_pos, z_pos);
	size = Vector3(TILE_SIZE, TILE_SIZE, 0.0f);
	velocity = Vector3(1.0, 0.0, 0.0f);
	acceleration = Vector3(0.0f, 0.0f, 0.0f);
	isStatic = stat;
}
void Entity::set_sprite(sheetsprite &sheet, GLuint text, float u_pos, float v_pos, float wid, float hei, float siz) {
	sheet.height = hei;
	sheet.width = wid;
	sheet.textureID = text;
	sheet.u = u_pos;
	sheet.v = v_pos;
	sheet.size = siz;

}
void Entity::update(float elapsed) {
	float penetration;
	int e1x, e1y, e2x, e2y;
	if (!isStatic) {
		velocity.y -= gravity * elapsed;
		position.y -= velocity.y * elapsed;
		for (int i = 0; i < 26; i++) {
			collidedBot = false;
			collidedTop = false;
			collidedLeft = false;
			collidedRight = false;
			if (collideswith(game.tile[i])) {
				if (collidedBot) {
					worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
					worldToTileCoordinates(game.tile[i].position.x, game.tile[i].position.y, &e2x, &e2y);
					//penetration = fabs(-TILE_SIZE * e2y - (e1y - TILE_SIZE / 2));
					penetration = fabs(e2y - game.tile[i].size.y /2 - e1y - size.y / 2);
					position.y += penetration;
					velocity.y = 0;
				}
			}
		}
	}
}
void Entity::render() {

	sprite.Draw(position.x, position.y, position.z);
	
}
bool Entity::collideswith(const Entity &entity) {
	int e1x, e1y, e2x, e2y;
	worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
	worldToTileCoordinates(entity.position.x, entity.position.y, &e2x, &e2y);
	if (e1y + size.y / 2 >= e2y - entity.size.y / 2)
		collidedTop = true;
	if (e1y - size.y / 2 <= e2y + entity.size.y / 2)
		collidedBot = true;
	if (e1x + size.x / 2 >= e2x - entity.size.x / 2)
		collidedRight = true;
	if (e1x - size.x / 2 <= e2x + entity.size.x / 2)
		collidedLeft = true;
	/*else
		return false;
	return true;*/
	return collidedTop && collidedBot && collidedRight && collidedLeft;
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

void renderlevel() {
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
				float u = (float)(((int)map.mapData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)map.mapData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
					});
				texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
					});
		}
	}

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	model_matrix.Identity();
	//model_matrix.Scale(0.5f, 0.5f, 1.0f);
	program.SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, tileText);
	glDrawArrays(GL_TRIANGLES, 0, 6 * LEVEL_HEIGHT * LEVEL_WIDTH);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void render_game(game_state &game) {
	game.player_1.render();
	game.key.render();
	for (int i = 0; i < 26; i++) {
		game.tile[i].render();
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	renderlevel();
	render_game(game);
}

void update(float elapsed) {
	view_matrix.Identity();
	view_matrix.Scale(0.05f, 0.05f, 0);
	view_matrix.Translate(-1 * game.player_1.position.x, -1 * game.player_1.position.y, -1 * game.player_1.position.z);
	
	program.SetViewMatrix(view_matrix);

	game.player_1.collideswith(game.key);
	game.player_1.update(elapsed);
}
void process_input(float elapsed) {
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) done = true;
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){ //&& game.player_1.collidedRight) {
				game.player_1.position.x += game.player_1.velocity.x * elapsed;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){// && game.player_1.collidedLeft) {
				game.player_1.position.x -= game.player_1.velocity.x * elapsed;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){// && game.player_1.collidedBot) {
				game.player_1.velocity.y = 2.0f;
				game.player_1.position.y += game.player_1.velocity.y * elapsed;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
				done = true;
			}
		}
	}
}