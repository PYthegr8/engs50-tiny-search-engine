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

int main(void) {
    char line[1000];

    printf("Please enter your input to the command line!\n");

    while (1) {
        printf("> ");

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("End of file signal entered!\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        char *token = strtok(line, " \t");
        if (!token) continue;

        int invalid = 0;
        char *words[100];
        int count = 0;

        while (token != NULL) {
            char *normalized = NormalizeWord(token);
            if (!normalized) {
                invalid = 1;
                break;
            }
            words[count++] = normalized;
            token = strtok(NULL, " \t");
        }

        if (invalid) {
            printf("[invalid query]\n");
            for (int i = 0; i < count; i++)
                free(words[i]);
            continue;
        }

        for (int i = 0; i < count; i++) {
            printf("%s", words[i]);
            if (i < count - 1) printf(" ");
            free(words[i]);
        }
        printf("\n");
    }

    return 0;
}

char *NormalizeWord(char *input) {
    int len = strlen(input);
    if (len < 1) return NULL;

    char *newWord = malloc(len + 1);
    if (!newWord) return NULL;

    for (int i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (!isalpha(c)) {
            free(newWord);
            return NULL;
        }
        newWord[i] = (char)tolower(c);
    }
    newWord[len] = '\0';
    return newWord;
}

