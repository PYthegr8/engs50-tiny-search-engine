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
#include <stdint.h>
#include <stdio.h>
#include "hash.h"

int32_t index_save(hashtable_t *ht, const char *indexnm);
hashtable_t *index_load(const char *indexnm);
