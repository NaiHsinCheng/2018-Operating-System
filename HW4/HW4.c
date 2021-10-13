#include "mm.h"

void print_array(int *array_memory, int num)
{
	printf("Address : %p\n",array_memory);	
	for(int i = 0 ; i < num ; ++i)
		printf("%3d ",array_memory[i]);
	printf("\n");
}

void test_mymalloc(int num)
{
	printf("test mymalloc\n");
	printf("============================\n");

	int *array_memory = (int *)mymalloc(sizeof(int) * num);

	for(int i = 0 ; i < num ; ++i)
		array_memory[i] = rand() % 50;
		
	print_array(array_memory, num);
	
	printf("============================\n\n");
	
	myfree(array_memory);
	
}


void test_mycalloc(int num)
{
	printf("test mycalloc\n");
	printf("============================\n");
	
	int *array_memory = (int *)mycalloc(num, sizeof(int));

	print_array(array_memory,num);

	for(int i = 0 ; i < num ; i++)
			array_memory[i] = rand() % 50;

	print_array(array_memory,num);
	
	printf("============================\n\n");
	
	myfree(array_memory);
	
}


void test_myrealloc(int num)
{
	printf("test myrealloc\n");
	printf("============================\n");

	int *array_memory = (int *)mymalloc(sizeof(int) * num);
	
	for(int i = 0 ; i < num ; ++i)
		array_memory[i] = rand() % 50;
		
	print_array(array_memory,num);
	
	array_memory = (int *)myrealloc(array_memory, sizeof(int) * (num + 5));	
	print_array(array_memory,num + 5);
	
	array_memory = (int *)myrealloc(array_memory, sizeof(int) * (num - 5));
	print_array(array_memory,num - 5);
		
		
		
	printf("============================\n");
	
	myfree(array_memory);
	
}

int main()
{
	srand(time(NULL));
	test_mymalloc(10);
	test_mycalloc(10);
	test_myrealloc(10);
	
	return 0;
}
