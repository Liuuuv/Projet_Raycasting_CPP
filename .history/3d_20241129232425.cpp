#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "liste.h"

const int WIDTH = 800;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH/(float)HEIGHT;
const float PI = 3.14159265f;
const int TARGET_FPS = 144;
const int FRAME_DELAY = 1000 / TARGET_FPS;
const float DELTA_TIME = 1.0f/TARGET_FPS;


// Définition de la carte, 0 : vide, 1 : mur
int map[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Définition des attributs du joueur
struct Player {
    float x;    // Position x
    float y;    // Position y
    float angle;    // Rotation horizontale
    float pitch = 0;    // Rotation verticale
    float horizontalFOV = 65 * PI / 180;    // Champ de vision horizontal
    float verticalFOV = 2.0f*atanf(tan(horizontalFOV/2.0f)*ASPECT_RATIO);   // Champ de vision vertical, dépend du format
    // float verticalFOV = 120 * PI / 180;
    
    float speed = 2.0f;     // Vitesse
    float thickness = 0.2f;     // Epaisseur (éviter de coller les murs)
    float sensitivity = 0.0008f;    // Sensibilité horizontale de la souris
    float verticalSensitivity = 0.0008f;    // Sensibilité horizontale de la souris
};

// Fonction pour borner un float entre deux float
float clamp(float value, float min, float max) {  
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

// Fonction pour dessiner un trait entre 2 points : (x_1, y_1) et (x_2, y_2)
void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}


// Fonction pour avoir l'intensité (float dans [0,1]) en fonction de la distance du mur
float getIntensity(float distance) {
    float maxDistance = 5.0f;
    float intensity = fmin(distance / maxDistance, 1.0f);
    intensity = 1 - intensity;
    return intensity;
    // Uint8 colorValue = static_cast<Uint8>((1 - intensity) * 230);
    // return {colorValue, colorValue, colorValue, 255};
}

// Fonction pour envoyer un rayon dans une direction
float sendRay(Player player, float rayAngle, float *distance, bool *flipTexture) {
    
    float rayX = cos(rayAngle);
    float rayY = sin(rayAngle);

    // Calcul des pas en x et y pour parcourir une case
    float xUnit = sqrtf(1 + powf(rayY / rayX, 2));
    float yUnit = sqrtf(1 + powf(rayX / rayY, 2));

    // Calcul des pas en x et y à faire pour s'avancer d'une case (selon la direction du rayon)
    int stepX = (rayX < 0) ? -1 : 1;
    int stepY = (rayY < 0) ? -1 : 1;

    // Calcul de la position initiale dans la grille
    int testX = (int)player.x;
    int testY = (int)player.y;

    // Calcul du premier point d'intersection avec la grille
    float sideDistX = (rayX < 0) ? (player.x - testX) * xUnit : (testX + 1.0f - player.x) * xUnit;
    float sideDistY = (rayY < 0) ? (player.y - testY) * yUnit : (testY + 1.0f - player.y) * yUnit;

    bool hit = false;   // True si on a touché un mur, False sinon
    bool hitVertical = false; // True si c'est un mur vertical, false si horizontal
    

    while (!hit) {

        // Comparer les distances et avancer selon l'axe le plus proche
        if (sideDistX < sideDistY) {
            sideDistX += xUnit;  // Prochaine intersection en X
            testX += stepX;      // Avancer dans la direction X
            hitVertical = true;  // On a frappé un potentiel mur vertical
        } else {
            sideDistY += yUnit;  // Prochaine intersection en Y
            testY += stepY;      // Avancer dans la direction Y
            hitVertical = false; // On a frappé un potentiel mur horizontal
        }

        if (testX < 0 || testX >= 10 || testY < 0 || testY >= 10 || map[testY][testX] == 1) {  // Rester dans la grille + Verifier mur touché
            hit = true;
            
            // La distance finale est la distance sur l'axe qui a été touché en premier (X ou Y)
            if (hitVertical) {
                *distance = (testX - player.x + (1 - stepX) / 2) / rayX;
            } else {
                *distance = (testY - player.y + (1 - stepY) / 2) / rayY;
            }
        }
    }

    float wallX;  // Position exacte où le rayon a frappé le mur
    if (hitVertical) {
        wallX = player.y + *distance * rayY;  // Intersection sur un mur vertical
    } else {
        wallX = player.x + *distance * rayX;  // Intersection sur un mur horizontal
    }

    wallX -= floor(wallX);  // Garder seulement la partie fractionnaire (position sur le mur)

    // Reversement horizontal de la texture si besoin
    if (hitVertical && rayX > 0) *flipTexture = true;
    if (not hitVertical && rayY < 0) *flipTexture = true;

    // Eviter l'effet fish eye
    *distance *= cos(player.angle - rayAngle);
    
    return wallX;
}

void render(SDL_Renderer* renderer, Player player, int walkOffset, SDL_Surface* wallSurface, SDL_Texture* wallTexture) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    float distance = 0.0f;  // Distance par rapport au mur
    bool flipTexture;
    int pasX = 1;  // Pas sur l'écran entre deux tracés de bande

    flipTexture = false;    // frue si besoin de renverser horizontalement la texture, false sinon
    
    for (int x = 0; x < WIDTH; x = x + pasX) {
        float rayAngle = player.angle - player.horizontalFOV / 2 + (x / (float)WIDTH) * player.horizontalFOV;   // Angle du rayon envoyé
        

        // ## Calcul du la coordonnée X de la texture ##
        float wallX = sendRay(player, rayAngle, &distance, &flipTexture);

        int texWidth = 16;   // Largeur de la texture
        int texHeight = 16;  // Hauteur de la texture

        // Calculer la coordonnée X dans la texture
        int texX = int(wallX * float(texWidth));
        


        // v3  ## CALCUL D'OU TRACER LES BANDES DES MURS ##
        float shakeIntensity = 5.0f;   // marche
        float wallHeight =  1.0f;    // Hauteur du mur dans le jeu
        float wallProportionScreen = wallHeight / distance;

        // Calcul de la hauteur du mur à afficher en pixels sur l'écran
        int wallHeightScreen = (int)floor(wallProportionScreen * HEIGHT);

        // Calcul du décalage vertical en fonction du pitch (plus on lève/baisse la tête, plus le mur est décalé)
        int verticalOffset = (int)(HEIGHT * tanf(player.pitch));

        // Calcul de la position de début et de fin du mur en prenant en compte le pitch et la hauteur du mur
        int drawStart = (HEIGHT / 2 - verticalOffset) - wallHeightScreen / 2 + walkOffset;
        int drawEnd = drawStart + wallHeightScreen;
        


        

        texX = texWidth - texX;
        if (texX < 0) texX = 0;
        if (texX >= texHeight) texX = texWidth - 1;
        // printf("X %f\n",(float)texX);

        // printf("%f\n",texX);
        if (flipTexture == true) texX = texWidth - texX - 1;  // Ajustement si le mur est vertical et la direction est opposée

        Uint32* pixels = (Uint32*)wallSurface->pixels;  // Uint32 si la surface est en 32 bits

        Uint8 r, g, b;
        Uint32 pixel;
        int formerTexY;
        int pasY = 1;
        // Dessiner la colonne de pixels correspondant à la texture
        for (int y = drawStart; y < drawEnd; y=y+pasY) {
            int d = y * 256 - HEIGHT * 128 + verticalOffset * 256 + wallHeightScreen * 128 - walkOffset * 256;  // Distance dans la texture
            int texY = ((d * texHeight) / wallHeightScreen) / 256;    // Calculer la coordonnée Y à utiliser dans la texture
            

            if (texY < 0) texY = 0;
            if (texY >= texHeight) texY = texHeight - 1;

            // Si besoin de changer de couleur
            if (texY != formerTexY) {
                pixel = pixels[texX + texWidth * texY];

                SDL_GetRGB(pixel, wallSurface->format, &r, &g, &b);
                float intensity = getIntensity(distance);
                r *= intensity;
                g *= intensity;
                b *= intensity;
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);      // Définir la couleur du pixel
            }
            

            // Dessiner le pixel à la position (x, y) sur l'écran
            SDL_RenderDrawPoint(renderer, x, y);  // Dessiner le pixel
            formerTexY = texY;

        }
    }

    SDL_RenderPresent(renderer);
}

// Fonction pour savoir si la position (x,y) est accessible
bool canMoveTo(float x, float y) {
    return (x >= 0 && x < 10 && y >= 0 && y < 10 && map[(int)y][(int)x] != 1);
}

// Fonction pour faire un rendu du text
void renderText(SDL_Renderer* renderer, TTF_Font* font, std::string text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


// Fonction pour faire le mouvement de camera lorsqu'on se déplace
float updateWalkOffset(bool isWalking, float walkCount) {
    float walkSpeed = 0.12f;
    float tolerance = 0.1f;
    if (isWalking) {
        walkCount += walkSpeed;
        if (walkCount > 2.0f * PI) {
            walkCount = fmod(walkCount, 2.0f * PI);
        }
    }
    else {
        // En fonction
        if (((walkSpeed < walkCount) && (walkCount <= PI / 2.0f)) || ((PI - walkSpeed <= walkCount) && (walkCount <= 3.0f * PI / 2.0f))) {
            walkCount -= walkSpeed;
        }
        else if (((PI / 2.0f <= walkCount) && (walkCount <= PI - walkSpeed)) || ((3.0f * PI / 2.0f <= walkCount) && (walkCount < 2.0f * PI - walkSpeed))) {
            walkCount += walkSpeed;
        }
        else {
            walkCount = 0.0f;
        }
    }
    return walkCount;
}

void loadSurfaces(SDL_Renderer* renderer, SDL_Surface** wallSurface) {
    // *wallSurface = SDL_LoadBMP("C:\\Users\\olivi\\kDrive\\cours\\UE_prog\\projet\\sprites\\brique.bmp");
    *wallSurface = SDL_LoadBMP("C:\\Users\\olivi\\kDrive\\cours\\UE_prog\\projet\\sprites\\hey.bmp");
    if (wallSurface==NULL) {
    printf("Erreur lors du chargement de l'image : %s\n", SDL_GetError());
    }
    *wallSurface = SDL_ConvertSurfaceFormat(*wallSurface, SDL_PIXELFORMAT_ARGB8888, 0);
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

    // printf("%f", player.vertical_fov);

    bool running = true;
    SDL_Event event;

    // NUL
    Uint32 frameStart;
    int frameTime;
    int frameCount = 0;
    float avgFPS = 0;

    bool isWalking;
    float walkCount = 0.0f;

    
    SDL_Surface *wallSurface;
    SDL_Texture *wallTexture;
    loadSurfaces(renderer, &wallSurface);

    // wallSurface = SDL_LoadBMP("C:\\Users\\olivi\\kDrive\\cours\\UE_prog\\projet\\sprites\\brique.bmp");
    // if (wallSurface==NULL) {
    // printf("Erreur lors du chargement de l'image : %s\n", SDL_GetError());
    // }
    // wallSurface = SDL_ConvertSurfaceFormat(wallSurface, SDL_PIXELFORMAT_ARGB8888, 0);

    // wallTexture = SDL_CreateTextureFromSurface(renderer, wallSurface);
    // if (wallTexture == NULL) {
    //     printf("Erreur de creation de la texture : %s\n", SDL_GetError());
    // }
    // SDL_FreeSurface(wallSurface);

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
        player.pitch += (mouseY - (HEIGHT / 2)) * player.verticalSensitivity;
        // player.pitch = clamp(player.pitch + (mouseY - (HEIGHT / 2)) * player.verticalSensitivity, - PI, PI);  // Ajuster le pitch
        SDL_WarpMouseInWindow(window, WIDTH / 2, HEIGHT / 2);


        // DEPLACEMENTS //
        

        const Uint8* state = SDL_GetKeyboardState(NULL);
        float pos_x = player.x;
        float pos_y = player.y;

        isWalking = false;
        if (state[SDL_SCANCODE_W]) {
            float new_x = pos_x + cos(player.angle) * player.speed * DELTA_TIME;
            float new_y = pos_y + sin(player.angle) * player.speed * DELTA_TIME;
            isWalking = true;

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
            isWalking = true;

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
            isWalking = true;

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
            isWalking = true;

            if (canMoveTo(new_x - sin(player.angle) * player.thickness, player.y + cos(player.angle) * player.thickness)) {
                pos_x = new_x;
            }
            if (canMoveTo(player.x - sin(player.angle) * player.thickness, new_y + cos(player.angle) * player.thickness)) {
                pos_y = new_y;
            }
        }

        player.x = pos_x;
        player.y = pos_y;

        walkCount = updateWalkOffset(isWalking, walkCount);
        float shakeIntensity = 10.0f;
        int walkOffset = (int)floor(sinf(walkCount) * shakeIntensity);
        render(renderer, player, walkOffset, wallSurface, wallTexture);

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
