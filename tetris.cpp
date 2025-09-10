#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <format>

#include "headers/gameobject.h"

using namespace std;

struct SDLState
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width, height, logW, logH;
    const bool *keys;
    SDLState() : keys(SDL_GetKeyboardState(nullptr)) {

    }
};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 16;

struct GameState {
    std::array<std::vector<GameObject>, 2> layers;
    std::vector<GameObject> bgTiles;
    std::vector<GameObject> fgTiles;
    //std::vector<GameObject> bullets;
    int blockIndex;
    SDL_FRect mapViewport;
    //float bg2Scroll, bg3Scroll, bg4Scroll;
    bool debugMode;

    GameState(const SDLState &state) {
        blockIndex = -1; // will change when map is loaded
        mapViewport = SDL_FRect {
            .x = 0,
            .y = 0,
            .w = static_cast<float>(state.logW),
            .h = static_cast<float>(state.logH)
        };
        debugMode = false;
    }
    GameObject &block() {
        return layers[LAYER_IDX_CHARACTERS][blockIndex];
    }
};

struct Resources { // TODO: Define new textures and const int anims
    /*const int ANIM_PLAYER_IDLE = 0; 
    const int ANIM_PLAYER_RUN = 1;
    const int ANIM_PLAYER_SLIDE = 2;
    const int ANIM_PLAYER_SHOOT = 3;
    const int ANIM_PLAYER_JUMP = 4;
    const int ANIM_PLAYER_DIE = 5;
    std::vector<Animation> playerAnims;
    const int ANIM_BULLET_MOVING = 0;
    const int ANIM_BULLET_HIT = 1;
    std::vector<Animation> bulletAnims;
    const int ANIM_ENEMY = 0;
    const int ANIM_ENEMY_DEAD = 1;
    std::vector<Animation> enemyAnims;
    */
    const int ANIM_BLOCK_IDLE = 0;
    std::vector<Animation> blockAnims;
    std::vector<SDL_Texture *> textures;
    SDL_Texture *texBlock, *texStone, *texBg;
    /*SDL_Texture *texIdle, *texRun, *texJump, *texSlide, *texShoot, *texDie, 
                *texGrass, *texStone, *texBrick, *texFence, *texBush, 
                *texBullet, *texBulletHit, *texSpiny, *texSpinyDead,
                *texBg1, *texBg2, *texBg3, *texBg4;*/

    SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filepath) { // load texture from filepath
        // load game assets
        SDL_Texture *tex = IMG_LoadTexture(renderer, filepath.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST); // pixel perfect
        textures.push_back(tex);
        return tex;
    }

    void load(SDLState &state) { // TODO; make new texture and anims arrays 
        blockAnims.resize(1);
        blockAnims[ANIM_BLOCK_IDLE] = Animation(1, 1.0f); // 1 frames, 1.6 seconds
        texBlock = loadTexture(state.renderer, "data/block.png");
        texStone = loadTexture(state.renderer, "data/stone.png");
        texBg = loadTexture(state.renderer, "data/bg.png");
        /*playerAnims.resize(6); // 
        playerAnims[ANIM_PLAYER_IDLE] = Animation(1, 1.6f); // 1 frames, 1.6 seconds
        playerAnims[ANIM_PLAYER_RUN] = Animation(3, 0.3f);
        playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
        playerAnims[ANIM_PLAYER_SHOOT] = Animation(1, 0.3f);
        playerAnims[ANIM_PLAYER_JUMP] = Animation(1, 1.0f); 
        playerAnims[ANIM_PLAYER_DIE] = Animation(1, 1.0f);
        bulletAnims.resize(2); // 
        bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.5f);
        bulletAnims[ANIM_BULLET_HIT] = Animation(3, 0.5f);
        enemyAnims.resize(2);
        enemyAnims[ANIM_ENEMY] = Animation(2, 0.6f);
        enemyAnims[ANIM_ENEMY_DEAD] = Animation(1, 1.0f);

        if (real) {
            texIdle = loadTexture(state.renderer, "data/IdleL.png");
            texRun = loadTexture(state.renderer, "data/WalkLRL.png");
            texJump = loadTexture(state.renderer, "data/JumpL.png");
            texSlide = loadTexture(state.renderer, "data/SlideL.png");
            texShoot = loadTexture(state.renderer, "data/ShootL.png");
            texDie = loadTexture(state.renderer, "data/DieL.png");
            texBullet = loadTexture(state.renderer, "data/fireballL.png");
            texBulletHit = loadTexture(state.renderer, "data/fireballHitL.png");
        } else {
            texIdle = loadTexture(state.renderer, "data/IdleM.png");
            texRun = loadTexture(state.renderer, "data/WalkLRM.png");
            texJump = loadTexture(state.renderer, "data/JumpM.png");
            texSlide = loadTexture(state.renderer, "data/SlideM.png");
            texShoot = loadTexture(state.renderer, "data/ShootM.png");
            texDie = loadTexture(state.renderer, "data/DieM.png");
            texBullet = loadTexture(state.renderer, "data/fireballM.png");
            texBulletHit = loadTexture(state.renderer, "data/fireballHitM.png");
        }
        texGrass = loadTexture(state.renderer, "data/grass.png");
        texBrick = loadTexture(state.renderer, "data/brick.png");
        texStone = loadTexture(state.renderer, "data/stone.png");
        texBush = loadTexture(state.renderer, "data/bush.png");
        texFence = loadTexture(state.renderer, "data/fence.png");
        texBg1 = loadTexture(state.renderer, "data/bg_layer1.png");
        texBg2 = loadTexture(state.renderer, "data/bg_layer2.png");
        texBg3 = loadTexture(state.renderer, "data/bg_layer3.png");
        texBg4 = loadTexture(state.renderer, "data/bg_layer4.png");
        texSpiny = loadTexture(state.renderer, "data/Spiny.png");
        texSpinyDead = loadTexture(state.renderer, "data/SpinyDead.png");*/
    }

    void unload() {
        for (SDL_Texture *tex : textures) {
            SDL_DestroyTexture(tex);
        }
    }
};

bool initialize(SDLState &state);
void cleanup(SDLState &state);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float width, float height, float deltaTime);
void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime);
void createTiles(const SDLState &state, GameState &gs, const Resources &res);
void handleKeyInput(const SDLState &state, GameState &gs, GameObject &obj,
                    SDL_Scancode key, bool keyDown);

bool running = true;

int main(int argc, char** argv) { // SDL needs to hijack main to do stuff; include argv/argc
    SDLState state;
    state.width = 1600;
    state.height = 900;
    state.logW = 640;
    state.logH = 480;

    if (!initialize(state)) {
        return 1;
    }
    // load game assets
    Resources res;
    res.load(state);

    // setup game data
    GameState gs(state);
    createTiles(state, gs, res);
    uint64_t prevTime = SDL_GetTicks();

    // start game loop
    while (running) {
        uint64_t nowTime = SDL_GetTicks(); // take time from previous frame to calculate delta
        float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds from ms
        SDL_Event event { 0 };
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                {
                    running = false;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED: 
                {
                    state.width = event.window.data1;
                    state.height = event.window.data2;
                    
                    //printf("Width = %d, Height = %d", state.width, state.height);
                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                {
                    handleKeyInput(state, gs, gs.block(), event.key.scancode, true);
                    break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    handleKeyInput(state, gs, gs.block(), event.key.scancode, false);
                    if (event.key.scancode == SDL_SCANCODE_F12) {
                        gs.debugMode = !gs.debugMode;
                    }
                    break;
                }
            }
        }

        // update objs
        for (auto &layer : gs.layers) {
            for (GameObject &obj : layer) {
                update(state, gs, res, obj, deltaTime);
            }
        }
        // update bullets, probably won't need
        /*for (GameObject &bullet : gs.bullets) {
            update(state, gs, res, bullet, deltaTime);
        }*/ 
        // used for camera system, shouldn't need
        gs.mapViewport.x = (gs.block().pos.x + TILE_SIZE / 2) - (gs.mapViewport.w / 2); 
        //draw stuff
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        // draw background
        SDL_RenderTexture(state.renderer, res.texBg, nullptr, nullptr);
        //drawParallaxBackground(state.renderer, res.texBg4, gs.player().vel.x, gs.bg4Scroll, 0.075f, deltaTime);
        //drawParallaxBackground(state.renderer, res.texBg3, gs.player().vel.x, gs.bg3Scroll, 0.15f, deltaTime);
        //drawParallaxBackground(state.renderer, res.texBg2, gs.player().vel.x, gs.bg2Scroll, 0.3f, deltaTime);

        // draw bg tiles
        for (GameObject &obj : gs.bgTiles) {
            SDL_FRect dst {
                .x = obj.pos.x - gs.mapViewport.x,
                .y = obj.pos.y,
                .w = static_cast<float>(obj.texture->w),
                .h = static_cast<float>(obj.texture->h)
            };
            SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
        }

        // draw objs
        for (auto &layer : gs.layers) {
            for (GameObject &obj : layer) {
                drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
            }
        }

        // draw bullets
        /*for (GameObject &bullet : gs.bullets) {
            if (bullet.data.bullet.state != BulletState::inactive) {
                drawObject(state, gs, bullet, bullet.collider.w, bullet.collider.h, deltaTime);
            }
        }*/

        // draw fg tiles
        for (GameObject &obj : gs.fgTiles) {
            SDL_FRect dst {
                .x = obj.pos.x - gs.mapViewport.x,
                .y = obj.pos.y,
                .w = static_cast<float>(obj.texture->w),
                .h = static_cast<float>(obj.texture->h)
            };
            SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
        }

        if (gs.debugMode) {
        // debug info
            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            /*SDL_RenderDebugText(state.renderer, 5, 5,
                            std::format("State: {}, Bullet: {}, Grounded: {}", 
                            static_cast<int>(gs.player().data.player.state), gs.bullets.size(), gs.player().grounded).c_str());*/
        }
        //swap buffers and present
        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }

    res.unload();
    cleanup(state);
    return 0;
}

bool initialize(SDLState &state) {
    bool initSuccess = true;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error Initializing SDL3", nullptr);
        initSuccess = false;
    } 
    state.window = SDL_CreateWindow("TetriSDL", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if (!state.window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error Creating Window", nullptr);
        cleanup(state);
        initSuccess = false;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error Creating Renderer", nullptr);
        cleanup(state);
        initSuccess = false;
    }

    SDL_SetRenderVSync(state.renderer, 1); // turn this SHIT off

    // configure presentation
    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return initSuccess;
}

void cleanup(SDLState &state) {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float width, float height, float deltaTime) {
        float srcX = obj.curAnimation != -1 
                     ? obj.animations[obj.curAnimation].currentFrame() * width 
                     : (obj.spriteFrame - 1) * width;
        SDL_FRect src {
            .x = srcX,
            .y = 0,
            .w = width,
            .h = height
        };

        SDL_FRect dst {
            .x = obj.pos.x - gs.mapViewport.x,
            .y = obj.pos.y,
            .w = width,
            .h = height
        };
        SDL_FlipMode flipMode = obj.dir == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        if (!obj.shouldFlash) {
            SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode); // src is for sprite stripping, dest is for where sprite should be drawn
        } else {
            // flash with white tint
            SDL_SetTextureColorModFloat(obj.texture, 2.5f, 2.5f, 2.5f);  
            SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
            SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);
            if (obj.flashTimer.step(deltaTime)) {
                obj.shouldFlash = false;
            }
        }


        if (gs.debugMode) {
            SDL_FRect rectA {
                .x = obj.pos.x + obj.collider.x - gs.mapViewport.x, 
                .y = obj.pos.y + obj.collider.y,
                .w = obj.collider.w, 
                .h = obj.collider.h
            };
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);

            SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
            SDL_RenderFillRect(state.renderer, &rectA);
            SDL_FRect sensor{
			    .x = obj.pos.x + obj.collider.x - gs.mapViewport.x,
			    .y = obj.pos.y + obj.collider.y + obj.collider.h,
			    .w = obj.collider.w, 
                .h = 1
		    };
		    SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 150);
		    SDL_RenderFillRect(state.renderer, &sensor);

            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
        }
}

void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime) {
    // update animation
    if (obj.curAnimation != -1) {
        obj.animations[obj.curAnimation].step(deltaTime);
    }
    if (obj.dynamic && !obj.grounded) {
        //obj.vel += glm::vec2(0, 700) * deltaTime; // gravity
        //printf("x=%d, y=%d\n", obj.pos.x, obj.pos.y);
    }
    float currentDirection = 0;
    /*if (obj.type == ObjectType::player) {
        if (obj.data.player.state != PlayerState::dead) {
            if (state.keys[SDL_SCANCODE_A]) {
                currentDirection += -1;
            }
            if (state.keys[SDL_SCANCODE_D]) {
                currentDirection += 1;
            }
            Timer &weaponTimer = obj.data.player.weaponTimer;
            weaponTimer.step(deltaTime);
            const auto handleShooting = [&state, &gs, &res, &obj, &weaponTimer]() {
                if (state.keys[SDL_SCANCODE_J]) {
                    // bullets!
                     // in 2.5 hour video, go to 1:54:19 if you want to sync up shooting sprites with animations for running
                    if (weaponTimer.isTimeOut()) {
                        /*if (obj.data.player.state == PlayerState::idle) {
                            obj.texture = res.texShoot;
                            obj.curAnimation = res.ANIM_PLAYER_SHOOT;
                        }
                        weaponTimer.reset();
                        GameObject bullet;
                        bullet.data.bullet = BulletData();
                        bullet.type = ObjectType::bullet;
                        bullet.dir = gs.player().dir;
                        bullet.texture = res.texBullet;
                        bullet.curAnimation = res.ANIM_BULLET_MOVING;
                        bullet.collider = SDL_FRect {
                            .x = 0,
                            .y = 0,
                            .w = static_cast<float>(res.texBullet->h),
                            .h = static_cast<float>(res.texBullet->h)
                        };
                        const float left = 0;
                        const float right = 24;
                        const float t = (obj.dir + 1) / 2.0f; // results in 0 to 1
                        const float xOffset = left + right * t; // LERP between left and right
                        const float yVariation = 40;
                        const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
                        bullet.vel = glm::vec2(
                        obj.vel.x + 300.0f * obj.dir, yVelocity);
                        //printf("bullet.vel.x = %f\n", bullet.vel.x);
                        bullet.maxSpeedX = 1000.0f;
                        bullet.animations = res.bulletAnims;
                        bullet.pos = glm::vec2( 
                            obj.pos.x + xOffset,
                            obj.pos.y + TILE_SIZE / 2 + 1
                        );
                        // try to reuse old inactive bullets
                        bool foundInactive = false;
                        for (int i = 0; i < gs.bullets.size() && !foundInactive; i++) {
                            if (gs.bullets[i].data.bullet.state == BulletState::inactive) {
                                foundInactive = true;
                                gs.bullets[i] = bullet;
                            }
                        }
                        // otherwise push new bullet
                        if (!foundInactive) {
                            gs.bullets.push_back(bullet);
                        }
                    }
                }
            };
            switch (obj.data.player.state) {
                case PlayerState::idle:
                {
                    if(currentDirection) { // if moving change to running
                        obj.data.player.state = PlayerState::running;
                    }
                    else {
                        if (obj.vel.x) { // slow player down when idle
                            const float factor = obj.vel.x > 0 ? -1.5f : 1.5f;
                            float amount = factor * obj.acc.x * deltaTime;
                            if (std::abs(obj.vel.x) < std::abs(amount)) {
                                obj.vel.x = 0;
                            }
                            else {
                                obj.vel.x += amount;
                            }
                        }
                    }
                    obj.texture = res.texIdle;
                    obj.curAnimation = res.ANIM_PLAYER_IDLE;
                    handleShooting();
                    break;
                }
                case PlayerState::running:
                {
                    if (!currentDirection && obj.grounded) { // if not moving return to idle
                        obj.data.player.state = PlayerState::idle;
                    }
                    if (obj.vel.x * obj.dir < 0 && obj.grounded) { // moving in different direction of vel, sliding
                        obj.texture = res.texSlide;
                        obj.curAnimation = res.ANIM_PLAYER_SLIDE;
                    } else {
                        obj.texture = res.texRun;
                        obj.curAnimation = res.ANIM_PLAYER_RUN;
                    }
                    handleShooting();
                    break;
                }
                case PlayerState::jumping:
                {
                    obj.texture = res.texJump;
                    obj.curAnimation = res.ANIM_PLAYER_JUMP;
                    handleShooting();
                    break;
                }
            }
            if (obj.pos.y - gs.mapViewport.y > state.logH) {
                obj.data.player.state = PlayerState::dead; // die if you fall off
                obj.vel.x = 0;
            }
            //printf("Player x = %f, Player y = %f\n", obj.pos.x, obj.pos.y);
        } else { // player is dead, reset map
            Timer &deathTimer = obj.data.player.deathTimer;
            deathTimer.step(deltaTime);
            if (deathTimer.isTimeOut()) {
                running = false; // exit program
            }
        }
        
    } else if (obj.type == ObjectType::bullet) {
        switch (obj.data.bullet.state) {
            case BulletState::moving: {
                if (obj.pos.x - gs.mapViewport.x < 0 || // left side
                    obj.pos.x - gs.mapViewport.x > state.logW || // right side
                    obj.pos.y - gs.mapViewport.y < 0 || // up
                    obj.pos.y - gs.mapViewport.y > state.logH) // down
                { 
                    obj.data.bullet.state = BulletState::inactive;
                }
                break;
            }
            case BulletState::colliding: {
                if (obj.animations[obj.curAnimation].isDone()) {
                    obj.data.bullet.state = BulletState::inactive;
                }
            }
        }
    } else if (obj.type == ObjectType::enemy) {
        EnemyData &d = obj.data.enemy;
        switch (d.state) {
            case EnemyState::idle: {
                glm::vec2 playerDir = gs.player().pos - obj.pos;
                if (glm::length(playerDir) < 100) {
                    currentDirection = playerDir.x < 0 ? -1 : 1;
                } else {
                    obj.acc = glm::vec2(0);
                    obj.vel.x = 0;
                }
                break;
            } // this is for proximity based movement, ignore
            case EnemyState::damaged:
            {
                if (d.damagedTimer.step(deltaTime)) {
                    // do nothing
                }
                break;
            }
            case EnemyState::dead: {
                obj.vel.x = 0;
                if (obj.curAnimation != -1 && obj.animations[obj.curAnimation].isDone()) {
                    obj.curAnimation = -1;
                    obj.spriteFrame = 1;
                }
                break;
            }
        }
    }*/
    if (currentDirection) {
        obj.dir = currentDirection;
    }
    obj.vel += currentDirection * obj.acc * deltaTime;
    if (std::abs(obj.vel.x) > obj.maxSpeedX) {
        obj.vel.x = currentDirection * obj.maxSpeedX;
    }
    // add vel to pos
    obj.pos += obj.vel * deltaTime;
    // collision
    bool foundGround = false;
    for (auto &layer : gs.layers) {
        for (GameObject &objB : layer) {
            if (&obj != &objB) {
                //checkCollision(state, gs, res, obj, objB, deltaTime);
                if (objB.type == ObjectType::level) {
                    // grounded sensor
                    const float inset = 2.0;
                    SDL_FRect sensor {
                        .x = obj.pos.x + obj.collider.x + 1,
                        .y = obj.pos.y + obj.collider.y + obj.collider.h,
                        .w = obj.collider.w - inset,
                        .h = 1
                    };
                    SDL_FRect rectB {
                        .x = objB.pos.x + objB.collider.x,
                        .y = objB.pos.y + objB.collider.y,
                        .w = objB.collider.w,
                        .h = objB.collider.h
                    };
                    SDL_FRect rectC { 0 };
                    if (SDL_GetRectIntersectionFloat(&sensor, &rectB, &rectC)) {
                        foundGround = true;
                    }
                }    
            }
        }
    }
    /*if (obj.grounded != foundGround) { // changing state
        obj.grounded = foundGround;
        if (foundGround && obj.type == ObjectType::player && obj.data.player.state != PlayerState::dead) {
            obj.data.player.state = PlayerState::running;
        }
    }*/
}

void createTiles(const SDLState &state, GameState &gs, const Resources &res) { // 50 x 5
    /*
        1 - Block 
        2 - Wall
    */
    short map[MAP_ROWS][MAP_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
    short foreground[MAP_ROWS][MAP_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
    short background[MAP_ROWS][MAP_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
    const auto loadMap = [&state, &gs, &res](short layer[MAP_ROWS][MAP_COLS])
    {
        const auto createObject = [&state](int r, int c, SDL_Texture *tex, ObjectType type) {
            GameObject o;
            o.type = type; 
            o.pos = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE); // subtract r from map rows to not be backwards. drawn top to bottom and flush with resolution
            o.texture = tex;
            o.collider = {
                .x = 0,
                .y = 0,
                .w = TILE_SIZE,
                .h = TILE_SIZE
            };
            return o;
        };
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                switch (layer[r][c]) {
                    case 1: // player
                    {
                        GameObject block = createObject(r, c, res.texBlock, ObjectType::block);
                        block.data.block = BlockData(); // initialize block data to idle
                        block.animations = res.blockAnims; // load anims
                        block.curAnimation = res.ANIM_BLOCK_IDLE; // set player anim to idle
                        //block.acc = glm::vec2(300, 0);
                        //player.maxSpeedX = 150;
                        block.dynamic = true;
                        block.collider = { 
                            .x = 0,
                            .y = 0,
                            .w = TILE_SIZE,
                            .h = TILE_SIZE 
                        };
                        gs.layers[LAYER_IDX_CHARACTERS].push_back(block); // put into array
                        gs.blockIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;
                        break;
                    }
                    case 2: // wall
                    {
                        GameObject o = createObject(r, c, res.texStone, ObjectType::level);
                        gs.layers[LAYER_IDX_LEVEL].push_back(o);
                        break;
                    }
                }
            }
        }
    };
    loadMap(map);
    loadMap(background);
    loadMap(foreground);
    assert(gs.blockIndex != -1);
}

void handleKeyInput(const SDLState &state, GameState &gs, GameObject &obj,
                    SDL_Scancode key, bool keyDown) {

}