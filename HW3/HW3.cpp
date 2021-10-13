#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <pthread.h> // for pthread series
#include <unistd.h>  // for fork()
#include <signal.h>  // for signal()
using namespace std;

#define TRUE 1
#define FALSE 0
#define N 5            // number of philosophers 
#define LEFT (i+N-1)%N // number of i's left neighbor 
#define RIGHT (i+1)%N  // number of i's right neighbor
#define THINKING 0     // philosopher is thinking
#define HUNGRY 1       // philosopher is trying to get forks
#define EATING 2       // philosopher is eating

void* philosopher(void* i);
void program_stop(int i);
int  running;
int count = 0;

class Philosopher_Monitor{
private:
	int state[N];           // array to keep track of everyone's state
	pthread_mutex_t mutex;  // mutual exclusion for critical regions
	pthread_cond_t s[N];    // one semaphore per philosopher
public:
	Philosopher_Monitor()
	{
		pthread_mutex_init(&mutex,NULL);
		for(int i = 0 ; i < N ; ++i)
			pthread_cond_init(&s[i],NULL);
	}
	void take_forks(int i);
	void put_forks(int i);
	void test(int i);
	void think();
	void eat();
	void show();
};
	
Philosopher_Monitor monitor;

int main()
{
	pthread_t philosopher_thread[N];
	int* philosopher_num[N];
	running = TRUE;
	signal(SIGINT, program_stop); // Ctrl + C

	for(int i = 0 ; i < N ; ++i)
	{
		philosopher_num[i] = (int*)malloc(sizeof(int));
		*philosopher_num[i] = i;
		philosopher_thread[i] = i;
		pthread_create(&philosopher_thread[i], NULL, philosopher, (void*)philosopher_num[i]);
	}

	while(TRUE && running)
	{
		monitor.show();
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
		monitor.think();         // philosopher is thinking
		monitor.take_forks(num); // acquire two forks or block
		monitor.eat();           // yum-yum, spaghetti
		monitor.put_forks(num);  // put both forks back on table
	}
	
	printf("\n");
	printf("philosopher %d STOP!",num);

	return NULL;
}

void program_stop(int i)
{
	running = FALSE;
}

void Philosopher_Monitor::take_forks(int i) // i: philosopher number, from 0 to N1
{
	pthread_mutex_lock(&mutex);   // enter critical region
	state[i] = HUNGRY;            // record fact that philosopher i is hungry
	test(i);                      // try to acquire 2 forks 
	pthread_mutex_unlock(&mutex); // exit critical region
}

void Philosopher_Monitor::put_forks(int i) // i: philosopher number, from 0 to N1
{
	pthread_mutex_lock(&mutex);     // enter critical region
	state[i] = THINKING;            // philosopher has finished eating
	pthread_cond_signal(&s[LEFT]);
	pthread_cond_signal(&s[RIGHT]);
	pthread_mutex_unlock(&mutex); // exit critical region
}

void Philosopher_Monitor::test(int i) // i: philosopher number, from 0 to N1
{
	while(TRUE && running)
	{
		if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
		{
			state[i] = EATING;
			break;
		}
		else
			pthread_cond_wait(&s[i],&mutex);
	}
}

void Philosopher_Monitor::think()
{
	sleep(rand()%3+1);
}

void Philosopher_Monitor::eat()
{
	sleep(rand()%3+1);
}

void Philosopher_Monitor::show()
{
	pthread_mutex_lock(&mutex);
	printf("======\x1b[;36;1;1m%d\x1b[0m======\n",count);
	for(int i = 0 ; i < N ; ++i)
	{
		if(state[i] == THINKING)
			printf("%d : Thinking\n", i);
		else if(state[i] == HUNGRY)
			printf("%d : Hungry\n", i);
		else
			printf("%d : Eating\n", i);
	}
	pthread_mutex_unlock(&mutex);	
}
