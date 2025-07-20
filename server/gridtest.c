/* Todd Rosenbaum
 * CS50 Nuggest
 * 
 * gridtest.c - test file for grid module
 * 
 * March 6, 2025
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "grid.h"
#include "../libcs50/hashtable.h"

void hashtablePrintHelp(FILE* fp, const char* key, void* item);
void hashtableDeleteHelp(void* item);
void test_grid(grid_t *grid);

/**************** main ****************/
int main(){
    printf("===== GRID MODULE TESTS =====\n\n");

    FILE *fp = fopen("maps/main.txt", "r");
    if (!fp) {
        printf("Could not open file. Exiting tests.\n");
        return 1;
    }
    grid_t *grid = grid_fromFile(fp);
    fclose(fp);

    if (!grid) {
        printf("Failed to load grid from file. Exiting tests.\n");
        return 1;
    }

    test_grid(grid);
    grid_delete(grid);
    return 0;
}


/**************** test_grid ****************/
void test_grid(grid_t *grid) {
    printf("--- Testing grid_print() ---\n");
    grid_print(grid);
    printf("\n\n");
    
    printf("--- Testing grid_getWidth() and grid_getHeight() ---\n");
    printf("Grid width: %d\n", grid_getWidth(grid));
    printf("Grid height: %d\n\n", grid_getHeight(grid));
    
    printf("--- Testing grid_get() and grid_set() ---\n");
    printf("Character at index 85: %c\n", grid_get(grid, 85));
    printf("Setting index 85 to '*'\n");
    if (grid_set(grid, 85, '*')) {
        printf("Set successful!\n");
    } else {
        printf("Set failed!\n");
    }
    grid_print(grid);
    grid_set(grid, 85, '.');
    printf("\n\n");
    
    printf("--- Testing grid_makeGold() ---\n");
    hashtable_t* goldMap = grid_makeGold(grid, 10, 30, 250);
    if (goldMap) {
        printf("Gold placement successful!\n");
    } else {
        printf("Gold placement failed!\n");
    }
    hashtable_print(goldMap, stdout, hashtablePrintHelp);
    grid_print(grid);
    hashtable_delete(goldMap, hashtableDeleteHelp);
    printf("\n\n");
    
    printf("--- Testing grid_numGoldPiles() ---\n");
    printf("Number of gold piles: %d\n\n", grid_numGoldPiles(grid));
    
    printf("--- Testing grid_getWalls() ---\n");
    int *walls = grid_getWalls(grid);
    if (walls) {
        printf("Wall indices: ");
        for (int i = 0; walls[i] != -1; i++) {
            printf("%d ", walls[i]);
            if ((char)grid_get(grid, walls[i]) != '|' && (char)grid_get(grid, walls[i]) != '-' && (char)grid_get(grid, walls[i]) != '+' && (char)grid_get(grid, walls[i]) != '#'){
                printf("Non-wall index in int* walls - char at index: %c\n", (char)grid_get(grid, walls[i]));
            }
        }
        printf("\n");
        free(walls);
    } else {
        printf("Failed to retrieve walls.\n");
    }
    printf("\n");

    printf("--- Testing grid_makeEmpty() ---\n");
    grid_makeEmpty(grid);
    printf("Grid after grid_makeEmpty:\n");
    grid_print(grid);
    printf("\n\n");
}



/**************** hashtablePrintHelp ****************/
void hashtablePrintHelp(FILE* fp, const char* key, void* item)
{
    printf(" %s -> %d ", key, *(int*)item);
}

/**************** hashtableDeleteHelp ****************/
void hashtableDeleteHelp(void* item)
{
    free(item);
}