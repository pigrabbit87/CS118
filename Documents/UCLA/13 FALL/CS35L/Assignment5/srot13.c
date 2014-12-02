#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	return rot13cmp(*(const char**)a, *(const char **)b);
}
int main()
{
	int c, count, wordCount, n, i, w;
	char result;
        char **word = malloc(50*sizeof(char*));
        char *newWord = NULL;
	char *cword = NULL;

	if (word == NULL) {fprintf(stderr, "BADBADBAD"); exit(1);}
	wordCount = 0;
	w = 1;
	

        while (1)
        {
		if (wordCount == 50*w)
		{
			w++;
			word = (char**)realloc(word, 50*w*sizeof(char*));
			if (word == NULL) {fprintf(stderr, "YOU HAVE A BAD TIME"); exit(1);}
		}	
                c = 0;
                count = 0;
                n = 1;
                c = getchar();
		if (c == EOF && wordCount == 0) {printf("you forgot something \n");}		
		if (c == EOF)
		{break;}
                newWord = malloc(50*n*sizeof(char));
		if (newWord == NULL) {fprintf(stderr, "YOU HAVE A BAD TIME"); exit(1);}

                while (c != EOF)
		{
			if (count == 50*n)
			{
				n++;
				newWord = (char*)realloc(newWord, 50*n*sizeof(char));
				if (newWord == NULL) {fprintf(stderr, "YOU HAVE A BAD TIME");exit(1);}

			}			
		
			if (c == 10) {newWord[count] = '\n';  break;}
			
			newWord[count] = c;
		
			c = getchar();
			
			count++;
		}
		if (c == EOF)
		{newWord[count] = '\n';}
                word[wordCount] = newWord;
                wordCount++;
        }
		
	qsort(word, wordCount, sizeof(char*),cmp);

	c = 0;
	i = 0;
	while (i < wordCount)
	{
	cword = word[i];
	count = 0;
	while (1)
	{
		if (cword[count] == '\n')
		{
			putchar(10);
			break;
		}				
		result = cword[count];
		putchar(result);
		count++;
	}
	i++;
	}
	
	i = 0;
        free(newWord);
        free(word);
        return 0;
}

