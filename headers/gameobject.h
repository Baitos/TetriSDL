#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "../ext/glm/glm.hpp"
#include "../headers/animation.h"

enum class BlockState {
    moving, placed
};
struct BlockData {};
struct LevelData {};

union ObjectData {
    BlockData block;
    LevelData level;
};

enum class ObjectType {
    block, level
};

struct GameObject {
    ObjectType type;
    ObjectData data;
    glm::vec2 pos;
    float dir;
    float maxSpeedX;
    std::vector<Animation> animations;
    int curAnimation;
    SDL_Texture *texture;
    SDL_FRect collider; // rectangle for collision
    int spriteFrame;
    GameObject() : data{.level = LevelData()}, collider{ 0 }
    {
        type = ObjectType::level;
        dir = 1;
        maxSpeedX = 0;
        pos = glm::vec2(0);
        curAnimation = -1;
        texture = nullptr; 
        spriteFrame = 1;
    }
};

/*enum class BulletState {
    moving, colliding, inactive
};
enum class EnemyState {
    idle, damaged, dead
};

struct PlayerData {
    PlayerState state;
    Timer weaponTimer;
    Timer deathTimer;
    int healthPoints;
    PlayerData() : weaponTimer(0.3f), deathTimer(3.0f)
    {
        state = PlayerState::idle;
        healthPoints = 1;
    }
};*/

/*struct EnemyData {
    EnemyState state;
    Timer damagedTimer;
    int healthPoints;
    EnemyData() : state(EnemyState::idle), damagedTimer(0.5f) {
        healthPoints = 3;
    }
};
struct BulletData {
    BulletState state;
    BulletData() : state(BulletState::moving)
    {

    }
};*/
