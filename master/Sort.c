#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "Master.h"

long getNumberOfLines(char *filename)
{
	FILE *fptr = fopen(filename, "r");
        char words[MAX_LINE_LENGTH];
        long ind=0;

	if (fptr == NULL)
	{
		printf("Error opening input file... \n");
		exit(0);
	}

         while ( fgets ( words, MAX_LINE_LENGTH, fptr ) != NULL ){
                ind++;
         }

          fclose(fptr);
         return ind;  
}


char* getLower(char *str)
{
	char *strcopy = malloc(sizeof(char)*MAX_LINE_LENGTH);
	int i;
	for(i = 0; str[i]; i++)
		strcopy[i] = str[i];
	for(i = 0; strcopy[i]; i++)
		strcopy[i] = tolower(strcopy[i]);
	return strcopy;
}


int compareTwoChars (const void * a, const void * b) 
{ 
    return strcmp (getLower(*(char **) a), getLower(*(char **) b));
} 

  
void sort(char **arr, int num) 
{ 
    qsort(arr, num, sizeof(char *), compareTwoChars); 
}


void getWords(char *filename, char **words)
{
	FILE *fptr = fopen(filename, "r");
	if (fptr == NULL)
	{
		printf("Error opening input file... \n");
		exit(0);
	}
	int ind=0;
	while ( fgets ( words[ind], MAX_LINE_LENGTH, fptr ) != NULL ){
                ind++;
         }
        fclose(fptr);
}


void convertToOneD(char *oneDArr, char **words, int count_lines)
{
	int prev_length = 0, tot_length, i;
	for(i=0; i<count_lines; i++)		
	{
		tot_length = (i==count_lines-1)?strlen(words[i])+1:strlen(words[i]);
		memcpy(oneDArr + prev_length, words[i], tot_length);
		prev_length += tot_length;
	}
}


char* sortFileContent(char* fileName, int *fileSizeParam)
{
	int count_lines=0, i;
	char **words=NULL;
        int32_t fileSize =0;
        char *oneDArr=NULL; 
        count_lines = getNumberOfLines(fileName);
	words = malloc(sizeof(char*)*count_lines);
	if (words == NULL)
	{
		printf("Memory allocation for words array failed.");
		exit(0);
	}
	for (i=0; i<count_lines; i++)
	{
		words[i] = malloc(sizeof(char)*MAX_LINE_LENGTH);
		if (words[i] == NULL)
		{
			printf("Memory allocation for %d word failed.", i+1);
			exit(0);
		}
                memset(words[i],'\0',sizeof(char)*MAX_LINE_LENGTH);
	}
	getWords(fileName, words);
	sort(words, count_lines);
        fileSize = sizeof(char)*count_lines*MAX_LINE_LENGTH;
        *fileSizeParam = fileSize;
        oneDArr = malloc(fileSize);
        if(oneDArr==NULL){
		printf("Memory allocation for words array failed.");
		exit(0);
       }
        memset(oneDArr,'\0',fileSize);
        
	convertToOneD(oneDArr, words, count_lines);

	for (i=0; i<count_lines; i++){
             free(words[i]);
        }
        free(words);

    return oneDArr; 
}
#if 0
int main(){
     char *oneDArr=NULL;
     char *fileName= "test.txt_0";
     int fileSize=0;
       printf("%d\n",sizeof(UserMonteCarloQueryTable));
  // oneDArr= sortFileContent(fileName, &fileSize);
    // printf("%d:%s\n",fileSize,oneDArr);
}
#endif
