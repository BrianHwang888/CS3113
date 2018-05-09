#include <string>
#include <stdio.h>
#include "Entity.h"
#include "FlareMap.h"
#include "SDL_Mixer.h"
#include <iostream>

extern game_state level_1;
extern game_state level_2;
extern float gravity;
extern ShaderProgram program;
extern enum GameMode mode;
extern bool done;
extern Mix_Chunk *spotted;
extern Mix_Chunk *death;
extern Mix_Chunk *key_get;
extern Mix_Chunk *e_death;

float lerp(float v0, float v1, float t) {
	return (1.0f - t) * v0 + t * v1;
}
void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}


sheetsprite::sheetsprite() {
	textureID = 0;
	size = 0.1f;
	u = 0;
	v = 0;
	width = 1.0f / 16.0f;
	height = 1.0f / 8.0f;
}
sheetsprite::sheetsprite(unsigned int textID) {
	textureID = textID;
}
sheetsprite::sheetsprite(unsigned int textID, float u_cor, float v_cor, float wid, float hei, float siz) {
	size = siz;
	textureID = textID;
	u = u_cor;
	v = v_cor;
	width = wid;
	height = hei;
}
void sheetsprite::Draw(float x, float y, float z) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		float aspect = width / height;
		float vertices[12] = {
			-0.5f * invertx, -0.5f + boop,
			0.5f * invertx, -0.5f + boop,
			0.5f * invertx, 0.5f + boop,
			0.5f * invertx, 0.5f + boop,
			-0.5f * invertx, 0.5f + boop,
			-0.5f * invertx, -0.5f + boop
		};
		GLfloat texCoord[] = {
			u, v + height,
			u + width, v + height,
			u + width, v,
	
			u + width, v,
			u, v,
			u, v + height,
		};
		modelMatrix.Identity();
		modelMatrix.Translate(x + TILE_SIZE / 2, y - TILE_SIZE / 2, z);
		modelMatrix.Scale(TILE_SIZE, TILE_SIZE, 1.0f);
		program.SetModelMatrix(modelMatrix);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
	
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
		glEnableVertexAttribArray(program.texCoordAttribute);
	
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
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
	size = Vector3(2.0f, 2.0f, 0.0f);
	velocity = Vector3(0.0f, 0.0f, 0.0f);
	acceleration = Vector3(0.0f, 0.0f, 0.0f);
	isStatic = false;
}
Entity::Entity(sheetsprite entity, float x_pos, float y_pos, float z_pos) {
	sprite = entity;
	position = Vector3(x_pos, y_pos, z_pos);
}
Entity::Entity(sheetsprite entity, float x_pos, float y_pos, float z_pos, float width, float height, bool stat, std::string type) {
	sprite = entity;
	position = Vector3(x_pos, y_pos, z_pos);
	size = Vector3(width, height, 0.0f);
	velocity = Vector3(0.0f, 0.0f, 0.0f);
	acceleration = Vector3(0.0f, 0.0f, 0.0f);
	isStatic = stat;
	if (type == "tile")
		entityType = ENTITY_TILE;
	if (type == "Player")
		entityType = ENTITY_PLAYER;
	if (type == "Key")
		entityType = ENTITY_KEY;
	if (type == "red")
		entityType = ENTITY_ENEMY;
	if (type == "legs")
		entityType = ENTITY_ENEMY;
	if (type == "spike")
		entityType = ENTITY_SPIKE;
}
void Entity::render() {
	sprite.Draw(position.x, position.y, position.z);
}
bool Entity::collideswith(const Entity &entity) {
	return (position.x + size.x / 2 >= entity.position.x - entity.size.x / 2
		&& position.x - size.x / 2 <= entity.position.x + entity.size.x / 2
		&& position.y + size.y / 2 >= entity.position.y - entity.size.x / 2
		&& position.y - size.y / 2 <= entity.position.y + entity.size.y / 2);
	
}

void player::update(game_state &level, float elapsed) {
	static float timeElapsed = 0.0f;
	float framesPerSecond = 0.3f;
	float penetration;
	int e1x, e1y, e2x, e2y;
	if (collideswith(level.key)) 
		done = true;

	if (collideswith(level.key_1)) {
		Mix_PlayChannel(-1, key_get, 0);
		mode = STATE_GAME_LEVEL_2;
		level.Enemy.clear();
		level.red_Enemy.clear();
		level.tiles.clear();
	}
	if (collideswith(level.key_2)) {
		Mix_PlayChannel(-1, key_get, 0);
		mode = STATE_GAME_LEVEL_3;
		level.Enemy.clear();
		level.red_Enemy.clear();
		level.tiles.clear();
	}

	if (position.y <= -64.0f) {
		Mix_PlayChannel(-1, death, 0);
		mode = STATE_GAME_OVER;
	}

	if (collideswith(level.key))
		done = true;
	for (int i = 0; i < level.Enemy.size(); i++) {
		if (collideswith(level.Enemy[i])) {
			if (kill_mode){// && ( e_left && level.Enemy[i].e_left || e_right && level.Enemy[i].e_right)) {
				Mix_PlayChannel(-1, e_death, 0);
				level.Enemy[i].position.x = -50.0f;
				level.Enemy[i].position.y = -50.0f;
				level.Enemy[i].velocity.y = level.Enemy[i].velocity.x = 0;
			}
			else{
				Mix_PlayChannel(-1, death, 0);
				mode = STATE_GAME_OVER;
			}
		}
	}
	timeElapsed += elapsed;
	if (timeElapsed > 1.0 / framesPerSecond) {
		level.player_1.kill_mode = false;
		timeElapsed = 0.0;
	}
	for (int i = 0; i < level.red_Enemy.size(); i++) {
		if (collideswith(level.red_Enemy[i])) {
			Mix_PlayChannel(-1, death, 0);
			mode = STATE_GAME_OVER;
		}
	}

	velocity.y += gravity * elapsed;
	velocity.y = lerp(velocity.y, gravity, elapsed * 0.25f);

	velocity.x += acceleration.x * elapsed;
	velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
	if (!kill_mode) {
		if (acceleration.x < 0 && sprite.invertx != -1){
			sprite.invertx = -1;
			e_right = false;
			e_left = true;
			}
		if (acceleration.x > 0) {
			sprite.invertx = 1;
			e_right = true;
			e_left = false;
		}
	}
	if (kill_mode) {
		if (acceleration.x < 0 && anger.invertx != -1) {
			anger.invertx = -1;
			e_left = true;
			e_right = false;
		}
		if (acceleration.x > 0) {
			anger.invertx = 1;
			e_right = true;
			e_left = false;
		}
	}
	position.y += velocity.y * elapsed;
	worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
	for (int i = 0; i < level.tiles.size(); i++) {
		if (collideswith(level.tiles[i])) {
			if (level.tiles[i].entityType == ENTITY_SPIKE) {
				Mix_PlayChannel(-1, death, 0);
				mode = STATE_GAME_OVER;
			}
			worldToTileCoordinates(level.tiles[i].position.x, level.tiles[i].position.y, &e2x, &e2y);
			collidedBot = false;
			collidedTop = false;
			if (velocity.y < 0) {
				penetration = fabs((-TILE_SIZE * e2y + TILE_SIZE / 2) - (position.y - size.y / 2));
				position.y += penetration + 0.0001f;
				velocity.y = 0;
				collidedBot = true;
			}
			if (velocity.y > 0) {
				penetration = fabs((-TILE_SIZE * e2y - TILE_SIZE / 2) - TILE_SIZE - (position.y - size.y / 2));
				position.y -= penetration + 0.0001f;
				velocity.y = 0;
				collidedTop = true;
			}
		}
	}
		
			
	position.x += velocity.x * elapsed;
	worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
	for (int i = 0; i < level.tiles.size(); i++) {
		if (collideswith(level.tiles[i])) {
			if (level.tiles[i].entityType == ENTITY_SPIKE) {
				Mix_PlayChannel(-1, death, 0);
				mode = STATE_GAME_OVER;
			}
			worldToTileCoordinates(level.tiles[i].position.x, level.tiles[i].position.y, &e2x, &e2y);
			collidedLeft = false;
			collidedRight = false;
			if (velocity.x > 0) {
				penetration = fabs((TILE_SIZE * e2x - TILE_SIZE / 2) - (position.x + size.x / 2));
				position.x -= penetration + 0.0001f;
				velocity.x = 0;
				collidedRight = true;
			}
			if (velocity.x < 0) {
				penetration = fabs((TILE_SIZE * e2x + TILE_SIZE / 2) - (position.x - size.x / 2));
				position.x += penetration + 0.0001f;
				velocity.x = 0;
				collidedLeft = true;
			}
		}
	}
	acceleration.x = 0.0f;
	acceleration.y = 0.0f;
}
void player::render() {
	if (!kill_mode) {
		sprite.Draw(position.x, position.y, position.z);
	}
	if (kill_mode) {
		anger.Draw(position.x, position.y, position.z);
	}
}
void enemy::render() {
	head.Draw(position.x, position.y + TILE_SIZE * 1.5f, position.z);
	sprite.Draw(position.x, position.y, position.z);
	torso.Draw(position.x, position.y + TILE_SIZE / 2, position.z);
}
void enemy::update(game_state &level, float elapsed) {
	const float runanimation[] = { 64.0f, 65.0f, 66.0f, 67.0f, 68.0f, 69.0f, 70.0f, 71.0f };
	static float animationElapsed = 0.0f;
	float framesPerSecond = 1.0f;
	const int numFrames = 8;
	static int index = 0;

	float penetration;
	int e1x, e1y, e2x, e2y;

	velocity.y += gravity * elapsed;
	velocity.y = lerp(velocity.y, gravity, elapsed * -0.03f);

	position.y += velocity.y * elapsed;
	worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
	for (int i = 0; i < level.tiles.size(); i++) {
		if (collideswith(level.tiles[i])) {
				worldToTileCoordinates(level.tiles[i].position.x, level.tiles[i].position.y, &e2x, &e2y);
			if (velocity.y < 0 && e1x == e2x) {
				penetration = fabs((-TILE_SIZE * e2y + TILE_SIZE / 2) - (position.y - size.y / 2));
				position.y += penetration + 0.0001f;
				velocity.y = 0;
			}
			if (velocity.y > 0 && e1x == e2x) {
				penetration = fabs((-TILE_SIZE * e2y - TILE_SIZE / 2) - TILE_SIZE - (position.y - size.y / 2));
				position.y -= penetration + 0.0001f;
				velocity.y = 0;
			}
		}
	}

	

	position.x += velocity.x * elapsed;
	worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
	for (int i = 0; i < level.tiles.size(); i++) {
		worldToTileCoordinates(level.tiles[i].position.x, level.tiles[i].position.y, &e2x, &e2y);
		if (collideswith(level.tiles[i])) {
			if (velocity.x > 0 && e2y >= e1y && e1x != e2x) {
				penetration = fabs((TILE_SIZE * e2x - TILE_SIZE / 2) - (position.x + size.x / 2));
				position.x -= penetration + 0.0001f;
				velocity.x *= -1.0f;
				sprite.invertx *= -1;
				head.invertx *= -1;
				torso.invertx *= -1;
				e_right = false;
				e_left = true;
			}
			if (velocity.x < 0 && e2y >= e1y && e1x >= e2x) {
				penetration = fabs((TILE_SIZE * e2x + TILE_SIZE / 2) - (position.x - size.x / 2));
				position.x += penetration + 0.0001f;
				velocity.x *= -1.0f;
				sprite.invertx *= -1;
				head.invertx *= -1;
				torso.invertx *= -1;
				e_right = true;
				e_left = false;
			}
		}
	}
	animationElapsed += elapsed;
	if (animationElapsed > 1.0 / framesPerSecond) {
		
		index++;
		animationElapsed = 0.0;
		if (index >= numFrames - 1)
			index = 0;
	}
	sprite.u = runanimation[index] / SPRITE_COUNT_X;
	acceleration.y = 0;

}

void red_enemy::update(game_state &level, float elapsed) {\
	if (!isStatic) {
		const float runanimation[] = { 64.0f, 65.0f, 66.0f, 67.0f, 68.0f, 69.0f, 70.0f, 71.0f };
		static float animationElapsed = 0.0f;
		float framesPerSecond = 2.0f;
		const int numFrames = 8;
		static int index = 0;

		float penetration;
		int e1x, e1y, e2x, e2y;

		collidedBot = false;
		collidedLeft = false;
		collidedRight = false;

		velocity.y += gravity * elapsed;
		velocity.y = lerp(velocity.y, gravity, elapsed * 1.0f);
		position.y += velocity.y * elapsed;

		if (position.y <= -55.0f)
			isStatic = true;

		worldToTileCoordinates(position.x, position.y, &e1x, &e1y);
		for (int i = 0; i < level.tiles.size(); i++) {
			if (collideswith(level.tiles[i])) {
				worldToTileCoordinates(level.tiles[i].position.x, level.tiles[i].position.y, &e2x, &e2y);
				if (velocity.y < 0) {
					penetration = fabs((-TILE_SIZE * e2y + TILE_SIZE / 2) - (position.y - size.y / 2));
					position.y += penetration + 0.0001f;
					velocity.y = 0;
					collidedBot = true;
				}
				if (velocity.y > 0) {
					penetration = fabs((-TILE_SIZE * e2y - TILE_SIZE / 2) - TILE_SIZE - (position.y - size.y / 2));
					position.y -= penetration + 0.0001f;
					velocity.y = 0;
					collidedTop = true;
				}
			}
		}

		if (!spots && fabs(position.x - level.player_1.position.x) < 20.0f && fabs(position.y - level.player_1.position.y) < 2.0f) {
			spots = true;
			Mix_PlayChannel(-1, spotted, 0);

			velocity.x = 10.0f;
			if (e_right && (position.x - level.player_1.position.x > 0.0f)) {
				if(velocity.x > 0.0f)
					velocity.x *= -1;
				sprite.invertx *= -1;
				head.invertx *= -1;
				torso.invertx *= -1;
				e_right = false;
				e_left = true;
			}
			else if (e_left && (position.x - level.player_1.position.x < 0.0f)) {
				sprite.invertx *= -1;
				head.invertx *= -1;
				torso.invertx *= -1;
				e_left = false;
				e_right = true;
			}
			else if (e_left) {
				velocity.x *= -1;

			}

		}
		if (fabs(position.y - level.player_1.position.y) > 3.0f) 
			spots = false;

		position.x += velocity.x * elapsed;
		worldToTileCoordinates(position.x, position.y, &e1x, &e1y);

		if (!spots && level.map.mapData[e1y + 2][e1x + 1] == 11 || level.map.mapData[e1y + 2][e1x - 1] == 11 && !spots && collidedBot) {
			velocity.x *= -1;
			sprite.invertx *= -1;
			head.invertx *= -1;
			torso.invertx *= -1;
			if (e_right) {
				e_right = false;
				e_left = true;
			}
			else if (e_left) {
				e_left = false;
				e_right = true;
			}
		}

		for (int i = 0; i < level.tiles.size(); i++) {
			worldToTileCoordinates(level.tiles[i].position.x, level.tiles[i].position.y, &e2x, &e2y);
			if (collideswith(level.tiles[i])) {
				if (velocity.x > 0 && e2y >= e1y && e1x != e2x) {
					collidedRight = true;
					penetration = fabs((TILE_SIZE * e2x - TILE_SIZE / 2) - (position.x + size.x / 2));
					position.x -= penetration + 0.001f;
					
					velocity.x *= -1.0f;
					sprite.invertx *= -1;
					head.invertx *= -1;
					torso.invertx *= -1;
					
					e_right = true;
					e_left = false;
				}
				if (velocity.x < 0 && e2y >= e1y && e1x >= e2x) {
					collidedLeft = true;
					penetration = fabs((TILE_SIZE * e2x + TILE_SIZE / 2) - (position.x - size.x / 2));
					position.x += penetration + 0.001f;
					
					velocity.x *= -1.0f;
					sprite.invertx *= -1;
					head.invertx *= -1;
					torso.invertx *= -1;
					
					e_right = false;
					e_left = true;
				}
			}
		}

		animationElapsed += elapsed;
		if (animationElapsed > 1.0 / framesPerSecond) {
			index++;
			animationElapsed = 0.0;
			if (index >= numFrames - 1)
				index = 0;
		}
		sprite.u = runanimation[index] / SPRITE_COUNT_X;
		acceleration.y = 0;
	}
}
void game_state::set_entity(Entity &entity, sheetsprite &sprite, float x_pos, float y_pos, float z_pos, float width, float height, bool stat, std::string type) {
		entity.sprite = sprite;
		entity.position.x = x_pos;
		entity.position.y = y_pos;
		entity.position.z = z_pos;
		entity.isStatic = stat;
		entity.velocity.x = 0;
		entity.velocity.y = 0;
		entity.velocity.z = 0;
		entity.size.x = width;
		entity.size.y = height;
		entity.size.z = 0;
		if (type == "Player")
			entity.entityType = ENTITY_PLAYER;
		if (type == "Key")
			entity.entityType = ENTITY_KEY;
		if (type == "torse")
			entity.entityType = ENTITY_ENEMY;
		if (type == "head")
			entity.entityType = ENTITY_ENEMY;
		if (type == "legs")
			entity.entityType = ENTITY_ENEMY;
	}