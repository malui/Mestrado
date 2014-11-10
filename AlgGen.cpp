#include "AlgGen.h"

#include <time.h>       /* time */


AlgGen::AlgGen()
{
	emoHandler = NULL;
	treshold = 0.9;
}

AlgGen::~AlgGen()
{
	emoHandler = NULL;
}








/*              ALGORITMO GENETICO                  */

/*
Estados mutacao(Estados estados)
retorna estado mutado

aleatoriamente escolhe bit que sofrerá mutação:
- caso tenha valor 1, passará a ser 0, 
- caso tenha valor 0, passará a ter valor 1.
*/
AlgGen::Estados AlgGen::mutacao(AlgGen::Estados estados) 
{
	 // aleatoriamente escolhe bit que sofrerá mutação, ou seja, caso tenha valor 1, passará a ser 0, caso tenha valor 0, passará a ter valor 1
	srand (time(NULL));
	int bitQueSofreraMutacao = (rand() % estados.size());

	//  transforma 0 em 1, e 1 em 0 . //A função abs retorna o valor absoludo de um número
	estados[bitQueSofreraMutacao]=abs(estados[bitQueSofreraMutacao] - 1); 
	return estados;
}


/*	Estados toBinary(int number, Estados cenario)
	Transforma um valor inteiro (number) para binario e returna na forma de um vetor
*/
AlgGen::Estados AlgGen::toBinary(int number, AlgGen::Estados cenario) {
	int remainder;

	if(number <= 1) 
	{
		cenario.push_back(number);
		return cenario;
	}
	else
	{
		remainder = number%2;  // % - operação resto de divisão
		cenario.push_back(remainder);
		return toBinary(number >> 1, cenario);
	}
}
/*
crossoverEstados crossoverDeUmPonto(Estados estado1,Estados estado2)

Função crossover de um ponto aleatório.
Faz um crossover dividindo o vetor em duas partes. 
O tamanho do corte é definido aleatóriamente

retorna uma tupla com dois Estados
*/
AlgGen::crossoverEstados AlgGen::crossoverDeUmPonto(AlgGen::Estados estado1, AlgGen::Estados estado2) 
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
	AlgGen::crossoverEstados crossoverTupla= make_tuple(primeira_parte_estado1,primeira_parte_estado2);
	return crossoverTupla;
}

/*
crossoverMascaraAleatoria(Estados estados1,Estados estados2)
Retorna um estado filho com taxa de mutacao de 5%

cria uma mascara de zeros e uns do tamanho de cenario.estados, 
se a posição da mascara tem valor um, o valor do primeiro pai é replicado para o filho na mesma posição,
caso tenha o valor 0, o valor do segundo pai é replicado para o filho
*/
AlgGen::Estados AlgGen::crossoverMascaraAleatoria(AlgGen::Estados estados1,AlgGen::Estados estados2) 
{
	srand (time(NULL));		//	inicializa semente aleatória 
	AlgGen::Estados estadosFilho;

	for(int i=0; i < estados1.size(); i++)
	{ 
		// cria aleatóriamente um número que será 0 ou 1, caso for 0, o valor do primeiro pai é replicado para o filho
		//, caso for 1 o valor do segundo pai é replicado para o filho  
		if ((rand() % 2) == 0)	 {
			estadosFilho.push_back(estados1[i]); }
		else 
		{
			estadosFilho.push_back(estados2[i]);
		}
	}
	// rand() % 20 retorna números entre 0 e 19, quando o valor retornado for 0, ou seja 5% das vezes, teremos uma taxa de mutação 5%

	if ((rand() % 20) == 0) {
		return mutacao(estadosFilho);}
	else {
		return estadosFilho;}
}


/*
criaEstadosAleatorio(int tamanhoEstados)
retorna vector<int> estados do tamanho de tamanhoEstados
*/
std::vector<int> AlgGen::criaEstadosAleatorio(int tamanhoEstados)  // cria estados aleatorio
{
	std::vector<int> estados;
	//std::random_device rd;
	//srand (rd());
	srand (time(NULL));
	for(int i=0; i<tamanhoEstados; i++)
		// rand() % 2 retorna aleatoriamente 0 ou 1, cenario é preenchido com 0 e 1
		estados.push_back(rand() % 2);	
	return estados;
}

/*	void insereCenarioCodificador(std::vector<int> cenario)
	chamada quando se tem certeza que o cenario existe para ser inserido ao vectod codCenariosExistentes
*/
void AlgGen::insereCenarioCodificador(std::vector<int> cenario){
	codCenariosExistentes[codificaCenario(cenario)]++; 
}

/*	Estados getCenarioNaoUsado( int tamanhoEstadosCenario)
	resgata do vetor codCenariosExixtentes um cenario ainda não usado e o retorn

	Quando todos os cenarios ja foram usados, retorna um vector com {-1}
*/
AlgGen::Estados AlgGen::getCenarioNaoUsado( int tamanhoEstadosCenario){
	int codificacao;

	for(int i=0; i < codCenariosExistentes.size(); i++)
	{
		if (codCenariosExistentes[i] == 0)
		{
			return decodificaCenario(i, tamanhoEstadosCenario);
		}
	}

	// Representa a utilização de todos os cenarios

	AlgGen::Estados todosCenariosUsados;
	todosCenariosUsados.push_back(-1);

	return todosCenariosUsados;
}

/*   Estados isCenarioRepetido(std::vector<int> cenario)
     - retorna -1 se cenario eh repetido e todos os cenarios ja foram usados
	 - retorna novo cenario nao usado se o cenario eh repetido
	 - retorna o mesmo cenario de entrada se ele nao for repetido
	 */
AlgGen::Estados AlgGen::isCenarioRepetido(std::vector<int> cenario)
{
	//AlgGen::Estados todosCenariosUsados;

	if (codCenariosExistentes[codificaCenario(cenario)]>0) //Cenario eh repetido:
	{
		emoHandler->ofs <<"isCenarioRepetido 1" << std::endl;
		// Faz mutação até encontrar um estado não existente
		cenario = mutacao(cenario);
		emoHandler->ofs <<"isCenarioRepetido 2" << std::endl;
		if(codCenariosExistentes[codificaCenario(cenario)]>0) //se cenario mutado tambem eh repetido,
		{
			emoHandler->ofs <<"isCenarioRepetido 3" << std::endl;
			//pega proximo cenario nao usado:
			return getCenarioNaoUsado(cenario.size());
			// retorna -1 se todos os cenarios possiveis ja foram usados
			// se nao, retorna o proximo cenario possivel ainda nao usado
		}
		else //novo cenario nao eh repetido
		{
			return cenario;
		}
	}
	else
	{
		emoHandler->ofs <<"isCenarioRepetido 4" << std::endl;
		return cenario; // retorna o mesmo cenario de entrada se cenario nao eh repetido
	}
}

/*  int codificaCenario(std::vector<int> cenario)
    faz uma codificacao por posicao, caso a primeira posicao esteja peenchida, somamos 1, 
	caso a segunda somamos 2, caso a terceira somamos 4, e assim por diante  
	dessa forma temos uma codificação em potencia de 2 baseada nas posições que devolve 
	um código único para o estado recebido
*/
int AlgGen::codificaCenario(std::vector<int> cenario){
	int codificacao = 0;

	for(int i=0; i<cenario.size(); i++)
	{
		if (cenario[i] == 1){
			codificacao = codificacao + pow(double(2),double(i));}
	}
	return codificacao;
}


/*	Estados decodificaCenario(int codificacao, int tamanhoCenario)
	faz o inverso da função codificacao, a partir de um numero descodifica em zeros e uns
*/
AlgGen::Estados AlgGen::decodificaCenario(int codificacao, int tamanhoCenario){
	AlgGen::Estados cenario; 

	cenario = toBinary(codificacao, cenario);
	cenario.resize(tamanhoCenario,  0);

	return cenario;
}


/*
void inicializaCenariosPrimeiraGeracao(int tamanhoCenario)
Inicializa a primeira geração de cenarios
com cenario.estados = aleatorio 0 ou 1
cenario.engagement = -1 pois ainda nao foram avaliados
*/
void AlgGen::inicializaCenariosPrimeiraGeracao(int tamanhoCenario)
{ 
	struct CENARIO cen;

	for(int i=0; i<TAMANHO_GERACAO_INICIAL; i++)
	{
		emoHandler->ofs <<"inicializaCenariosPrimeiraGeracao 1" << std::endl;
		cen.estados.clear();
		//seta aleatoriamente os estados de um cenario e ja verifica se ele eh repetido:
		cen.estados = isCenarioRepetido(criaEstadosAleatorio(tamanhoCenario));
		emoHandler->ofs <<"inicializaCenariosPrimeiraGeracao 2" << std::endl;
		if ( cen.estados[0] == -1)  //o cenario criado eh repetido e todos os cenarios ja foram usados
		{
			//numero de cenarios possiveis < TAMANHO_GERACAO_INICIAL
			emoHandler->ofs <<"inicializaCenariosPrimeiraGeracao 3" << std::endl;
			return;
		}
		else //novo cenario ainda nao usado
		{
			emoHandler->ofs <<"inicializaCenariosPrimeiraGeracao 4" << std::endl;
			insereCenarioCodificador(cen.estados); //cria codigo para novo cenario
			cen.engagement = -1; //inicializa engagement com -1 pois ainda nao foi avaliado
			geracaoAtual.push_back(cen); //insere novo cenario na geracaoAtual
			contadorCenariosNaoAValiados++; //incrementa contador de cenarios nao avaliados
			emoHandler->ofs <<"inicializaCenariosPrimeiraGeracao 5" << std::endl;
		}
	}
}

/*
criaNovaGeracao(int qtdElementosReplicados)

cria nova geração com os 'qtdElementosReplicados' melhores elementos da geração inicial
Cria elementos Para Crossover e criar mais 3 filhos:
   - Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, 
   - e crossoverMascaraAleatoria com o primeiro e com o terceiro, 
Insere elementos do crossover na geração atual
*/
void AlgGen::criaNovaGeracao(int qtdElementosReplicados)
{ 

	struct CENARIO cen;
	cen.engagement = -1;
	contadorCenariosNaoAValiados = 0;
	geracaoPassada = geracaoAtual;    
	geracaoAtual.clear();						// geração atual é limpa

	// inicializa geracaoAtual com 'qtdElementosReplicados' número de elementos da geração inicial 
	//(que serão replicados para a próxima geração)
	for(int i=0; i<qtdElementosReplicados; i++)
	{
		geracaoAtual.push_back(geracaoPassada[i]); // melhores elementos da geração passada passam a ser elementos da geração atual
	}
	//contadorNumeroDeGeracoes++; // contador do número de gerações é incrementado
	
	//Cria elementos Para Crossover, Utiliza a função crossoverDeUmPonto com os dois primeiros elementos, e crossoverMascaraAleatoria com o primeiro e com o terceiro, desse modo criamos mais 3 filhos

	//cria cenario com crossoverMascaraAleatoria e ja verifica se ele eh repetido:
	cen.estados.clear();
	cen.estados = isCenarioRepetido( crossoverMascaraAleatoria(geracaoAtual[0].estados ,geracaoAtual[2].estados) ); /// BUG! certificar que existe geracaoAtual[x].estados

	if ( cen.estados[0] == -1)  //o cenario criado eh repetido e todos os cenarios ja foram usados
	{
		//TODO:
		// pega cenario de maior engagement e seta 
		// novo treshold = maior engagement
		return; // nao tem porque tentar novos cenarios
	}
	else //novo cenario ainda nao usado
	{
		insereCenarioCodificador(cen.estados); //cria codigo para novo cenario
		cen.engagement = -1; //inicializa engagement com -1 pois ainda nao foi avaliado
		geracaoAtual.push_back(cen); //insere novo cenario na geracaoAtual
		contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados
		contadorCenariosNaoAValiados++;
	}
	
	//variáveis utilizadas na manipulação de elementos do crossover
	crossoverEstados tuplaCrossoverEstados;

	//faz crossover com crossoverDeUmPonto e retorna dois filhos em tuplaCrossoverEstados
	tuplaCrossoverEstados = crossoverDeUmPonto(geracaoAtual[0].estados ,geracaoAtual[1].estados); 

	//verifica se primeiro filho eh repetido:
	cen.estados.clear();
	cen.estados = isCenarioRepetido(std::get<0>(tuplaCrossoverEstados));
	if ( cen.estados[0] == -1)  //o cenario criado eh repetido e todos os cenarios ja foram usados
	{
		//TODO:
		// pega cenario de maior engagement e seta 
		// novo treshold = maior engagement
		return; // nao tem porque tentar novos cenarios
	}
	else //novo cenario ainda nao usado
	{
		insereCenarioCodificador(cen.estados); //cria codigo para novo cenario
		cen.engagement = -1; //inicializa engagement com -1 pois ainda nao foi avaliado
		geracaoAtual.push_back(cen); //insere novo cenario na geracaoAtual
		contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados
		contadorCenariosNaoAValiados++;
	}
	
	//verifica se segundo filho eh repetido:
	cen.estados.clear();
	cen.estados = isCenarioRepetido(std::get<1>(tuplaCrossoverEstados));
	if ( cen.estados[0] == -1)  //o cenario criado eh repetido e todos os cenarios ja foram usados
	{
		//TODO:
		// pega cenario de maior engagement e seta 
		// novo treshold = maior engagement
		return; // nao tem porque tentar novos cenarios
	}
	else //novo cenario ainda nao usado
	{
		insereCenarioCodificador(cen.estados); //cria codigo para novo cenario
		cen.engagement = -1; //inicializa engagement com -1 pois ainda nao foi avaliado
		geracaoAtual.push_back(cen); //insere novo cenario na geracaoAtual
		contadorCrossoversNaoAvaliados++;		//incrementa o numeor de cenarios criados com crossover que ainda devem ser avaliados
		contadorCenariosNaoAValiados++;
	}
}

/*
void verificaCondicoesDeParada()
Verifica condicao de parada ao fim de uma geracao (numero de geracoes)
Criterios de parada:
engagement >= ENGAGEMENT_ALVO 0.95
OU
Numero de Geracoes >= LIMITE_GERACOES 4
*/
void AlgGen::verificaCondicoesDeParada()
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

/*    condicaoParadaEngagement
      verifica condições de parada a cada chamada de Engagement
*/
bool AlgGen::condicaoParadaEngagement(float engagement){
	if (engagement >= treshold) //ENGAGEMENT_ALVO) 
	{
		// geracaoAtual.front() retorna o prieiro elemento do vector geração atual, ou seja, verifica se o mais apto satisfaz a condição de parada
		emoHandler->ofs <<"Engagement Level alvo atingido, Maior engagement:" << engagement  << std::endl;
		treshold = engagement;
		// setar o cenario com maior engagement?
		return true;
	}
	else
	{
		return false;
	}
}

/* ---------------------------------------------------------------------------------*/
/*                                PRINTS                                            */


/*	void printCenario (const vector<int>& v)
	Imprimie vector de inteiros v em emoHandler->ofs
	OBS: mudar para printEstados
*/
void AlgGen::printCenario (const vector<int> &v)
{

	for( std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
		emoHandler->ofs << *i << ' ';
}

/*	void printVector (const vector<float>& v)
	Imprimie vector de floats v em emoHandler->ofs
	OBS: mudar para printEstados
*/
void AlgGen::printVector (const vector<float> &v)
{

	for( std::vector<float>::const_iterator i = v.begin(); i != v.end(); ++i)
		emoHandler->ofs << *i << ' ';
}

/*	void printPopulacao( VectorCenarios populacao)
	Imprime a população de uma geração
*/
void AlgGen::printPopulacao ( VectorCenarios populacao)
{
	for(int i=0; i<populacao.size(); i++)
	{
		printCenario(populacao[i].estados);
		emoHandler->ofs <<  populacao[i].engagement << endl;

	}
}

/* --------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------------*/
/*                            FIM  ALGORITMO GENETICO                               */
/* ---------------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------------*/
/*                                 DSP                                              */
/* ---------------------------------------------------------------------------------*/

/*
Faz a media harmonica ponderada de um vetor
com pesos = indice 

M = ( soma(pesos) / (soma (pesoi/xi) ) )
*/
float AlgGen::mediaHarmonicaPonderada(vector<float> v)
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

/* ---------------------------------------------------------------------------------*/
/*                                 FIM DSP                                          */
/* ---------------------------------------------------------------------------------*/
