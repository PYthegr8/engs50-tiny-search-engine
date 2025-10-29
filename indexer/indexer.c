/* 
 * indexer.c --- 
 * 
 * Author: Engs 50 25F, Team MergeConflict
 * Created: 10-28-2025
 * Version: 1.0
 * 
 * Description: prints Hello on screen
 * 
 */


 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <inttypes.h>
 #include <unistd.h>
 #include <webpage.h>

int main() {
    webpage_t* loaded_page = pageload(1, char *dirnm);
    if (loaded_page) {
        int pos = 0;
        char *result;
        while ((pos = webpage_getNextWord(loaded_page, pos, &result)) > 0) {
            printf("word : %s\n", result);
            free(result);
        }
    }


    char *NormalizeWord(char *input){
         if (strlen(input) < 3) {
            printf("word less than 3 characters \n");
            return EXIT_FAILURE;
         }

         int i = 0;
         char* str = (char*)malloc(strlen(input) + 1);
         strcpy(str,input);
         while (input[i] != '\0') {
             if (isalnum(input[i])) {
                str[i] = tolower(input[i]);
             }
             i++;
         }
         return str;
    }
}
