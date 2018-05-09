#pragma once

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

#define TILE_SIZE 2.0f
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
class game_state;
enum GameMode {
	STATE_GAME_MENU,
	STATE_GAME_LEVEL,
	STATE_GAME_LEVEL_2,
	STATE_GAME_LEVEL_3,
	STATE_GAME_OVER,

};
enum EntitiyType { ENTITY_PLAYER, 
	ENTITY_ENEMY, 
	ENTITY_KEY,
	ENTITY_TILE,
	ENTITY_SPIKE};
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
	sheetsprite(unsigned int text);
	sheetsprite(unsigned int textureID, float u, float v, float width, float height, float size);

	void Draw(float x, float y, float z);
	float size;
	float u;
	float v;
	float width;
	float height;
	float boop = 0;

	int invertx = 1;
	GLuint textureID;
	Matrix modelMatrix;

};
class Entity {
public:
	Entity();
	Entity(sheetsprite entity, float x_pos, float y_pos, float z_pos);
	Entity(sheetsprite entity, float x_pos, float y_pos, float z_pos, float width, float height, bool stat, std::string type);
	//virtual void update(float elapsed) {};
	virtual void render();
	bool collideswith(const Entity &entity);

	sheetsprite sprite;

	Vector3 position;
	Vector3 size;
	Vector3 velocity;
	Vector3 acceleration;
	bool isStatic;
	EntitiyType entityType;

	bool collidedTop, collidedBot, collidedLeft, collidedRight;

	bool e_right, e_left;
};
class enemy : public Entity {
public:
	sheetsprite head;
	sheetsprite torso;
	enemy() : Entity() {};
	enemy(sheetsprite entity, float x_pos, float y_pos, float z_pos, float width, float height, bool stat, std::string type) : Entity(entity, x_pos, y_pos, z_pos, width, height, stat, type) {
	};

	void render();
	void update(game_state &level, float elapsed);



};
class player : public Entity {
public:
	bool kill_mode = false;
	sheetsprite anger;
	player() : Entity() {};
	player(sheetsprite entity, float x_pos, float y_pos, float z_pos, float width, float height, bool stat, std::string type) : Entity(entity, x_pos, y_pos, z_pos, width, height, stat, type) {};
	void update(game_state &level, float elapsed);
	void render();
};
class red_enemy : public enemy {
public:
	bool spots = false;
	red_enemy():enemy() {};
	red_enemy(sheetsprite entity, float x_pos, float y_pos, float z_pos, float width, float height, bool stat, std::string type) : enemy(entity, x_pos, y_pos, z_pos, width, height, stat, type) {
	};
	
	void update(game_state & level, float elapsed);
};
class game_state {
public:
	~game_state()
	{
		Enemy.clear();
		red_Enemy.clear();
		tiles.clear();
	}
	player player_1;
	Entity key;
	Entity key_1;
	Entity key_2;
	FlareMap map;
	std::vector<enemy> Enemy;
	std::vector<red_enemy> red_Enemy;
	std::vector<Entity> tiles;

	void set_entity(Entity &entity, sheetsprite &sprite, float x_pos, float y_pos, float z_pos, float width, float height, bool static, std::string type);
};