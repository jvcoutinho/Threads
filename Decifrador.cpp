#include <iostream>
#include <thread>
#include <string>

using namespace std;

string senhaDecifrada(10, 0);

void decifrar(string senha, int n) {

	for(int i = 33; i <= 126; i++)
		if(i == senha[n]) {
			senhaDecifrada[n] = i;
			break;
		}
}

void decifrarRestante(thread *thread1, string senha, int numThreads) {

	for(int i = numThreads - 1; i < 10; i++) {
		*thread1 = thread(decifrar, senha, i);
		thread1->join();
	}
} 

int main() {

	int numThreads; 
	cout << "Digite quantas threads dever�o ser usadas para decifrar: ";
	cin >> numThreads;

	thread threads[numThreads];

	cout << "Decifrando!" << endl;

	/* Declara��o da senha e inicializa��o das threads. */
	string senha = "HelLOWoRlD";
	for(int i = 0; i < numThreads; i++)
		threads[i] = thread(decifrar, senha, i);

	/* Sincroniza��o das threads. */
	for(int i = 0; i < numThreads; i++)
		threads[i].join();

	/* Caso haja menos threads que caracteres, o programa usar� a 1� thread para decifrar o restante. */	
	if(numThreads < 10)
		decifrarRestante(&threads[0], senha, numThreads);		

	cout << "A senha �: " << senhaDecifrada << endl;

	return 0;
}