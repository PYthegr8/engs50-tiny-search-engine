/* 
* test.c --- 
* 
* Author: MergeConflict 
* Created: 10-28-2025
* Version: 1.0
* 
* Description: Testing the pageload method in pageio.c
* 
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "webpage.h"
#include "pageio.h"

int main() {
    webpage_t *page;
    page = pageload(1, ".");
    pagesave(page, 2, ".");
}
