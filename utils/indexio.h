#pragma once
/*
 * indexio.h --- 
 * 
 * Author: engs50 Team MergeConflict
 * Created: 10-29-2025
 * Version: 1.0
 * 
 * Description: saving and loading index files into indexnm
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <webpage.h>


int32_t index_save(hashtable_t *ht, int id, char *indexnm);
hashtable_t *index_load(char *indexnm);
