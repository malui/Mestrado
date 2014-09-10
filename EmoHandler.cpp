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


void EmoHandler::emoConnect() {

	const unsigned short composerPort	= 1726;

/*	
//case EmoEngine:

	if (EE_EngineConnect() != EDK_OK) 
	{
		//throw std::exception("Emotiv Engine start up failed.");
		std::cout<<"Emotiv Engine start up failed. "<<std::endl; //mudado para tirar exception
	}

*/
//case EmoComposer:
	if (EE_EngineRemoteConnect("127.0.0.1", composerPort) != EDK_OK)
		{
			std::cout<<"Cannot connect to EmoComposer."<<std::endl; //mudado para tirar exception
			// bug: fazer algo para parar o codigo
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
