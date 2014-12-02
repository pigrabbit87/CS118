#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int total = 0;

int rot13cmp (char const *a, char const *b)
{
	char tmp_a, tmp_b;
    	do
	{
		//upper case
        	if (*a >= 65 && *a <= 77)
		{tmp_a = *a + 13;}
		else if (*a >= 78 && *a <= 90)
		{tmp_a = *a - 13;}
                //lower case
		else if (*a >= 97 && *a <= 109)
		{tmp_a = *a + 13;}
		else if (*a >= 110 && *a <= 122)
		{tmp_a = *a - 13;}
		else
		{tmp_a = *a;}

		if (*b >= 65 && *b <= 77)
        	{tmp_b = *b + 13;}
	        else if (*b >= 78 && *b <= 90)
	        {tmp_b = *b - 13;}
		else if (*b >= 97 && *b <= 109)
		{tmp_b = *b + 13;}
		else if (*b >= 110 && *b <= 122)
		{tmp_b = *b - 13;}
	        else
	        {tmp_b = *b;}

		if (tmp_b == 10 || tmp_a == 10)
			break;
		a++;
		b++;
	}
	while (tmp_b - tmp_a == 0);
	//tmp_a, tmp_b are in their right value
	return tmp_a - tmp_b;
}

int cmp (const void *a, const void *b)
{
	total++;
	return rot13cmp(*(const char**)a, *(const char **)b);
}
int main()
{
	int c, count, wordCount, n, i, w, tcount;
	int mode = 0;
	int size;
    char **word = malloc(50*sizeof(char*));
    char *newWord = NULL;
	struct stat fileStat;
	int f_size;
	int tmp = fstat(0,&fileStat); 
	if(tmp != 0) {fprintf(stderr, "BADBADBAD"); exit(1);}
	
	if (!S_ISREG(fileStat.st_mode)) {mode = 1;}
	if (word == NULL) {fprintf(stderr, "BADBADBAD"); exit(1);}
	wordCount = 0;
	tcount = 0;
	w = 1;
	f_size = fileStat.st_size;
	
	while(fstat(0,&fileStat) == 0)
	{
		if (f_size != fileStat.st_size)
			f_size == fileStat.st_size;
		else
			break;
	}

	n = 1;
	char *whole = NULL; //array holds the entire file
	if (mode == 0)
	{
		whole = (char*)realloc(whole, f_size);
		if (read(0, whole, f_size) == 0) {return 1;}
	}
	else
	{
		whole = (char*)realloc(whole, 50*n*sizeof(char));
		size = read(0, whole, 50*n*sizeof(char));
		c = whole[size-1];

		while (size == 50*n)
		{
			n++;
			whole = (char*)realloc(whole, 50*n*sizeof(char));
			size += read(0, whole+50, 50*n*sizeof(char));
		}
	
		i = 0;
		while (i < size) i++;
		f_size = i;
		if (whole[f_size-1] != 10) {whole[f_size] = 10;}
	}


    while (1)
    {
		if (wordCount == 50*w)
		{
			w++;
			word = (char**)realloc(word, 50*w*sizeof(char*));
			if (word == NULL) {fprintf(stderr, "YOU HAVE A BAD TIME"); exit(1);}
		}
		
		if (tcount >= f_size) {break;}
		c = 0; //each character
		count = 0; //number of character per word
       	n = 1; //index for newWord
        	c = whole[tcount]; tcount++; //first character	
		if (c == EOF){break;}
		
		newWord = malloc(50*n*sizeof(char));
		if (newWord == NULL) {fprintf(stderr, "YOU HAVE A BAD TIME"); exit(1);}
		
	    	while (tcount <= f_size+1)
		{
			if (count == 50*n)
			{
				n++;
				newWord = (char*)realloc(newWord, 50*n*sizeof(char));
				if (newWord == NULL) {fprintf(stderr, "YOU HAVE A BAD TIME");exit(1);}
			}			

			if (c == 10) {newWord[count] = '\n';  break;}
			newWord[count] = c;
			c = whole[tcount]; tcount++;
			count++;
		}

		if (tcount >= f_size) {newWord[count] = '\n';}
		word[wordCount] = newWord;
        	wordCount++;	
    }

	qsort(word, wordCount, sizeof(char*),cmp);
	
	i = 0;
	while (i < wordCount)
	{
		size = 0;
		while (word[i][size] != 10) size++;
		write(1, word[i], size+1);
		i++;
	}

	fprintf(stderr, "Number of comparisons: %d\n", total);
    	free(newWord);
    	free(word);
	free(whole);
    return 0;
}

