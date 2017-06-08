#include <iostream>
#include <string>
#include <iomanip>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>

using namespace std;

vector<bool> arquivosLidos;
vector<mutex> mutexes;
vector<int> produtos;
int produtosLidos;

string fileName(int i) {
	char indice = i + 48;
	string str;
	str += indice;
	str += ".in";
	return str;
}

void contabilizarVenda(int produto) {
	mutexes[produto].lock();
	produtos[produto]++;
	mutexes[produto].unlock();
}

void abrirArquivo(int i) {

	/* Setando o nome do arquivo. */
	string nomeArquivo = fileName(i);
	
	/* Abrindo o arquivo. */
	fstream fs;
	fs.open(nomeArquivo);
	
	int produto;
	while(!fs.eof()) {
		fs >> produto;
		if(!fs.eof()) { // Impedir que ele conte um arquivo a mais.
			contabilizarVenda(produto);
			produtosLidos++;
		}	
	}

	fs.close(); 

}

void contar(int numThread) { 
	
	/* Por esse algoritmo, cada thread lerá o arquivo com x igual ao seu índice.
	   Inicialmente, se houver mais arquivos que threads, a última thread lerá o restante, porém, graças ao escalonador, há
	casos raros em que as threads lêem mais arquivos (mas nunca um arquivo já lido). */

	// cout << "Thread " << numThread << " aqui!" << endl;
	for(int i = numThread; i < arquivosLidos.size(); i = i + 1) {	
		if(arquivosLidos.at(i) == false) {
			arquivosLidos.at(i) = true;		
			abrirArquivo(i + 1);
		}
	}
}

int main() {

	produtosLidos = 0;

	int numThreads, numProdutos, numArquivos;
	cin >> numArquivos >> numThreads >> numProdutos;

	produtos = vector<int>(numProdutos, 0); // Array de produtos (contadores).
	mutexes = vector<mutex>(numProdutos); // Array de mutexes, um por produto.
	arquivosLidos = vector<bool>(numArquivos, 0); // Array de booleanos, um por arquivo.

	/* Inicialização das Threads. */
	thread threads[numThreads]; // Array de threads.
	for(int i = 0; i < numThreads; i++)
		threads[i] = thread(contar, i);

	/* Sincronização. */
	for(int i = 0; i < numThreads; i++)
		threads[i].join();

	/* Total de produtos lidos. */
	cout << produtosLidos << " produtos foram lidos no total." << endl;

	/* Porcentagem de venda para cada produto. */
	double porcentagem;
	for(int i = 0; i < numProdutos; i++) {
		porcentagem = (100 * produtos[i]) / (double) produtosLidos;
		cout << "Produto " << i << ": " << setprecision(3) << porcentagem << "% de venda." << endl;
	}


	return 0;
}
