/*
 * Justin Penny 11126992
 * Operating Systems 4600
 * 
 * Program 1
 * 
 * TA Student Problem
 * 
 * This program deals with concurrency. Having student thrads wait their turn to be helped by a TA thread. 
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <pthread.h> 
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#define NUM_SEAT 3

#define SLEEP_MAX 5

#define HELP_MAX 3

#define MAX_HELP 3

#define MIN_STUDENTS 5

// Sematphores
sem_t sem_stu;
sem_t sem_ta;

// Sem_t mutex;
pthread_mutex_t mutex;

// Threads
pthread_t *students;
pthread_t ta;

int chair[3];
int count = 0;
int next_seat = 0;
int next_teach = 0;
int help_count[5];
int help_check[5];

int rand_sleep();
void* stu_programming(void* stu_id);
void* ta_teaching();
int students_waiting();
int finished_help();

int main(int argc, char **argv){
	
	int* student_ids;
	int student_num = 0;
	int i;

    // Getting number of students
    while(student_num < 5){
        printf("\nEnter number of students:");
        scanf("%d", &student_num);
        
        if(student_num < 5){
            printf("\nERROR: number of students must be <= 5");
        }
    }
    printf("\n");

	// Initialize
	students = (pthread_t*)malloc(sizeof(pthread_t) * student_num);
	student_ids = (int*)malloc(sizeof(int) * student_num);

	memset(student_ids, 0, student_num);

	sem_init(&sem_stu,0,0);
	sem_init(&sem_ta,0,1);
	
	// Seeding random
	srand(time(NULL));

	// sem_init(&mutex,0,1);
	pthread_mutex_init(&mutex,NULL);

	// Creating TA thread
	pthread_create(&ta,NULL,ta_teaching,NULL);

	// Creating n student threads
	for(i=0; i<student_num; i++)
	{
		student_ids[i] = i+1;
		pthread_create(&students[i], NULL, stu_programming, (void*) &student_ids[i]);
	} 

	pthread_join(ta, NULL);
	
	for(i=0; i<student_num;i++)
	{
		pthread_join(students[i],NULL);
	}	
	
	return 0;
}

void* stu_programming(void* stu_id)
{
	int id = *(int*)stu_id;
    
	printf("\nStudent %d is programming",id);		
	
	while(1)
	{
        
        // If a student has been helped 3 times, remove that thread id
        if(help_count[id-1] == 3){
            printf("\nStudent %d has been helped 3 times and is going home", id);
            help_check[id-1] = 1;
            
            // Once all students have been helped, remove the TA thread
            if(finished_help() == 1){
                printf("\n\n\n\nAll students have been helped 3 times.\n\n\n\n");
                pthread_exit(&ta);
            }
            
            pthread_exit(&students[id-1]);
        }
        
        int wait_time;
        
		wait_time = rand_sleep();
        
        // This is the time that the student will wait before trying to go get a seat
        sleep(wait_time);

		// Locking mutex
		pthread_mutex_lock(&mutex);

		if(count < NUM_SEAT)	
		{
            
			chair[next_seat] = id;
			count++;
			
            // Tracking number of times a studend has been helped
            help_count[id-1]++;
            
			printf("\nStudent %d is waiting\n",id);
            
            printf("\nStudent %d spent %d second(s) programming", id, wait_time);
    
			next_seat = (next_seat+1) % NUM_SEAT;
			
			// Unlocking mutex
			pthread_mutex_unlock(&mutex);
	
			// Calling for the TA
			sem_post(&sem_stu);
			sem_wait(&sem_ta);
		}
		else
		{
			pthread_mutex_unlock(&mutex);
			
			printf("\nStudent %d could not find a seat and is programming again\n",id);		
		}
	}				
}

void* ta_teaching()
{
	while(1)
	{
        int wait_time;
        int num_waiting;
        
		sem_wait(&sem_stu);	
		
		// Locking mutex
		pthread_mutex_lock(&mutex);
		
		printf("\nTA is teaching student %d",chair[next_teach]);
        
        wait_time = rand_sleep();
        sleep(wait_time);
        
		chair[next_teach]=0;
		count--;
        
        num_waiting = students_waiting();
		
		printf("\nNumber of students waiting: %d\nWaiting student ids : (%d) (%d) (%d)",num_waiting, chair[0],chair[1],chair[2]);
		next_teach = (next_teach + 1) % NUM_SEAT;

		printf("\nTA Finished teaching in %d seconds\n", wait_time);	

        // Unlocking mutex
		pthread_mutex_unlock(&mutex);

		sem_post(&sem_ta);
        
        // Ending the program if all students have been helped
        if(finished_help() == 1){
                exit(0);
        }
	}	
}

// Generating a random sleep time
int rand_sleep(){
	int time = rand() % SLEEP_MAX + 1;
	return time;
}


// Printing the number of students currently waiting for help from the TA
int students_waiting(){
    int num = 0;
    
    if (chair[0] != 0){
        num++;
    }
    if (chair[1] != 0){
        num++;
    }
    if (chair[2] != 0){
        num++;
    }
    
    return num;
}

// Checking if all of the studends have been helped
int finished_help(){
    int check = 0;
    for(int i = 0; i < 5; i++){
        if(help_check[i] == 1){
            check++;
        }
    }
    if(check == 5){
        return(1);
    }
    else{
        return(0);
    }
}
