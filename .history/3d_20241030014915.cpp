#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH/(float)HEIGHT;
const float PI = 3.14159265f;
const int TARGET_FPS = 144;
const int FRAME_DELAY = 1000 / TARGET_FPS;
const float DELTA_TIME = 1.0f/TARGET_FPS;

int map[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
    float x;
    float y;
    float angle;
    float pitch = 0;  // Nouvelle variable pour le "head pitch"
    float horizontalFOV = 70 * PI / 180;
    float verticalFOV = 2.0f*atanf(tan(horizontalFOV/2.0f)*ASPECT_RATIO);
    // float verticalFOV = 120 * PI / 180;
    
    float speed = 2.0f;
    float thickness = 0.1f;
    float sensitivity = 0.0008f;
    float verticalSensitivity = 0.0008f;  // Sensibilité pour le mouvement vertical
};

float clamp(float value, float min, float max) {  
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

SDL_Color getColor(float distance) {
    float maxDistance = 5.0f;
    float intensity = fmin(distance / maxDistance, 1.0f);
    Uint8 colorValue = static_cast<Uint8>((1 - intensity) * 230);
    return {colorValue, colorValue, colorValue, 255};
}

void render(SDL_Renderer* renderer, Player player, int walkOffset, SDL_Surface* wallSurface, SDL_Texture* wallTexture) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);


    
    for (int x = 0; x < WIDTH; x++) {
        float rayAngle = player.angle - player.horizontalFOV / 2 + (x / (float)WIDTH) * player.horizontalFOV;
        float rayX = cos(rayAngle);
        float rayY = sin(rayAngle);

        // Calcul des pas unitaires pour avancer dans les axes X et Y
        float xUnit = sqrtf(1 + powf(rayY / rayX, 2)); // Combien d'unités en X on parcourt
        float yUnit = sqrtf(1 + powf(rayX / rayY, 2)); // Combien d'unités en Y on parcourt

        // Détermination des directions (avancer ou reculer)
        int stepX = (rayX < 0) ? -1 : 1;
        int stepY = (rayY < 0) ? -1 : 1;

        // Position initiale dans la grille
        int testX = (int)player.x;
        int testY = (int)player.y;

        // Calcul du premier point d'intersection avec la grille
        float sideDistX = (rayX < 0) ? (player.x - testX) * xUnit : (testX + 1.0f - player.x) * xUnit;
        float sideDistY = (rayY < 0) ? (player.y - testY) * yUnit : (testY + 1.0f - player.y) * yUnit;

        // Nouvelle variable pour savoir si on touche un côté vertical ou horizontal
        bool hit = false;
        bool hitVertical = false; // True si c'est un mur vertical, false si horizontal
        float distance = 0;

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

            // Vérifier si on a touché un mur
            if (testX < 0 || testX >= 10 || testY < 0 || testY >= 10 || map[testY][testX] == 1) {  // rester dans la grille + verifier mur touché
                hit = true;
                
                // La distance finale est la distance sur l'axe qui a été touché en premier (X ou Y)
                if (hitVertical) {
                    distance = (testX - player.x + (1 - stepX) / 2) / rayX;
                } else {
                    distance = (testY - player.y + (1 - stepY) / 2) / rayY;
                }
            }
        }

        float wallX;  // Position exacte sur le mur où le rayon a frappé
        if (hitVertical) {
            wallX = player.x + distance * sideDistX;  // Intersection sur un mur vertical
        } else {
            wallX = player.y + distance * sideDistY;  // Intersection sur un mur horizontal
        }
        // printf("X %f\n",wallX);

        distance *= cos(player.angle - rayAngle);

        // v1 //
        // int wallHeight = (int)(HEIGHT / (distance + 0.0001));
        // int drawStart = (HEIGHT - wallHeight) / 2 + player.pitch*0;
        // int drawEnd = drawStart + wallHeight;

        // v2 //
        // float wallHeight =  1.0f;    // Hauteur du mur dans le jeu
        // float wallProportionScreen = wallHeight/ (distance * tanf(player.verticalFOV/2.0f));
        
        // int wallHeightScreen = (int)floor(wallProportionScreen * HEIGHT);
        // int drawStart = HEIGHT/2 - (int)floor(wallProportionScreen * HEIGHT) + player.pitch*100;
        // int drawEnd = drawStart + wallHeightScreen;


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
        
        


        



        wallX -= floor(wallX);  // Garder seulement la partie fractionnaire (position sur le mur)
        printf("X %f\n",wallX);
        

        int pitch = 0;

        int texWidth = 16;   // Largeur de la texture
        int texHeight = 16;  // Hauteur de la texture

        // Calculer la coordonnée X dans la texture
        int texX = int(wallX * float(texWidth));
        // printf("X %f\n",(float)texX);

        // printf("%f\n",texX);
        if (hitVertical && rayX > 0) texX = texWidth - texX - 1;  // Ajustement si le mur est vertical et la direction est opposée
        if (!hitVertical && rayY < 0) texX = texWidth - texX - 1; // Ajustement pour les murs horizontaux

        Uint32* pixels = (Uint32*)wallSurface->pixels;  // Uint32 si la surface est en 32 bits

        Uint8 r, g, b;
        // Dessiner la colonne de pixels correspondant à la texture
        for (int y = drawStart; y < drawEnd; y++) {
            int d = y * 256 - HEIGHT * 128 + verticalOffset * 256 + wallHeightScreen * 128;  // Distance dans la texture
            int texY = ((d * texHeight) / wallHeightScreen) / 256;    // Calculer le pixel Y à utiliser dans la texture
            
            // printf("Y %f\n",(float)texY);

            if (texY < 0) texY = 0;
            if (texY >= texHeight) texY = texHeight - 1;
 
            Uint32 pixel = pixels[texY + texWidth * testX];

            
            SDL_GetRGB(pixel, wallSurface->format, &r, &g, &b);
            // printf("f\n",r);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);      // Définir la couleur du pixel

            // Dessiner le pixel à la position (x, y) sur l'écran
            SDL_RenderDrawPoint(renderer, x, y);  // Dessiner le pixel


            // // Déclaration du pointeur pour accéder aux pixels
            // void* pixels = NULL;
            
            // // Verrouiller la texture pour accès direct aux pixels
            // if (SDL_LockTexture(wallTexture, NULL, &pixels, &pitch) == 0) {
            //     // Vérifiez que texY est dans les limites de la texture
            //     if (texY < 0) texY = 0;
            //     if (texY >= texHeight) texY = texHeight - 1;
                
            //     printf("a");
                

            //     // Calculer la position du pixel dans la texture
            //     Uint32* pixelData = (Uint32*)pixels;  // Cast du pointeur de pixels en Uint32
            //     Uint32 color = pixelData[texY * (pitch / 4) + texX];  // Diviser pitch par 4 car on accède à des Uint32

            //     // Déverrouiller la texture après modification/lecture
            //     SDL_UnlockTexture(wallTexture);

            //     // Exemple de vérification d'erreur après le verrouillage de la texture
            //     if (SDL_LockTexture(wallTexture, NULL, &pixels, &pitch) < 0) {
            //         printf("Échec du verrouillage de la texture : %s\n", SDL_GetError());
            //         continue; // Ignore cette itération et passe à la suivante
            //     }

            //     // Extraire les valeurs RGB du pixel
            //     Uint8 r, g, b;
            //     SDL_GetRGB(color, wallSurface->format, &r, &g, &b);  // Extraire les valeurs RGB
            //     // printf("f\n",r);
            //     SDL_SetRenderDrawColor(renderer, r, g, b, 255);      // Définir la couleur du pixel

            //     // Dessiner le pixel à la position (x, y) sur l'écran
            //     SDL_RenderDrawPoint(renderer, x, y);  // Dessiner le pixel
            // } else {
            //         printf("Échec du verrouillage de la texture : %s\n", SDL_GetError());
            // }
        }

        // printf("SDL Error: %s\n", SDL_GetError());



        // SDL_Color color = getColor(distance);
        // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // drawLine(renderer, x, drawStart, x, drawEnd);
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

void loadTextures(SDL_Renderer* renderer, SDL_Surface* wallSurface, SDL_Texture* wallTexture) {
    wallSurface = SDL_LoadBMP("C:\\Users\\olivi\\kDrive\\cours\\UE_prog\\projet\\sprites\\brique.bmp");
    wallTexture = SDL_CreateTextureFromSurface(renderer, wallSurface);
    if (wallTexture == NULL) {
        printf("Erreur de creation de la texture : %s\n", SDL_GetError());
    }
    SDL_FreeSurface(wallSurface);
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

    
    SDL_Surface* wallSurface;
    SDL_Texture* wallTexture;
    // loadTextures(renderer, wallSurface, wallTexture);

    wallSurface = SDL_LoadBMP("C:\\Users\\olivi\\kDrive\\cours\\UE_prog\\projet\\sprites\\brique.bmp");
    if (wallSurface==NULL) {
    printf("Erreur lors du chargement de l'image : %s\n", SDL_GetError());
    }
    wallSurface = SDL_ConvertSurfaceFormat(wallSurface, SDL_PIXELFORMAT_ARGB8888, 0);
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
