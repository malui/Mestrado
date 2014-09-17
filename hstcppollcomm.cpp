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
	emoHandler = NULL;
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
//	qDebug() << "HsTcpPollComm::pollProcess";
    // A maquina de estados deve ficar em um lugar soh. Para poll e para read.
	qint64 result = 0;

    switch(sessionState)
    {
        case DISCONNECTED: qDebug() << "Sessão HS inativa";
                // Verificar se conexao caiu. Iniciar nova tentativa de conexao
                // ou seguir a partir do ponto de autenticacao.
                break;
        case CHECK_AUTH: //sendRequest();
              tcpSocket.write(makeGetUnit(&unit,1).toLatin1());
                qDebug() << "Send check auth";
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
        case SENT_REQUEST: //resposta_pronta = false;
				//tcpSocket.write(makeGetUnit(EQUIPAMENTO_UNITS,3).toLatin1());//toStdString().c_str());
				controle();
                sessionState = WAITING_RESPONSE;
                qDebug() << "Send get unit";
                break;
        case WAITING_RESPONSE: qDebug() << "Wait get unit";
                break;
		default: std::cout << "SessionState desconhecido: "<< sessionState << std::endl;
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
*5S*unit*valor*
*/
void HsTcpPollComm::setUnit(int unit, int value)
{
    QString strSet;
    // Header
    strSet.append(QString("5S%1*%2*").arg(unit).arg(value));
    tcpSocket.write(strSet.toLatin1());
    qDebug() << "setUnit: "<< strSet;
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
		resposta_pronta = true;
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
		std::cout << "WAITING_RESPONSE resposta = " << response.data() <<std::endl; 
		sessionState = SENT_REQUEST;
		break;

	case SENT_REQUEST:
		qDebug() << "teste";
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

void HsTcpPollComm::controle()
{

	static int i = 0;
	static int j = 0;
	//float* engagementLevels = new float [tamanho_engagementLevel]; //cria vetor dinamicamente
	static float* engagementLevels = new float [2];
	static int posicao_maior_engagement = 0;

	if (emoHandler)
	{
		if (resposta_pronta)
		{
			int* estado = ESTADOS[i];
			const int tamanho_engagementLevel = TAMANHO_ESTADOS[i]; //=3
			
			if ( i < EQUIPAMENTOS_TAMANHO ) //lista de equipamentos e lista de estados
			{
				if ( j < TAMANHO_ESTADOS[i] ) //lista de estados de cada equipamento ate o final, qual  ofinal?
				{

					//envia estado j para o equipamento i
					setUnit(EQUIPAMENTO_UNITS[i], estado[j]);

					//otimizar para nao guardar tal buffer, so precisa dos dois ultimos valores:
					//pega os estados de engagement para todos os estados do equipamento i e poe num buffer
					engagementLevels[j] = emoHandler->affectivEngagementBoredom;
					std::cout<<"Engagement level com equipamento "<<i<<" e estado "<<j<<": "<<engagementLevels[j]<<std::endl;

					//verifica qual a posicao do maior estado de engagement do buffer
					if (j > 0)
					{
						if (engagementLevels[j] > engagementLevels[posicao_maior_engagement])
						{
							posicao_maior_engagement = j;
						} //if maior engagement
					}// if j>=0 

					++j;

				}// if tamanho dos estados 
				else if ( j >= TAMANHO_ESTADOS[i] )
				{
					std::cout<<"Maior Engagement Level para o Equipamento "<<i<<" foi com o estado "<<posicao_maior_engagement<<": "<<engagementLevels[posicao_maior_engagement]<<std::endl;
					//delete[] engagementLevels;
					std::cout<<"Setando Equipamento "<<i<<" com o estado "<<posicao_maior_engagement<<std::endl;
					//seta o estado do equipamento i conforme o maior estado de engagement equivalente
					setUnit(EQUIPAMENTO_UNITS[i], estado[posicao_maior_engagement]);
					std::cout<<"i: "<<i<<" j: "<<j<<std::endl;
					i++;
					j = 0;
				}

			} //if i < equipamentos tamanho
		}
	}
}