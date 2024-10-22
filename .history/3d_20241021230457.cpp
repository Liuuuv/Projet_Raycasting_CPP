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
    float angle;
    float fov = 45 * PI / 180;
    float speed = 0.003;
};

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

// Fonction pour calculer la couleur en fonction de la distance
SDL_Color getColor(float distance) {
    float maxDistance = 7.0f; // Peut être ajusté
    float intensity = fmin(pow(1.5f,distance) / maxDistance, 1.0f); // Valeur entre 0 et 1
    Uint8 colorValue = static_cast<Uint8>((1 - intensity) * 200); // Couleur foncée
    return {colorValue, colorValue, colorValue, 255}; // Couleur en niveaux de gris
}

void render(SDL_Renderer* renderer, Player& player) {
    // Effacer l'écran
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int x = 0; x < WIDTH; x++) {
        float rayAngle = player.angle - player.fov / 2 + (x / (float)WIDTH) * player.fov;
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
    // Vérifie si la position est dans les limites du tableau et pas un mur
    return (x >= 0 && x < 10 && y >= 0 && y < 10 && map[(int)y][(int)x] != 1);

    
}

// Vérifie si le joueur est collé à un mur
bool isAgainstWall(float playerX, float playerY) {
    // Vérifie si le joueur est à proximité d'un mur
    return (map[(int)playerY][(int)playerX] == 1);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("RAYCASTING WHOUUUUU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Player player = {1.5f, 1.5f, 0}; // Position et angle de départ

    // Capturer la souris
    SDL_SetRelativeMouseMode(SDL_TRUE); // Activer le mode souris relative

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Mettre à jour l'angle du joueur en fonction du mouvement de la souris
        player.angle += (mouseX - (WIDTH / 2)) * 0.001f; // Ajuster la sensibilité si nécessaire
        SDL_WarpMouseInWindow(window, WIDTH / 2, HEIGHT / 2); // Remettre la souris au centre

        const Uint8* state = SDL_GetKeyboardState(NULL);
        // (en qwerty)


    // Met à jour le mouvement
    float pos_x;
    float pos_y;

    // Initialiser pos_x et pos_y à la position actuelle du joueur
    pos_x = player.x;
    pos_y = player.y;

    // Mouvement
    if (state[SDL_SCANCODE_W]) {  // Avancer
        float new_x = pos_x + cos(player.angle) * player.speed;
        float new_y = pos_y + sin(player.angle) * player.speed;

        if (canMoveTo(new_x, new_y)) {
            pos_x = new_x;
            pos_y = new_y;
        }
    }
    if (state[SDL_SCANCODE_S]) {  // Reculer
        float new_x = pos_x - cos(player.angle) * player.speed;
        float new_y = pos_y - sin(player.angle) * player.speed;

        if (canMoveTo(new_x, new_y)) {
            pos_x = new_x;
            pos_y = new_y;
        }
    }
    if (state[SDL_SCANCODE_A]) {  // Gauche
        float new_x = pos_x + sin(player.angle) * player.speed;
        float new_y = pos_y - cos(player.angle) * player.speed;

        if (canMoveTo(new_x, new_y)) {
            pos_x = new_x;
            pos_y = new_y;
        }
    }
    if (state[SDL_SCANCODE_D]) {  // Droite
        float new_x = pos_x - sin(player.angle) * player.speed;
        float new_y = pos_y + cos(player.angle) * player.speed;

        if (canMoveTo(new_x, new_y)) {
            pos_x = new_x;
            pos_y = new_y;
        }
    }

    // Met à jour la position du joueur seulement si le mouvement est valide
    if (canMoveTo(pos_x, pos_y)) {
        player.x = pos_x;
        player.y = pos_y;
    }
            
        render(renderer, player);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
