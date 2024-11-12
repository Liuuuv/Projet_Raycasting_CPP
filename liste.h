#include <SDL2/SDL.h>

// liste dont les éléments pointent vers le suivant


struct cellule
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    struct cellule *suivant;
};

typedef struct cellule cellule_t;

struct liste {
    cellule_t *tete;
};

typedef struct liste liste_t;