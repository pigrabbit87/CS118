#include <stdio.h>
#include <stdlib.h>

int main()
{ 
	int c = getchar();
	while (c != EOF)
	{
		putchar(c);
		c = getchar();
	}
}
