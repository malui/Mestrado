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
//#include "hstcppollcomm.h"
#include "DefinicoesGlobais.h"

#pragma comment(lib, "./lib/edk.lib")

 class EmoHandler : public QObject
 {
     Q_OBJECT

 public:
		 explicit EmoHandler(QObject *parent = 0);
		 ~EmoHandler();
		 int emoConnect();
		 float affectivEngagementBoredom;
		 //std::string logFileName;
		 std::ofstream ofs;

 public slots:
		 void emoAffectivEngagementBoredom();

 private:
	    QTimer pollTimer;
		EmoEngineEventHandle eEvent;
		EmoStateHandle eState;
 };


#endif