#include "EmoHandler.h"
#include <QDebug>
#include <sstream>
#include <windows.h>
#include <map>

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

#pragma comment(lib, "./lib/edk.lib")

EE_DataChannel_t targetChannelList[] = {
		ED_COUNTER,
		ED_AF3, ED_F7, ED_F3, ED_FC5, ED_T7, 
		ED_P7, ED_O1, ED_O2, ED_P8, ED_T8, 
		ED_FC6, ED_F4, ED_F8, ED_AF4, ED_GYROX, ED_GYROY, ED_TIMESTAMP, 
		ED_FUNC_ID, ED_FUNC_VALUE, ED_MARKER, ED_SYNC_SIGNAL
	};

const char header[] = "COUNTER,AF3,F7,F3, FC5, T7, P7, O1, O2,P8" 
                      ", T8, FC6, F4,F8, AF4,GYROX, GYROY, TIMESTAMP, "   
                      "FUNC_ID, FUNC_VALUE, MARKER, SYNC_SIGNAL,";

EmoHandler::EmoHandler(QObject *parent) :
    QObject(parent)
{
	connect(&pollTimer, SIGNAL(timeout()), this, SLOT(run()));
	pollTimer.start(2000);

	eEvent			= EE_EmoEngineEventCreate();
	eState			= EE_EmoStateCreate();
	userID			= 0;
	composerPort	= 1726;
	option          = 0;
	state           = 0;
	hsTcpPollComm   = NULL;
	estadoHandler   = 0;
	flag            = true;

}

void EmoHandler::run() {
	
	EmoEngineEventHandle eEvent			= EE_EmoEngineEventCreate();
	EmoStateHandle eState				= EE_EmoStateCreate();
	unsigned int userID					= 0;
	const unsigned short composerPort	= 1726;
	float secs							= 1;
	unsigned int datarate				= 0;
	bool readytocollect					= false;
	int option							= 0;
	int state							= 0;


	std::string input;
	
	try {

		std::cout << "===================================================================" << std::endl;
		std::cout << "Example to show how to log EEG Data from EmoEngine/EmoComposer."	   << std::endl;
		std::cout << "===================================================================" << std::endl;
		std::cout << "Press '1' to start and connect to the EmoEngine                    " << std::endl;
		std::cout << "Press '2' to connect to the EmoComposer                            " << std::endl;
		std::cout << ">> ";
		
		std::getline(std::cin, input, '\n');
		option = atoi(input.c_str());
		
		// Conexao com Emotiv
		switch (option) {
			case 1:
			{
				if (EE_EngineConnect() != EDK_OK) {
					throw std::exception("Emotiv Engine start up failed.");
				}
				break;
			}
			case 2:
			{
				std::cout << "Target IP of EmoComposer? [127.0.0.1] ";
				std::getline(std::cin, input, '\n');

				if (input.empty()) {
					input = std::string("127.0.0.1");
				}

				if (EE_EngineRemoteConnect(input.c_str(), composerPort) != EDK_OK) {
					std::string errMsg = "Cannot connect to EmoComposer on [" + input + "]";
					throw std::exception(errMsg.c_str());
				}
				break;
			}
			default:
				throw std::exception("Invalid option...");
				break;
		}//switch
		
		
		std::cout << "Start receiving EEG Data! Press any key to stop logging...\n" << std::endl;

		while (!_kbhit()) {
			
			state = EE_EngineGetNextEvent(eEvent);

			if (state == EDK_OK) {

				EE_Event_t eventType = EE_EmoEngineEventGetType(eEvent);
				EE_EmoEngineEventGetUserId(eEvent, &userID);

				// Log the EmoState if it has been updated
				if (eventType == EE_UserAdded) {
					std::cout << "User added";
					EE_DataAcquisitionEnable(userID,true);
					readytocollect = true;
				}//if user added
			}//if edk ok
			

		// LIGAR EQUIPOS HS
			//logEmoState(userID, EmoStateHandle eState);
		/////////////////////
			
			if (readytocollect) {
				qDebug() << "readytocollect"; //codigo em looping aqui
	 if ( hsTcpPollComm->resposta_pronta )
		{
			std::cout<<"Engagement level enviado: "<<ES_AffectivGetEngagementBoredomScore(eState)<<std::endl;
			
			if (flag)
				{
				 flag = false;
				 
				for (int i = 0 ; i < EQUIPAMENTOS_TAMANHO ; ++i) //lista de equipamentos e lista de estados
				{
					int* estado = ESTADOS[i];
					int posicao_maior_engagement = 0;
					const int tamanho_engagementLevel = TAMANHO_ESTADOS[i];
					float* engagementLevels = new float [tamanho_engagementLevel]; //cria vetor dinamicamente
					//float* engagementLevels = new float [2];
					engagementLevels[0] = -1;
					engagementLevels[1] = -1;
					
					std::cout<<"Testando equipamento: "<<i<<std::endl;

					for (int j = 0 ; j < TAMANHO_ESTADOS[i] ; ++j) //lista de estados de cada equipamento ate o final, qual  ofinal?
					{
						//envia estado j para o equipamento i
						hsTcpPollComm->setUnit(EQUIPAMENTO_UNITS[i], estado[j]);
						Sleep(4000); // 1seg = 1.000
						
						std::cout<<"Testando estado: "<<j<<std::endl;

						//otimizar para nao guardar tal buffer, so precisa dos dois ultimos valores:
						//pega os estados de engagement para todos os estados do equipamento i e poe num buffer
				
						
						engagementLevels[j] = ES_AffectivGetEngagementBoredomScore(eState);
												
						std::cout<<"Engagement level com equipamento "<<i<<" e estado "<<j<<": "<<engagementLevels[j]<<std::endl;
						
						//verifica qual a posicao do maior estado de engagement do buffer
						if (j > 0)
						{
							if (engagementLevels[j] > engagementLevels[j-1])
							{
								posicao_maior_engagement = j;
							} //if maior engagement
						}// if j>0
					
					}// for tamanho dos estados
					std::cout<<"Maior Engagement Level para o Equipamento "<<i<<" foi com o estado "<<posicao_maior_engagement<<": "<<engagementLevels[posicao_maior_engagement]<<std::endl;
					delete[] engagementLevels;
					std::cout<<"Setando Equipamento "<<i<<" com o estado "<<posicao_maior_engagement<<std::endl;
					//seta o estado do equipamento i conforme o maior estado de engagement equivalente
					hsTcpPollComm->setUnit(EQUIPAMENTO_UNITS[i], estado[posicao_maior_engagement]);
				
				} //for equipamentos tamanho
		
			}//if flag
			
	 } // if resposta pronta

			}  //end if (readytocollect)
			
		} //end while
		
		Sleep(100);
		
	}//try
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		std::cout << "Press any key to exit..." << std::endl;
		getchar();
	}

	EE_EngineDisconnect();
	EE_EmoStateFree(eState);
	EE_EmoEngineEventFree(eEvent);
}


 void EmoHandler::logEmoState(unsigned int userID, EmoStateHandle eState) 
 {
	 qDebug() << "Rodando logEmoState";
	 if ( hsTcpPollComm->resposta_pronta )
		{
			std::cout<<"Engagement level enviado: "<<ES_AffectivGetEngagementBoredomScore(eState)<<std::endl;
			if (flag)
				{
				 flag = false;

				for (int i = 0 ; i < EQUIPAMENTOS_TAMANHO ; ++i) //lista de equipamentos e lista de estados
				{
					int* estado = ESTADOS[i];
					int posicao_maior_engagement = 0;
					const int tamanho_engagementLevel = TAMANHO_ESTADOS[i];
					//float* engagementLevels = new float [tamanho_engagementLevel]; //cria vetor dinamicamente
					float* engagementLevels = new float [2];
					engagementLevels[0] = -1;
					engagementLevels[1] = -1;

					std::cout<<"Testando equipamento: "<<i<<std::endl;

					for (int j = 0 ; j < TAMANHO_ESTADOS[i] ; ++j) //lista de estados de cada equipamento ate o final, qual  ofinal?
					{
						//envia estado j para o equipamento i
						hsTcpPollComm->setUnit(EQUIPAMENTO_UNITS[i], estado[j]);
						Sleep(4000); // 1seg = 1.000

						std::cout<<"Testando estado: "<<j<<std::endl;

						//otimizar para nao guardar tal buffer, so precisa dos dois ultimos valores:
						//pega os estados de engagement para todos os estados do equipamento i e poe num buffer
				

						engagementLevels[j] = ES_AffectivGetEngagementBoredomScore(eState);

						std::cout<<"Engagement level com equipamento "<<i<<" e estado "<<j<<": "<<engagementLevels[j]<<std::endl;

						//verifica qual a posicao do maior estado de engagement do buffer
						if (j > 0)
						{
							if (engagementLevels[j] > engagementLevels[j-1])
							{
								posicao_maior_engagement = j;
							}
						}
					
					}
					std::cout<<"Maior Engagement Level para o Equipamento "<<i<<" foi com o estado "<<posicao_maior_engagement<<": "<<engagementLevels[posicao_maior_engagement]<<std::endl;
					delete[] engagementLevels;
					std::cout<<"Setando Equipamento "<<i<<" com o estado "<<posicao_maior_engagement<<std::endl;
					//seta o estado do equipamento i conforme o maior estado de engagement equivalente
					hsTcpPollComm->setUnit(EQUIPAMENTO_UNITS[i], estado[posicao_maior_engagement]);
				}
		
			}
			/*if ( ES_AffectivGetEngagementBoredomScore(eState) > 0.8 )
			{
				hsTcpPollComm->setUnit(EQUIPAMENTO_UNITS[0], 1);
			}
			else 
			{
				hsTcpPollComm->setUnit(EQUIPAMENTO_UNITS[0], 0);
			}*/
	 }
 }

 void EmoHandler::pegaEstadoEquipamentos()
 {
	 if (hsTcpPollComm != NULL)
	 {


		 if (hsTcpPollComm->resposta_pronta)
		 {

			 //inicializa getunit

			// QString getUnitsString = hsTcpPollComm->makeGetUnit( EQUIPAMENTO_UNITS, EQUIPAMENTOS_TAMANHO );
			// std::cout<<getUnitsString.toStdString()<<std::endl;
			 std::cout<<"Pegando Resposta do valor das units"<<std::endl;
			 std::cout<<hsTcpPollComm->resposta.toStdString()<<std::endl;
		 }


		 //exibeEstadoEquipamentos();
		// system("pause"); //comando windows: pause - pressione uma tecla pra continuar
	 }
 }
/*
  void EmoHandler::exibeEstadoEquipamentos() const //essa funcao nao altera nenhum valor das variaveis da classe EmoHandler
 {
	 //exibe estados dos equipmaentos
	 std::cout<<"Exibindo estados dos equipamentos"<<std::endl;

	 std::cout<<"SOM: "<<std::endl;
	 std::cout<<"\t unit: "<<equipamentos.som.unit<<std::endl;
	 std::cout<<"\t ligado: "<<equipamentos.som.ligado<<std::endl;
	 std::cout<<"\t volume: "<<equipamentos.som.volume<<std::endl;
	 std::cout<<"\t musica_id: "<<equipamentos.som.musica_id<<std::endl;

	 std::cout<<"Lampada1: "<<std::endl;
	 std::cout<<"\t unit: "<<equipamentos.lampada1.unit<<std::endl;
	 std::cout<<"\t ligado: "<<equipamentos.lampada1.ligado<<std::endl;

	 std::cout<<"Lampada2: "<<std::endl;
	 std::cout<<"\t unit: "<<equipamentos.lampada2.unit<<std::endl;
	 std::cout<<"\t ligado: "<<equipamentos.lampada2.ligado<<std::endl;

	 std::cout<<"Lampada3: "<<std::endl;
	 std::cout<<"\t unit: "<<equipamentos.lampada3.unit<<std::endl;
	 std::cout<<"\t ligado: "<<equipamentos.lampada3.ligado<<std::endl;

	 std::cout<<"ArCondicionado: "<<std::endl;
	 std::cout<<"\t unit: "<<equipamentos.ac.unit<<std::endl;
	 std::cout<<"\t ligado: "<<equipamentos.ac.ligado<<std::endl;
	 std::cout<<"\t temperatura: "<<equipamentos.ac.temperatura<<std::endl;

	 std::cout<<std::endl;
 
 }*/