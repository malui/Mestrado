
#include "hstcppollcomm.h"
#include <iostream>
#include <stdlib.h>     /* srand, rand */
//#include <time.h>       /* time */

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
	algGen = NULL;
	sessionState = DISCONNECTED;
	resposta_pronta = false;
	controleFluxo = SET_CENARIOS_INICIAIS;
	ack = 0;
	cont_sinal = 0;
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
		qDebug() << "SENT_REQUEST";
		{
			if (ack == 0){
				algGen->emoHandler->ofs << "controle: ack = " << ack << std::endl;
				//pega o ultimo valor de engagement e poe no vector
				const float eng_temp = algGen->emoHandler->emoAffectivEngagementBoredom();
				if (eng_temp != -1)
				{
					algGen->emoHandler->affectivEngagementBoredomVector.push_back(eng_temp);
					algGen->emoHandler->ofs <<"affectivEngagementBoredomVector: ";
					algGen->printVector(algGen->emoHandler->affectivEngagementBoredomVector);
					algGen->emoHandler->ofs << std::endl;
					cont_sinal ++;
					algGen->emoHandler->ofs <<"num sinais aproveitados: " << cont_sinal << std::endl;

				}
				//faz a media do vector e tem o engagement do cenario anterior
				algGen->emoHandler->ofs <<"Vetor entrada antes daMediaHarmonicaPonderada: ";
					algGen->printVector(algGen->emoHandler->affectivEngagementBoredomVector);
					algGen->emoHandler->ofs << std::endl;
				algGen->emoHandler->affectivEngagementBoredom = algGen->mediaHarmonicaPonderada(algGen->emoHandler->affectivEngagementBoredomVector);
				qDebug() << "emoHandler->affectivEngagementBoredom " << algGen->emoHandler->affectivEngagementBoredom;
				//limpa o vector de engagements para comecar um vector novo com o novo cenario
				algGen->emoHandler->affectivEngagementBoredomVector.clear();
				controle();
				ack ++; 
			}
			else if (ack > 0){
				algGen->emoHandler->ofs << "acknoledge: ack = " << ack<< std::endl;
				//tcpSocket.write(makeGetUnit(&unit,1).toLatin1()); //send ack
				setUnit(1,1); //manda ack
				const float eng_temp = algGen->emoHandler->emoAffectivEngagementBoredom();
				if (eng_temp != -1)
				{
					algGen->emoHandler->affectivEngagementBoredomVector.push_back(eng_temp);
					algGen->emoHandler->ofs <<"affectivEngagementBoredomVector: ";
					algGen->printVector(algGen->emoHandler->affectivEngagementBoredomVector);
					algGen->emoHandler->ofs << std::endl;
					cont_sinal ++;
					algGen->emoHandler->ofs <<"num sinais aproveitados: " << cont_sinal << std::endl;
				}
				ack++; 
				if (ack >= 5) {
					algGen->emoHandler->ofs << "acknoledge: ack = " << ack<< std::endl;
					const float eng_temp = algGen->emoHandler->emoAffectivEngagementBoredom();
					if (eng_temp != -1)
					{
						algGen->emoHandler->affectivEngagementBoredomVector.push_back(eng_temp);
						algGen->emoHandler->ofs <<"affectivEngagementBoredomVector: ";
						algGen->printVector(algGen->emoHandler->affectivEngagementBoredomVector);
						algGen->emoHandler->ofs << std::endl;
						cont_sinal ++;
						algGen->emoHandler->ofs <<"num sinais aproveitados: " << cont_sinal << std::endl;
					}
					
					ack = 0;
					algGen->emoHandler->ofs << "setting ack to:  = " << ack << std::endl;
					cont_sinal = 0;
				}
			}
		sessionState = WAITING_RESPONSE;
		qDebug() << "Send get unit";
		}
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
	//strSet.append(QString("5S%1*%2*").arg(units[0]).arg(values[0]));
	qDebug() << "setCenario inicio";
	if ( units.size() != values.size() )
	{
		//erro!
		qDebug() << "setCenario erro: tamanho units = " << units.size(); 
		qDebug() << "tamanho values = " << values.size();
		return 0;
	}
	else
	{
		for (int i=0; i<units.size(); i++)
		{
			strSet.append(QString("5S%1*%2*").arg(units[i]).arg(values[i]));
		}

		tcpSocket.write(strSet.toLatin1());
		qDebug() << "setCenario: "<< strSet;

		algGen->emoHandler->ofs << "SetCenario:Eviado para HS cenario:";
		algGen->printCenario(values);
		algGen->emoHandler->ofs << std::endl;

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

/* ---------------------------------------------------------------------------------*/
/*                                    CONTROLE                                      */
/* ---------------------------------------------------------------------------------*/


/*
o cenario de maior engagement vai estar na primeira posicao do vecto geracaoAtual[0]


*/
void HsTcpPollComm::controle()      // a variavel controle fluxo deve ser setada antes da chamada da função controle
{							
	// todas variaveis utilizadas em mais de um dos CASES devem ser definidas antes do switch
	     
	int unitsInicializador[] = {118,119,120};//{1187, 1188, 1191, 2058, 2061, 2062, 1415};
	std::vector<int> units(unitsInicializador,&unitsInicializador[sizeof(unitsInicializador)/sizeof(unitsInicializador[0])]);
//	crossoverCenarios tuplaCrossoverCenarios;
//	Cenario cenarioFilho1;

	//AlgGen* HsTcpPollComm::
	algGen->emoHandler->ofs <<"controleFluxo = "<< controleFluxo << std::endl;
	qDebug() <<"controleFluxo = "<< controleFluxo;

	switch(controleFluxo)           
	{								
	case SET_CENARIOS_INICIAIS:
		algGen->treshold = 0.9;
		//	Cria n cenarios iniciais com valores aleatórios
		//inicializaCodificadorCenariosExistentes(units.size());
		algGen->codCenariosExistentes.resize( pow( double(2),double(units.size()) ), 0 );

		algGen->inicializaCenariosPrimeiraGeracao(units.size());

		algGen->printPopulacao(algGen->geracaoAtual);

		//contadorPrimeiraGeracao Armazena tamanho da população inicial para que se possa fazer o controle de uso posteriormente
		algGen->contadorPrimeiraGeracao = algGen->geracaoAtual.size() - 1; //pois vector inicia em 0
		algGen->contadorCrossoversNaoAvaliados=0;
		algGen->contadorNumeroDeGeracoes=1;

		setCenario(units, algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].estados);			// seta um dos cenarios iniciais
		algGen->emoHandler->ofs <<"Cenario: ";
		algGen->printCenario(algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].estados);
		algGen->emoHandler->ofs << std::endl;

		controleFluxo = AVALIA_CENARIOS_INICIAIS;
		break;
	case AVALIA_CENARIOS_INICIAIS:
		// retorna ultima tupla que foi usada na setCenario para avalia-la
		//tuplaCenario = geracaoAtual[contadorPrimeiraGeracao];
		algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].engagement =  algGen->emoHandler->affectivEngagementBoredom;	
		std::cout <<"Engagement_level:  " << algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].engagement << std::endl;
		algGen->emoHandler->ofs <<"Engagement_level:  " << algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].engagement << std::endl;

		algGen->condicaoParadaEngagement(algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].engagement);
		algGen->contadorPrimeiraGeracao--;		// decrementamos contadorPrimeiraGeracao para na proxima iteração pegarmos o próximo cenario inicial que ainda não foi avaliado
		
		if(algGen->contadorPrimeiraGeracao>=0){
			//Ainda há cenarios iniciais a serem avaliados
			//Setamos o próximo cenario a ser avaliado para não perder tempo esperando a proxima chamada da controle()
			//tuplaCenario = geracaoAtual[contadorPrimeiraGeracao];	// retorna uma tupla que concem um dos cenarios inicias 
			setCenario(units, algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].estados);			// seta um dos cenarios iniciais
			algGen->emoHandler->ofs <<"Cenario: ";
			algGen->printCenario(algGen->geracaoAtual[algGen->contadorPrimeiraGeracao].estados);
			algGen->emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS_INICIAIS;
		}
		else{
			//Não há mais cenarios iniciais a serem avaliados
			sort(algGen->geracaoAtual.begin(),algGen->geracaoAtual.end(),[](const AlgGen::CENARIO& a,const AlgGen::CENARIO& b) -> bool{return a.engagement > b.engagement;});
			//sort(geracaoAtual.begin(),geracaoAtual.end());

			algGen->emoHandler->ofs <<"Todos Cenarios Iniciais Avaliados"<<std::endl;
			algGen->emoHandler->ofs <<"Cenarios Elite:";
			algGen->printCenario((algGen->geracaoAtual.front()).estados);
			algGen->emoHandler->ofs <<  std::endl;
			algGen->emoHandler->ofs <<" Elite possui Engagement_level:  " << (algGen->geracaoAtual.front()).engagement << std::endl;

			//BUG???? Nao tem q verificar verificaCondicoesDeParada(); ??

			//Para otimizar o tempo de execução, e não experar até a próxima chamada da função controle() já executamos o código abaixo para setar um cenario
			//Alem de criar uma nova geração, cria 3 elementos utilizando os dois tipos de crossover,e mutação
			 // o ELEMENTOS_REPLICADOS_PROXIMA_GERACAO igual a 4 representa o número de elementos que será replicado da geração atual, para a proxima geração, TRANSFORMAR EM CONSTANTE!!
			std::cout <<"Criando geração de numero:  " << algGen->contadorNumeroDeGeracoes << std::endl;
			algGen->emoHandler->ofs<<"Criando geração de numero:  " << algGen->contadorNumeroDeGeracoes << std::endl;
			algGen->criaNovaGeracao(ELEMENTOS_REPLICADOS_PROXIMA_GERACAO);
			
			// retorna uma tupla que contem um cenario crossover não avaliado
			//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	
			setCenario(units, algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].estados);			// seta um dos cenarios do crossover
			algGen->emoHandler->ofs <<"Cenario: ";
			algGen->printCenario(algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].estados);
			algGen->emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
		}
		break;

	case AVALIA_CENARIOS:
		// retorna ultima tupla que foi usada na setCenario para avalia-la
		//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];
		algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].engagement =  algGen->emoHandler->affectivEngagementBoredom;
		std::cout <<"Engagement_level:  " << algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].engagement << std::endl;
		algGen->emoHandler->ofs <<"Engagement_level:  " << algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].engagement << std::endl;
		algGen->condicaoParadaEngagement(algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].engagement);

		// decrementamos contadorCrossoversNaoAvaliados para na proxima iteração pegarmos o próximo cenario do crossover que ainda não foi avaliado
		algGen->contadorCrossoversNaoAvaliados--;	

		if (algGen->contadorCrossoversNaoAvaliados == 0)
		{
			//Toda Geração avaliada
			// Deixa a geracaoAtual por ordem de maior engagement:
			sort(algGen->geracaoAtual.begin(),algGen->geracaoAtual.end(),[](const AlgGen::CENARIO& a,const AlgGen::CENARIO& b) -> bool{return a.engagement > b.engagement;});

			algGen->emoHandler->ofs <<"Todos Cenarios do Crossover Avaliados"<<std::endl;
			algGen->emoHandler->ofs <<"Cenarios Elite:";
			algGen->printCenario(algGen->geracaoAtual.front().estados);
			algGen->emoHandler->ofs <<  std::endl;
			algGen->emoHandler->ofs <<" Elite possui Engagement_level:  " << algGen->geracaoAtual.front().engagement << std::endl;

			algGen->verificaCondicoesDeParada();
			 
			// cria nova geração
			//Cria elementos Para Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
			// Insere crossover na geração atual
			// o ELEMENTOS_REPLICADOS_PROXIMA_GERACAO igual a 4 representa o número de elementos que será replicado da geração atual, para a proxima geração
			std::cout <<"Criando geração de numero:  " << algGen->contadorNumeroDeGeracoes << std::endl;
			algGen->emoHandler->ofs<<"Criando geração de numero:  " << algGen->contadorNumeroDeGeracoes << std::endl;
			algGen->criaNovaGeracao(ELEMENTOS_REPLICADOS_PROXIMA_GERACAO); 
				
			// retorna uma tupla que contem um cenario crossover não avaliado
			//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	
			setCenario(units, algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].estados);			// seta um dos cenarios do crossover
			algGen->emoHandler->ofs <<"Cenario: ";
			algGen->printCenario(algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].estados);
			algGen->emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
			
		}
		else{
			//Ainda temos elementos da geração atual a avaliar
			//Setamos o próximo cenario a ser avaliado para não perder tempo esperando a proxima chamada da controle()
			//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	// retorna uma tupla que concem um dos cenarios do crossover ainda não avaliado
			setCenario(units,algGen-> geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].estados);			// seta um dos cenarios do crossover
			algGen->emoHandler->ofs <<"Cenario: ";
			algGen->printCenario(algGen->geracaoAtual[algGen->geracaoAtual.size()-algGen->contadorCrossoversNaoAvaliados].estados);
			algGen->emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
		}
		break;

	default: std::cout << "controleFluxo desconhecido: "<< controleFluxo << std::endl;

	}


}

/* ---------------------------------------------------------------------------------*/
/*                                 FIM CONTROLE                                     */
/* ---------------------------------------------------------------------------------*/