#include <QtCore/QCoreApplication>
#include "hstcppollcomm.h"
#include "EmoHandler.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv); //QObject::startTimer: Timers can only be used with threads started with QThread

    HsTcpPollComm hsTcpPollComm;
	EmoHandler emoHandler;
	//emoHandler.setHsTcpPollComm(&hsTcpPollComm); //as duas threads Emo e Hs tem o mesmo valor da hsTcpPollComm
	
	hsTcpPollComm.emoHandler = &emoHandler;

	//Inicia tentativa de conexao com emotiv
	emoHandler.emoConnect(argc);

    // Inicia tentativa de conexao com o systembox   
    hsTcpPollComm.tcpConnect("10.1.7.37");   //("192.168.0.210");

	return app.exec();
}
