#include <QtCore/QCoreApplication>
#include "hstcppollcomm.h"
#include "EmoHandler.h"

//int main(int argc, char *argv[])
int main(char *argv[])
{
    //QCoreApplication app(argc, argv);   //cria contexto do Qt
	//QCoreApplication app(argv);
    HsTcpPollComm hsTcpPollComm;
	EmoHandler emoHandler;
	emoHandler.setHsTcpPollComm(&hsTcpPollComm); //as duas threads Emo e Hs tem o mesmo valor da hsTcpPollComm

	// Inicia tentativa de conexao com o Emotiv
	//emoHandler.emoConnect();

    // Inicia tentativa de conexao com o systembox   
    hsTcpPollComm.tcpConnect("10.1.7.37");   //("192.168.0.210");

	//Emotiv:
	//emoHandler.run(argc, argv);
	emoHandler.run(argv);

	//Pega estado e inicializaca os equipamentos
	//emoHandler.pegaEstadoEquipamentos();

    //return app.exec();
	return 0;
}
