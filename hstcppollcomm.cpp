  
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
	crossFlag =false;
    // tcp signals
    connect(&tcpSocket, SIGNAL(connected()),this,SLOT(tcpOnConnect()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpOnDisconnect()));
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpOnError(QAbstractSocket::SocketError)));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(tcpOnRead()));
    // timer signals
    connect(&pollTimer, SIGNAL(timeout()), this, SLOT(pollProcess()));
    pollTimer.start(4000);
}

bool HsTcpPollComm::tcpConnect(QString host, int port)
{
    if ( host == "localhost" )
        tcpSocket.connectToHost(QHostAddress::LocalHost, port );
    else
        tcpSocket.connectToHost(host, port );
#ifdef HS
    if (tcpSocket.waitForConnected(2000)) //3000
	{
		qDebug() << "Conectado com sucesso aleluia amen!! Sarava!";
	}
#endif
#ifdef KIT
    if (tcpSocket.waitForConnected(3000))
	{
		qDebug() << "Conectado com sucesso aleluia amen!! Sarava!";
	}
#endif
	
    return true;
}

void HsTcpPollComm::pollProcess(void)
{
	int unit = 1;
	QString strGet = 0;

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
				//resposta_pronta = true;
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
kit: *5S*unit*valor*
HS: 5S*unit1*valor1*5S*unit2*valor2*5S*unit_n*valor_n*
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
Nao funciona no KIT, so na HS
HsTcpPollComm::setCenario(std::vector<int> units, std::vector<int> values)
Retorna 1 se sucesso e 0 se erro
Envia comandos "values" para as "units"
Seta e envia string pronta para mudar o cenario 
HS: 5S*unit1*valor1*5S*unit2*valor2*5S*unit_n*valor_n*

 int unitsInicializador[] = {118,119};
 int valuesInicializador[] = {1,1};

 std::vector<int> units(unitsInicializador,&unitsInicializador[sizeof(unitsInicializador)/sizeof(unitsInicializador[0])]);
 std::vector<int> values(valuesInicializador,&valuesInicializador[sizeof(valuesInicializador)/sizeof(valuesInicializador[0])]);
 hsTcpPollComm.setCenario(units, values);
*/
int HsTcpPollComm::setCenario(std::vector<int> units, std::vector<int> values)
{
    QString strSet;
    // Header
	strSet.append(QString("5S%1*%2*").arg(units[0]).arg(values[0]));
	if ( units.size() != values.size() )
	{
		//erro!
		return 0;
	}
	else
	{
		for (int i=1; i<units.size(); i++)
		{
			strSet.append(QString("%1*%2*").arg(units[i]).arg(values[i]));
		}

		tcpSocket.write(strSet.toLatin1());
		qDebug() << "setCenario: "<< strSet;

		return 1; //sucesso!
	}
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


/////////GUILHERME BEGIN//////////////////

void HsTcpPollComm::printCenario (const vector<int>& v)
{

    for( std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
        std::cout << *i << ' ';
}

void HsTcpPollComm::printPopulacao ( Populacao population)
{
    for(int i=0; i<population.size(); i++)
    {
        printCenario(std::get<0>(population[i]));
        cout <<  std::get<1>(population[i]) << endl;

    }
}

float HsTcpPollComm::avaliacaoAptidaoCenario(Cenario cenarioAAvaliar){
    return 0.96;
}

HsTcpPollComm::crossoverCenarios HsTcpPollComm::crossover(Cenario cenario1,Cenario cenario2)
{
    std::size_t const half_size = cenario1.size() / 2;
    std::vector<int> split_first_cenario1(cenario1.begin(), cenario1.begin() + half_size);
    std::vector<int> split_last_cenario1(cenario1.begin() + half_size, cenario1.end());

    std::vector<int> split_first_cenario2(cenario1.begin(), cenario1.begin() + half_size);
    std::vector<int> split_last_cenario2(cenario1.begin() + half_size, cenario1.end());

    split_first_cenario1.insert(split_first_cenario1.end(), split_last_cenario2.begin(), split_last_cenario2.end());
    split_first_cenario2.insert(split_first_cenario2.end(), split_last_cenario1.begin(), split_last_cenario1.end());
    crossoverCenarios crossoverTupla= make_tuple(split_first_cenario1,split_first_cenario2);
    return crossoverTupla;
}


////////GUILHERME END////////////////////


void HsTcpPollComm::controle()      // a variavel controle fluxo deve ser setada antes da chamada da função controle
{									// controleFluxo= PRIMEIRA_EXECUÇAO_RESPOSTA_SET_CENARIO;
	TuplaCenario tuplaCenario;      //controleFluxo = ENESIMA_EXECUÇAO;
	int unitsInicializador[] = {118,119};
    int valuesInicializador[] = {0,0};
	std::vector<int> units(unitsInicializador,&unitsInicializador[sizeof(unitsInicializador)/sizeof(unitsInicializador[0])]);
    std::vector<int> values(valuesInicializador,&valuesInicializador[sizeof(valuesInicializador)/sizeof(valuesInicializador[0])]);
	crossoverCenarios tuplaCrossoverCenarios;
	Cenario cenarioFilho1;
	Cenario cenarioFilho2;
	
	switch(controleFluxo)           //controleFluxo = ENESIMA_EXECUÇAO_RESPOSTA_SET_CENARIO;
	{									//controleFluxo = ENESIMA_EXECUÇAO;	
    case PRIMEIRA_EXECUCAO:
		

		valuesInicializador[0] = 1; 
		valuesInicializador[1] = 1;
		
		values.pop_back();
		values.pop_back();
		values.push_back(valuesInicializador[0]);
		values.push_back(valuesInicializador[1]);

        if (crossFlag)
			setCenario(units, cenarioFilho1); 

		else 
			setCenario(units, values);  
		
        tuplaCenario= make_tuple(values,-1);

        geracaoAtual.push_back(tuplaCenario);
		controleFluxo = SEGUNDA_EXECUCAO;
		break;
	case SEGUNDA_EXECUCAO:
		tuplaCenario = geracaoAtual.back();
        std::get<1>(tuplaCenario) =  emoHandler->affectivEngagementBoredom;	  


		valuesInicializador[0] = 1; 
		valuesInicializador[1] = 0;

		values.pop_back();
		values.pop_back();
		values.push_back(valuesInicializador[0]);
		values.push_back(valuesInicializador[1]);

        if (crossFlag)
			setCenario(units, cenarioFilho2); 
		else 
			setCenario(units, values); 

        tuplaCenario= make_tuple(values,-1);
		
        geracaoAtual.push_back(tuplaCenario);
		controleFluxo= ENESIMA_EXECUCAO;
		break;
	case ENESIMA_EXECUCAO:
		tuplaCenario = geracaoAtual.back();
        std::get<1>(tuplaCenario) =  emoHandler->affectivEngagementBoredom;
        
		sort(geracaoAtual.begin(),geracaoAtual.end(),
             [](const TuplaCenario& a,
                const TuplaCenario& b) -> bool
        {
            return std::get<1>(a) > std::get<1>(b);
        });
		
		
		tuplaCrossoverCenarios = crossover(std::get<0>(geracaoAtual[0]) ,std::get<0>(geracaoAtual[1])); // std::get<0>geracaoAtual[0] retorna o cenario da primeira tupla contida no vetor geracaoAtual,
																										// std::get<0>geracaoAtual[1] retorna o cenario da segunda tupla contida no vetor geracaoAtual,
		cenarioFilho1 = std::get<0>(tuplaCrossoverCenarios);
		cenarioFilho2 = std::get<1>(tuplaCrossoverCenarios);
		crossFlag = true;
		controleFluxo = PRIMEIRA_EXECUCAO;
		break;
	default: std::cout << "SessionState desconhecido: "<< sessionState << std::endl;
	}
		
	
}

void logControle(std::ostream& os, unsigned int userID, EmoStateHandle eState, bool withHeader) {

	// Create the top header
	if (withHeader) {
		os << "Time,";
		os << "UserID,";
		os << "Wireless Signal Status,";

		os << "Equipamento,";
		os << "Estado";

		os << "Short Term Excitement,";
		os << "Long Term Excitement,";
		os << "Engagement/Boredom,";
		os << std::endl;
	}

	// Log the time stamp and user ID
	os << ES_GetTimeFromStart(eState) << ",";
	os << userID << ",";
	os << static_cast<int>(ES_GetWirelessSignalStatus(eState)) << ",";

	// Affectiv Suite results
	os << ES_AffectivGetExcitementShortTermScore(eState) << ",";
	os << ES_AffectivGetExcitementLongTermScore(eState) << ",";

	os << ES_AffectivGetEngagementBoredomScore(eState) << ",";

	os << std::endl;
}

void logAffectiveSuiteState(std::ostream& os, unsigned int userID, EmoStateHandle eState, bool withHeader) {

	// Create the top header
	if (withHeader) {
		os << "Time,";
		os << "UserID,";
		os << "Wireless Signal Status,";

		os << "Short Term Excitement,";
		os << "Long Term Excitement,";
		os << "Engagement/Boredom,";
		os << std::endl;
	}

	// Log the time stamp and user ID
	os << ES_GetTimeFromStart(eState) << ",";
	os << userID << ",";
	os << static_cast<int>(ES_GetWirelessSignalStatus(eState)) << ",";

	// Affectiv Suite results
	os << ES_AffectivGetExcitementShortTermScore(eState) << ",";
	os << ES_AffectivGetExcitementLongTermScore(eState) << ",";

	os << ES_AffectivGetEngagementBoredomScore(eState) << ",";

	os << std::endl;
}
