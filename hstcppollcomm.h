#ifndef HSTCPPOLLCOMM_H
#define HSTCPPOLLCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

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
