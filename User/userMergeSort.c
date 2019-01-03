# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define MAX_LINE_LENGTH 100
#define FILE_NAME_LENGTH 256

int getNumberOfLines(char *filename)
{
	FILE *fptr = fopen(filename, "r");
        char words[MAX_LINE_LENGTH];
        int ind=0;

	if (fptr == NULL)
	{
		printf("Error opening input file... \n");
		exit(0);
	}

         while ( fgets ( words, MAX_LINE_LENGTH, fptr ) != NULL ){
                ind++;
         }

          fclose(fptr);
         return ind++;  
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
               // printf("%s",words[ind]);
                ind++;
         }
        fclose(fptr);
}


char* getLower(char *str,char *strcopy)
{
	
	int i=0;
        int wordLength = strlen(str); 
        memcpy(strcopy,str,wordLength);
        
	for(i = 0; i < wordLength ; i++)
		strcopy[i] = tolower(strcopy[i]);
	return strcopy;
}


char* mergeSort(char **words1, char **words2, int count_lines1, int count_lines2, char *result)
{
      int i =0;
      int j =0;
      char *word_one =NULL;
      char *word_two =NULL;
      
      word_one = malloc(sizeof(char)*MAX_LINE_LENGTH);
      word_two = malloc(sizeof(char)*MAX_LINE_LENGTH);
      
      memset(word_one,'\0',sizeof(char)*MAX_LINE_LENGTH);
      memset(word_two,'\0',sizeof(char)*MAX_LINE_LENGTH);
          
      word_one = getLower(words1[0],word_one);
      word_two = getLower(words2[0],word_two);

      for(i=0, j= 0 ; i<count_lines1 && j < count_lines2;){
          
          if (strcmp(word_one, word_two) > 0){
              strcat(result,words2[j]);
              j++;
              memset(word_two,'\0',sizeof(char)*MAX_LINE_LENGTH);
              if(j < count_lines2)
                 word_two = getLower(words2[j],word_two);
              else
                  break;
          }
          else{
             strcat(result,words1[i]);
             i++;
             memset(word_one,'\0',sizeof(char)*MAX_LINE_LENGTH);
             if(i < count_lines1) 
               word_one = getLower(words1[i],word_one);
             else
                break;
          }
      }

      if(i >= count_lines1 && j < count_lines2){

            while(j < count_lines2){
                  strcat(result,words2[j]);
                  j++;
            }
   
      } 

      if(j >= count_lines2 && i < count_lines1){

            while(i < count_lines1){
                  strcat(result,words1[i]);
                  i++;
            }
      }

      free(word_one);
      free(word_two);
      return result;

}

char* fileMergeSort(char *file1, char*file2,char *file3)
{
        int i=0;

        char **wordListFile_1=NULL;
        char **wordListFile_2=NULL;
        char *resultFileContent_3=NULL;
  
        int numberOfLines_file1=0;
        int numberOfLines_file2=0;
        int numberOfLines_file3=0; 
        
        numberOfLines_file1 = getNumberOfLines(file1);
        numberOfLines_file2 = getNumberOfLines(file2);

        numberOfLines_file3 = numberOfLines_file1+numberOfLines_file2; 

        wordListFile_1 = malloc(sizeof(char*)*numberOfLines_file1);
        wordListFile_2 = malloc(sizeof(char*)*numberOfLines_file2);
        resultFileContent_3 = malloc(sizeof(char*)*(numberOfLines_file3*MAX_LINE_LENGTH));
	if (wordListFile_1 == NULL)
	{
		printf("Memory allocation for wordListFile_1 failed.");
		exit(0);
	}
        if (wordListFile_2 == NULL)
	{
		printf("Memory allocation for wordListFile_2 failed.");
		exit(0);
	}

        if (resultFileContent_3 == NULL)
	{
		printf("Memory allocation for resultFileContent_3 failed.");
		exit(0);
	}
        
        memset(resultFileContent_3,'\0',sizeof(char*)*(numberOfLines_file3*MAX_LINE_LENGTH));

	for (i=0; i<numberOfLines_file1; i++)
	{
		wordListFile_1[i] = malloc(sizeof(char)*MAX_LINE_LENGTH);
		if (wordListFile_1[i] == NULL)
		{
			printf("Memory allocation for %d wordListFile_1 failed.", i+1);
			exit(0);
		}
                memset(wordListFile_1[i],'\0',sizeof(char)*MAX_LINE_LENGTH);
	}

        for (i=0; i<numberOfLines_file2; i++)
	{
		wordListFile_2[i] = malloc(sizeof(char)*MAX_LINE_LENGTH);
		if (wordListFile_2[i] == NULL)
		{
			printf("Memory allocation for %d wordListFile_2 failed.", i+1);
			exit(0);
		}
                memset(wordListFile_2[i],'\0',sizeof(char)*MAX_LINE_LENGTH);
	}

        getWords(file1, wordListFile_1);
	getWords(file2, wordListFile_2);
 
	mergeSort(wordListFile_1, wordListFile_2, numberOfLines_file1, numberOfLines_file2, resultFileContent_3);
	printf("result:\n%s", resultFileContent_3);
        
        for (i=0; i<numberOfLines_file2; i++)
	{
              free(wordListFile_2[i]); 
        }

        for (i=0; i<numberOfLines_file1; i++)
	{
              free(wordListFile_1[i]); 
        }
      
          free(wordListFile_1);
          free(wordListFile_2);

         FILE *fptr = fopen(file3, "w");
         fwrite(resultFileContent_3,sizeof(char),strlen(resultFileContent_3),fptr);
         fclose(fptr);
	 return resultFileContent_3;
}

#if 0
int main(){
      char *sort ="SORT";
      int index=0;
      char *token=NULL;
      char tempFileName[FILE_NAME_LENGTH];
      char file_1[FILE_NAME_LENGTH];
      char file_2[FILE_NAME_LENGTH];
      char sortedFileName[FILE_NAME_LENGTH];
      char underScore[2]="_";
      char zero[2]="0";
      char one[2]="1";
      char *fileName = "test.txt_0";
      int queryId=0;

      memset(tempFileName,'\0',FILE_NAME_LENGTH);
      memset(file_1,'\0',FILE_NAME_LENGTH);
      memset(file_2,'\0',FILE_NAME_LENGTH);
      memset(sortedFileName,'\0',FILE_NAME_LENGTH);
      /*Get the Exact File Name By removing the Index*/
      strcpy(tempFileName,fileName);
      token = strtok(tempFileName,underScore);
      /*SORT_FILENAME*/
      sprintf(sortedFileName,"%s%s%s",sort,underScore,token);
      /*FILNAME_0*/
      sprintf(file_1,"%s%s%s",token,underScore,zero);
      /*FILNAME_1*/
      sprintf(file_2,"%s%s%s",token,underScore,one);

      /*First do MergeSort the Filename_0 and Filename_1*/
       printf("Merge Sort the Two File %s%s\n",file_1,file_2);
       fileMergeSort(file_1,file_2,sortedFileName);
      for(index=2; index< 3; index++){
                     memset(file_1,'\0',FILE_NAME_LENGTH);
                     /*FileNAME_IndexNumber*/
                     sprintf(file_1,"%s%s%d",token,underScore,index);
                     printf("Merge Sort the Two File %s%s\n",sortedFileName,file_1);
                     fileMergeSort(sortedFileName,file_1,sortedFileName);
      }
       printf("Sorted File for the Query Id : %d is : %s\n",queryId,sortedFileName);

}
#endif
