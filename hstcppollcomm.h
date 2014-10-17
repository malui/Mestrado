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

enum TControleFluxo{PRIMEIRA_EXECUCAO,SEGUNDA_EXECUCAO,ENESIMA_EXECUCAO };


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
	EmoHandler* emoHandler;
	void controle();
	void logAffectiveSuiteState(std::ostream& os, unsigned int userID, EmoStateHandle eState, bool withHeader);
	//GUILHERME BEGIN
	typedef std::vector<int> Cenario;                             // Vetor de zeros e uns que representam um estado
	typedef std::tuple< Cenario,float> TuplaCenario;             // Tupla que liga o estado ao valor de seu stress/parametro
	typedef std::vector<TuplaCenario> Populacao;                // Vetor que contem todos individuos de uma geração
	typedef std::tuple<Cenario,Cenario> crossoverCenarios;     // Resultado do cross over de dois vetores
			//GUILHERME BEGIN	

	void printCenario(const vector<int>& v);                            // Imprime um estado do tipo vector<int>
	void printPopulacao(const Populacao);                              // Imprime a população de uma geração
	crossoverCenarios crossover(Cenario cenario1,Cenario cenario2);   // Implementação do crossover devolve os dois novos estados
	float avaliacaoAptidaoCenario(Cenario cenarioAAvaliar);              // Função que se comunicará com o progrma principal, e esperará a avaliação de um estado

	Populacao geracaoAtual;
	Populacao geracaoPassada;
	bool crossFlag;
	//GUILHERME END
	//GUILHERME END

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
