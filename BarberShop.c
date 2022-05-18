//TO COMPILE USE: gcc -o BarberShop BarberShop.c -lpthread

#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static int count = 0;  

struct job {
	int clientnum;
	struct job *next; 
};

struct args{
  int HaircutTime;
  int barberNum; 
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
	struct job* next_job;
	printf("Barber sleeping\n");
    sem_wait (&job_queue_count);
	printf("Barber %d wake up\n",arg->barberNum);
	pthread_mutex_lock (&job_queue_mutex);
  	while (1) {
		next_job = job_queue;
		/* remove da lista  */
		job_queue = job_queue->next;
		/* Libera a lista para o próximo thread  */
		pthread_mutex_unlock (&job_queue_mutex);
		printf("Barber %d is cutting client %d\n",arg->barberNum,next_job->clientnum);
		sleep(arg->HaircutTime);
		printf("Barber %d finished client %d\n",arg->barberNum,next_job->clientnum);
		free (next_job);
		pthread_mutex_lock (&job_queue_mutex);
		if(job_queue==NULL){
			pthread_mutex_unlock (&job_queue_mutex);
			printf("Barber %d is sleeping\n",arg->barberNum);
			sem_wait (&job_queue_count);
			printf("Barber %d wake up\n",arg->barberNum);
			pthread_mutex_lock (&job_queue_mutex);
		}
		
  }
  return NULL;
}

void* enqueue_job (void *p)
{
	int* QueueSize = (int*) p;
	struct job *new_job;
	struct job *aux;
	count++;           //Cliente chegou
	int queue=0;
    printf("Client %d enter the barber shop \n",count);
	/* Alocar o novo objeto de tarefa. */
	new_job = (struct job*) malloc (sizeof (struct job));
	new_job->clientnum=count;
	pthread_mutex_lock (&job_queue_mutex);
	/* Colocar a nova tarefa no fim da fila */
	aux=job_queue;
	if(aux!=NULL){
		while(aux->next!=NULL){
			queue++;
			aux=aux->next;
		}
		if(queue>=*QueueSize){
			printf("Client %d left without a haircut. Full waiting room.\n",count);
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
	int NUMbarbers,NUMchairs, cutS, clientS, NUMclients;
	pthread_t thread_idQueue;
	if(argc<6){
			printf("\t\tNo arguments\n \t\tHOW TO USE:\n ./program <Num. barbers> <Num. Max queue> <Cutting time> <Time between clients> <Number of clients\nSetting values 4 6 2 1 100\n");
			sleep(2);
			NUMbarbers=4;
			NUMchairs=6;
			cutS=2;
			clientS=1;
			NUMclients=100;
	}
	else{
		NUMbarbers=atoi(argv[1]);
		NUMchairs=atoi(argv[2]);
		cutS=atoi(argv[3]);
		clientS=atoi(argv[4]);
		NUMclients=atoi(argv[5]);
	}
	struct args *arg=(struct args *)malloc(NUMbarbers * sizeof(struct args));
	pthread_t *thread_id=(pthread_t *)malloc(NUMbarbers * sizeof(pthread_t));
	for(int i=0;i<NUMbarbers;i++){
		arg[i].HaircutTime=cutS;
		arg[i].barberNum=i+1;
		pthread_create(&thread_id[i],NULL,&thread_function,&arg[i]);
	}
	sleep(1);
	while(NUMclients>0){
		pthread_create(&thread_idQueue,NULL,&enqueue_job,&NUMchairs);
		sleep(clientS);
		NUMclients--;
	}
	while(job_queue!=NULL){ //Doesnt need to be atomic because you'll not change it
		sleep(1); //Wait all the clients go out
	}
	free(thread_id);
	free(arg);
}
