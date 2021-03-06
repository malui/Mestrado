#ifndef HSTCPPOLLCOMM_H
#define HSTCPPOLLCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include "DefinicoesGlobais.h"
enum TSessionState{DISCONNECTED, SENT_AUTH, CHECK_AUTH, WAITING_CONN, SENT_REQUEST, WAITING_RESPONSE, DISCONNECTING};


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
