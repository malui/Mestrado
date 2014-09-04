#pragma once

#ifndef DEFINICOESGLOBAIS_H_INCLUDED
#define DEFINICOESGLOBAIS_H_INCLUDED

//enum EQUIPAMENTOS_ID {SOM, LAMPADA1, LAMPADA2, LAMPADA3, AC};
enum EQUIPAMENTOS_ID {LAMPADA1, LAMPADA2, LAMPADA3};  //kit systembox
const int EQUIPAMENTOS_TAMANHO = 3;
const int EQUIPAMENTO_UNITS[] = {118, 119, 120}; //kit systembox

//lista estados possiveis dos equipamentos: (kit HS so usa lampadas)
//extern int SOM_ESTADOS[]; // colocar musicas
extern int LAMPADA_ESTADOS[];
//extern int AC_ESTADOS[];
extern int TAMANHO_ESTADOS[];
extern int* ESTADOS[];

#endif

