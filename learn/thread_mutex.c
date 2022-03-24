#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;
char buf[80],buf1[80];
int n,buf_has_item=0;

void writer_function(void){
 // while(1){
    pthread_mutex_lock(&mutex);
    if(buf_has_item==0){
      printf("等待从键盘读一个非空串\n");
      if((n=read(0,buf,sizeof(buf)))>0)
         buf_has_item=1;
      printf("A thread write:%s\n",buf);
    }
    pthread_mutex_unlock(&mutex);
  //}
}

void reader_function(void){
  int i=0;
    //for(i=0; i < 5; i++){
    if (buf_has_item != 1)
    {
        printf("正在等待输入...\n");
    pthread_mutex_lock(&mutex);
    }
    if(buf_has_item==1){
       strcpy(buf1,buf); 
       buf_has_item=0;
       printf("A thread read:%s\n",buf1);
    }
    pthread_mutex_unlock(&mutex);
  //}     
}

/*
void reader_function_1(void){
  while(1){
    pthread_mutex_lock(&mutex);
	printf("im not the clone one\n"); 
	buf_has_item=0;	
    pthread_mutex_unlock(&mutex);
  }     
}
  */           
void *thread_reader(void *in){
  reader_function();
  pthread_exit((void**)0);
}
/*
void *thread_reader_1(void *in){
  reader_function_1();
  pthread_exit((void**)0);
}
*/

int main(){
  pthread_t reader,writer;
  pthread_t ptr;
  pthread_mutex_init(&mutex,NULL);
  pthread_create(&ptr,NULL,thread_reader,(void*)0);
  //pthread_create(&reader,NULL,thread_reader,(void*)0);
  writer_function();
  pthread_join(ptr,(void**)0);
  //pthread_join(reader,(void**)0);
  return 0;
}
