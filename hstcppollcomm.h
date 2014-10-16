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
//GUILHERME ENG


#include "EmoHandler.h"

#include "DefinicoesGlobais.h"
enum TSessionState{DISCONNECTED, SENT_AUTH, CHECK_AUTH, WAITING_CONN, SENT_REQUEST, WAITING_RESPONSE, DISCONNECTING};//, SESSAO_ATIVA};


class HsTcpPollComm : public QObject
{
    Q_OBJECT
public:
    explicit HsTcpPollComm(QObject *parent = 0);
    bool tcpConnect(QString host = "localhost", int port = 2000);
    void setUnit(int unit, int value);
	QString makeGetUnit( const int *buffUnit, int numUnit );
	QString resposta;
	bool resposta_pronta;
	TSessionState sessionState;
	EmoHandler* emoHandler;
	void controle();
	void logAffectiveSuiteState(std::ostream& os, unsigned int userID, EmoStateHandle eState, bool withHeader);
	//GUILHERME BEGIN
	typedef std::vector<int> Senario;                             // Vetor de zeros e uns que representam um estado
	typedef std::tuple< Senario,float> TuplaSenario;             // Tupla que liga o estado ao valor de seu stress/parametro
	typedef std::vector<TuplaSenario> Populacao;                // Vetor que contem todos individuos de uma geração
	typedef std::tuple<Senario,Senario> crossoverSenarios;     // Resultado do cross over de dois vetores
			//GUILHERME BEGIN	

	void printSenario(const vector<int>& v);                            // Imprime um estado do tipo vector<int>
	void printPopulacao(const Populacao);                              // Imprime a população de uma geração
	crossoverSenarios crossover(Senario senario1,Senario senario2);   // Implementação do crossover devolve os dois novos estados
	float avaliacaoAptidaoSenario(Senario senarioAAvaliar);              // Função que se comunicará com o progrma principal, e esperará a avaliação de um estado

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
