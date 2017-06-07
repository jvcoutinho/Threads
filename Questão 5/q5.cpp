#include <iostream>
#include <vector>
#include <thread>
#include <utility>
#include <fstream>
using namespace std;

typedef pair<int, double> par;
typedef vector<par> sparseVector;
typedef vector<double> denseVector;

class SparseMatrix {

private:
	vector<sparseVector> matrizEsparsa_1;
	vector<sparseVector> matrizEsparsa_2;
	vector<double> vetorDenso;
	vector<denseVector> matrizDensa;
	vector<sparseVector> solucaoEsparsa; // Matriz x matriz.
	vector<par> solucaoDensa; // Matriz x vetor.
	int numThreads;

public:
	
	SparseMatrix(int operacao) {
		if(setOperands(operacao))
				setSolution();
		else {
			cout << "Entrada inválida para multiplicação." << endl;
			exit(1);
		}
	};

	thread createThread(int numThread, int operacao) {
		if(operacao == 1)
			return thread([=] { timesDenseVector(numThread); });
		else if(operacao == 2)
			return thread([=] { timesSparseMatrix(numThread); });
		else
			return thread([=] { timesDenseMatrix(numThread); });	
	};
	

	bool setOperands(int operacao) {
		fstream entrada;
		entrada.open(inputString(operacao));

		/* Pegando a matriz esparsa. */
		vector<denseVector> M = fetchMatrix(entrada);
		numThreads = M.size(); // É o número de linhas.

		switch(operacao) {

			case 1: { /* Pegando o vetor esparso. */
				vetorDenso = fetchVector(entrada);

				if(vetorDenso.size() != M[0].size())
					return false;
				
				break;
			}

			case 2: { /* Pegando a matriz esparsa. */
				vector<denseVector> M2 = fetchMatrix(entrada);

				if(M2.size() == M[0].size()) 
					matrizEsparsa_2 = getSparseMatrix(M2, 2);
			
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

		matrizEsparsa_1 = getSparseMatrix(M, 1);
		return true;
	};

	string inputString(int operacao) {
		string input = "";
		input += (operacao + 48);
		input += ".in";
		return input;
	};

	vector<denseVector> fetchMatrix(fstream &entrada) {
		int numLinhas, numColunas;
		int elemento;

		entrada >> numLinhas >> numColunas;

		vector<denseVector> matriz(numLinhas);
		for(int i = 0; i < numLinhas; i++) {

			matriz[i] = vector<double>(numColunas);

			for(int j = 0; j < numColunas; j++) {
				entrada >> elemento;
				matriz[i][j] = elemento;
			}
		}

		return matriz;
	};

	vector<double> fetchVector(fstream &entrada) {
		int numLinhas;
		int elemento;

		entrada >> numLinhas;

		vector<double> vetor(numLinhas);
		for(int i = 0; i < numLinhas; i++) {
			entrada >> elemento;
			vetor[i] = elemento;
		}

		return vetor;
	};

	void setSolution() {
		solucaoEsparsa = vector<sparseVector>(numThreads);
		solucaoDensa = vector<par>(numThreads);
	};

	/* A 2ª matriz esparsa será armazenada diferente, para facilitar a multiplicação. */
	vector<sparseVector> getSparseMatrix(vector<denseVector> M, int numMatriz) {
		vector<sparseVector> sparseM;

		if(numMatriz == 1) {
			for(int i = 0; i < M.size(); i++) 
				sparseM.push_back(getSparseVector(M[i]));
			return sparseM;
		} else { // A 2ª matriz é armazenada de uma forma que os elementos de mesma coluna estão na mesma linha.
			for(int i = 0; i < M[0].size(); i++) {
				vector<par> sparseV;
				for(int j = 0; j < M.size(); j++)
					if(M[j][i] != 0)
						sparseV.push_back(par(j, M[j][i]));
				sparseM.push_back(sparseV);
			}
		}
	};

	vector<par> getSparseVector(vector<double> V) {
		vector<par> sparseV;
		for(int i = 0; i < V.size(); i++) 
			if(V[i] != 0)
				sparseV.push_back(par(i, V[i]));
		return sparseV;
	};

	void timesSparseMatrix(int numThread) {

		double soma = 0;
		
		for(int j = 0; j < matrizEsparsa_2.size(); j++) {
			soma = 0;
			for(int i = 0; i < matrizEsparsa_1[numThread].size(); i++) 
					for(int k = 0; k < matrizEsparsa_2[j].size(); k++) {
						if(matrizEsparsa_1[numThread][i].first == matrizEsparsa_2[j][k].first)
							soma += matrizEsparsa_1[numThread][i].second * matrizEsparsa_2[j][k].second;
					
					}
			if(soma != 0)
				solucaoEsparsa[numThread].push_back(par(j, soma));						
		}		
	};

	void timesDenseVector(int numThread) {

		double soma = 0;

		for(int i = 0; i < matrizEsparsa_1[numThread].size(); i++) 
			soma += matrizEsparsa_1[numThread][i].second * vetorDenso[matrizEsparsa_1[numThread][i].first];

		if(soma != 0)
			solucaoDensa[numThread] = par(0, soma);
	};

	void timesDenseMatrix(int numThread) {};

	void printSolution(int operacao) {
		cout << endl << "Solução:" << endl;
		int i, j;
		if(operacao == 1) {
			cout << "{ ";
			for(i = 0; i < solucaoDensa.size() - 1; i++)
				cout << "(" << solucaoDensa[i].first << ", " << solucaoDensa[i].second << ")" << ", ";
			cout << "(" << solucaoDensa[i].first << ", " << solucaoDensa[i].second << ")" << " }" << endl;
		} else {
			for(i = 0; i < solucaoEsparsa.size(); i++) {
				cout << "{ ";
				for(j = 0; j < solucaoEsparsa[i].size() - 1; j++)
					cout << "(" << solucaoEsparsa[i][j].first << ", " << solucaoEsparsa[i][j].second << ")" << ", ";
				cout << "(" << solucaoEsparsa[i][j].first << ", " << solucaoEsparsa[i][j].second << ")" << " }" << endl;
			}
		}
	};

	int getNumThreads() {
		return numThreads;
	};
		 
};

int main(int argc, char const *argv[]) {

	/* Atenção! O programa usa arquivos para sua entrada! 
	Modifique os arquivos x.in, onde x é a operação a ser realizada, para alterar a entrada. */

	cout << "1 - Matriz esparsa x Vetor denso" << endl;
	cout << "2 - Matriz esparsa x Matriz esparsa" << endl;
	cout << "3 - Matriz esparsa x Matriz densa" << endl;

	int operacao;
	cout << "Digite que operação você quer fazer: ";
	cin >> operacao;

	cout << "Abrindo arquivo " << operacao << ".in..." << endl;

	SparseMatrix M(operacao);

	/* Criação das threads. */
	int numThreads = M.getNumThreads();
	thread multiplicadores[numThreads];

	for(int i = 0; i < numThreads; i++)
		multiplicadores[i] = M.createThread(i, operacao);

	/* Sincronização das threads. */
	for(int i = 0; i < numThreads; i++)
		multiplicadores[i].join();	

	M.printSolution(operacao);
	
	return 0;
}