#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

// Función a ejecutar en el thread. Imprime el identificador del thread en el que está.
void *threadFunction(void *arg)
{
	struct timeval t1, t2;
	double time_taken;
	gettimeofday(&t1, NULL);

	printf("Hello, I am thread #%d.\n", *(int *)arg);
	volatile unsigned long long i;
	for (i = 0; i < 1000000000ULL; i++); // Operación random para que tarde el hilo un tiempo interesante para el ejemplo.
	//printf("Bye! Thread #%d with number.\n", *(int *)arg);
	gettimeofday(&t2, NULL);
	time_taken = (t2.tv_sec - t1.tv_sec)*1000;
	time_taken += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	printf("It took thread #%d %f miliseconds to execute \n", *(int *)arg, time_taken);
	return 0;
}
int main(void)
{
	pthread_t thread1, thread2, thread3, thread4; // Creación de los pthread_t identificadores para los 4 threads.
	int i = 1;
	int j = 2;
	int k = 3;
	int l = 4;
 // Creación de los 4 pthreads pasándoles como argumentos el identificador del thread, la función a ejecutar y el argumento para la función a ejecutar.
	if (pthread_create(&thread1, NULL, threadFunction, &i)!= 0){
		return 0;
	}
 	if (pthread_create(&thread2, NULL, threadFunction, &j)!= 0){
	 	return 0;
 	}
 	if (pthread_create(&thread3, NULL, threadFunction, &k)!= 0){
	 	return 0;
 	}
 	if (pthread_create(&thread4, NULL, threadFunction, &l)!= 0){
	 	return 0;
 	}
 // Espera a terminación de los 4 threads creados
 	if (pthread_join(thread1, NULL)!=0){
	 	return 0;
 	};
 	if (pthread_join(thread2, NULL)!=0){
		return 0;
 	};
 	if (pthread_join(thread3, NULL)!=0){
		return 0;
 	}
 	if (pthread_join(thread4, NULL)!=0){
		return 0;
 	}
 	return 0;
}
