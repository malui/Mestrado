#ifndef HSTCPPOLLCOMM_H
#define HSTCPPOLLCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
//#include "EmoHandler.h"
#include "AlgGen.h"

//#include "DefinicoesGlobais.h"

enum TSessionState{DISCONNECTED, SENT_AUTH, CHECK_AUTH, WAITING_CONN, SENT_REQUEST, WAITING_RESPONSE, DISCONNECTING};//, SESSAO_ATIVA};
enum TControleFluxo{SET_CENARIOS_INICIAIS, AVALIA_CENARIOS_INICIAIS,SET_CENARIOS,AVALIA_CENARIOS};

/*
// Definição de constantes usadas no algoritmo genético
#define LIMITE_GERACOES 4						  // número limite de gerações
//#define ENGAGEMENT_ALVO 0.95					 // número limite de engagement
#define TAMANHO_GERACAO_INICIAL 3				// tamanho da geração inicial
#define ELEMENTOS_REPLICADOS_PROXIMA_GERACAO 3 // numeros de elementos replidados de uma geração para outra
*/
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
	int cont_sinal;
	TControleFluxo controleFluxo;
	int ack;
	AlgGen* algGen;
	void controle();
	
// end public
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
	TSessionState sessionState;
	QString CryptPass(char * szPassword);
    QTcpSocket tcpSocket;
    QTimer pollTimer;

};

#endif // HSTCPPOLLCOMM_H
