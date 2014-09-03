#include "DefinicoesGlobais.h"

int SOM_ESTADOS[] = {0, 1}; // colocar musicas
int LAMPADA_ESTADOS[] = {0, 1};
int AC_ESTADOS[] = {0, 10, 20, 30};
//int TAMANHO_ESTADOS[] = {2, 2, 2, 2, 4};
int TAMANHO_ESTADOS[] = {2, 2, 2};//kit systembox
//int* ESTADOS[] = {&SOM_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &AC_ESTADOS[0]};
int* ESTADOS[] = {&LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0]};  //kit systembox