#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Variaveis do ambiente:
#define P 5
#define C 5
#define B 11

#define MAX 40

typedef struct elem{
   int value;
   struct elem *next;
}elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   elem *head,*last;
}blockingQueue;

pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;
int buffer = B;
blockingQueue Q;

/*	- newblockingQueue: cria uma nova fila Bloqueante do tamanho do valor passado.
*   - putblockingQueue: insere um elemento no final da fila bloqueante Q, bloqueando a
* thread que está inserindo, caso a fila esteja cheia.
*	- takeblockingQueue: retira o primeiro elemento da fila bloqueante Q, bloqueando a thread
* que está retirando, caso a fila esteja vazia.
 */

elem* newElem(){
	return malloc(1*sizeof(elem));
}

void newBlockingQueue(int sz){
	int i;

	if(sz>B){
		sz = B;
	}
	
	Q.sizeBuffer = B;
	Q.statusBuffer = sz;

	Q.head = newElem();
	elem* aux = Q.head;

	for(i=0; i<sz; i++){
		aux->value = i+1;
		if(i!=sz-1){
			aux->next = newElem();
			aux=aux->next;
		}
	}
	
	aux->next = NULL;
	Q.last = aux;
}

void putBlockingQueue(int newValue, int number){
	if(Q.statusBuffer==0){
		Q.last = newElem();
		Q.head = Q.last;
	}else{
		Q.last->next = newElem();
		Q.last = Q.last->next;
	}
	
	Q.last->value = newValue;
	Q.last->next = NULL;
	printf("Elemento Adicionado [Produtor %d]: %d\n", number, Q.last->value);
	
	Q.statusBuffer++;
}

int takeBlockingQueue(int number){	
	printf("Elemento Retirado [Consumidor %d]: %d\n", number, Q.head->value);
	Q.head = Q.head->next;
	Q.statusBuffer--;
	if(Q.statusBuffer==0){
		Q.last == NULL;
	}
}


void *producer(void* t) {
	int i = B;
	int number = (int) t;
	while(1){
		i++;
		pthread_mutex_lock(&the_mutex);	
		while (Q.statusBuffer != 0)		      
		  	pthread_cond_wait(&condp, &the_mutex);
		putBlockingQueue(i, number);
		pthread_cond_signal(&condc);	
		pthread_mutex_unlock(&the_mutex);	
	}
}

void *consumer(void* i) {
	int number = (int) i;
	while(1){
		pthread_mutex_lock(&the_mutex);	
		while (Q.statusBuffer == 0)			
		  pthread_cond_wait(&condc, &the_mutex);
		takeBlockingQueue(number);
		pthread_cond_signal(&condp);	
		pthread_mutex_unlock(&the_mutex);	
	}
}

int main() {
	newBlockingQueue(10);
	pthread_t pro[P], con[C];
	long i;

	// Initialize the mutex and condition variables
	pthread_mutex_init(&the_mutex, NULL);	
	pthread_cond_init(&condc, NULL);		/* Initialize consumer condition variable */
	pthread_cond_init(&condp, NULL);		/* Initialize producer condition variable */

	// Create the threads
	for(i=0; i<P; i++){
		printf("Criando Produtores...\n");
		pthread_create(&pro[i], NULL, producer, (void*) i);
	}

	for(i=0; i<C; i++){
		printf("Criando Consumidores...\n");
		pthread_create(&con[i], NULL, consumer, (void*) i);
	}

	for(i=0; i<P; i++){
		printf("Esperando Produtores...\n");
		pthread_join(pro[i], NULL);
	}

	for(i=0; i<C; i++){
		printf("Esperando Consumidores...\n");
		pthread_join(con[i], NULL);
	}
	
	pthread_mutex_destroy(&the_mutex);	
	pthread_cond_destroy(&condc);		
	pthread_cond_destroy(&condp);		

}