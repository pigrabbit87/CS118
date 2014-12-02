#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
void *initThread(void * arg);
void *updateThread(void *arg);
struct arg_struct
{
	int *data;
	int length;
	int nthreads;
	int id;
};
int main( int argc, char **argv )
{
	int i;
	int length=pow(2,30);
	int *mydata;
	int nthreads;
	int rc;
	nthreads=15;
	pthread_t threads[nthreads];
	mydata=malloc(length*sizeof(int));
	struct arg_struct args[nthreads];
	for (i = 0; i < nthreads; i++)
	{
		args[i].data = mydata;
		args[i].length=length;
		args[i].nthreads = nthreads;
	}

	for (i = 0; i < nthreads; i++)
	{
		args[i].id = i;
		rc = pthread_create(&threads[i], NULL, &initThread, &args[i]);
	}

	for (i = 0; i < nthreads; i++)
	{
		rc = pthread_join(threads[i], NULL);
	}

	for (i = 0; i < nthreads; i++)
	{
		args[i].id = i;
		rc = pthread_create(&threads[i], NULL, &updateThread, &args[i]);
	}

	for (i = 0; i < nthreads; i++)
	{
		rc = pthread_join(threads[i], NULL);
	}
	for(i=0;i<length;i+=100000)
	{
		printf("%d\n",mydata[i]);
	}
	printf("%d\n",mydata[length-1]);

}
void *initThread(void * arg)
{
	struct arg_struct *args = (struct arg_struct *) arg;
	int block=(int)(args->length/args->nthreads);
	int index;
	int id;
	int j;
	int *data_ptr;
	data_ptr=args->data;
	id=args->id;
	for (index=block*id;index<(id+1)*block & index<args->length;index++)
	{
		data_ptr[index]=0;

	}
	if(id==args->nthreads-1)
	{
		for (index=block*(id+1);index<args->length;index++)
		{
			data_ptr[index]=0;

		}

	}

}
void *updateThread(void *arg)
{
	struct arg_struct *args = (struct arg_struct *) arg;
	int block=(int)(args->length/args->nthreads);
	int index;
	int id;
	int j;
	int *data_ptr;
	data_ptr=args->data;
	id=args->id;
	for (index=block*id;index<(id+1)*block & index<args->length;index++)
	{
		data_ptr[index]=data_ptr[index]+index;

	}
	if(id==args->nthreads-1)
	{
		for (index=block*(id+1);index<args->length;index++)
		{
			data_ptr[index]=data_ptr[index]+index;

		}

	}

}
