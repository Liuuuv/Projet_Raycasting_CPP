#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>  // Inclure SDL_ttf pour le texte
#include <cmath>
#include <iostream>
#include <sstream>

const int WIDTH = 800;
const int HEIGHT = 600;
const float PI = 3.14159265f;
const int TARGET_FPS = 60; // Nombre de FPS souhaité
const int FRAME_DELAY = 1000 / TARGET_FPS; // Durée d'une frame en millisecondes
const float DELTA_TIME = 1.0f/TARGET_FPS;

// Map data (0 = vide, 1 = mur)
int map[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
    float x;
    float y;
    float angle;
    float fov = 60 * PI / 180;
    float speed = 5.0f;
    float thickness = 0.1f;
    float sensitivity = 0.05f;
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
        float rayAngle = player.angle - player.fov / 2 + (x / (float)WIDTH) * player.fov;
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

        int wallHeight = (int)(HEIGHT / (distance + 0.0001));
        int drawStart = (HEIGHT - wallHeight) / 2;
        int drawEnd = drawStart + wallHeight;

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

// Fonction pour rendre du texte à l'écran
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
    TTF_Init();  // Initialiser SDL_ttf

    SDL_Window* window = SDL_CreateWindow("RAYCASTING WHOUUUUU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24); // Charge la police (remplacer par le chemin de votre police)
    if (!font) {
        std::cerr << "Erreur de chargement de la police : " << TTF_GetError() << std::endl;
        return -1;
    }

    Player player = {1.5f, 1.5f, 0}; // Position et angle de départ
    SDL_SetRelativeMouseMode(SDL_TRUE); // Activer le mode souris relative

    bool running = true;
    SDL_Event event;

    Uint32 frameStart;
    int frameTime;
    int frameCount = 0;
    float avgFPS = 0;

    while (running) {
        frameStart = SDL_GetTicks(); // Chronomètre pour mesurer le temps de la frame

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        player.angle += (mouseX - (WIDTH / 2)) * player.sensitivity * DELTA_TIME;
        SDL_WarpMouseInWindow(window, WIDTH / 2, HEIGHT / 2);

        const Uint8* state = SDL_GetKeyboardState(NULL);
        float pos_x = player.x;
        float pos_y = player.y;
        bool sliding = false;

        if (state[SDL_SCANCODE_W]) {
            float new_x = pos_x + cos(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y + sin(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x + cos(player.angle) * player.thickness * DELTA_TIME, new_y + sin(player.angle) * player.thickness * DELTA_TIME)) {
                pos_x = new_x;
                pos_y = new_y;
            }
        }
        if (state[SDL_SCANCODE_S]) {
            float new_x = pos_x - cos(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y - sin(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x - cos(player.angle) * player.thickness * DELTA_TIME, new_y - sin(player.angle) * player.thickness * DELTA_TIME)) {
                pos_x = new_x;
                pos_y = new_y;
            }
        }
        if (state[SDL_SCANCODE_A]) {
            float new_x = pos_x + sin(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y - cos(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x + sin(player.angle) * player.thickness * DELTA_TIME, new_y - cos(player.angle) * player.thickness * DELTA_TIME)) {
                pos_x = new_x;
                pos_y = new_y;
            }
        }
        if (state[SDL_SCANCODE_D]) {
            float new_x = pos_x - sin(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y + cos(player.angle) * player.speed * DELTA_TIME;

            if (canMoveTo(new_x - sin(player.angle) * player.thickness * DELTA_TIME, new_y + cos(player.angle) * player.thickness * DELTA_TIME)) {
                pos_x = new_x;
                pos_y = new_y;
            }
        }

        if (isAgainstWall(pos_x, pos_y)) {
            sliding = true;
        } else {
            sliding = false;
        }

        if (sliding) {
            float slideAngle = player.angle + (M_PI / 2);
            float slideX = cos(slideAngle) * player.speed * 0.5f * DELTA_TIME;
            float slideY = sin(slideAngle) * player.speed * 0.5f * DELTA_TIME;

            if (canMoveTo(pos_x + slideX, pos_y + slideY)) {
                pos_x += slideX;
                pos_y += slideY;
            }
        }

        if (canMoveTo(pos_x, pos_y)) {
            player.x = pos_x;
            player.y = pos_y;
        }

        render(renderer, player);

        frameTime = SDL_GetTicks() - frameStart; // Calculer le temps pris par la frame
        frameCount++;
        avgFPS = frameCount / (SDL_GetTicks() / 1000.0f); // Calculer les FPS moyens

        std::stringstream ss;
        ss << "FPS: " << avgFPS; // Créer un string pour les FPS
        SDL_Color textColor = {255, 255, 255, 255}; // Couleur blanche
        renderText(renderer, font, ss.str(), textColor, 10, 10); // Afficher le texte à l'écran

        SDL_RenderPresent(renderer);

        // Attendre pour maintenir le taux de FPS désiré
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
