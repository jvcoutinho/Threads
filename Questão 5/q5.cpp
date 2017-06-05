#include <iostream>
#include <vector>
#include <utility>
#include <thread>
#include <fstream>
using namespace std;

typedef pair<int, double> parEsparso;

class Matrizes {

private:
	int numLinhas;
	vector< vector<parEsparso> > matrizEsparsa1;
	vector< vector<parEsparso> > matrizEsparsa2;
	vector<parEsparso> vetorEsparso;
	vector< vector<double> > matrizDensa;

	vector< vector<parEsparso> > solucaoMM;
	vector<parEsparso> solucaoMV;

public:

	thread createThread(int numThread, int operacao) {

		if(operacao == 1)
			return thread([=] { timesSparseVector(numThread); });
		else if(operacao == 2)
			return thread([=] { timesSparseMatrix(numThread); });
		else
			return thread([=] { timesDenseMatrix(numThread); });
			
	}
	
	Matrizes(int operacao) {
		if(setOperands(operacao))
			setSolution();
		else {
			cout << "Entrada inválida." << endl;
			exit(1);
		}

		
	};

	bool setOperands(int operacao);
	void setSolution();
	string inputString(int operacao);

	vector<double> fetchVector(fstream &entrada);
	vector<parEsparso> getSparseVector(vector<double> V);
	vector< vector<double> > fetchMatrix(fstream &entrada);
	vector< vector<parEsparso> > getSparseMatrix(vector< vector<double> > M);

	int getNumLinhas() {
		return numLinhas;
	}

	void timesSparseVector(int numThread);
	void timesSparseMatrix(int numThread);
	void timesDenseMatrix(int numThread) {};

	void printSolution(int operacao);
	
};

vector< vector<double> > Matrizes::fetchMatrix(fstream &entrada) {
	int numLinhas, numColunas;
	int elemento;

	entrada >> numLinhas >> numColunas;

	vector< vector<double> > matriz(numLinhas);
	for(int i = 0; i < numLinhas; i++) {

		matriz[i] = vector<double>(numColunas);

		for(int j = 0; j < numColunas; j++) {
			entrada >> elemento;
			matriz[i][j] = elemento;
		}
	}

	return matriz;
}

vector<double> Matrizes::fetchVector(fstream &entrada) {
	int numLinhas;
	int elemento;

	entrada >> numLinhas;

	vector<double> vetor(numLinhas);
	for(int i = 0; i < numLinhas; i++) {
		entrada >> elemento;
		vetor[i] = elemento;
	}

	return vetor;
}

bool Matrizes::setOperands(int operacao) {
	fstream entrada;
	entrada.open(inputString(operacao));

	/* Pegando a matriz esparsa. */
	vector< vector<double> > M = fetchMatrix(entrada);
	numLinhas = M.size();
	

	switch(operacao) {

		case 1: { /* Pegando o vetor esparso. */
			vector<double> V = fetchVector(entrada);

			if(V.size() == M[0].size())
				vetorEsparso = getSparseVector(V);
			else
				return false;
			
			break;
		}

		case 2: { /* Pegando a matriz esparsa. */
			vector< vector<double> > M2 = fetchMatrix(entrada);

			if(M2.size() == M[0].size())
				matrizEsparsa2 = getSparseMatrix(M2);
			else 
				return false;
			
			break;
		}

		case 3: { /* Pegando a matriz densa. */
			matrizDensa = fetchMatrix(entrada);

			if(matrizDensa.size() != M[0].size()) 
				return false;

			break;
		}	
	}

	entrada.close();

	matrizEsparsa1 = getSparseMatrix(M);
	return true;
}

void Matrizes::setSolution() {
	solucaoMM = vector< vector<parEsparso> >(numLinhas);
	solucaoMV = vector<parEsparso>(numLinhas);	
}

vector<parEsparso> Matrizes::getSparseVector(vector<double> V) {
	vector<parEsparso> sparseV;
	for(int i = 0; i < V.size(); i++) 
		if(V[i] != 0)
			sparseV.push_back(pair<int, double>(i, V[i]));
	return sparseV;
}

vector< vector<parEsparso> > Matrizes::getSparseMatrix(vector< vector<double> > M) {
	vector< vector<parEsparso> > sparseM;
	for(int i = 0; i < M.size(); i++) 
		sparseM.push_back(getSparseVector(M[i]));
	return sparseM;
}

string Matrizes::inputString(int operacao) {
	string input = "";
	input += (operacao + 48);
	input += ".in";
	return input;
}

void Matrizes::timesSparseVector(int numThread) {

	double soma = 0;

	for(int j = 0; j < matrizEsparsa1[numThread].size(); j++) {
		int elemento = matrizEsparsa1[numThread][j].first;
		for(int k = 0; k < vetorEsparso.size(); k++)
			if(vetorEsparso[k].first == elemento) 
				soma += matrizEsparsa1[numThread][j].second * vetorEsparso[k].second;
	}
	
	if(soma != 0)
		solucaoMV[numThread] = pair<int,double>(numThread, soma);
}

void Matrizes::timesSparseMatrix(int numThread) { /*parei aqui*/

	double soma = 0;

	for(int i = 0; i < matrizEsparsa1[numThread].size(); i++) {
		int elemento = matrizEsparsa1[numThread][i].first;
		for(int j = 0; j < matrizEsparsa2.size(); j++)
			for(int k = 0; k < matrizEsparsa2[j].size(); k++)
		if(soma != 0)
			solucaoMM[numThread].push_back(pair<int,double>(numThread, soma));
		soma = 0;
	}

}

void Matrizes::printSolution(int operacao) {
	cout << "Solução:" << endl;
	if(operacao == 1) 
		for(int i = 0; i < solucaoMV.size(); i++)
			cout << "(" << solucaoMV[i].first << ", " << solucaoMV[i].second << ")" << " ";
	else {
		for(int i = 0; i < solucaoMM.size(); i++) {
			for(int j = 0; j < solucaoMM[i].size(); j++)
				cout << "(" << solucaoMM[i][j].first << ", " << solucaoMM[i][j].second << ")" << " ";
			cout << endl;
		}
	}
	cout << endl;
}

int main(int argc, char const *argv[]) {
	
	/* Atenção! O programa usa arquivos para sua entrada! 
	Modifique os arquivos x.in, onde x é a operação a ser realizada, para alterar a entrada. */

	cout << "Multiplicação entre Matrizes" << endl;
	cout << "1 - Matriz esparsa x Vetor esparso" << endl;
	cout << "2 - Matriz esparsa x Matriz esparsa" << endl;
	cout << "3 - Matriz esparsa x Matriz densa" << endl;

	int operacao;
	cout << "Digite que operação você quer fazer: ";
	cin >> operacao;

	vector< vector<double> > ME1; // Matriz esparsa 1.
	int numLinhas1, numColunas1;

	Matrizes M(operacao);

	/* Criação das threads. */
	int numThreads = M.getNumLinhas();
	thread multiplicadores[numThreads];

	for(int i = 0; i < numThreads; i++)
		multiplicadores[i] = M.createThread(i, operacao);

	/* Sincronização das threads. */
	for(int i = 0; i < numThreads; i++)
		multiplicadores[i].join();	

	M.printSolution(operacao);

	return 0;
}