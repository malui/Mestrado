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

// Definição de constantes usadas no algoritmo genético
#define LIMITE_GERACOES 4						  // número limite de gerações
#define TAMANHO_GERACAO_INICIAL 3				// tamanho da geração inicial
#define ELEMENTOS_REPLICADOS_PROXIMA_GERACAO 3 // numeros de elementos replidados de uma geração para outra



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
	int contadorPrimeiraGeracao;		// armazena o numero de cenario na primeira geração, é usado para controlar a avaliação dos cenarios
	int contadorCrossoversNaoAvaliados; // armazena o numero de cenarios que sairam do crossover e ainda não foram avaliados
	int contadorNumeroDeGeracoes;		// Controla o numero de gerações
	std::vector<int> codCenariosExistentes;
	// funcoes:
	//void controle();

	// funcoes algoritmo genetico:
	AlgGen::Estados mutacao(Estados estados);
	AlgGen::Estados toBinary(int number, Estados cenario);
	AlgGen::crossoverEstados crossoverDeUmPonto(Estados estado1,Estados estado2);   // Implementação do crossover devolve os dois novos estados que são a recombinação de seus pais
	AlgGen::Estados crossoverMascaraAleatoria(Estados estados1,Estados estados2);	   // Implementação do crossover devolve os dois ovos estados
	void inicializaCenariosPrimeiraGeracao(int tamanhoCenario);  // Função responsavel por definir qual a população inicial de cenarios, utiliza criaCenarioAleatorio
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