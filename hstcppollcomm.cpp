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
        case SENT_REQUEST: resposta_pronta = false;
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

void HsTcpPollComm::printSenario (const vector<int>& v)
{

    for( std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
        std::cout << *i << ' ';
}

void HsTcpPollComm::printPopulacao ( Populacao population)
{
    for(int i=0; i<population.size(); i++)
    {
        printSenario(std::get<0>(population[i]));
        cout <<  std::get<1>(population[i]) << endl;

    }
}

float HsTcpPollComm::avaliacaoAptidaoSenario(Senario senarioAAvaliar){
    return 0.96;
}

HsTcpPollComm::crossoverSenarios HsTcpPollComm::crossover(Senario senario1,Senario senario2)
{
    std::size_t const half_size = senario1.size() / 2;
    std::vector<int> split_first_senario1(senario1.begin(), senario1.begin() + half_size);
    std::vector<int> split_last_senario1(senario1.begin() + half_size, senario1.end());

    std::vector<int> split_first_senario2(senario1.begin(), senario1.begin() + half_size);
    std::vector<int> split_last_senario2(senario1.begin() + half_size, senario1.end());

    split_first_senario1.insert(split_first_senario1.end(), split_last_senario2.begin(), split_last_senario2.end());
    split_first_senario2.insert(split_first_senario2.end(), split_last_senario1.begin(), split_last_senario1.end());
    crossoverSenarios crossoverTupla= make_tuple(split_first_senario1,split_first_senario2);
    return crossoverTupla;
}

////////GUILHERME END////////////////////
/*
void HsTcpPollComm::controle()
{

	//GUILHERME BEGIN
	int senarioInicial1[] = {1,0,0,0};
    int senarioInicial2[] = {1,1,0,0};
    int senarioInicial3[] = {1,1,1,0};
    int senarioInicial4[] = {1,1,1,1};

    Senario senario1(senarioInicial1,&senarioInicial1[sizeof(senarioInicial1)/sizeof(senarioInicial1[0])]);
    Senario senario2(senarioInicial2,&senarioInicial2[sizeof(senarioInicial2)/sizeof(senarioInicial2[0])]);
    Senario senario3(senarioInicial3,&senarioInicial3[sizeof(senarioInicial3)/sizeof(senarioInicial3[0])]);
    Senario senario4(senarioInicial4,&senarioInicial4[sizeof(senarioInicial4)/sizeof(senarioInicial4[0])]);


	TuplaSenario tuplaSenario1= make_tuple(senario1,0.9);
    TuplaSenario tuplaSenario2= make_tuple(senario2,0.4);
    TuplaSenario tuplaSenario3= make_tuple(senario3,0.7);
    TuplaSenario tuplaSenario4= make_tuple(senario4,0.6);
	
	Populacao primeiraGeracao;
    primeiraGeracao.push_back(tuplaSenario1);
    primeiraGeracao.push_back(tuplaSenario2);
    primeiraGeracao.push_back(tuplaSenario3);
    primeiraGeracao.push_back(tuplaSenario4);

	 // Ordenação dos senarios de acordo com a aptidáo
    sort(primeiraGeracao.begin(),primeiraGeracao.end(),
         [](const TuplaSenario& a,
            const TuplaSenario& b) -> bool
    {
        return std::get<1>(a) > std::get<1>(b);
    });
	printPopulacao(primeiraGeracao);

	 // Criação da próxima geração
    Populacao proximaGeracao;
    // Copia Elite da geração passada para a geração atual
    proximaGeracao.push_back(primeiraGeracao[0]);

    //cria dois novos estados através de crossover, a partir de elite e do segundo mais apto
    crossoverSenarios newSenariosTupla1;
    newSenariosTupla1 = crossover(std::get<0>(primeiraGeracao[0]),std::get<0>(primeiraGeracao[1]));
    Senario stateToBeEvaluated1 = std::get<0>(newSenariosTupla1);
    Senario stateToBeEvaluated2 = std::get<1>(newSenariosTupla1);

    //cria dois novos estados através de crossover, a partir de elite e do terceiro mais apto
    crossoverSenarios newSenariosTupla2;
    newSenariosTupla2 =crossover(std::get<0>(primeiraGeracao[0]),std::get<0>(primeiraGeracao[2]));
    Senario stateToBeEvaluated3 = std::get<0>(newSenariosTupla2);
    Senario stateToBeEvaluated4 = std::get<1>(newSenariosTupla2);

    // evaluateSenario é a função que irá retornar o grau de aptdão do estado
    float senario1Classification = avaliacaoAptidaoSenario(stateToBeEvaluated1);
    TuplaSenario newTuplaSenario1= make_tuple(stateToBeEvaluated1,senario1Classification);

    //Adicionamos o novo estado na geração atual, e ordenamos para que a elite fique no topo
    proximaGeracao.push_back(newTuplaSenario1);
    sort(proximaGeracao.begin(),proximaGeracao.end(),
         [](const TuplaSenario& a,
            const TuplaSenario& b) -> bool
    {
        return std::get<1>(a) > std::get<1>(b);
    });

    printPopulacao(proximaGeracao);

	//GUILHERME END
}*/

void HsTcpPollComm::controle()
{
	static int i = 0;
	static int j = 0;
	//float* engagementLevels = new float [tamanho_engagementLevel]; //cria vetor dinamicamente
	static float* engagementLevels = new float [2];
	static int posicao_maior_engagement = 0;

	//std::ofstream ofs(emoHandler->logFileName);

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

					/*
					 int unitsInicializador[] = {118,119};
 int valuesInicializador[] = {1,1};

 std::vector<int> units(unitsInicializador,&unitsInicializador[sizeof(unitsInicializador)/sizeof(unitsInicializador[0])]);
 std::vector<int> values(valuesInicializador,&valuesInicializador[sizeof(valuesInicializador)/sizeof(valuesInicializador[0])]);
 setCenario(units, values);*/
					setUnit(EQUIPAMENTO_UNITS[i], estado[j]);

					//otimizar para nao guardar tal buffer, so precisa dos dois ultimos valores:
					//pega os estados de engagement para todos os estados do equipamento i e poe num buffer
					engagementLevels[j] = emoHandler->affectivEngagementBoredom;
					emoHandler->ofs <<"Engagement level com equipamento "<<i<<" e estado "<<j<<": "<<engagementLevels[j]<<std::endl;
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
					emoHandler->ofs<<"Maior Engagement Level para o Equipamento "<<i<<" foi com o estado "<<posicao_maior_engagement<<": "<<engagementLevels[posicao_maior_engagement]<<std::endl;
					std::cout<<"Maior Engagement Level para o Equipamento "<<i<<" foi com o estado "<<posicao_maior_engagement<<": "<<engagementLevels[posicao_maior_engagement]<<std::endl;
					//delete[] engagementLevels;
					emoHandler->ofs<<"Setando Equipamento "<<i<<" com o estado "<<posicao_maior_engagement<<std::endl;
					std::cout<<"Setando Equipamento "<<i<<" com o estado "<<posicao_maior_engagement<<std::endl;
					//seta o estado do equipamento i conforme o maior estado de engagement equivalente
					setUnit(EQUIPAMENTO_UNITS[i], estado[posicao_maior_engagement]);
					//std::cout<<"i: "<<i<<" j: "<<j<<std::endl;
					i++;
					j = 0;
				}
				
				
			} //if i < equipamentos tamanho
			if (i >= EQUIPAMENTOS_TAMANHO)
			{
				emoHandler->ofs.close();
			}
			
		}
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