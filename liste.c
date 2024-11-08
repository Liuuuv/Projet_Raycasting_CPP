#include "liste.h"

cellule_t* nouvelleCellule()
{
    cellule_t* cel = (cellule_t*) malloc(sizeof(cellule_t));
    cel->suivant = NULL;
    return cel;
}

void detruireCellule(cellule_t* cel)
{
    free(cel);
}