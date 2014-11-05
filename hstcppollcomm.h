#ifndef HSTCPPOLLCOMM_H
#define HSTCPPOLLCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;



#include "EmoHandler.h"

#include "DefinicoesGlobais.h"
enum TSessionState{DISCONNECTED, SENT_AUTH, CHECK_AUTH, WAITING_CONN, SENT_REQUEST, WAITING_RESPONSE, DISCONNECTING};//, SESSAO_ATIVA};

enum TControleFluxo{SET_CENARIOS_INICIAIS, AVALIA_CENARIOS_INICIAIS,SET_CENARIOS,AVALIA_CENARIOS,};

// Defini��o de constantes usadas no algoritmo gen�tico
#define LIMITE_GERACOES 4						  // n�mero limite de gera��es
#define ENGAGEMENT_ALVO 0.95					 // n�mero limite de engagement
#define TAMANHO_GERACAO_INICIAL 3				// tamanho da gera��o inicial
#define ELEMENTOS_REPLICADOS_PROXIMA_GERACAO 4 // numeros de elementos replidados de uma gera��o para outra

class HsTcpPollComm : public QObject
{
    Q_OBJECT
public:
    explicit HsTcpPollComm(QObject *parent = 0);
    bool tcpConnect(QString host = "localhost", int port = 2000);
    void setUnit(int unit, int value);
	int setCenario(std::vector<int> units, std::vector<int> values);
	QString makeGetUnit( const int *buffUnit, int numUnit );
	QString resposta;
	bool resposta_pronta;
	TSessionState sessionState;
	TControleFluxo controleFluxo;
	int ack;
	int cont_sinal;
	EmoHandler* emoHandler;
	void controle();
	void write_vec(const vector<int>& vec);
	void logAffectiveSuiteState(std::ostream& os, unsigned int userID, EmoStateHandle eState, bool withHeader);
	
	struct CENARIO {   // Declare CENARIO struct type
    std::vector<int> estados;   // Declare member types
    float            engagement;
    } cenario;   // Define object of type CENARIO

	//typedef std::vector<int> Cenario;                             // Vetor de zeros e uns que representam um estado
	typedef std::vector<int> Estados;
	typedef std::vector<CENARIO> VectorCenarios;
	//typedef std::tuple< Estados,float> TuplaCenario;             // Tupla que liga o estado ao valor de seu stress/parametro
	//typedef std::vector<TuplaCenario> Populacao;                // Vetor que contem todos individuos de uma gera��o
	typedef std::tuple<Estados,Estados> crossoverEstados;     // Resultado do cross over de dois vetores
			

	void printCenario(const vector<int> &v);                            // Imprime um estado do tipo vector<int>
	void printPopulacao(const VectorCenarios populacao);                              // Imprime a popula��o de uma gera��o
	void printVector (const vector<float> &v);
	
	crossoverEstados HsTcpPollComm::crossoverDeUmPonto(Estados estado1,Estados estado2);   // Implementa��o do crossover devolve os dois novos estados que s�o a recombina��o de seus pais
	Estados HsTcpPollComm::crossoverMascaraAleatoria(Estados estados1,Estados estados2);	   // Implementa��o do crossover devolve os dois ovos estados
	Estados HsTcpPollComm::mutacao(Estados estados);
	
	void inicializaCenariosPrimeiraGeracao(int tamanhoCenario);  // Fun��o responsavel por definir qual a popula��o inicial de cenarios, utiliza criaCenarioAleatorio
	std::vector<int> HsTcpPollComm::criaEstadosAleatorio(int tamanhoEstados);
	
	// cria nova gera��o onde qtdElementosReplicados � o n�mero de elementos na gera��o inicial que ser�o replicados para a pr�xima gera��o
	// Cria elementos 3 elementos com Crossover, Utiliza a fun��o crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
	// Insere elementos do crossover na gera��o atual
	// o qtdElementosReplicados representa o n�mero de elementos que ser� replicado da gera��o atual, para a proxima gera��o
	void criaNovaGeracao(int qtdElementosReplicados);	
	//Populacao geracaoAtual;
	//Populacao geracaoPassada;
	VectorCenarios geracaoAtual;
	VectorCenarios geracaoPassada;

	void verificaCondicoesDeParada();				// verifica condi��es de parada ao fim de uma gera��o
	void condicaoParadaEngagement(float engagement); // verifica condi��es de parada a cada chamada de Engagement

	float mediaHarmonicaPonderada(vector<float> v);
	
	int contadorPrimeiraGeracao;		// armazena o numero de cenario na primeira gera��o, � usado para controlar a avalia��o dos cenarios
	int contadorCrossoversNaoAvaliados; // armazena o numero de cenarios que sairam do crossover e ainda n�o foram avaliados
	int contadorNumeroDeGeracoes;		// Controla o numero de gera��es

	std::vector<int> codCenariosExistentes;
	
	//Inicializa com 0 em todas as posi��es o codificador de cenarios existentes com um numero de posi��es igual a 2^(tamanhoCenario) 
	void inicializaCodificadorCenariosExistentes(int tamanhoCenario);
	// Recebe um cenario, usa codificaCenario() para codificar, e verifica se cenario j� existe
	bool isCenarioRepetido(Estados estado);

	//faz uma codificacao por posicao, caso a primeira posicao esteja peenchida, somamos 1, 
	//caso a segunda somamos 2, caso a terceira somamos 4, e assim por diante  
	// dessa forma temos uma codifica��o em potencia de 2 baseada nas posi��es que devolve 
	//um c�digo �nico para o estado recebido
	int codificaCenario(Estados estado);

	//chamada quando se tem certeza que o cenario existe para ser inserido ao vectod codCenariosExistentes
	void insereCenarioCodificador(Estados estado);

	//faz o inverso da fun��o codificacao, a partir de um numero descodifica em zeros e uns
	Estados decodificaCenario(int codificacao);

	// resgata do vetor codCenariosExixtentes um cenario ainda n�o usado e o retorn
	Estados getCenarioNaoUsado();

	//Transforma um valor inteiro para binario e returna na forma de um vetor
	HsTcpPollComm::Estados HsTcpPollComm::toBinary(int number);

// end public
signals:
    void tcpClosed();

public slots:
    void pollProcess(void);
    void tcpOnConnect(void);
    void tcpOnDisconnect(void);
    void tcpOnRead(void);
    void tcpOnError(QAbstractSocket::SocketError error);
//	void sendRequest(void);

private:
	//TSessionState sessionState;
	QString CryptPass(char * szPassword);
    QTcpSocket tcpSocket;
    QTimer pollTimer;



};

#endif // HSTCPPOLLCOMM_H
