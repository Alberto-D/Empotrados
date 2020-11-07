#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>

#define TIME_WAIT_NS 1000000
#define MAX_LATENCIES 60000 //if i sleep 1 milisecond and the latency is 0, then the maximum number of posible latencies is this


//Whit this function you can see if the policy and the parameters are correct
static void
display_sched_attr(int policy, struct sched_param *param)
{
    printf("    policy=%s, priority=%d\n",
            (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
            (policy == SCHED_RR)    ? "SCHED_RR" :
            (policy == SCHED_OTHER) ? "SCHED_OTHER" :
            "???",
            param->sched_priority);
	

}
//I read the array of latencies and calculate de average (i use long and long long in case the latencies are too high)
// if a latency is 0 or less, the rest of the array are 0 and i stop adding them and save the number of latencies in c to make the average
void calculate_mid_max(long latencies[], long *mid){
	long maximum = 0;
	long long medium = 0;
	int c = 0;
	for (int i = 0 ; i< MAX_LATENCIES; i++){
		if(latencies[i]<=0){
			break;
		}
		c++;
		medium += latencies[i];

		if(latencies[i]>maximum){
			maximum = latencies[i];
		}
		
	}
	*mid = medium/c;
}



//I take tame and meuse every cicle to check if one minute has passed 
//every cicle i algo measure the latency and save in an array which i will pass to a function to demerminate the avegare and the maximum, then i print themm
void *thread_client(void *i){
	int newi = *((int *) i);
	////Uncomment this lines to check the policy and parameters
	//int policy;
    //struct sched_param param;
	//pthread_getschedparam(pthread_self(), &policy, &param);
    //display_sched_attr(policy, &param);

	struct timeval t1, t2;
	long double time_taken = 0.0;
	long  mid = 0;
	struct timespec tin, tfin, deadline;
	long latencies[MAX_LATENCIES] = {0};
	int counter = 0;
	deadline.tv_sec = 0;
	deadline.tv_nsec = TIME_WAIT_NS;
	long maximum =0, temporal =0;

	if (gettimeofday(&t1, NULL)!= 0){
			perror("Error in get time of the day");
		}
	while(time_taken < 60){
		
		if(clock_gettime(CLOCK_MONOTONIC,&tin) < 0){
			perror("Error in gettime");
		}
		if( clock_nanosleep(CLOCK_MONOTONIC,0,&deadline,NULL)!= 0){
			perror("Error in nanosleep");
		}

		if(clock_gettime(CLOCK_MONOTONIC,&tfin)<0){
			perror("Error in gettime");
		}
		temporal = (tfin.tv_nsec-tin.tv_nsec - TIME_WAIT_NS);
		latencies[counter] = temporal;
		counter++;

		if(temporal>maximum){
			maximum = temporal;
		}
		
		if (gettimeofday(&t2, NULL)!= 0){
			perror("Error in get time of the day");
		}
		time_taken = (t2.tv_sec - t1.tv_sec)*1000;
		time_taken += (t2.tv_usec - t1.tv_usec) / 1000.0;
		time_taken = time_taken/1000;		
	}
	
	calculate_mid_max(latencies, &mid);
	printf("[%d]	latencia media =   %09ld ns. | max = %09ld ns\n", newi, mid, maximum);

	free(i);
  return 0;
}

//I configure the parameres and the cpu, bind each thread to a core and lauch all of them, then i join them, if an error hapens, i notifiy it 
int main(void)
{

	int latency_target_value = 0;
	int latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
	int err = write(latency_target_fd, &latency_target_value, 4);
	if(err<0){
		perror("Error in write");
	}
	struct sched_param param;
	param.sched_priority= 99;
	int N = (int) sysconf(_SC_NPROCESSORS_ONLN);
	if(N<0){
		perror("Error in sysconf");
	}
	pthread_t thread[N];
	cpu_set_t cpuset;

	for (int i =0; i<N;i++){
		int *number = malloc(sizeof(*number));
		*number = i;					//I use malloc so that every for will have its own "i" and they dont overwite it
		thread[i]= pthread_self();
		CPU_ZERO(&cpuset);

		CPU_SET(i, &cpuset);
		if(pthread_setaffinity_np(thread[i], sizeof(cpu_set_t), &cpuset)!=0){
			perror("En set afiniti");
		}
		if (pthread_setschedparam(thread[i], SCHED_FIFO, &param) !=0){
			perror("Error en setparam");
		}
		
		if(pthread_create(&thread[i], NULL, thread_client, (void *)number) != 0){
			perror("Thread error");
		}
		
	}

	for (int b =0; b<N;b++){
		if ( pthread_join(thread[b], NULL)!= 0){
			perror("Error joining");
		}
	}

 return 0;
}
