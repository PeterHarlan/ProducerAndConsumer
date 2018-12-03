#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

// n is shared variable between producers
// m is a shared variable between consumers
int n, m;

//create semaphores
sem_t psem, mutex, empty, full;

//create function pointers
void *producer(void *param);
void *consumer(void *param);

//Define struct
struct v
{
  int tid;
  int buff_sz;
  int up_lim;
};

struct timeval stop, start;

//Define Buffer
int *buffer;
int buff_in, buff_out;

int main(int argc, char *argv[])
{
  //Check num arguments
  if ( argc == 5)
  {
    //get cmd line arguments
    int buff_sz = atoi(argv[1]);
    int numP = atoi(argv[2]);
    int numC = atoi(argv[3]);
    int up_lim = atoi(argv[4]);
    
    //Define buffer
    buffer =  malloc(sizeof *buffer * buff_sz);
 
    //Thread identifier
    pthread_t tid_producer[numP], tid_consumer[numC];

    //set of attributes
    pthread_attr_t attr_producer[numP], attr_consumer[numC];
      
    //Create clock variables and start
    gettimeofday(&start, NULL);

    //Initialize semaphores
    sem_init(&psem, 1, 1);
    sem_init(&mutex, 1, 1);
    sem_init(&empty, 1, buff_sz);
    sem_init(&full, 1, 0);
    int i;


    //Create producers	
    for( i = 0; i < numP; i++)
    {
      //initialize struct info
      struct v *data = (struct v *) malloc(sizeof(struct v));
      data->tid = i;
      printf("Producer creation %d\n", i); 
      data->buff_sz = buff_sz;
      data->up_lim = up_lim;

      //Create thread
      pthread_attr_init(&attr_producer[i]);
      pthread_create(&tid_producer[i], &attr_producer[i], producer, data);
    }				
    
    //Create consumers
    for(  i = 0; i < numC; i++)
    {
      //initialize struct info
      struct v *data = (struct v *) malloc(sizeof(struct v));
      data->tid = i; 
      data->buff_sz = buff_sz;
      data->up_lim = up_lim;
       
      //Create thread
      pthread_attr_init(&attr_consumer[i]);
      pthread_create(&tid_consumer[i], &attr_consumer[i], consumer, data);

    }			

    //Wait for threads to complete
    for(i = 0; i< numP; i++)
    {
      pthread_join(tid_producer[i], NULL);
    }
    for(i = 0; i< numC; i++)
    {
      pthread_join(tid_consumer[i], NULL);
    }


    gettimeofday(&stop, NULL);

    printf("TOTAL: %lu\n", stop.tv_usec - start.tv_usec);
   
    //Free buffer memory
    free( buffer );
  }
   else 
  {
    printf("There are an incorrect number of arguments\n");
  }
    
}

void *producer( void *param)
{
  //Access struct data
  struct v *data;
  data = (struct v*) param;
  int buff_size = (*data).buff_sz;
  int upper_lim = (*data).up_lim;
  int tid = (*data).tid;

  while(n <= upper_lim+1) 
  {
    printf("PRINT EMPTY Producer: %d\n", tid);
    printf("PRINT EMPTY Producer: %d\n", tid);
    
    //Wait until empty space in buffer
    sem_wait(&empty);
    printf("PRINT EMPTY Producer: %d\n", tid);

    //Wait until buffer is available
    sem_wait(&mutex);
    printf("PRINT MUTEX Producer: %d\n", tid);
 
    //If limit reached while process waiting, do not execute     
    if(n <= upper_lim){ 
      printf("Producer: %d Enters IF\n", tid);

      //Insert n into buffer, increment buffer in and n
      //critical section for buffer
      buffer[buff_in % buff_size] = n;
      buff_in++;
    }else{

      printf("Producer: %d Enters ELSE\n", tid);
      //Release waiting producers if job done
      sem_post(&empty);
    }

    //Post to buffer mutex and full semaphore
    sem_post(&mutex);
    sem_post(&full);


    //Wait for shared variable and increment 
    sem_wait(&psem);
    n++;
    sem_post(&psem);

  }

  printf("Producer: %d Exits", tid);
  pthread_exit(0);
}

void *consumer(void *param)
{
  //Access struct data
  struct v *data;
  data = (struct v*) param;
  int buff_size = (*data).buff_sz;
  int upper_lim = (*data).up_lim;
  int tid = (*data).tid;

  //Loop to user specified upper limit
  while(m != upper_lim)
  {

    //iait if buffer not full, or if in use
    sem_wait(&full);
    sem_wait(&mutex); 
 
    //If the upper limit has not been reached
    //execute, otherwise
    //Do not execute and signal release
    if(m != upper_lim){ 

      printf("Consumer: %d Enters IF\n", tid);
      // remove element from buffer increment buff_out
      // and print value of m
      m = buffer[buff_out % buff_size]; 
      buff_out++; 
      printf("PRINT: %d   FROM: %d\n", m, tid);

    }else{

      printf("Consumer: %d Enters ELSE\n", tid);
     //if upper limit reached release waiting consumers
     sem_post(&full);
    }
  
    //Release buffer and post to not empty
    sem_post(&mutex);
    sem_post(&empty);

  }//end while

  pthread_exit(0);
}//end consumer
 
