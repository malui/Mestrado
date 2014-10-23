
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
				qDebug() << "controle: ack = " << ack;
				controle();
				ack ++; 
			}
			else if (ack > 0){
				qDebug() << "acknoledge: ack = " << ack;
				tcpSocket.write(makeGetUnit(&unit,1).toLatin1()); //send ack
				ack++; 
				if (ack >= 4) {
					qDebug() << "acknoledge: ack = " << ack;
					ack = 0;
					qDebug() << "setting ack to:  = " << ack;
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


/////////GUILHERME BEGIN//////////////////

void HsTcpPollComm::printCenario (const vector<int>& v)
{

	for( std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
		emoHandler->ofs << *i << ' ';
}

void HsTcpPollComm::printPopulacao ( Populacao population)
{
	for(int i=0; i<population.size(); i++)
	{
		printCenario(std::get<0>(population[i]));
		emoHandler->ofs <<  std::get<1>(population[i]) << endl;

	}
}

HsTcpPollComm::crossoverCenarios HsTcpPollComm::crossoverDeUmPonto(Cenario cenario1,Cenario cenario2) 
{
	// Função crossover de um ponto aleatório, faz um crossover dividindo o vetor em duas partes. O tamanho do corte é definido aleatóriamente

	//	inicializa semente aleatória 
	srand (time(NULL));				
	int tamanhoCenario = cenario1.size();

	// cria aleatóriamente um pondo de corte onde cada vetor será separado
	int tamanhoCorteCrossover = rand() % tamanhoCenario;	

	std::vector<int> primeira_parte_cenario1(cenario1.begin(), cenario1.begin() + tamanhoCorteCrossover);	
	std::vector<int> primeira_parte_cenario2(cenario1.begin() + tamanhoCorteCrossover, cenario1.end());

	std::vector<int> ultima_parte_cenario1(cenario2.begin(), cenario2.begin() + tamanhoCorteCrossover);
	std::vector<int> ultima_parte_cenario2(cenario2.begin() + tamanhoCorteCrossover, cenario2.end());

	//em primeira_parte_cenario1 temos o primeiro filho, sua primeira parteé composta por parte do seu primeiro pai,e sua segunda parte é composta por parte do seu segundo pai
	primeira_parte_cenario1.insert(primeira_parte_cenario1.end(), ultima_parte_cenario2.begin(), ultima_parte_cenario2.end()); 


	//em primeira_parte_cenario2 temos o segundo filho, sua primeira parte composta por parte do seu segundo pai,e sua segunda parte é composta por parte do seu primeiro pai
	primeira_parte_cenario2.insert(primeira_parte_cenario2.end(), ultima_parte_cenario1.begin(), ultima_parte_cenario1.end());

	//cria uma tupla que é a recombinação 
	crossoverCenarios crossoverTupla= make_tuple(primeira_parte_cenario1,primeira_parte_cenario2);
	return crossoverTupla;
}

HsTcpPollComm::Cenario HsTcpPollComm::crossoverMascaraAleatoria(Cenario cenario1,Cenario cenario2) 
{
	// Função crossover Mascara aleatória, cria uma mascara de zeros e uns do tamanho do cenario, se a posição da mascara tem valor um, o valor do primeiro pai é replicado para o filho na mesma posição
	//caso tenha o valor 0, o valor do segundo pai é replicado para o filho
	srand (time(NULL));		//	inicializa semente aleatória 
	Cenario cenarioFilho;
	for(int i=0; i<cenario1.size(); i++)
	{
		// cria aleatóriamente um número que será 0 ou 1, caso for 0, o valor do primeiro pai é replicado para o filho
		//, caso for 1 o valor do segundo pai é replicado para o filho  
		if ((rand() % 2) == 0)	 
			cenarioFilho.push_back(cenario1[i]);
		else 
			cenarioFilho.push_back(cenario2[i]);
	}
	// rand() % 20 retorna números entre 0 e 19, quando o valor retornado for 0, ou seja 5% das vezes, teremos uma taxa de mutação de mutação 5%
	if ((rand() % 20) == 0) 
		return mutacao(cenarioFilho);
	else
		return cenarioFilho;
}

HsTcpPollComm::Cenario HsTcpPollComm::mutacao(Cenario cenario) 
{
	 // aleatoriamente escolhe bit que sofrerá mutação, ou seja, caso tenha valor 1, passará a ser 0, caso tenha valor 0, passará a ter valor 1
	int bitQueSofreraMutacao = (rand() % cenario.size());

	//  transforma 0 em 1, e 1 em 0 . //A função abs retorna o valor absoludo de um número
	cenario[bitQueSofreraMutacao]=abs(cenario[bitQueSofreraMutacao] - 1); 
	return cenario;
}

HsTcpPollComm::Cenario HsTcpPollComm::criaCenarioAleatorio(int tamanhoCenario)  // cria cenario aleatorio
{
	Cenario cenario;
	for(int i=0; i<tamanhoCenario; i++)
		// rand() % 2 retorna aleatoriamente 0 ou 1, cenario é preenchido com 0 e 1
		cenario.push_back(rand() % 2);	
	return cenario;
}

void HsTcpPollComm::inicializaCenariosPrimeiraGeracao(int tamanhoCenario){ 
	// Cria a primeira geração de cenarios de forma aleatória, como nenhum cenario foi avaliado, recebem valor -1
	TuplaCenario tuplaCenario; 

	tuplaCenario= make_tuple(criaCenarioAleatorio(tamanhoCenario),-1);
	geracaoAtual.push_back(tuplaCenario);					//1
	tuplaCenario= make_tuple(criaCenarioAleatorio(tamanhoCenario),-1);
	geracaoAtual.push_back(tuplaCenario);					//2
	tuplaCenario= make_tuple(criaCenarioAleatorio(tamanhoCenario),-1);
	geracaoAtual.push_back(tuplaCenario);					//3
	tuplaCenario= make_tuple(criaCenarioAleatorio(tamanhoCenario),-1);
	geracaoAtual.push_back(tuplaCenario);					//4
	tuplaCenario= make_tuple(criaCenarioAleatorio(tamanhoCenario),-1);
	geracaoAtual.push_back(tuplaCenario);					//5
	tuplaCenario= make_tuple(criaCenarioAleatorio(tamanhoCenario),-1);	
	geracaoAtual.push_back(tuplaCenario);					//6 cenarios adicionados na geração inicial
}

void HsTcpPollComm::criaNovaGeracao(int qtdElementosReplicados){ 
	// cria nova geração onde qtdElementosReplicados é o número de elementos na geração inicial que serão replicados para a próxima geração
	geracaoPassada.swap(geracaoAtual);			// geração passada passa a ser a geração atual
	geracaoAtual.clear();						// geração atual é limpa
	for(int i=0; i<qtdElementosReplicados; i++)
	{
		geracaoAtual.push_back(geracaoPassada[i]); // melhores elementos da geração passada passam a ser elementos da geração atual
	}
	contadorNumeroDeGeracoes++; // contador do número de gerações é incrementado

	//variáveis utilizadas na manipulação de elementos do crossover
	TuplaCenario tuplaCenario; 
	crossoverCenarios tuplaCrossoverCenarios;
	Cenario cenarioFilho1;
	
	//Cria elementos Para Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
	// cria um cenario com crossoverMascaraAleatoria
	cenarioFilho1 = crossoverMascaraAleatoria(std::get<0>(geracaoAtual[0]) ,std::get<0>(geracaoAtual[2])); 
	tuplaCenario= make_tuple(cenarioFilho1,-1);
	contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados
	geracaoAtual.push_back(tuplaCenario);  // Insere crossover na geração atual

	//faz crossover com crossoverDeUmPonto e retorna dois filhos
	tuplaCrossoverCenarios = crossoverDeUmPonto(std::get<0>(geracaoAtual[0]) ,std::get<0>(geracaoAtual[1])); 																								

	tuplaCenario= make_tuple(std::get<0>(tuplaCrossoverCenarios),-1);
	contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados
	geracaoAtual.push_back(tuplaCenario);  // Insere crossover na geração atual

	tuplaCenario= make_tuple(std::get<1>(tuplaCrossoverCenarios),-1);
	contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados
	geracaoAtual.push_back(tuplaCenario);  // Insere crossover na geração atual

}
void HsTcpPollComm::verificaCondicoesDeParada(){
	if (std::get<1>(geracaoAtual.back()) >= 0.95) 
	{
		// geracaoAtual.back() retorna o prieiro elemento do vector geração atual, ou seja, verifica se o mais apto satisfaz a condição de parada
		emoHandler->ofs <<"Engagement Level alvo atingido, Maior engagement:" << std::get<1>(geracaoAtual.back())  << std::endl;
		exit(0);
	}
	// O  mais apto não satisfaz a condição de parada
	if(contadorNumeroDeGeracoes>=4){ // TRANSFORMAR EM CONSTANTE
	// Programa para pois alcançou o numero de gerações Limite
		emoHandler->ofs <<"Engagement Level alvo não atingindo atingido após 4 gerações, Maior engagement:" << std::get<1>(geracaoAtual.back()) << std::endl;				
		exit(0);
	}
}

void HsTcpPollComm::condicaoParadaEngagement(float engagement){
	if (engagement >= 0.95) 
	{
		// geracaoAtual.back() retorna o prieiro elemento do vector geração atual, ou seja, verifica se o mais apto satisfaz a condição de parada
		emoHandler->ofs <<"Engagement Level alvo atingido, Maior engagement:" << engagement  << std::endl;
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
	crossoverCenarios tuplaCrossoverCenarios;
	Cenario cenarioFilho1;
	TuplaCenario tuplaCenario; 

	switch(controleFluxo)           
	{								
	case SET_CENARIOS_INICIAIS:
		//	Cria n cenarios iniciais com valores aleatórios
		//inicializaCenariosPrimeiraGeracao(sizeof(unitsInicializador));
		inicializaCenariosPrimeiraGeracao(units.size());
		qDebug() << "units.size():" << units.size();

		//contadorPrimeiraGeracao Armazena tamanho da população inicial para que se possa fazer o controle de uso posteriormente
		contadorPrimeiraGeracao = geracaoAtual.size(); 
		contadorCrossoversNaoAvaliados=0;
		contadorNumeroDeGeracoes=1;
		

		tuplaCenario = geracaoAtual[contadorPrimeiraGeracao-1];	// retorna uma tupla que contem um dos cenarios inicias 
		//std::cout <<"std::get<0>(tuplaCenario):" 
		//	std::cout <<"std::get<0>(tuplaCenario):" << std::get<0>(tuplaCenario)  << std::endl;<< std::get<0>(tuplaCenario).size()  << std::endl;
		
		setCenario(units, std::get<0>(tuplaCenario));			// seta um dos cenarios iniciais
		emoHandler->ofs <<"Cenario: ";
		printCenario(std::get<0>(tuplaCenario));
		emoHandler->ofs << std::endl;

		controleFluxo = AVALIA_CENARIOS_INICIAIS;
		break;
	case AVALIA_CENARIOS_INICIAIS:
		// retorna ultima tupla que foi usada na setCenario para avalia-la
		tuplaCenario = geracaoAtual[contadorPrimeiraGeracao-1];
		std::get<1>(tuplaCenario) =  emoHandler->affectivEngagementBoredom;	
		std::cout <<"Engagement_level:  " << std::get<1>(tuplaCenario) << std::endl;
		emoHandler->ofs <<"Engagement_level:  " << std::get<1>(tuplaCenario) << std::endl;
		condicaoParadaEngagement(std::get<1>(tuplaCenario));
		contadorPrimeiraGeracao--;		// decrementamos contadorPrimeiraGeracao para na proxima iteração pegarmos o próximo cenario inicial que ainda não foi avaliado
		if(contadorPrimeiraGeracao>=0){
			//Ainda há cenarios iniciais a serem avaliados
			//Setamos o próximo cenario a ser avaliado para não perder tempo esperando a proxima chamada da controle()
			tuplaCenario = geracaoAtual[contadorPrimeiraGeracao-1];	// retorna uma tupla que concem um dos cenarios inicias 
			setCenario(units, std::get<0>(tuplaCenario));			// seta um dos cenarios iniciais
			emoHandler->ofs <<"Cenario: ";
			printCenario(std::get<0>(tuplaCenario));
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS_INICIAIS;
		}
		else{
			//Não há mais cenarios iniciais a serem avaliados
			sort(geracaoAtual.begin(),geracaoAtual.end(),[](const TuplaCenario& a,const TuplaCenario& b) -> bool{return std::get<1>(a) > std::get<1>(b);});
			
			emoHandler->ofs <<"Todos Cenarios Iniciais Avaliados"<<std::endl;
			emoHandler->ofs <<"Cenarios Elite:";
			printCenario(std::get<0>(geracaoAtual.back()));
			emoHandler->ofs <<  std::endl;
			emoHandler->ofs <<" Elite possui Engagement_level:  " << std::get<1>(geracaoAtual.back()) << std::endl;


			//Para otimizar o tempo de execução, e não experar até a próxima chamada da função controle() já executamos o código abaixo para setar um cenario
			//Alem de criar uma nova geração, cria 3 elementos utilizando os dois tipos de crossover,e mutação
			 // o 4 representa o número de elementos que será replicado da geração atual, para a proxima geração, TRANSFORMAR EM CONSTANTE!!
			std::cout <<"Criando geração de numero:  " << contadorNumeroDeGeracoes << std::endl;
			criaNovaGeracao(4);
			
			// retorna uma tupla que contem um cenario crossover não avaliado
			tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	
			setCenario(units, std::get<0>(tuplaCenario));			// seta um dos cenarios do crossover
			emoHandler->ofs <<"Cenario: ";
			printCenario(std::get<0>(tuplaCenario));
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
		}
		break;

	case AVALIA_CENARIOS:
		// retorna ultima tupla que foi usada na setCenario para avalia-la
		tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];
		std::get<1>(tuplaCenario) =  emoHandler->affectivEngagementBoredom;
		std::cout <<"Engagement_level:  " << std::get<1>(tuplaCenario) << std::endl;
		emoHandler->ofs <<"Engagement_level:  " << std::get<1>(tuplaCenario) << std::endl;
		condicaoParadaEngagement(std::get<1>(tuplaCenario));
		// decrementamos contadorCrossoversNaoAvaliados para na proxima iteração pegarmos o próximo cenario do crossover que ainda não foi avaliado
		contadorCrossoversNaoAvaliados--;		
		if (contadorCrossoversNaoAvaliados == 0){
			//Toda Geração avaliada
			sort(geracaoAtual.begin(),geracaoAtual.end(),[](const TuplaCenario& a,const TuplaCenario& b) -> bool{return std::get<1>(a) > std::get<1>(b);});	
			
			emoHandler->ofs <<"Todos Cenarios do Crossover Avaliados"<<std::endl;
			emoHandler->ofs <<"Cenarios Elite:";
			printCenario(std::get<0>(geracaoAtual.back()));
			emoHandler->ofs <<  std::endl;
			emoHandler->ofs <<" Elite possui Engagement_level:  " << std::get<1>(geracaoAtual.back()) << std::endl;

			verificaCondicoesDeParada();
			 
			// cria nova geração
			//Cria elementos Para Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
			// Insere crossover na geração atual
			// o 4 representa o número de elementos que será replicado da geração atual, para a proxima geração
			std::cout <<"Criando geração de numero:  " << contadorNumeroDeGeracoes << std::endl;
			criaNovaGeracao(4); 
				
			// retorna uma tupla que contem um cenario crossover não avaliado
			tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	
			setCenario(units, std::get<0>(tuplaCenario));			// seta um dos cenarios do crossover
			emoHandler->ofs <<"Cenario: ";
			printCenario(std::get<0>(tuplaCenario));
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
			
		}
		else{
			//Ainda temos elementos da geração atual a avaliar
			//Setamos o próximo cenario a ser avaliado para não perder tempo esperando a proxima chamada da controle()
			tuplaCenario = geracaoAtual[geracaoAtual.size()-contadorCrossoversNaoAvaliados];	// retorna uma tupla que concem um dos cenarios do crossover ainda não avaliado
			setCenario(units, std::get<0>(tuplaCenario));			// seta um dos cenarios do crossover
			emoHandler->ofs <<"Cenario: ";
			printCenario(std::get<0>(tuplaCenario));
			emoHandler->ofs << std::endl;

			controleFluxo = AVALIA_CENARIOS;
		}
		break;

	default: std::cout << "controleFluxo desconhecido: "<< controleFluxo << std::endl;

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
