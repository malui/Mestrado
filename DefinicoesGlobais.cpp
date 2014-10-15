#include "DefinicoesGlobais.h"

#ifdef KIT
//int SOM_ESTADOS[] = {0, 1}; // colocar musicas nao esta sendo utilizado: teste so com lamadas (kit)
int LAMPADA_ESTADOS[] = {0, 1};
//int AC_ESTADOS[] = {0, 10, 20, 30}; nao esta sendo utilizado: teste so com lamadas (kit)
//int TAMANHO_ESTADOS[] = {2, 2, 2, 2, 4};
int TAMANHO_ESTADOS[] = {2, 2, 2};//kit systembox
//int* ESTADOS[] = {&SOM_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &AC_ESTADOS[0]};
int* ESTADOS[] = {&LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0]};  //kit systembox
#endif


#ifdef HS
//int SOM_ESTADOS[] = {0, 1}; // colocar musicas
int LAMPADA_ESTADOS[] = {0, 100};
int CORTINA_ESTADOS[] = {0, 1}; //1- abre; 0-fecha
int TAMANHO_ESTADOS[] = {2, 2, 2, 2, 2, 2};
//int* ESTADOS[] = {&SOM_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &AC_ESTADOS[0]};
int* ESTADOS[] = {&LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &LAMPADA_ESTADOS[0], &CORTINA_ESTADOS[0], &CORTINA_ESTADOS[0], &CORTINA_ESTADOS[0]};
#endif