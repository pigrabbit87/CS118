#include <unistd.h>

int main()
{
	char data[1];
	while (read(0, data, 1) > 0)
	{
		write(1, data, 1);
	} 
}
