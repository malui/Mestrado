#pragma once
#ifndef EMOHANDLER_H_INCLUDED
#define EMOHANDLER_H_INCLUDED

//#include <QThread>
// Qt
#include <QObject>
#include <QTimer>

// Emotiv
#include <iostream>
#include <fstream>
#include <conio.h>
#include <sstream>
#include <windows.h>
#include <map>

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

//Emotiv enxerga HomeSystem
#include "hstcppollcomm.h"
#include "DefinicoesGlobais.h"

#pragma comment(lib, "./lib/edk.lib")

 class EmoHandler : public QObject
 {
     Q_OBJECT

 public:
		 explicit EmoHandler(QObject *parent = 0);
		 void emoConnect();
		 void logEmoState(unsigned int userID, EmoStateHandle eState);
		 inline void setHsTcpPollComm(HsTcpPollComm *hs){ hsTcpPollComm = hs; };
		 void pegaEstadoEquipamentos();
		 //void exibeEstadoEquipamentos() const;  //essa funcao nao altera nenhum valor das variaveis da classe EmoHandler

 public slots:
	     void run();

 private:
	    QTimer pollTimer;
		EmoEngineEventHandle eEvent;
		EmoStateHandle eState;
		unsigned int userID;
		unsigned short composerPort;
		int option;
		int state;
		std::string input;
		HsTcpPollComm *hsTcpPollComm;
		//Equipamentos equipamentos;
		int estadoHandler;
		bool flag;

 };


#endif