#include <QtCore/QCoreApplication>
#include "hstcppollcomm.h"
#include "EmoHandler.h"

//int main(char *argv[])
int main()
{
    HsTcpPollComm hsTcpPollComm;
	EmoHandler emoHandler;
	emoHandler.setHsTcpPollComm(&hsTcpPollComm); //as duas threads Emo e Hs tem o mesmo valor da hsTcpPollComm
	
    // Inicia tentativa de conexao com o systembox   
    hsTcpPollComm.tcpConnect("10.1.7.37");   //("192.168.0.210");

	//Emotiv: 
	//  -conexao com emotiv
	//  -inteligencia
	emoHandler.run();

	return 0;
}
