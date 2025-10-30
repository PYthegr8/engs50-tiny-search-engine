/* 
 * indexio.c --- 
 * 
 * Author: engs50 Team MergeConflict
 * Created: 10-29-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */


int32_t index_save(hashtable_t *ht, int id, char *indexnm);
hashtable_t *index_load(char *indexnm);




int32_t index_save(hashtable_t *ht, int id, char *indexnm){
    char filepath[256];
    snprintf(filepath, sizeof filepath, "%s", indexnm);
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        fprintf(stderr, "[Error: Could not open file %s]\n", filepath);
        return EXIT_FAILURE;
    }
    int rc = index_dump_multi(ht, file);
    fclose(file);
    return (rc == 0) ? 0 : EXIT_FAILURE;
}