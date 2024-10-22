#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <sstream>

const int WIDTH = 800;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH/(float)HEIGHT;
const float PI = 3.14159265f;
const int TARGET_FPS = 144;
const int FRAME_DELAY = 1000 / TARGET_FPS;
const float DELTA_TIME = 1.0f/TARGET_FPS;

int map[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
    float x;
    float y;
    float angle;
    float pitch = 0;  // Nouvelle variable pour le "head pitch"
    float horizontal_fov = 130 * PI / 180;
    //float vertical_fov = 2.0f*atanf(tan(horizontal_fov/2.0f)*ASPECT_RATIO);
    float vertical_fov = 1000 * PI / 180;
    
    float speed = 2.0f;
    float thickness = 0.1f;
    float sensitivity = 0.0011f;
    float verticalSensitivity = 0.005f;  // Sensibilité pour le mouvement vertical
};

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

SDL_Color getColor(float distance) {
    float maxDistance = 7.0f;
    float intensity = fmin(pow(1.5f, distance) / maxDistance, 1.0f);
    Uint8 colorValue = static_cast<Uint8>((1 - intensity) * 200);
    return {colorValue, colorValue, colorValue, 255};
}

void render(SDL_Renderer* renderer, Player& player) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    

    for (int x = 0; x < WIDTH; x++) {
        float rayAngle = player.angle - player.horizontal_fov / 2 + (x / (float)WIDTH) * player.horizontal_fov;
        float rayX = cos(rayAngle);
        float rayY = sin(rayAngle);
        float distance = 0;

        while (true) {
            int testX = (int)(player.x + rayX * distance);
            int testY = (int)(player.y + rayY * distance);
            if (testX < 0 || testX >= 10 || testY < 0 || testY >= 10 || map[testY][testX] == 1) {
                break;
            }
            distance += 0.01;
        }
        distance /= cos(player.angle - rayAngle);

        // int wallHeight = (int)(HEIGHT / (distance + 0.0001));
        // int drawStart = (HEIGHT - wallHeight) / 2 + player.pitch*0;
        // int drawEnd = drawStart + wallHeight;
        float wallHeight =  0.5f;    // Hauteur du mur dans le jeu
        int drawStart = (int)floor(distance *tanf(player.vertical_fov/2.0f) - wallHeight/2.0f);
        int drawEnd = (int)floor((float)HEIGHT - distance * tanf(player.vertical_fov/2.0f) + wallHeight/2.0f);
        

        SDL_Color color = getColor(distance);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        drawLine(renderer, x, drawStart, x, drawEnd);
    }

    SDL_RenderPresent(renderer);
}

bool canMoveTo(float x, float y) {
    return (x >= 0 && x < 10 && y >= 0 && y < 10 && map[(int)y][(int)x] != 1);
}

bool isAgainstWall(float playerX, float playerY) {
    return (map[(int)playerY][(int)playerX] == 1);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, std::string text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("RAYCASTING WHOUUUUU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
    if (!font) {
        std::cerr << "Erreur de chargement de la police : " << TTF_GetError() << std::endl;
        return -1;
    }

    Player player = {1.5f, 1.5f, 0};
    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool running = true;
    SDL_Event event;

    Uint32 frameStart;
    int frameTime;
    int frameCount = 0;
    float avgFPS = 0;

    while (running) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        player.angle += (mouseX - (WIDTH / 2)) * player.sensitivity;
        player.pitch -= (mouseY - (HEIGHT / 2)) * player.verticalSensitivity;  // Ajuster le pitch
        SDL_WarpMouseInWindow(window, WIDTH / 2, HEIGHT / 2);

        const Uint8* state = SDL_GetKeyboardState(NULL);
        float pos_x = player.x;
        float pos_y = player.y;

        if (state[SDL_SCANCODE_W]) {
            float new_x = pos_x + cos(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y + sin(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x + cos(player.angle) * player.thickness, player.y + sin(player.angle) * player.thickness)) {
                pos_x = new_x;
            }
            if (canMoveTo(player.x + cos(player.angle) * player.thickness, new_y + sin(player.angle) * player.thickness)) {
                pos_y = new_y;
            }
        }
        if (state[SDL_SCANCODE_S]) {
            float new_x = pos_x - cos(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y - sin(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x - cos(player.angle) * player.thickness, player.y - sin(player.angle) * player.thickness)) {
                pos_x = new_x;
            }
            if (canMoveTo(player.x - cos(player.angle) * player.thickness, new_y - sin(player.angle) * player.thickness)) {
                pos_y = new_y;
            }
        }
        if (state[SDL_SCANCODE_A]) {
            float new_x = pos_x + sin(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y - cos(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x + sin(player.angle) * player.thickness, player.y - cos(player.angle) * player.thickness)) {
                pos_x = new_x;
            }
            if (canMoveTo(player.x + sin(player.angle) * player.thickness, new_y - cos(player.angle) * player.thickness)) {
                pos_y = new_y;
            }
        }
        if (state[SDL_SCANCODE_D]) {
            float new_x = pos_x - sin(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y + cos(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x - sin(player.angle) * player.thickness, player.y + cos(player.angle) * player.thickness)) {
                pos_x = new_x;
            }
            if (canMoveTo(player.y - sin(player.angle) * player.thickness, new_y + cos(player.angle) * player.thickness)) {
                pos_y = new_y;
            }
        }

        player.x = pos_x;
        player.y = pos_y;

        render(renderer, player);

        frameTime = SDL_GetTicks() - frameStart;
        frameCount++;
        avgFPS = frameCount / (SDL_GetTicks() / 1000.0f);

        std::stringstream ss;
        ss << "FPS: " << avgFPS;
        SDL_Color textColor = {255, 255, 255, 255};
        renderText(renderer, font, ss.str(), textColor, 10, 10);

        SDL_RenderPresent(renderer);

        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
