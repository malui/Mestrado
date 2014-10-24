#include "EmoHandler.h"
#include <QDebug>
#include <sstream>
#include <windows.h>
#include <map>
#include <tchar.h> //tratar acentos em std::cout

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

#pragma comment(lib, "./lib/edk.lib")

EmoHandler::EmoHandler()//(QObject *parent) :
    //QObject(parent)
{
	//connect(&pollTimer, SIGNAL(timeout()), this, SLOT(emoAffectivEngagementBoredom()));
	
	eEvent			= EE_EmoEngineEventCreate();
	eState			= EE_EmoStateCreate();
	affectivEngagementBoredom = -1.0f;
	//affectivEngagementBoredomVector.clear();

	//pollTimer.start(1000);
}

EmoHandler::~EmoHandler()
{
	EE_EngineDisconnect();
	EE_EmoStateFree(eState);
	EE_EmoEngineEventFree(eEvent);
}


int EmoHandler::emoConnect() {

	std::string input;
	int option = 0;
	int result = 0;
	std::string logfilename;

	const unsigned short composerPort	= 1726;

	_tsetlocale(LC_ALL,_T("portuguese"));


	try {

		//if (argc != 2) {
			//throw std::exception("Please supply the log file name.\nUsage: EmoStateLogger [log_file_name].");
		//}

		std::cout << "==============================================================================" << std::endl;
		std::cout << "SISTEMA DE CONTROLE BASEADO EM EMO��ES ATRAV�S DE INTERFACE C�REBRO COMPUTADOR" << std::endl;
		std::cout << "Eng. Maria Luiza Recena Menezes		   		         PPGEE - UFRGS" << std::endl;
		std::cout << "==============================================================================" << std::endl;

		std::cout << "Please supply the log file name:                                   " << std::endl;
		std::cout << ">> ";
		std::getline(std::cin, logfilename, '\n');

		while ( logfilename == "" )
		{
			std::cout << "Please supply a valid log file name:                                   " << std::endl;
			std::cout << ">> ";
			std::getline(std::cin, logfilename, '\n');
		}

		std::cout << "Press '1' to start and connect to the EmoEngine                    " << std::endl;
		std::cout << "Press '2' to connect to the EmoComposer                            " << std::endl;
		std::cout << ">> ";
		std::getline(std::cin, input, '\n');
		option = atoi(input.c_str());

		
		int emoConnectionStatus = -1;

		switch (option) 
		{
			case 1: //EmoEngine
			{
				emoConnectionStatus = EE_EngineConnect();
				if (emoConnectionStatus != EDK_OK) 
				{
					result = 0;
					throw std::exception("Emotiv Engine start up failed.");
				}
				else if (emoConnectionStatus == EDK_OK)
				{
					result = 1;
					ofs.open(logfilename);
					ofs << "Emotiv Engine start up succeed. Ready to receive Emotiv signals.";
					throw std::exception("Emotiv Engine start up successed. Ready to receive Emotiv signals.");
				}
				break;
			}
			case 2://case EmoComposer:
			{
				emoConnectionStatus = EE_EngineRemoteConnect("127.0.0.1", composerPort);
				if (emoConnectionStatus != EDK_OK)
				{
					result = 0;
					std::string errMsg = "Cannot connect to EmoComposer on 127.0.0.1";
					throw std::exception(errMsg.c_str());
				}
				else if (emoConnectionStatus == EDK_OK)
				{
					result = 1;
					ofs.open(logfilename);
					ofs << "Emotiv Composer start up successed. Ready to receive Emotiv signals.";
					throw std::exception("Emotiv Composer start up succeed. Ready to receive Emotiv signals.");
				}
				break;
			}
			default:
				result = 0;
				throw std::exception("Invalid option...");
				break;
		}//end switch
	}//end try
	catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
	std::cout << "Press any key to continue..." << std::endl;
	getchar();
	}//end catch
	return result;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float EmoHandler::emoAffectivEngagementBoredom() {

	eEvent								= EE_EmoEngineEventCreate();
	eState								= EE_EmoStateCreate();
	int state							= 0;
																											
	state = EE_EngineGetNextEvent(eEvent);																		
	if (state == EDK_OK)																							
	{																											
		//std::cout << "state == EDK_OK"<< std::endl;																
																													
		EE_Event_t eventType = EE_EmoEngineEventGetType(eEvent);
															
		// if signal is not noisy and Affective is active:															
		if (ES_AffectivIsActive(eState, AFF_ENGAGEMENT_BOREDOM)) 																	
		{																										
			//std::cout << "eventType == EE_EmoStateUpdated"<< std::endl;											
			EE_EmoEngineEventGetEmoState(eEvent, eState);														
			//std::cout<<"Engagement level:" <<ES_AffectivGetEngagementBoredomScore(eState)<<std::endl;

			return ES_AffectivGetEngagementBoredomScore(eState);
		}	
		else 
			return -1;
	}	
	else 
		return -1;
}
