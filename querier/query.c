/*
Author: MergeConflict
November 2025
ENGS 50
Module 6: Querier
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


char *NormalizeWord(char *input);

int main () {
    printf("Please enter your input to the command line!\n");
    while (1){
        char *word = (char *)malloc(100*sizeof(char));
        printf("> ");
        int result = scanf("%s", word);
        if (result == EOF) {
            printf("End of file signal entered!\n");
            return 1;
        }
        char *normalized = NormalizeWord(word);
        if (normalized) {
            printf("The word you entered is: %s\n", normalized);
        }
        else {
            printf("[invalid query]\n");
            return 1;
        }
    }
    return 0;
}

char *NormalizeWord(char *input)
{
    int len = strlen(input);
    if (len < 3) return NULL;
    char *newWord = malloc(len + 1);
    if (!newWord) return NULL;
    for (int i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (!isalpha(c)) { free(newWord); return NULL; }
        newWord[i] = (char)tolower(c);
    }
    newWord[len] = '\0';
    return newWord;
}
