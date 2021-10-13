#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // for pthread series
#include <unistd.h>  // for fork()
#include <signal.h>  // for signal()

#define TRUE 1
#define FALSE 0
#define N 5 // number of philosophers 
#define LEFT (i+N-1)%N // number of i's left neighbor 
#define RIGHT (i+1)%N  // number of i's right neighbor
#define THINKING 0 // philosopher is thinking
#define HUNGRY 1   // philosopher is trying to get forks
#define EATING 2   // philosopher is eating

typedef pthread_mutex_t semaphore; // semaphores are a special kind of int
int state[N];    // array to keep track of everyone's state
semaphore mutex; // mutual exclusion for critical regions
semaphore s[N];  // one semaphore per philosopher
pthread_t philosopher_thread[N];
int running;

void* philosopher(void* i);
void take_forks(int i);
void put_forks(int i);
void test(int);
void think();
void eat();
void program_stop();

int main()
{
	int* philosopher_num[N];
	running = TRUE;
	signal(SIGINT, program_stop); // Ctrl + C
	pthread_mutex_init(&mutex, NULL);
	int count = 0;

	for(int i = 0 ; i < N ; ++i)
	{
		philosopher_num[i] = malloc(sizeof(int));
		*philosopher_num[i] = i;
		pthread_mutex_init(&s[i], NULL);   // initialize the mutex
		pthread_mutex_lock(&s[i]);         // lock the mutex
		philosopher_thread[i] = i;
		pthread_create(&philosopher_thread[i], NULL, philosopher, (void*)philosopher_num[i]);
	}

	while(TRUE && running)
	{
		pthread_mutex_lock(&mutex);
		printf("======\x1b[;36;1;1m%d\x1b[0m======\n",count);
		for(int i = 0 ; i < N ; ++i)
		{
			if(state[i] == THINKING)
				printf("%d : THINKING\n", i);
			else if(state[i] == HUNGRY)
				printf("%d : HUNGRY\n", i);
			else
				printf("%d : EATING\n", i);
		}
		pthread_mutex_unlock(&mutex);
		sleep(1);
		++count;
	}

	for(int i = 0 ; i < N ; ++i)
	{
		free(philosopher_num[i]);
		pthread_join(philosopher_thread[i], NULL);
	}

	printf("\nAll philosopher STOP!!\n");

	return 0;
}


void* philosopher(void* i) // i: philosopher number, from 0 to N1
{
	int num = *( (int*)i );
	while (TRUE && running)
	{
		think();         // philosopher is thinking
		take_forks(num); // acquire two forks or block
		eat();           // yum-yum, spaghetti
		put_forks(num);  // put both forks back on table
	}
	
	printf("\n");
	printf("philosopher %d STOP!",num);

	return NULL;
}

void take_forks(int i) // i: philosopher number, from 0 to N1
{
	pthread_mutex_lock(&mutex); // enter critical region
	state[i] = HUNGRY; // record fact that philosopher i is hungry
	test(i);           // try to acquire 2 forks 
	pthread_mutex_unlock(&mutex); // exit critical region
	pthread_mutex_lock(&s[i]);    // block if forks were not acquired
}

void put_forks(int i) // i: philosopher number, from 0 to N1
{
	pthread_mutex_lock(&mutex); // enter critical region
	state[i] = THINKING; // philosopher has finished eating
	test(LEFT);  // see if left neighbor can eat now
	test(RIGHT); // see if right neighbor can now eat
	pthread_mutex_unlock(&mutex); // exit critical region
}

void test(int i) // i: philosopher number, from 0 to N1
{
	if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
	{
		state[i] = EATING;
		pthread_mutex_unlock(&s[i]);
	}
}

void think()
{
	sleep(1);
}

void eat()
{
	sleep(1);
}

void program_stop()
{
	running = FALSE;
}
