#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

int main( int argc, char **argv )
{
	int i;
	int length=pow(2,30);
	int *mydata;
	mydata=(int *)malloc(length*sizeof(int));
	for (i=0;i<length;i++)
	{
		mydata[i]=0;
	}

	for (i=0;i<length;i++)
	{
		mydata[i]=mydata[i]+i;
	}
	for(i=0;i<length;i+=100000)
	{
		printf("%d\n",mydata[i]);
	}
	printf("%d\n",mydata[length-1]);
}

