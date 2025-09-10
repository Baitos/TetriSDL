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
    int blockIndex;
    bool debugMode;

    GameState(const SDLState &state) {
        blockIndex = -1; // will change when map is loaded
        debugMode = false;
    }
    GameObject &block() {
        return layers[LAYER_IDX_CHARACTERS][blockIndex];
    }
};

struct Resources { // TODO: Define new textures and const int anims
    const int ANIM_BLOCK_IDLE = 0;
    std::vector<Animation> blockAnims;
    std::vector<SDL_Texture *> textures;
    SDL_Texture *texBlock, *texStone, *texBg;

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
        //draw stuff
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        // draw background
        SDL_RenderTexture(state.renderer, res.texBg, nullptr, nullptr);

        // draw bg tiles
        for (GameObject &obj : gs.bgTiles) {
            SDL_FRect dst {
                .x = obj.pos.x,
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
                .x = obj.pos.x,
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
            .x = obj.pos.x,
            .y = obj.pos.y,
            .w = width,
            .h = height
        };
        SDL_FlipMode flipMode = SDL_FLIP_HORIZONTAL; // don't need to rotate, should change
        SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode); // src is for sprite stripping, dest is for where sprite should be drawn
        if (gs.debugMode) {
            SDL_FRect rectA {
                .x = obj.pos.x + obj.collider.x, 
                .y = obj.pos.y + obj.collider.y,
                .w = obj.collider.w, 
                .h = obj.collider.h
            };
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);

            SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
            SDL_RenderFillRect(state.renderer, &rectA);
            SDL_FRect sensor{
			    .x = obj.pos.x + obj.collider.x,
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
    if (obj.type == ObjectType::block) {

    }
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