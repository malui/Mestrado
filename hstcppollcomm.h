#ifndef HSTCPPOLLCOMM_H
#define HSTCPPOLLCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
//GUILHERME BEGIN
#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;
//GUILHERME END


#include "EmoHandler.h"

#include "DefinicoesGlobais.h"
enum TSessionState{DISCONNECTED, SENT_AUTH, CHECK_AUTH, WAITING_CONN, SENT_REQUEST, WAITING_RESPONSE, DISCONNECTING};//, SESSAO_ATIVA};

enum TControleFluxo{SET_CENARIOS_INICIAIS, AVALIA_CENARIOS_INICIAIS,SET_CENARIOS,AVALIA_CENARIOS,};


class HsTcpPollComm : public QObject
{
    Q_OBJECT
public:
    explicit HsTcpPollComm(QObject *parent = 0);
    bool tcpConnect(QString host = "localhost", int port = 2000);
    void setUnit(int unit, int value);
	int setCenario(std::vector<int> units, std::vector<int> values);
	QString makeGetUnit( const int *buffUnit, int numUnit );
	QString resposta;
	bool resposta_pronta;
	TSessionState sessionState;
	TControleFluxo controleFluxo;
	int ack;
	EmoHandler* emoHandler;
	void controle();
	void write_vec(const vector<int>& vec);
	void logAffectiveSuiteState(std::ostream& os, unsigned int userID, EmoStateHandle eState, bool withHeader);
	
	typedef std::vector<int> Cenario;                             // Vetor de zeros e uns que representam um estado
	typedef std::tuple< Cenario,float> TuplaCenario;             // Tupla que liga o estado ao valor de seu stress/parametro
	typedef std::vector<TuplaCenario> Populacao;                // Vetor que contem todos individuos de uma geração
	typedef std::tuple<Cenario,Cenario> crossoverCenarios;     // Resultado do cross over de dois vetores
			

	void printCenario(const vector<int>& v);                            // Imprime um estado do tipo vector<int>
	void printPopulacao(const Populacao);                              // Imprime a população de uma geração
	
	crossoverCenarios crossoverDeUmPonto(Cenario cenario1,Cenario cenario2);   // Implementação do crossover devolve os dois novos estados que são a recombinação de seus pais
	Cenario crossoverMascaraAleatoria(Cenario cenario1,Cenario cenario2);	   // Implementação do crossover devolve os dois ovos estados
	Cenario HsTcpPollComm::mutacao(Cenario cenario);
	
	void inicializaCenariosPrimeiraGeracao(int tamanhoCenario);  // Função responsavel por definir qual a população inicial de cenarios, utiliza criaCenarioAleatorio
	Cenario criaCenarioAleatorio(int tamanhoCenario);			
	
	// cria nova geração onde qtdElementosReplicados é o número de elementos na geração inicial que serão replicados para a próxima geração
	// Cria elementos 3 elementos com Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
	// Insere elementos do crossover na geração atual
	// o qtdElementosReplicados representa o número de elementos que será replicado da geração atual, para a proxima geração
	void criaNovaGeracao(int qtdElementosReplicados);	
	Populacao geracaoAtual;
	Populacao geracaoPassada;

	int contadorPrimeiraGeracao;		// armazena o numero de cenario na primeira geração, é usado para controlar a avaliação dos cenarios
	int contadorCrossoversNaoAvaliados; // armazena o numero de cenarios que sairam do crossover e ainda não foram avaliados
	int contadorNumeroDeGeracoes;		// Controla o numero de gerações
	
signals:
    void tcpClosed();

public slots:
    void pollProcess(void);
    void tcpOnConnect(void);
    void tcpOnDisconnect(void);
    void tcpOnRead(void);
    void tcpOnError(QAbstractSocket::SocketError error);
//	void sendRequest(void);

private:


	//TSessionState sessionState;
	QString CryptPass(char * szPassword);
    QTcpSocket tcpSocket;
    QTimer pollTimer;


};

#endif // HSTCPPOLLCOMM_H
