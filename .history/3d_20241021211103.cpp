#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>

const int WIDTH = 800;
const int HEIGHT = 600;
const float PI = 3.14159265f;

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
    float angle; // Angle de vue
};

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void render(SDL_Renderer* renderer, Player& player) {
    // Effacer l'écran
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int x = 0; x < WIDTH; x++) {
        // Calculer l'angle de projection
        float rayAngle = player.angle - (PI / 4) + (x / (float)WIDTH) * (PI / 2);

        float rayX = cos(rayAngle);
        float rayY = sin(rayAngle);
        float distance = 0;

        // Raycasting
        while (true) {
            int testX = (int)(player.x + rayX * distance);
            int testY = (int)(player.y + rayY * distance);
            if (testX < 0 || testX >= 10 || testY < 0 || testY >= 10 || map[testY][testX] == 1) {
                break;
            }
            distance += .01;
        }

        // Dessiner la ligne
        int wallHeight = (int)(HEIGHT / (distance + 0.0001)); // Éviter la division par zéro
        int drawStart = (HEIGHT - wallHeight) / 2;
        int drawEnd = drawStart + wallHeight;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawLine(renderer, x, drawStart, x, drawEnd);
    }

    // Afficher les rendus
    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raycasting Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Player player = {1.5f, 1.5f, 0}; // Position et angle de départ

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_LEFT]) player.angle -= 0.001f; // Tourner à gauche
        if (state[SDL_SCANCODE_RIGHT]) player.angle += 0.001f; // Tourner à droite
        if (state[SDL_SCANCODE_UP]) {
            player.x += cos(player.angle) * 0.001f;
            player.y += sin(player.angle) * 0.001f; // Avancer
        }
        if (state[SDL_SCANCODE_DOWN]) {
            player.x -= cos(player.angle) * 0.001f;
            player.y -= sin(player.angle) * 0.001f; // Reculer
        }

        render(renderer, player);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
