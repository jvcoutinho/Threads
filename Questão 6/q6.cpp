#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <queue>
#include <pthread.h>
using namespace std;

mutex mtx1;
condition_variable cv1;

mutex mtx2;
condition_variable cv2;

mutex mtx3;
condition_variable cv3;

pthread_barrier_t barrier;

class Controle {

private:
	vector<int> tamSensores;
	int numSensoresCheios;
	int menorSensor;

public:

	static queue<int> esteiras;
	static queue<int> sensores;
	static vector<int> bagagensSize;
	static vector<bool> esteiraRecebida;
	static vector<bool> bagagemEnviada;
	static vector<bool> sensoresBloqueados;
	static bool sinalManutencao;
	static bool fimdasBagagens;

	Controle(int numSensores) {
		tamSensores = vector<int>(numSensores, 0);
		numSensoresCheios = 0;
		menorSensor = 0;
	};
	
	thread createThread() {
		return thread([=] { controle(); });
	};

	void controle() {
		while(true) {
			requisicaodeEnvio(); /* Pega uma esteira da fila (se houver). */
			enviaparaMenorSensor(); /* Envia para o sensor de menor fila. */
			verificaSensor(); /* Verifica o estado do sensor para o qual acabou de enviar. */
			atualizaMenorSensor(); /* Atualiza o número do sensor de menor fila. */
			verificaSensores(); /* Verifica o estado geral de todos os sensores. */
			manutencao(); /* Faz a Manutenção, se necessário. */
		}
	};

	void requisicaodeEnvio() {

		while(esteiras.empty() && !fimdasBagagens);

		if(fimdasBagagens) {
			numSensoresCheios = tamSensores.size(); /* Manutenção final. */
			verificaSensores();
			manutencao();
			cout << "Aviso: Manutencao final dos sensores." << endl;
			exit(1);
		} else { /* Processar uma bagagem. */
			unique_lock<mutex> notificaEsteira(mtx1);
			esteiraRecebida[esteiras.front()] = true;
			esteiras.pop();
			cv1.notify_all();
		}
	}

	void enviaparaMenorSensor() {
		tamSensores[menorSensor]++;
		bagagemEnviada[menorSensor] = true;
		unique_lock<mutex> enviaBagagem(mtx2);
		cv2.notify_all();
	}

	void verificaSensor() {
		if(tamSensores[menorSensor] == 200) {
		//	cout << menorSensor << " bloqueado!" << endl;
			sensoresBloqueados[menorSensor] = true;
			numSensoresCheios++;
			if(bagagensSize[menorSensor] > 0) {
		//		cout << "Relocando!" << endl;
				relocacao();
			}
		}
	}

	void relocacao() {
		int menorBagagens = 200, indiceMenor = tamSensores.size() - 1;
		for(int i = 0; i < tamSensores.size(); i++)
			if(bagagensSize[i] < menorBagagens && sensoresBloqueados[i] != true && i != menorSensor) {
				menorBagagens = tamSensores[i];
				indiceMenor = i;
			}

		/* Comparação simbólica. Do jeito que foi implementado, esse condicional:
		- quase nunca ocorre porque não há relocação;
		- sempre ocorre quando há relocação. */
		if(bagagensSize[indiceMenor] + bagagensSize[menorSensor] <= 5)
			bagagensSize[indiceMenor] += bagagensSize[menorSensor];
	}

	void atualizaMenorSensor() {
		int indiceMenor = indiceMenorSensor();		
		menorSensor = indiceMenor;
	}

	int indiceMenorSensor() {
		int menorBagagens = 200, indiceMenor = tamSensores.size() - 1;
		for(int i = 0; i < tamSensores.size(); i++)
			if(bagagensSize[i] < menorBagagens && sensoresBloqueados[i] != true) {
				menorBagagens = tamSensores[i];
				indiceMenor = i;
			}
		return indiceMenor;
	};

	void verificaSensores() {
		if(numSensoresCheios == tamSensores.size()) {

			/* Manutenção. */
			unique_lock<mutex> manutencao(mtx2);
			sinalManutencao = true;
			cv2.notify_all();
			//this_thread::sleep_for(chrono::milliseconds(1500));
		}
	};

	void manutencao() {
		if(sinalManutencao) {

			while(sensores.size() != tamSensores.size()); /* Espera todos os sensores entrarem na fila. */

			/* Começa a retirada na fila. */
			for(int count = 0; count < tamSensores.size(); count++) {
				cout << "Sensor " << sensores.front() << ": Manutencao." << endl;
				sensores.pop();
			}	

			/* Zera os contadores e variáveis de estado. */
			for(int i = 0; i < tamSensores.size(); i++) {
				tamSensores[i] = 0;
				sensoresBloqueados[i] = false;
				bagagensSize[i] = 0;
			}
			numSensoresCheios = 0;

			/* Acorda os sensores. */
			cout << "Aviso: Todos os sensores receberam manutencao." << endl << endl;
			unique_lock<mutex> manutencaoFeita(mtx3);
			sinalManutencao = false;
			cv3.notify_all();
		
		}
	};
};

queue<int> Controle::esteiras;
queue<int> Controle::sensores;
vector<int> Controle::bagagensSize;
vector<bool> Controle::esteiraRecebida;
vector<bool> Controle::bagagemEnviada;
vector<bool> Controle::sensoresBloqueados;
bool Controle::sinalManutencao;
bool Controle::fimdasBagagens = false;



class Sensor {

private:
	queue<int> bagagens;
	int numSensor;

public:

	thread createThread() {
		return thread([=] { sensor(); });
	};

	void setNumSensor(int numSensor) {
		this->numSensor = numSensor;
	};

	void sensor() {
		while(true) {
			unique_lock<mutex> waitControle(mtx2);
				while(Controle::bagagemEnviada[numSensor] == false && Controle::sinalManutencao == false)
					cv2.wait(waitControle);

				if(Controle::sinalManutencao == true) {

					waitControle.unlock();
					manutencao(); /* Manutenção: coloca-se numa fila. */
					esperaControle(); /* Espera o controle se manifestar (quando a manutenção for finalizada). */
					
			} else {

				Controle::bagagemEnviada[numSensor] = false;
				bagagens.push(0); // Push simbólico de bagagem.
				
				Controle::bagagensSize[numSensor]++;
				/* Essa parte do código foi comentada porque 0,5 s por bagagem é muito tempo. */
				//this_thread::sleep_for(chrono::milliseconds(500));
				
				/* A questão foi nada clara sobre como as bagagens saem da fila.
				Consideremos que sempre que são analisadas, saem. Isso tem um efeito colateral na relocação, porém. */
				bagagens.pop();
				Controle::bagagensSize[numSensor]--;
			}
			
		}
		
	};

	void manutencao() {
		Controle::sensores.push(numSensor);
		//cout << "Sensor " << numSensor << " preparado para manutenção!" << endl;
	};

	void esperaControle() {
		unique_lock<mutex> manutencaoTerminada(mtx3);
		while(Controle::sinalManutencao)
			cv3.wait(manutencaoTerminada);	
	};

};

class Esteira {

private:
	int numBagagens;
	int numEsteira;

public:

	Esteira(int numBagagens) {
		this->numBagagens = numBagagens;
	};

	thread createThread() {
		return thread([=] { esteira(); });
	};

	void esteira() {

		while(numBagagens > 0) {

			enviaBagagem(); /* Envio: coloca a esteira em uma fila. */
			esperaControle(); /* Espera ser notificada pelo controle que sua bagagem foi entregue. */
		}

		/* Fim das bagagens: uma vez que todas as esteiras terminaram, o controle iniciará a manutenção final. */
		pthread_barrier_wait(&barrier); 
		Controle::fimdasBagagens = true;
	};

	void enviaBagagem() {
		Controle::esteiras.push(numEsteira);
		numBagagens--;
	};

	void esperaControle() {
		unique_lock<mutex> esperaControle(mtx1);
		while(Controle::esteiraRecebida[numEsteira] != true)
			cv1.wait(esperaControle);

		Controle::esteiraRecebida[numEsteira] = false;
	};

	void setNumEsteira(int numEsteira) {
		this->numEsteira = numEsteira;
	};

		
};

int main(int argc, char const *argv[]) {
	
	int numEsteiras, numSensores, numBagagens;
	cout << "Digite o número de esteiras, de sensores e de bagagens por esteira:" << endl;
	cin >> numEsteiras >> numSensores >> numBagagens;

	Esteira E[numEsteiras](numBagagens);
	Sensor S[numSensores];
	Controle C(numSensores);

	Controle::esteiraRecebida = vector<bool>(numEsteiras, false);
	Controle::bagagemEnviada = vector<bool>(numSensores, false);
	Controle::sensoresBloqueados = vector<bool>(numSensores, false);
	Controle::sinalManutencao = false;
	Controle::bagagensSize = vector<int>(numSensores, 0);

	for(int i = 0; i < numSensores; i++)
		S[i].setNumSensor(i);

	for(int i = 0; i < numEsteiras; i++)
		E[i].setNumEsteira(i);

	/* Inicialização da barreira. */
	pthread_barrier_init(&barrier, NULL, numEsteiras);

	/* Criação das threads. */
	thread esteiras[numEsteiras];
	thread sensores[numSensores];
	thread controle;

	for(int i = 0; i < numEsteiras; i++)
		esteiras[i] = E[i].createThread(); 

	for(int i = 0; i < numSensores; i++)
		sensores[i] = S[i].createThread();

	controle = C.createThread();

	/* Sincronização das threads. */
	for(int i = 0; i < numEsteiras; i++)
		esteiras[i].join();

	for(int i = 0; i < numSensores; i++)
		sensores[i].join();

	controle.join();

	pthread_barrier_destroy(&barrier);

	return 0;
}