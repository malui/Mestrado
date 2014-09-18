#include "EmoHandler.h"
#include <QDebug>
#include <sstream>
#include <windows.h>
#include <map>

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

#pragma comment(lib, "./lib/edk.lib")

EmoHandler::EmoHandler(QObject *parent) :
    QObject(parent)
{
	connect(&pollTimer, SIGNAL(timeout()), this, SLOT(emoAffectivEngagementBoredom()));
	
	eEvent			= EE_EmoEngineEventCreate();
	eState			= EE_EmoStateCreate();
	affectivEngagementBoredom = -1.0f;

	pollTimer.start(1000);
}

EmoHandler::~EmoHandler()
{
	EE_EngineDisconnect();
	EE_EmoStateFree(eState);
	EE_EmoEngineEventFree(eEvent);
}


void EmoHandler::emoConnect(int argc) {

	std::string input;
	int option = 0;

	const unsigned short composerPort	= 1726;


	try {

		//if (argc != 2) {
			//throw std::exception("Please supply the log file name.\nUsage: EmoStateLogger [log_file_name].");
		//}

		std::cout << "===================================================================" << std::endl;
		std::cout << "Example to show how to log the EmoState from EmoEngine/EmoComposer." << std::endl;
		std::cout << "===================================================================" << std::endl;
		std::cout << "Press '1' to start and connect to the EmoEngine                    " << std::endl;
		std::cout << "Press '2' to connect to the EmoComposer                            " << std::endl;
		std::cout << ">> ";

		std::getline(std::cin, input, '\n');
		option = atoi(input.c_str());

		switch (option) 
		{
			case 1: //EmoEngine
			{
				std::cout<<"Emotiv Engine start up: "<< EE_EngineConnect() <<std::endl;
				if (EE_EngineConnect() != EDK_OK) 
				{
					throw std::exception("Emotiv Engine start up failed.");
					//std::cout<<"teste Emotiv Engine start up failed. "<<std::endl; //mudado para tirar exception
				}
				break;
			}
			case 2://case EmoComposer:
			{
				if (EE_EngineRemoteConnect("127.0.0.1", composerPort) != EDK_OK)
				{
					std::string errMsg = "Cannot connect to EmoComposer on 127.0.0.1";
					throw std::exception(errMsg.c_str());
					//std::cout<<"Cannot connect to EmoComposer."<<std::endl; //mudado para tirar exception
					// bug: fazer algo para parar o codigo
				}
				break;
			}
			default:
				throw std::exception("Invalid option...");
				break;
		}//end switch
	}//end try
	catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
	std::cout << "Press any key to exit..." << std::endl;
	getchar();
	}

	std::cout << "Ready to receive Emotiv signals" << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EmoHandler::emoAffectivEngagementBoredom() {

	eEvent								= EE_EmoEngineEventCreate();
	eState								= EE_EmoStateCreate();
	int state							= 0;

	Sleep(100);
																											
	state = EE_EngineGetNextEvent(eEvent);																		
	if (state == EDK_OK)																							
	{																											
		//std::cout << "state == EDK_OK"<< std::endl;																
																													
		EE_Event_t eventType = EE_EmoEngineEventGetType(eEvent);
															
		// Log the EmoState if it has been updated																
		if (eventType == EE_EmoStateUpdated) 																	
		{																										
			//std::cout << "eventType == EE_EmoStateUpdated"<< std::endl;											
			EE_EmoEngineEventGetEmoState(eEvent, eState);														
			//std::cout<<"Engagement level:" <<ES_AffectivGetEngagementBoredomScore(eState)<<std::endl;

			affectivEngagementBoredom = ES_AffectivGetEngagementBoredomScore(eState);
		}	

	}	
}
