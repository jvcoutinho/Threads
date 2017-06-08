#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <cstdlib>
using namespace std;

/* Esta classe contém todos os mutexes, váriaveis de condição e variáveis compartilhadas do programa. */
class CriticalRegions { 

protected:
	static queue<int> buffer;
	static queue<int> requests;
	static int serverAnswer;
	static bool pageReady;
	static bool pageReceived;
	static mutex bufferMtx;
	static mutex requestMtx;
	static mutex receiveMtx;
	static mutex clientMtx;
	static condition_variable not_full;
	static condition_variable not_empty;
	static condition_variable no_request;
	static condition_variable not_ready;
	static condition_variable not_received;
};

class Servidor : public CriticalRegions {

private:
	thread servidor;
	
public:
	Servidor() {
		servidor = thread(server);
	};

	void syncronize() {
		servidor.join();
	};

	static void server();
	static void fillBuffer();
	static void waitforRequest();
	static void waitforClient();
};

class Cliente : public CriticalRegions {

private:
	vector<thread> clientes;

public:
	Cliente(int numClientes) {
		clientes = vector<thread>(numClientes);
		for(int i = 0; i < numClientes; i++)
			clientes[i] = thread(client, i);
	};
	void syncronize() {
		for(int i = 0; i < clientes.size(); i++)
			clientes[i].join();
	};

	static void client(int numCliente);
	static int fetchPage();
	static void request(int numCliente);
	static void returnAnswer();

};

/* Inicialização das variáveis compartilhadas: */
queue<int> CriticalRegions::buffer;
queue<int> CriticalRegions::requests;
int CriticalRegions::serverAnswer;
bool CriticalRegions::pageReady = false;
bool CriticalRegions::pageReceived = false;
mutex CriticalRegions::bufferMtx;
mutex CriticalRegions::requestMtx;
mutex CriticalRegions::receiveMtx;
mutex CriticalRegions::clientMtx;
condition_variable CriticalRegions::not_full;
condition_variable CriticalRegions::not_empty;
condition_variable CriticalRegions::no_request;
condition_variable CriticalRegions::not_ready;
condition_variable CriticalRegions::not_received;

/* SERVIDOR */
void Servidor::fillBuffer() {

	unique_lock<mutex> bufferLock(bufferMtx);

	while(buffer.size() == 5) {
		not_full.wait(bufferLock);
	}

	int valor = rand() % 40;
	buffer.push(valor);

	not_empty.notify_all();
}

void Servidor::waitforRequest() {

	unique_lock<mutex> waitClientLock(requestMtx);
	while(requests.size() == 0)
		no_request.wait(waitClientLock);

	serverAnswer = buffer.front();
	//buffer.pop();

	pageReady = true;
	not_ready.notify_one();
}

void Servidor::waitforClient() {

	unique_lock<mutex> answerLock(receiveMtx);
	while(!pageReceived)
		not_received.wait(answerLock);

	buffer.pop();
	pageReceived = false;
	pageReady = false;
}


void Servidor::server() {

	/* O servidor está em laço infinito. Assim que os clientes pararem as requisições, o servidor ficará bloqueado. */
	for(int i = 0; true; i++) {

		// O servidor põe uma página no buffer a cada iteração, se não estiver cheio;
		fillBuffer();
		// possui a implementação padrão: é sensível a requisições de clientes;
		waitforRequest();
		// e espera pela resposta do cliente para apagar o elemento do buffer.
		waitforClient();

		/* A latência entre o servidor e o cliente altera o modo como o programa executa:
		- latência alta causa maior aleatoriedade nas requisições, mas pode fazer com que clientes recebam a mesma página.
		- latência baixa causa menor aleatoriedade, mas páginas diferentes são garantidas. */
		this_thread::sleep_for(chrono::milliseconds(20));
	}
}

/* CLIENTE */
int Cliente::fetchPage() {

	unique_lock<mutex> readyLock(requestMtx);
	while(pageReady == 0) {
		not_ready.wait(readyLock);
	}

	not_full.notify_all();

	return serverAnswer;
}

void Cliente::request(int numCliente) {

	unique_lock<mutex> bufferLock(bufferMtx);

	while(buffer.size() == 0) {
		not_empty.wait(bufferLock);
	}

	requests.push(numCliente);
	no_request.notify_all();

}

void Cliente::returnAnswer() {

	unique_lock<mutex> answerLock(receiveMtx);
	requests.pop();
	pageReceived = true;
	not_received.notify_all();
}

void Cliente::client(int numCliente) {

	for(int i = 0; true; i++) {

		
		/* O cliente requisita uma página. Caso o buffer esteja vazio, ele esperará.
		Sua implementação permite que mais de um cliente requisite uma página por vez. Desse modo, são organizados em uma fila. */
		request(numCliente);
		// com a página requisitada, recebe-a do servidor;
		int pagina = fetchPage();
		// e envia sua resposta ao servidor.		
		returnAnswer();
		clientMtx.lock();
		cout << "CLIENTE " << numCliente << " recebeu página " << pagina << '.' << endl;
		clientMtx.unlock();

		this_thread::sleep_for(chrono::milliseconds(100));
		
	}
	
}

int main(int argc, char const *argv[]) {

	int numClientes;
	cout << "Digite o número de clientes: ";
	cin >> numClientes;

	srand(time(NULL));

	Servidor S;
	Cliente C(numClientes);
	S.syncronize();
	C.syncronize();

	return 0;
}
