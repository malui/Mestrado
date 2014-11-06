
#include "hstcppollcomm.h"
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

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
	controleFluxo = SET_CENARIOS_INICIAIS;
	ack = 0;
	cont_sinal = 0;
	treshold = 0.9;
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
				emoHandler->ofs << "controle: ack = " << ack << std::endl;
				//pega o ultimo valor de engagement e poe no vector
				const float eng_temp = emoHandler->emoAffectivEngagementBoredom();
				if (eng_temp != -1)
				{
					emoHandler->affectivEngagementBoredomVector.push_back(eng_temp);
					emoHandler->ofs <<"affectivEngagementBoredomVector: ";
					printVector(emoHandler->affectivEngagementBoredomVector);
					emoHandler->ofs << std::endl;
					cont_sinal ++;
					emoHandler->ofs <<"num sinais aproveitados: " << cont_sinal << std::endl;

				}
				//faz a media do vector e tem o engagement do cenario anterior
				emoHandler->ofs <<"Vetor entrada antes daMediaHarmonicaPonderada: ";
					printVector(emoHandler->affectivEngagementBoredomVector);
					emoHandler->ofs << std::endl;
				emoHandler->affectivEngagementBoredom = mediaHarmonicaPonderada(emoHandler->affectivEngagementBoredomVector);
				qDebug() << "emoHandler->affectivEngagementBoredom " << emoHandler->affectivEngagementBoredom;
				//limpa o vector de engagements para comecar um vector novo com o novo cenario
				emoHandler->affectivEngagementBoredomVector.clear();
				controle();
				ack ++; 
			}
			else if (ack > 0){
				emoHandler->ofs << "acknoledge: ack = " << ack<< std::endl;
				//tcpSocket.write(makeGetUnit(&unit,1).toLatin1()); //send ack
				setUnit(1,1); //manda ack
				const float eng_temp = emoHandler->emoAffectivEngagementBoredom();
				if (eng_temp != -1)
				{
					emoHandler->affectivEngagementBoredomVector.push_back(eng_temp);
					emoHandler->ofs <<"affectivEngagementBoredomVector: ";
					printVector(emoHandler->affectivEngagementBoredomVector);
					emoHandler->ofs << std::endl;
					cont_sinal ++;
					emoHandler->ofs <<"num sinais aproveitados: " << cont_sinal << std::endl;
				}
				ack++; 
				if (ack >= 5) {
					emoHandler->ofs << "acknoledge: ack = " << ack<< std::endl;
					const float eng_temp = emoHandler->emoAffectivEngagementBoredom();
					if (eng_temp != -1)
					{
						emoHandler->affectivEngagementBoredomVector.push_back(eng_temp);
						emoHandler->ofs <<"affectivEngagementBoredomVector: ";
						printVector(emoHandler->affectivEngagementBoredomVector);
						emoHandler->ofs << std::endl;
						cont_sinal ++;
						emoHandler->ofs <<"num sinais aproveitados: " << cont_sinal << std::endl;
					}
					
					ack = 0;
					emoHandler->ofs << "setting ack to:  = " << ack << std::endl;
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
		qDebug() << "setCenario erro: tamanho units = , " << units.size(); 
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

/*
printCenario (const vector<int>& v)
Imprimie vetor v em emoHandler->ofs
OBS: mudar para printEstados
*/
/*void HsTcpPollComm::printCenario (const vector<int>& v)
{

	for( std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
		emoHandler->ofs << *i << ' ';
}*/
void HsTcpPollComm::printCenario (const vector<int> &v)
{

	for( std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
		emoHandler->ofs << *i << ' ';
}

void HsTcpPollComm::printVector (const vector<float> &v)
{

	for( std::vector<float>::const_iterator i = v.begin(); i != v.end(); ++i)
		emoHandler->ofs << *i << ' ';
}

/*
void printPopulacao(const VectorCenarios populacao);

*/
void HsTcpPollComm::printPopulacao ( VectorCenarios populacao)
{
	for(int i=0; i<populacao.size(); i++)
	{
		printCenario(populacao[i].estados);
		emoHandler->ofs <<  populacao[i].engagement << endl;

	}
}

/*
crossoverEstados crossoverDeUmPonto(Estados estado1,Estados estado2)

Função crossover de um ponto aleatório.
Faz um crossover dividindo o vetor em duas partes. 
O tamanho do corte é definido aleatóriamente

retorna uma tupla com dois Estados
*/
HsTcpPollComm::crossoverEstados HsTcpPollComm::crossoverDeUmPonto(Estados estado1,Estados estado2) 
{
	//	inicializa semente aleatória 
	srand (time(NULL));				
	int tamanhoEstados = estado1.size();

	// cria aleatóriamente um pondo de corte onde cada vetor será separado
	int tamanhoCorteCrossover = rand() % tamanhoEstados;	

	std::vector<int> primeira_parte_estado1(estado1.begin(), estado1.begin() + tamanhoCorteCrossover);	
	std::vector<int> primeira_parte_estado2(estado1.begin() + tamanhoCorteCrossover, estado1.end());

	std::vector<int> ultima_parte_estado1(estado2.begin(), estado2.begin() + tamanhoCorteCrossover);
	std::vector<int> ultima_parte_estado2(estado2.begin() + tamanhoCorteCrossover, estado2.end());

	//em primeira_parte_cenario1 temos o primeiro filho, sua primeira parte é composta por parte do seu primeiro pai,e sua segunda parte é composta por parte do seu segundo pai
	primeira_parte_estado1.insert(primeira_parte_estado1.end(), ultima_parte_estado2.begin(), ultima_parte_estado2.end()); 


	//em primeira_parte_cenario2 temos o segundo filho, sua primeira parte composta por parte do seu segundo pai,e sua segunda parte é composta por parte do seu primeiro pai
	primeira_parte_estado2.insert(primeira_parte_estado2.end(), ultima_parte_estado1.begin(), ultima_parte_estado1.end());

	//cria uma tupla que é a recombinação 
	crossoverEstados crossoverTupla= make_tuple(primeira_parte_estado1,primeira_parte_estado2);
	return crossoverTupla;
}

/*
crossoverMascaraAleatoria(Estados estados1,Estados estados2)
Retorna um estado filho com taxa de mutacao de 5%

cria uma mascara de zeros e uns do tamanho de cenario.estados, 
se a posição da mascara tem valor um, o valor do primeiro pai é replicado para o filho na mesma posição,
caso tenha o valor 0, o valor do segundo pai é replicado para o filho
*/
HsTcpPollComm::Estados HsTcpPollComm::crossoverMascaraAleatoria(Estados estados1,Estados estados2) 
{
	srand (time(NULL));		//	inicializa semente aleatória 
	Estados estadosFilho;
	emoHandler->ofs <<"dentro de crossoverMascaraAleatoria()" << std::endl;
	qDebug() <<"dentro de crossoverMascaraAleatoria()";

	emoHandler->ofs <<"estados1: ";
	printCenario(estados1);
	emoHandler->ofs << std::endl;

	emoHandler->ofs <<"estados2: ";
	printCenario(estados2);
	emoHandler->ofs << std::endl;


	for(int i=0; i < estados1.size(); i++)
	{ 
		// cria aleatóriamente um número que será 0 ou 1, caso for 0, o valor do primeiro pai é replicado para o filho
		//, caso for 1 o valor do segundo pai é replicado para o filho  
		if ((rand() % 2) == 0)	 {
			emoHandler->ofs <<"dentro de crossoverMascaraAleatoria() 1" << std::endl;
			qDebug() <<"dentro de crossoverMascaraAleatoria() 1";
			estadosFilho.push_back(estados1[i]); }
		else 
		{
			emoHandler->ofs <<"dentro de crossoverMascaraAleatoria() 2" << std::endl;
			qDebug() <<"dentro de crossoverMascaraAleatoria() 2";
			estadosFilho.push_back(estados2[i]);//////////////// BUG !!!!!!!!!!!! Programa crasheia aqui
			emoHandler->ofs <<"dentro de crossoverMascaraAleatoria() 2.1" << std::endl;
			qDebug() <<"dentro de crossoverMascaraAleatoria() 2.1";
		}
	}
	// rand() % 20 retorna números entre 0 e 19, quando o valor retornado for 0, ou seja 5% das vezes, teremos uma taxa de mutação 5%
	emoHandler->ofs <<"dentro de crossoverMascaraAleatoria() 2.2" << std::endl;
	qDebug() <<"dentro de crossoverMascaraAleatoria() 2.2";

	if ((rand() % 20) == 0) {
		emoHandler->ofs <<"dentro de crossoverMascaraAleatoria() 3" << std::endl;
		qDebug() <<"dentro de crossoverMascaraAleatoria() 3";
		return mutacao(estadosFilho);}
	else {
		emoHandler->ofs <<"dentro de crossoverMascaraAleatoria() 4" << std::endl;
	    qDebug() <<"dentro de crossoverMascaraAleatoria() 4";
		return estadosFilho;}
}

/*
Estados HsTcpPollComm::mutacao(Estados estados)
retorna estado mutado

aleatoriamente escolhe bit que sofrerá mutação:
- caso tenha valor 1, passará a ser 0, 
- caso tenha valor 0, passará a ter valor 1.
*/
HsTcpPollComm::Estados HsTcpPollComm::mutacao(Estados estados) 
{
	 // aleatoriamente escolhe bit que sofrerá mutação, ou seja, caso tenha valor 1, passará a ser 0, caso tenha valor 0, passará a ter valor 1
	int bitQueSofreraMutacao = (rand() % estados.size());

	//  transforma 0 em 1, e 1 em 0 . //A função abs retorna o valor absoludo de um número
	estados[bitQueSofreraMutacao]=abs(estados[bitQueSofreraMutacao] - 1); 
	return estados;
}
/*
criaEstadosAleatorio(int tamanhoEstados)
retorna vector<int> estados do tamanho de tamanhoEstados
*/
std::vector<int> HsTcpPollComm::criaEstadosAleatorio(int tamanhoEstados)  // cria estados aleatorio
{
	std::vector<int> estados;
	for(int i=0; i<tamanhoEstados; i++)
		// rand() % 2 retorna aleatoriamente 0 ou 1, cenario é preenchido com 0 e 1
		estados.push_back(rand() % 2);	
	return estados;
}


//Inicializa com 0 em todas as posições o codificador de cenarios existentes com um numero de posições igual a 2^(tamanhoCenario)  
//void HsTcpPollComm::inicializaCodificadorCenariosExistentes(int tamanhoCenario){

//	std::vector<int> codCenariosExistentes( pow( double(2),double(tamanhoCenario) ), 0 );
	//for(int i=0; i<pow(double(2),double(tamanhoCenario)); i++)
	//{
	//	codCenariosExistentes.push_back(0);	
	//}
//}

//chamada quando se tem certeza que o cenario existe para ser inserido ao vectod codCenariosExistentes
void HsTcpPollComm::insereCenarioCodificador(std::vector<int> cenario){
	codCenariosExistentes[codificaCenario(cenario)]++; 
}

// Recebe um cenario, usa codificaCenario() para codificar, e verifica se cenario já existe
bool HsTcpPollComm::isCenarioRepetido(std::vector<int> cenario){
		emoHandler->ofs <<"isCenarioRepetido() 1" << std::endl;
	qDebug() <<"isCenarioRepetido() 1";


	if (codCenariosExistentes[codificaCenario(cenario)]>0)
	{
		emoHandler->ofs <<"isCenarioRepetido() 2" << std::endl;
		qDebug() <<"isCenarioRepetido() 2";
		return true; 
	}
	else
	{
		emoHandler->ofs <<"isCenarioRepetido() 3" << std::endl;
		qDebug() <<"isCenarioRepetido() 3";
		return false;
	}
}

//faz uma codificacao por posicao, caso a primeira posicao esteja peenchida, somamos 1, 
//caso a segunda somamos 2, caso a terceira somamos 4, e assim por diante  
// dessa forma temos uma codificação em potencia de 2 baseada nas posições que devolve 
//um código único para o estado recebido
int HsTcpPollComm::codificaCenario(std::vector<int> cenario){
	int codificacao = 0;

	for(int i=0; i<cenario.size(); i++)
	{
		if (cenario[i] == 1){
			codificacao = codificacao + pow(double(2),double(i));}
	}
	return codificacao;
}

//faz o inverso da função codificacao, a partir de um numero descodifica em zeros e uns
HsTcpPollComm::Estados HsTcpPollComm::decodificaCenario(int codificacao, int tamanhoCenario){
	Estados cenario; 
	emoHandler->ofs <<"dentro de decodificaCenario()" << std::endl;
	qDebug() <<"dentro de decodificaCenario()";

	cenario = toBinary(codificacao, cenario); // NÃO ESTA COM O TAMANHO CERTO
	// preencher com zeros o vertor cenario atéque ele atinja o tamanho tamanhoCenario(pegar de algum lugar ???)
	emoHandler->ofs <<"dentro de decodificaCenario() 1" << std::endl;
	qDebug() <<"dentro de decodificaCenario() 1";
	cenario.resize(tamanhoCenario,  0);
	emoHandler->ofs <<"dentro de decodificaCenario() 2" << std::endl;
	qDebug() <<"dentro de decodificaCenario() 2";
	return cenario;
}

//Transforma um valor inteiro para binario e returna na forma de um vetor
HsTcpPollComm::Estados HsTcpPollComm::toBinary(int number, Estados cenario) {
	int remainder;
	//Estados cenario;

	emoHandler->ofs <<"dentro de toBinary()" << std::endl;
	qDebug() <<"dentro de toBinary()";

	if(number <= 1) 
	{
		cenario.push_back(number);
		emoHandler->ofs <<"dentro de toBinary()1" << std::endl;
		qDebug() <<"dentro de toBinary()1";
		return cenario;
	}

	remainder = number%2;  // % - operação resto de divisão 
	toBinary(number >> 1, cenario);
	cenario.push_back(remainder);
	emoHandler->ofs <<"dentro de toBinary()2" << std::endl;
	qDebug() <<"dentro de toBinary()2";
	return cenario; ///// BUG! NAo estava retornando nada aqui, entao coloquei pra retornar cenario!!!!!!!!

}

/* resgata do vetor codCenariosExixtentes um cenario ainda não usado e o retorn

Quando todos os cenarios ja foram usados, retorna um vector com {-1}
*/
HsTcpPollComm::Estados HsTcpPollComm::getCenarioNaoUsado( int tamanhoEstadosCenario){
	int codificacao;

	emoHandler->ofs <<"dentro de getCenarioNaoUsado" << std::endl;
	qDebug() <<"dentro de getCenarioNaoUsado";

	for(int i=0; i<codCenariosExistentes.size(); i++)
	{
		if (codCenariosExistentes[i] == 0)
		{
			emoHandler->ofs <<"dentro de getCenarioNaoUsado 1" << std::endl;
			qDebug() <<"dentro de getCenarioNaoUsado 1";

			return decodificaCenario(i, tamanhoEstadosCenario);
		}
	}

	// Representa a utilização de todos os cenarios
	emoHandler->ofs <<"dentro de getCenarioNaoUsado 2" << std::endl;
	qDebug() <<"dentro de getCenarioNaoUsado 2";

	Estados todosCenariosUsados;
	todosCenariosUsados.push_back(-1);

	emoHandler->ofs <<"todosCenariosUsados:" << todosCenariosUsados[0] << std::endl;
	qDebug() <<"todosCenariosUsados:" << todosCenariosUsados[0];

	emoHandler->ofs <<"todosCenariosUsados.size():" << todosCenariosUsados.size() << std::endl;
	qDebug() <<"todosCenariosUsados.size():" << todosCenariosUsados.size();

	return todosCenariosUsados;
}

/*
void HsTcpPollComm::inicializaCenariosPrimeiraGeracao(int tamanhoCenario)
Inicializa a primeira geração de cenarios
com cenario.estados = aleatorio 0 ou 1
cenario.engagement = -1 pois ainda nao foram avaliados
*/
void HsTcpPollComm::inicializaCenariosPrimeiraGeracao(int tamanhoCenario)
{ 
	struct CENARIO cen;

	for(int i=0; i<TAMANHO_GERACAO_INICIAL; i++)
	{
		cen.estados = criaEstadosAleatorio(tamanhoCenario);//seta aleatoriamente os estados do cenario

		while(isCenarioRepetido(cen.estados))
		{	// Faz mutação até encontrar um estado não existente
			cen.estados = mutacao(cen.estados);

			if(isCenarioRepetido(cen.estados))  //sempre vai retornar um cenario que nao foi usado, pois eh a primiera geracao
				cen.estados = getCenarioNaoUsado(tamanhoCenario);
		}
		insereCenarioCodificador(cen.estados);

		cen.engagement = -1; //inicializa engagement com -1 pois ainda nao foi avaliado
		geracaoAtual.push_back(cen);
	}
}

/*
criaNovaGeracao(int qtdElementosReplicados)

cria nova geração com os 'qtdElementosReplicados' melhores elementos da geração inicial
Cria elementos Para Crossover e criar mais 3 filhos:
   - Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, 
   - e crossoverMascaraAleatoria com o primeiro e com o terceiro, 
*/
void HsTcpPollComm::criaNovaGeracao(int qtdElementosReplicados)
{ 

	struct CENARIO cen;
	cen.engagement = -1;
	emoHandler->ofs <<"dentro de criaNovaGeracao(): geracaoAtual.size()" << geracaoAtual.size()<< std::endl;
	qDebug() <<"dentro de criaNovaGeracao(): geracaoAtual.size()" << geracaoAtual.size();
	//geracaoPassada.swap(geracaoAtual);			
	geracaoPassada = geracaoAtual;    
	geracaoAtual.clear();						// geração atual é limpa
	emoHandler->ofs <<"dentro de criaNovaGeracao(): geracaoPassada.size()" << geracaoPassada.size()<< std::endl;
	qDebug() <<"dentro de criaNovaGeracao(): geracaoPassada.size()" << geracaoPassada.size();

	emoHandler->ofs <<"geracaPassada: ";
	printPopulacao(geracaoPassada);
	emoHandler->ofs << std::endl;

	emoHandler->ofs <<"dentro de criaNovaGeracao()" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao()";

	// inicializa geracaoAtual com 'qtdElementosReplicados' número de elementos da geração inicial 
	//(que serão replicados para a próxima geração)
	for(int i=0; i<qtdElementosReplicados; i++)
	{
		emoHandler->ofs <<"dentro de criaNovaGeracao()0.01" << std::endl;
		qDebug() <<"dentro de criaNovaGeracao()0.01";

		emoHandler->ofs <<"geracaoPassada[" << i << "].estados = ";
		printCenario(geracaoPassada[i].estados);
		emoHandler->ofs << std::endl;
		geracaoAtual.push_back(geracaoPassada[i]); // melhores elementos da geração passada passam a ser elementos da geração atual
	}
	contadorNumeroDeGeracoes++; // contador do número de gerações é incrementado
	emoHandler->ofs <<"dentro de criaNovaGeracao()0.02" << std::endl;
		qDebug() <<"dentro de criaNovaGeracao()0.02";

	//variáveis utilizadas na manipulação de elementos do crossover
	//TuplaCenario tuplaCenario; 
	crossoverEstados tuplaCrossoverEstados;
	//Estados estadosFilho1;
	
	//Cria elementos Para Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
	
	// cria estadosFilho1 com crossoverMascaraAleatoria
	emoHandler->ofs <<"geracaoAtual.size()" << geracaoAtual.size()<< std::endl;
	qDebug() <<"geracaoAtual.size()" << geracaoAtual.size();

	cen.estados = crossoverMascaraAleatoria(geracaoAtual[0].estados ,geracaoAtual[2].estados); /// BUG! certificar que existe geracaoAtual[x].estados Pq nao existe geracaoAtual[2].estados ???
	emoHandler->ofs <<"dentro de criaNovaGeracao()0.3" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao()0.3";

	while(isCenarioRepetido(cen.estados))
	{	// Faz mutação até encontrar um estado não existente
			cen.estados = mutacao(cen.estados);
			if(isCenarioRepetido(cen.estados))
			{
				std::vector <int> cenarioNaoUsado = getCenarioNaoUsado(cen.estados.size());
				if ( cenarioNaoUsado[0] == -1) //todos os cenarios foram usados
				{
					//Usa cenario de maior engagement
					//novo treshold para maior engagement encontrado
					treshold = (geracaoAtual.front()).engagement;
					condicaoParadaEngagement(treshold);
				}
				else
				{
					cen.estados = cenarioNaoUsado;
				}
			}
	}
	insereCenarioCodificador(cen.estados);

	emoHandler->ofs <<"dentro de criaNovaGeracao() 1" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 1";

	//cen.estados = estadosFilho1;
	geracaoAtual.push_back(cen); //insere novo cenario (estadosFilho1 , -1) na geracaoAtual
	contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados


	//faz crossover com crossoverDeUmPonto e retorna dois filhos em tuplaCrossoverEstados
	tuplaCrossoverEstados = crossoverDeUmPonto(geracaoAtual[0].estados ,geracaoAtual[1].estados); 

	cen.estados = std::get<0>(tuplaCrossoverEstados);

	emoHandler->ofs <<"dentro de criaNovaGeracao() 2" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 2";
	
	while(isCenarioRepetido(cen.estados))
	{	// Faz mutação até encontrar um estado não existente
			cen.estados = mutacao(cen.estados);
			emoHandler->ofs <<"dentro de criaNovaGeracao() 2.1" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 2.1";
			if(isCenarioRepetido(cen.estados))
			{
				emoHandler->ofs <<"dentro de criaNovaGeracao() 2.1.1" << std::endl;
				qDebug() <<"dentro de criaNovaGeracao() 2.1.1";

				std::vector <int> cenarioNaoUsado = getCenarioNaoUsado(cen.estados.size());
				if ( cenarioNaoUsado[0] == -1) //todos os cenarios foram usados
				{
					emoHandler->ofs <<"cenarioNaoUsado[0] == -1" << std::endl;
					qDebug() <<"cenarioNaoUsado[0] == -1";
					//Usa cenario de maior engagement
					//novo treshold para maior engagement encontrado
					treshold = (geracaoAtual.front()).engagement;
					condicaoParadaEngagement(treshold);
				}
				else
				{
					cen.estados = cenarioNaoUsado;
					emoHandler->ofs <<"dentro de criaNovaGeracao() 2.1.1.1" << std::endl;
					qDebug() <<"dentro de criaNovaGeracao() 2.1.1.1";
				}
			}
	}
	emoHandler->ofs <<"dentro de criaNovaGeracao() 2.2" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 2.2";
	insereCenarioCodificador(cen.estados);
	emoHandler->ofs <<"dentro de criaNovaGeracao() 2.3" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 2.3";
	
	geracaoAtual.push_back(cen);  // Insere crossover na geração atual
	contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados

	emoHandler->ofs <<"dentro de criaNovaGeracao() 3" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 3";

	cen.estados = std::get<1>(tuplaCrossoverEstados);
////////////////////////////// BUG //////////////////////////////////////	
	while(isCenarioRepetido(cen.estados))
	{	// Faz mutação até encontrar um estado não existente
			cen.estados = mutacao(cen.estados);
			if(isCenarioRepetido(cen.estados))
			{
				std::vector <int> cenarioNaoUsado = getCenarioNaoUsado(cen.estados.size());

				emoHandler->ofs <<"cenarioNaoUsado: ";
				printCenario(cenarioNaoUsado);
				emoHandler->ofs << std::endl;
				qDebug() <<"cenarioNaoUsado ver arq log";

				if ( cenarioNaoUsado[0] == -1) //todos os cenarios foram usados
				{
					emoHandler->ofs <<"cenarioNaoUsado[0] == -1" << std::endl;
					qDebug() <<"cenarioNaoUsado[0] == -1";

					//Usa cenario de maior engagement
					//novo treshold para maior engagement encontrado
					treshold = (geracaoAtual.front()).engagement;
					condicaoParadaEngagement(treshold);
				}
				else
				{
					emoHandler->ofs <<"cenarioNaoUsado[0] != -1" << std::endl;
					qDebug() <<"cenarioNaoUsado[0] != -1";
					cen.estados = cenarioNaoUsado;
				}
			}
	}
/////////////////////////////////////////////////////////////////////////////////
	insereCenarioCodificador(cen.estados);

	emoHandler->ofs <<"dentro de criaNovaGeracao() 4" << std::endl;
	qDebug() <<"dentro de criaNovaGeracao() 4";
	
	geracaoAtual.push_back(cen);  // Insere crossover na geração atual
	contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados

}

/*
void HsTcpPollComm::verificaCondicoesDeParada()
Criterios de parada:
engagement >= ENGAGEMENT_ALVO 0.95
OU
Numero de Geracoes >= LIMITE_GERACOES 4
*/
void HsTcpPollComm::verificaCondicoesDeParada()
{
	if (((geracaoAtual.front()).engagement) >= treshold) //ENGAGEMENT_ALVO) 
	{
		// geracaoAtual.front() retorna o prieiro elemento do vector geração atual, ou seja, verifica se o mais apto satisfaz a condição de parada
		emoHandler->ofs <<"Criterio de parada atingido: Engagement Level >= "<< treshold << std::endl;  //ENGAGEMENT_ALVO
		emoHandler->ofs <<"Engagement Level Maximo = " << (geracaoAtual.front()).engagement  << std::endl;
		exit(0);
	}
	// O  mais apto não satisfaz a condição de parada
	if(contadorNumeroDeGeracoes>=LIMITE_GERACOES){ // TRANSFORMAR EM CONSTANTE
	// Programa para pois alcançou o numero de gerações Limite
		emoHandler->ofs <<"Engagement Level alvo não atingindo atingido após " << LIMITE_GERACOES << " gerações: abortar" << std::endl;
		emoHandler->ofs <<"Engagement Level Maximo = " << (geracaoAtual.front()).engagement  << std::endl;
		exit(0);
	}
}

void HsTcpPollComm::condicaoParadaEngagement(float engagement){
	if (engagement >= treshold) //ENGAGEMENT_ALVO) 
	{
		// geracaoAtual.front() retorna o prieiro elemento do vector geração atual, ou seja, verifica se o mais apto satisfaz a condição de parada
		emoHandler->ofs <<"Engagement Level alvo atingido, Maior engagement:" << engagement  << std::endl;
		// setar o cenario com maior engagement?
		exit(0);
	}
}

/*
o cenario de maior engagement vai estar na primeira posicao do vecto geracaoAtual[0]
pra pegar o cenario faz std::get<0>(geracaoAtual[0])
pra pegar o engagement faz std::get<1>(geracaoAtual[0])

*/
void HsTcpPollComm::controle()      // a variavel controle fluxo deve ser setada antes da chamada da função controle
{							
	// todas variaveis utilizadas em mais de um dos CASES devem ser definidas antes do switch
	     
	int unitsInicializador[] = {118,119,120};//{1187, 1188, 1191, 2058, 2061, 2062, 1415};
	std::vector<int> units(unitsInicializador,&unitsInicializador[sizeof(unitsInicializador)/sizeof(unitsInicializador[0])]);
//	crossoverCenarios tuplaCrossoverCenarios;
//	Cenario cenarioFilho1;

	emoHandler->ofs <<"controleFluxo = "<< controleFluxo << std::endl;
	qDebug() <<"controleFluxo = "<< controleFluxo;

	switch(controleFluxo)           
	{								
	case SET_CENARIOS_INICIAIS:
		//	Cria n cenarios iniciais com valores aleatórios
		//inicializaCodificadorCenariosExistentes(units.size());
		codCenariosExistentes.resize( pow( double(2),double(units.size()) ), 0 );

		inicializaCenariosPrimeiraGeracao(units.size());

		printPopulacao(geracaoAtual);

		//contadorPrimeiraGeracao Armazena tamanho da população inicial para que se possa fazer o controle de uso posteriormente
		contadorPrimeiraGeracao = geracaoAtual.size() - 1; //pois vector inicia em 0
		contadorCrossoversNaoAvaliados=0;
		contadorNumeroDeGeracoes=1;

		setCenario(units, geracaoAtual[contadorPrimeiraGeracao].estados);			// seta um dos cenarios iniciais
		emoHandler->ofs <<"Cenario: ";
		printCenario(geracaoAtual[contadorPrimeiraGeracao].estados);
		emoHandler->ofs << std::endl;

		controleFluxo = AVALIA_CENARIOS_INICIAIS;
		break;
	case AVALIA_CENARIOS_INICIAIS:
		// retorna ultima tupla que foi usada na setCenario para avalia-la
		//tuplaCenario = geracaoAtual[contadorPrimeiraGeracao];
		geracaoAtual[contadorPrimeiraGeracao].engagement =  emoHandler->affectivEngagementBoredom;	
		std::cout <<"Engagement_level:  " << geracaoAtual[contadorPrimeiraGeracao].engagement << std::endl;
		emoHandler->ofs <<"Engagement_level:  " << geracaoAtual[contadorPrimeiraGeracao].engagement << std::endl;

		condicaoParadaEngagement(geracaoAtual[contadorPrimeiraGeracao].engagement);
		contadorPrimeiraGeracao--;		// decrementamos contadorPrimeiraGeracao para na proxima iteração pegarmos o próximo cenario inicial que ainda não foi avaliado
		
		if(contadorPrimeiraGeracao>0){
			//Ainda há cenarios iniciais a serem avaliados
			//Setamos o próximo cenario a ser avaliado para não perder tempo esperando a proxima chamada da controle()
			//tuplaCenario = geracaoAtual[contadorPrimeiraGeracao];	// retorna uma tupla que concem um dos cenarios inicias 
			setCenario(units, geracaoAtual[contadorPrimeiraGeracao].estados);			// seta um dos cenarios iniciais
			emoHandler->ofs <<"Cenario: ";
			printCenario(geracaoAtual[contadorPrimeiraGeracao].estados);
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS_INICIAIS;
		}
		else{
			//Não há mais cenarios iniciais a serem avaliados
			sort(geracaoAtual.begin(),geracaoAtual.end(),[](const CENARIO& a,const CENARIO& b) -> bool{return a.engagement > b.engagement;});
			//sort(geracaoAtual.begin(),geracaoAtual.end());

			emoHandler->ofs <<"Todos Cenarios Iniciais Avaliados"<<std::endl;
			emoHandler->ofs <<"Cenarios Elite:";
			printCenario((geracaoAtual.front()).estados);
			emoHandler->ofs <<  std::endl;
			emoHandler->ofs <<" Elite possui Engagement_level:  " << (geracaoAtual.front()).engagement << std::endl;

			//BUG???? Nao tem q verificar verificaCondicoesDeParada(); ??

			//Para otimizar o tempo de execução, e não experar até a próxima chamada da função controle() já executamos o código abaixo para setar um cenario
			//Alem de criar uma nova geração, cria 3 elementos utilizando os dois tipos de crossover,e mutação
			 // o ELEMENTOS_REPLICADOS_PROXIMA_GERACAO igual a 4 representa o número de elementos que será replicado da geração atual, para a proxima geração, TRANSFORMAR EM CONSTANTE!!
			std::cout <<"Criando geração de numero:  " << contadorNumeroDeGeracoes << std::endl;
			emoHandler->ofs<<"Criando geração de numero:  " << contadorNumeroDeGeracoes << std::endl;
			criaNovaGeracao(ELEMENTOS_REPLICADOS_PROXIMA_GERACAO);
			
			// retorna uma tupla que contem um cenario crossover não avaliado
			//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	
			setCenario(units, geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].estados);			// seta um dos cenarios do crossover
			emoHandler->ofs <<"Cenario: ";
			printCenario(geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].estados);
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
		}
		break;

	case AVALIA_CENARIOS:
		// retorna ultima tupla que foi usada na setCenario para avalia-la
		//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];
		geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].engagement =  emoHandler->affectivEngagementBoredom;
		std::cout <<"Engagement_level:  " << geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].engagement << std::endl;
		emoHandler->ofs <<"Engagement_level:  " << geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].engagement << std::endl;
		condicaoParadaEngagement(geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].engagement);

		// decrementamos contadorCrossoversNaoAvaliados para na proxima iteração pegarmos o próximo cenario do crossover que ainda não foi avaliado
		contadorCrossoversNaoAvaliados--;	

		if (contadorCrossoversNaoAvaliados == 0)
		{
			//Toda Geração avaliada
			// Deixa a geracaoAtual por ordem de maior engagement:
			sort(geracaoAtual.begin(),geracaoAtual.end(),[](const CENARIO& a,const CENARIO& b) -> bool{return a.engagement > b.engagement;});

			emoHandler->ofs <<"Todos Cenarios do Crossover Avaliados"<<std::endl;
			emoHandler->ofs <<"Cenarios Elite:";
			printCenario(geracaoAtual.front().estados);
			emoHandler->ofs <<  std::endl;
			emoHandler->ofs <<" Elite possui Engagement_level:  " << geracaoAtual.front().engagement << std::endl;

			verificaCondicoesDeParada();
			 
			// cria nova geração
			//Cria elementos Para Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
			// Insere crossover na geração atual
			// o ELEMENTOS_REPLICADOS_PROXIMA_GERACAO igual a 4 representa o número de elementos que será replicado da geração atual, para a proxima geração
			std::cout <<"Criando geração de numero:  " << contadorNumeroDeGeracoes << std::endl;
			emoHandler->ofs<<"Criando geração de numero:  " << contadorNumeroDeGeracoes << std::endl;
			criaNovaGeracao(ELEMENTOS_REPLICADOS_PROXIMA_GERACAO); 
				
			// retorna uma tupla que contem um cenario crossover não avaliado
			//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	
			setCenario(units, geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].estados);			// seta um dos cenarios do crossover
			emoHandler->ofs <<"Cenario: ";
			printCenario(geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].estados);
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
			
		}
		else{
			//Ainda temos elementos da geração atual a avaliar
			//Setamos o próximo cenario a ser avaliado para não perder tempo esperando a proxima chamada da controle()
			//tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	// retorna uma tupla que concem um dos cenarios do crossover ainda não avaliado
			setCenario(units, geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].estados);			// seta um dos cenarios do crossover
			emoHandler->ofs <<"Cenario: ";
			printCenario(geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados].estados);
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
		}
		break;

	default: std::cout << "controleFluxo desconhecido: "<< controleFluxo << std::endl;

	}


}
/*
Faz a media harmonica ponderada de um vetor
com pesos = indice 

M = ( soma(pesos) / (soma (pesoi/xi) ) )
*/
float HsTcpPollComm::mediaHarmonicaPonderada(vector<float> v)
{
	float sumi = 0.0f, sumx = 0.0f;

	emoHandler->ofs <<"Vetor entrada MediaHarmonicaPonderada: ";
	printVector(v);
	emoHandler->ofs << std::endl;
	for( unsigned int i = 0; i < v.size(); ++i)
	{
		sumi += i;
		if (v[i] != 0.0f)
			sumx += (i/(v[i]));
	}

	if (sumx != 0)
		emoHandler->ofs <<"MediaHarmonicaPonderada: " << sumi/sumx << std::endl;
	else
		emoHandler->ofs <<"MediaHarmonicaPonderada divisao por zero, retornando valor zero"<< std::endl;
	return sumx == 0.0f ? 0.0f : sumi/sumx;
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
