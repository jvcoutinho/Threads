#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>
#include <vector>
#include <pthread.h>
#define NUM_INCOGNITAS 4
using namespace std;

class Jacobi {

private:
	vector< vector<double> > A;
	vector<double> B;
	vector<double> solucao;
	vector<double> proximaSolucao;
	vector< list<int> > incognitas;
	int numIncognitasporThread;

public:

	thread createThread(int numThread) {
		return thread([=] { solve(numThread); });
	}

	Jacobi() {
		initA();
		initB();
		initX();
	};

	void initA();
	void initB();
	void initX();
	void separarIncognitas(int numThreads);
	void solve(int numThread);
	void JacobiMethod(int posIncognita);
	void atualizaSolucao(int numThread);	
	void printSolution();
};

pthread_barrier_t barrier;

void Jacobi::initA() {
	A = vector< vector<double> >(NUM_INCOGNITAS);
	for(int i = 0; i < A.size(); i++)
		A[i] = vector<double>(NUM_INCOGNITAS);

	A[0] = { 10, -1, 2, 0 };
	A[1] = { -1, 11, -1, 3 };
	A[2] = { 2, -1, 10, -1 };
	A[3] = { 0, 3, -1, 8 };

	/*A[0] = { 2, 1 };
	A[1] = { 5, 7 };*/
}

void Jacobi::initB() {
	B = vector<double>(NUM_INCOGNITAS);
	B = { 6, 25, -11, 15 };
	//B = { 11, 13 };
}

void Jacobi::initX() {
	solucao = vector<double>(NUM_INCOGNITAS, 1); // X é iniciado com 1 em seus elementos.
	proximaSolucao = vector<double>(NUM_INCOGNITAS, 1);
}

void Jacobi::separarIncognitas(int numThreads) {

	/* incognitas é um array onde cada elemento é uma lista, no qual separa as incógnitas para cada thread. */
	incognitas = vector< list<int> >(numThreads); 

	if(numThreads > NUM_INCOGNITAS)
		numIncognitasporThread = 1;
	else
		numIncognitasporThread = NUM_INCOGNITAS / numThreads;

	cout << numIncognitasporThread << " incógnita(s)/thread." << endl;

	int thread = 1;
	for(int i = 1; i <= NUM_INCOGNITAS; i++) {
		incognitas[thread - 1].push_back(i - 1);

		if(numIncognitasporThread != 0 && i % numIncognitasporThread == 0 && thread < numThreads)
			thread++;
	}
}

void Jacobi::printSolution() {
	cout << endl << "Solução:" << endl; 
	for (int i = 0; i < NUM_INCOGNITAS; ++i)
		cout << solucao[i] << " ";
	cout << endl;
}

/* MÉTODO ITERATIVO DE JACOBI */
void Jacobi::solve(int numThread) {
	/* Se há mais threads que incógnitas, o excedente não passará pelo algoritmo, pois não há nenhuma incógnita em sua lista. */
	if(!incognitas[numThread].empty()) { 
		int numIteracoes = 30;

		for(int k = 0; k < numIteracoes; k++) {
			for(list<int>::iterator i = incognitas[numThread].begin(); i != incognitas[numThread].end(); i++)
				JacobiMethod(*i);

			/* Barreira: as threads devem parar aqui para esperar o restante terminar seus cálculos. */
			pthread_barrier_wait(&barrier);
			/* A implementação consiste que cada thread tem seu próprio espaço e nada mais. Considerando isso
			e o fato que temos mais uma barreira, tratamento para regiões críticas é dispensável. */
			atualizaSolucao(numThread);
			pthread_barrier_wait(&barrier);
		}
	}
}

void Jacobi::JacobiMethod(int posIncognita) {
	double soma = 0;
	for(int j = 0; j < NUM_INCOGNITAS; j++)
		if(posIncognita != j)
			soma += A[posIncognita][j] * solucao[j];

	proximaSolucao[posIncognita] = (B[posIncognita] - soma) / A[posIncognita][posIncognita];
}

void Jacobi::atualizaSolucao(int numThread) {
	for(list<int>::iterator i = incognitas[numThread].begin(); i != incognitas[numThread].end(); i++)
		solucao[*i] = proximaSolucao[*i];
}

int main(int argc, char const *argv[]) {
	
	/* Aumentar o número de threads acelera o algoritmo, aumentando a precisão nas iterações iniciais. */
	int numThreads;
	cout << "Digite o número de threads: ";
	cin >> numThreads;

	Jacobi solver;

	/* Separação das incógnitas. */
	solver.separarIncognitas(numThreads);

	/* Inicialização das barreiras. */
	if(numThreads > NUM_INCOGNITAS) // Haverá threads que não passarão pelo algoritmo.
		pthread_barrier_init(&barrier, NULL, NUM_INCOGNITAS);
	else
		pthread_barrier_init(&barrier, NULL, numThreads);

	/* Criação das threads. */
	vector<thread> solucionadores(numThreads);
	for(int i = 0; i < numThreads; i++)
		solucionadores[i] = solver.createThread(i);

	/* Sincronização. */
	for(int i = 0; i < numThreads; i++)
		solucionadores[i].join();

	solver.printSolution();

	pthread_barrier_destroy(&barrier);

	return 0;
}
