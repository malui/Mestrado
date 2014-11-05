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

// Definição de constantes usadas no algoritmo genético
#define LIMITE_GERACOES 4						  // número limite de gerações
#define ENGAGEMENT_ALVO 0.95					 // número limite de engagement
#define TAMANHO_GERACAO_INICIAL 3				// tamanho da geração inicial
#define ELEMENTOS_REPLICADOS_PROXIMA_GERACAO 4 // numeros de elementos replidados de uma geração para outra

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
	//typedef std::vector<TuplaCenario> Populacao;                // Vetor que contem todos individuos de uma geração
	typedef std::tuple<Estados,Estados> crossoverEstados;     // Resultado do cross over de dois vetores
			

	void printCenario(const vector<int> &v);                            // Imprime um estado do tipo vector<int>
	void printPopulacao(const VectorCenarios populacao);                              // Imprime a população de uma geração
	void printVector (const vector<float> &v);
	
	crossoverEstados HsTcpPollComm::crossoverDeUmPonto(Estados estado1,Estados estado2);   // Implementação do crossover devolve os dois novos estados que são a recombinação de seus pais
	Estados HsTcpPollComm::crossoverMascaraAleatoria(Estados estados1,Estados estados2);	   // Implementação do crossover devolve os dois ovos estados
	Estados HsTcpPollComm::mutacao(Estados estados);
	
	void inicializaCenariosPrimeiraGeracao(int tamanhoCenario);  // Função responsavel por definir qual a população inicial de cenarios, utiliza criaCenarioAleatorio
	std::vector<int> HsTcpPollComm::criaEstadosAleatorio(int tamanhoEstados);
	
	// cria nova geração onde qtdElementosReplicados é o número de elementos na geração inicial que serão replicados para a próxima geração
	// Cria elementos 3 elementos com Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos
	// Insere elementos do crossover na geração atual
	// o qtdElementosReplicados representa o número de elementos que será replicado da geração atual, para a proxima geração
	void criaNovaGeracao(int qtdElementosReplicados);	
	//Populacao geracaoAtual;
	//Populacao geracaoPassada;
	VectorCenarios geracaoAtual;
	VectorCenarios geracaoPassada;

	void verificaCondicoesDeParada();				// verifica condições de parada ao fim de uma geração
	void condicaoParadaEngagement(float engagement); // verifica condições de parada a cada chamada de Engagement

	float mediaHarmonicaPonderada(vector<float> v);
	
	int contadorPrimeiraGeracao;		// armazena o numero de cenario na primeira geração, é usado para controlar a avaliação dos cenarios
	int contadorCrossoversNaoAvaliados; // armazena o numero de cenarios que sairam do crossover e ainda não foram avaliados
	int contadorNumeroDeGeracoes;		// Controla o numero de gerações

	std::vector<int> codCenariosExistentes;
	
	//Inicializa com 0 em todas as posições o codificador de cenarios existentes com um numero de posições igual a 2^(tamanhoCenario) 
	void inicializaCodificadorCenariosExistentes(int tamanhoCenario);
	// Recebe um cenario, usa codificaCenario() para codificar, e verifica se cenario já existe
	bool isCenarioRepetido(Estados estado);

	//faz uma codificacao por posicao, caso a primeira posicao esteja peenchida, somamos 1, 
	//caso a segunda somamos 2, caso a terceira somamos 4, e assim por diante  
	// dessa forma temos uma codificação em potencia de 2 baseada nas posições que devolve 
	//um código único para o estado recebido
	int codificaCenario(Estados estado);

	//chamada quando se tem certeza que o cenario existe para ser inserido ao vectod codCenariosExistentes
	void insereCenarioCodificador(Estados estado);

	//faz o inverso da função codificacao, a partir de um numero descodifica em zeros e uns
	Estados decodificaCenario(int codificacao);

	// resgata do vetor codCenariosExixtentes um cenario ainda não usado e o retorn
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
