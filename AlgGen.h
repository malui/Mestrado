#ifndef ALGGEN_H
#define ALGGEN_H

#include <QDebug>
#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

#include "DefinicoesGlobais.h"
#include "EmoHandler.h"
//#include "HsTcpPollComm.h"

// Controle de fluxo
//enum TSessionState{DISCONNECTED, SENT_AUTH, CHECK_AUTH, WAITING_CONN, SENT_REQUEST, WAITING_RESPONSE, DISCONNECTING};//, SESSAO_ATIVA};

// Defini��o de constantes usadas no algoritmo gen�tico
#define LIMITE_GERACOES 4						  // n�mero limite de gera��es
#define TAMANHO_GERACAO_INICIAL 3				// tamanho da gera��o inicial
#define ELEMENTOS_REPLICADOS_PROXIMA_GERACAO 3 // numeros de elementos replidados de uma gera��o para outra



class AlgGen //: public QObject
{
   // Q_OBJECT
public:
	AlgGen();
	~AlgGen();
	//explicit Controle(QObject *parent = 0);
	//TSessionState sessionState;
	//TControleFluxo controleFluxo;
	//int cont_sinal;
	float treshold;

	// inclui outras classes:
	EmoHandler* emoHandler;
//	HsTcpPollComm* hstcppollcomm;

	// tipos de variaveis:
	struct CENARIO {
    std::vector<int> estados;   
    float            engagement;
    } cenario;  
	typedef std::vector<int> Estados;
	typedef std::vector<CENARIO> VectorCenarios;
	typedef std::tuple<Estados,Estados> crossoverEstados;     // Resultado do cross over de dois vetores
	//variaveis:
	VectorCenarios geracaoAtual;
	VectorCenarios geracaoPassada;
	int contadorPrimeiraGeracao;		// armazena o numero de cenario na primeira gera��o, � usado para controlar a avalia��o dos cenarios
	int contadorCrossoversNaoAvaliados; // armazena o numero de cenarios que sairam do crossover e ainda n�o foram avaliados
	int contadorNumeroDeGeracoes;		// Controla o numero de gera��es
	std::vector<int> codCenariosExistentes;
	// funcoes:
	//void controle();

	// funcoes algoritmo genetico:
	AlgGen::Estados mutacao(Estados estados);
	AlgGen::Estados toBinary(int number, Estados cenario);
	AlgGen::crossoverEstados crossoverDeUmPonto(Estados estado1,Estados estado2);   // Implementa��o do crossover devolve os dois novos estados que s�o a recombina��o de seus pais
	AlgGen::Estados crossoverMascaraAleatoria(Estados estados1,Estados estados2);	   // Implementa��o do crossover devolve os dois ovos estados
	void inicializaCenariosPrimeiraGeracao(int tamanhoCenario);  // Fun��o responsavel por definir qual a popula��o inicial de cenarios, utiliza criaCenarioAleatorio
	std::vector<int> criaEstadosAleatorio(int tamanhoEstados);

	void criaNovaGeracao(int qtdElementosReplicados);
	void verificaCondicoesDeParada();				
	void condicaoParadaEngagement(float engagement); 
	bool isCenarioRepetido(Estados estado);
	int codificaCenario(Estados estado);
	void insereCenarioCodificador(Estados estado);
	AlgGen::Estados decodificaCenario(int codificacao, int tamanhoCenario);
	AlgGen::Estados getCenarioNaoUsado(int tamanhoEstadosCenario);
	

	void printCenario(const vector<int> &v);     
	void printPopulacao(const VectorCenarios populacao);                           
	void printVector (const vector<float> &v);
	
	// funcoes DSP:
	float mediaHarmonicaPonderada(vector<float> v);
};


#endif  // CONTROLE_H