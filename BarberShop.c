//TO COMPILE USE: gcc -o BarberShop BarberShop.c -lpthread

#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static int count = 0;  

struct job {
	int clientenum;
	struct job *next; 
};

struct args{
  int tempoCorte;
  int barbeiroNum; 
};

struct job* job_queue;

pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t job_queue_count;


void initialize_job_queue ()
{
  job_queue = NULL;
  sem_init (&job_queue_count, 0, 0);
}


void* thread_function (void *p)
{
	struct args* arg = (struct args*) p;
	printf("Barbeiro dormindo\n");
    sem_wait (&job_queue_count);
	printf("Barbeiro %d acordou\n",arg->barbeiroNum);
  	while (1) {
		struct job* next_job;
		int test=0;
		pthread_mutex_lock (&job_queue_mutex);
		next_job = job_queue;
		/* remove da lista  */
		job_queue = job_queue->next;
		/* Libera a lista para o próximo thread  */
		pthread_mutex_unlock (&job_queue_mutex);
		printf("Barber %d is cutting client %d\n",arg->barbeiroNum,next_job->clientenum);
		sleep(arg->tempoCorte);
		printf("Barber %d finished client %d\n",arg->barbeiroNum,next_job->clientenum);
		free (next_job);
		pthread_mutex_lock (&job_queue_mutex);
		if(job_queue==NULL){
			pthread_mutex_unlock (&job_queue_mutex);
			printf("Barber %d is sleeping\n",arg->barbeiroNum);
			sem_wait (&job_queue_count);
			printf("Barbeiro %d wake up\n",arg->barbeiroNum);
		}
		else{
			pthread_mutex_unlock (&job_queue_mutex);
		}
		
  }
  return NULL;
}

void* enqueue_job (void *p)
{
	int* tamanhoFila = (int*) p;
	struct job *new_job;
	struct job *aux;
	count++;           //Cliente chegou
	int fila=0;
    printf("Client %d enter the barber shop \n",count);
	/* Alocar o novo objeto de tarefa. */
	new_job = (struct job*) malloc (sizeof (struct job));
	new_job->clientenum=count;
	pthread_mutex_lock (&job_queue_mutex);
	/* Colocar a nova tarefa no fim da fila */
	aux=job_queue;
	if(aux!=NULL){
		while(aux->next!=NULL){
			fila++;
			aux=aux->next;
		}
		if(fila>=*tamanhoFila){
			printf("Client %d foi embora sem cortar o cabelo. Sala de espera cheia.\n",count);
			sem_post (&job_queue_count);
			pthread_mutex_unlock (&job_queue_mutex);
			free(new_job);
			return NULL;
		}
		new_job->next = NULL;
		aux->next=new_job;
		/* Postar para o semáforo para indicar que outra tarefa está disponível. Se threads estão
		bloqueadas, esperando o semáforo, uma será desbloqueada e processará a tarefa. */
		sem_post (&job_queue_count);
		/* Destravar a mutex da fila de tarefas. */
		pthread_mutex_unlock (&job_queue_mutex);
	}
	else{
		new_job->next = NULL;
		job_queue = new_job;
		sem_post (&job_queue_count);
		/* Destravar a mutex da fila de tarefas. */
		pthread_mutex_unlock (&job_queue_mutex);
	}
}


int main (int argc, char* argv[]){  
	initialize_job_queue ();
	int NUMbarbeiros,NUMcadeiras, corteS, clienteS;
	pthread_t thread_idFila;
	if(argc<5){
			printf("\t\tFaltam argumentos\n \t\tCOMO USAR:\n ./program <Num. barbeiros> <Num. cadeiras> <Duracao do corte> <tempode chegada clientes>\nDefinindo valor padrao 4 6 2 1\n");
			sleep(2);
			NUMbarbeiros=4;
			NUMcadeiras=6;
			corteS=2;
			clienteS=1;
	}
	else{
		NUMbarbeiros=atoi(argv[1]);
		NUMcadeiras=atoi(argv[2]);
		corteS=atoi(argv[3]);
		clienteS=atoi(argv[4]);		
	}
	struct args arg[NUMbarbeiros];
	pthread_t thread_id[NUMbarbeiros];
	for(int i=0;i<NUMbarbeiros;i++){
		arg[i].tempoCorte=corteS;
		arg[i].barbeiroNum=i+1;
		pthread_create(&thread_id[i],NULL,&thread_function,&arg[i]);
	}
	sleep(1);
	while(1){
		pthread_create(&thread_idFila,NULL,&enqueue_job,&NUMcadeiras);
		sleep(clienteS);
	}
}
