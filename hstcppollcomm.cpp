#include "hstcppollcomm.h"
#include <iostream>

///////////////////////////////////
/*
#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
*/
///////////////////////////////////

/*----------------------------------------------------------------------------*/
/*
    - Para iniciar uma conexao crie uma instancia desta classe informando o
      endereco IP do systembox.

    - O metodo pollProcess() eh executado a cada dois segundos enviando uma
      solicitacao de status das units listadas no inicio do metodo. Este tempo
      esta definido no construtor -> pollTimer.start(2000)

    - Em pollProcess informar a lista de units que deseja obter o status
      A unit eh o identificador das entradas e saidas dos dispositivos.
      Para identificar as units abra a configuracao no ScapeWorks, selecione
      a unit desejada, clique com o botao direito e selecione propriedades.
      O campo numero interno corresponde a unit.

    - O metodo tcpOnRead eh executado de forma assincrona quando recebe uma resposta
      do systembox. A resposta vem no formato unit-valor. Esta identificado no
      metodo o local que deve ser feito o parse.

    - Para enviar um comando chame o metodo: setUnit(int unit, int value)
      informando a unit e o valor desejado.

    - Existem varias chamadas a qDebug imprimindo mensagens no console que
      facilitarao o entendimento.
*/
/*----------------------------------------------------------------------------*/


HsTcpPollComm::HsTcpPollComm(QObject *parent) :
    QObject(parent)
{
    sessionState = DISCONNECTED;
	resposta_pronta = false;
    // tcp signals
    connect(&tcpSocket, SIGNAL(connected()),this,SLOT(tcpOnConnect()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpOnDisconnect()));
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpOnError(QAbstractSocket::SocketError)));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(tcpOnRead()));
    // timer signals
    connect(&pollTimer, SIGNAL(timeout()), this, SLOT(pollProcess()));
    pollTimer.start(2000);
}

bool HsTcpPollComm::tcpConnect(QString host, int port)
{
    if ( host == "localhost" )
        tcpSocket.connectToHost(QHostAddress::LocalHost, port );
    else
        tcpSocket.connectToHost(host, port );

    if (tcpSocket.waitForConnected(3000))
	{
		qDebug() << "Conectado com sucesso aleluia amen!! Sarava!";
	}
	
    return true;
}

void HsTcpPollComm::pollProcess(void)
{
	int unit = 1;
	QString strGet = 0;
	qDebug() << "HsTcpPollComm::pollProcess";
    // A maquina de estados deve ficar em um lugar soh. Para poll e para read.
	qint64 result = 0;
	
    switch(sessionState)
    {
        case DISCONNECTED: qDebug() << "Sessão HS inativa";
                // Verificar se conexao caiu. Iniciar nova tentativa de conexao
                // ou seguir a partir do ponto de autenticacao.
                break;
        case CHECK_AUTH: //sendRequest();
			//setUnit(EQUIPAMENTO_UNITS[0], 1);
			/*result = tcpSocket.write(makeGetUnit(EQUIPAMENTO_UNITS,1).toLatin1()); //toStdString().c_str());
			qDebug() << "result tcpSocket.write: " << result;
			if (tcpSocket.isReadable() )
			{
				qDebug() << "tcpSocket is readable";
			}*/
			// strGet = makeGetUnit(&unit,1).toLatin1();
              tcpSocket.write(makeGetUnit(&unit,1).toLatin1());
                qDebug() << "Send check auth";
			//	qDebug() << "makegetunit= " << strGet;
                sessionState = WAITING_CONN; // Alterar sessionState em tcpOnRead
                break;
        case WAITING_CONN: qDebug() << "Wait connection";
                break;
        case SENT_AUTH: qDebug() << "Sent auth";
                {
                    QByteArray myArray;
                    QString user = "admin";
                    QString pass = CryptPass("admin");
                    myArray.append('p'); myArray.append('T');
                    myArray.append(user);
                    myArray.append('\0');
                    myArray.append(pass);
                    tcpSocket.write(myArray);
                }
                break;
        case SENT_REQUEST: resposta_pronta = false;
				tcpSocket.write(makeGetUnit(EQUIPAMENTO_UNITS,3).toLatin1());//toStdString().c_str());
                sessionState = WAITING_RESPONSE;
                qDebug() << "Send get unit";
                break;
        case WAITING_RESPONSE: qDebug() << "Wait get unit";
                break;
        default: qDebug() << "Sessão ativa";
    }
}


/* QString HsTcpPollComm::makeGetUnit( int *buffUnit, int numUnit )

Monta string para pegar o estado de todas as units
Request:
    5G*unit1*unit2*unitn**'\0'

*/
QString HsTcpPollComm::makeGetUnit( const int *buffUnit, int numUnit )
{
  QString strGet;

 // strGet.reserve(numUnit); malu que tirou da HS
  // Header
  strGet.append("5G*");
  // Preenche string com units
  for( int i = 0; i < numUnit; i++ )
    strGet.append(QString("%1*").arg(*buffUnit++));
  // Terminador
  strGet.append("*");
  qDebug() << "strGet: "<< strGet;
  return strGet;
}

/*
HsTcpPollComm::setUnit(int unit, int value)
Envia comando "value" para a "unit"
Seta e envia string pronta para mudar o valor da unit 
*/
void HsTcpPollComm::setUnit(int unit, int value)
{
    QString strSet;
    // Header
    strSet.append(QString("5S%1*%2*").arg(unit).arg(value));
    tcpSocket.write(strSet.toLatin1());
    qDebug() << strSet;
}

/*
 Slots bock
*/

void HsTcpPollComm::tcpOnConnect(void)
{
    qDebug() << "Tcp conectado...";
    sessionState = CHECK_AUTH;
    // emit tcpOpen();
    // emit pollOk ... etc.
}

void HsTcpPollComm::tcpOnDisconnect(void)
{
    qDebug() << "Tcp desconectado...";
    sessionState = DISCONNECTED;
    emit tcpClosed();
}

void HsTcpPollComm::tcpOnError(QAbstractSocket::SocketError error)
{
    qDebug() << "Tcp error: " << error;
    sessionState = DISCONNECTED;
}

void HsTcpPollComm::tcpOnRead(void)
{

  QByteArray response = tcpSocket.readAll();
  qDebug() << "HsTcpPollComm::tcpOnRead: sessionState =  " << sessionState;
  qDebug() << "HsTcpPollComm::tcpOnRead: response =  " << response;

  switch(sessionState)
  {
    case WAITING_CONN:
      if((response[0] == '\0') && (response[1] == '*') && (response[2] == '1'))
      {
        qDebug() << "Connection ok";
        sessionState = SENT_REQUEST;
      }
      else
      {
        qDebug() << "Connection fail";
        sessionState = SENT_AUTH;
      }
      break;
    case SENT_AUTH:
      if((response[0] == 'p') && (response[1] == 'T') && (response[2] == '1'))
      {
        qDebug() << "Auth ok";
        sessionState = SENT_REQUEST;
      }
      else
      {
        qDebug() << "Auth fail";
        pollTimer.stop();
        tcpSocket.disconnect();
      }

    case WAITING_RESPONSE:
      /*
        AQUI IMPLEMENTAR PARSE DA STRING PARA OBTER O RESULTADO
		  Response:    '\0'*unit1-val*unit2-val*unitn-val*'\0'

      */
		resposta = response.data();
		resposta_pronta = true;
		//setUnit(118,1);

		sessionState = SENT_REQUEST;
		break;
    }
    response[0] = '@';
    qDebug() << "Tcp read " << response.data();
}

QString HsTcpPollComm::CryptPass(char * szPassword)
{
    static char szPassKey[] =  "xo9dks09df,cPljinamc";

    char szHex[10];
    int i, c, k, n, len;
    static char szCripPassword[256];

    len = strlen(szPassword);
    if (len*2 >= (int)sizeof(szCripPassword))
        return NULL;

    szCripPassword[0] = 0;
    for (i = 0, n = 0; i < len; i++)
    {
        c = szPassword[i];
        if (c >= ' ')
        {
            k = szPassKey[n++%(sizeof(szPassKey)-1)];
            c ^= k;
            sprintf(szHex, "%02x", c);
            strcat(szCripPassword, szHex);
        }
        else
        {
            sprintf(szHex, "%c%c", c,c);
            strcat(szCripPassword, szHex);
        }
    }
    return szCripPassword;
}

/////////////////////////////////////
/*
void HsTcpPollComm::sendRequest(){
 
    // create custom temporary event loop on stack
    QEventLoop eventLoop;
 
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
 
    // the HTTP request
    QNetworkRequest req( QUrl( QString("http://10.1.7.37//monitor/monitor.cgi?ref_page=cmd&unit=119&newvalue=1") ) );
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called
 
    if (reply->error() == QNetworkReply::NoError) {
        //success
        qDebug() << "Success";// <<reply->readAll();
        delete reply;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
}*/