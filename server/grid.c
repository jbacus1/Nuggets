/* 
 * grid.c - implementation file for grid module
 *
 * A grid is a structure to save a map for the nuggets project
 * 
 * Todd Rosenbaum
 * CS 50 Nuggets
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "grid.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/file.h"

/**************** file-local global variables ****************/
/* none */

/**************** local types ****************/
/* none */

/**************** global types ****************/
typedef struct grid {
    char* map;
    int width;
    int height;
} grid_t;

/**************** functions ****************/


/**************** grid_new ****************/
/* Create a new grid from inputted string.
 * Store the height and width of the map string.
 * Assume there is a '\n' at the end of every line.
 * See grid.h for more information. */
grid_t* grid_new(char* string)
{
    // validate parameters
    if (string == NULL){
        fprintf(stderr, "Error: inputted string in NULL\n");
        return NULL;
    }

    // create and allocate memory for new grid object
    grid_t* grid = malloc(sizeof(grid_t));
    if (grid == NULL){
        fprintf(stderr, "Error: issue allocating memory for grid in grid_new\n");
        return NULL;
    }

    // create and allocate memory for string
    char* temp = malloc(strlen(string) + 1);
    if (temp == NULL){
        fprintf(stderr, "Error: issue allocating memory for string in grid_new\n");
        free(grid);
        return NULL;
    }
    // copy string to temp
    strcpy(temp, string);
    // set grid->map
    grid->map = temp;

    // loop through to find grid's width (first occurance of '\n')
    // also stop if you reach end of grid->map
    int i = 0;
    while (grid->map[i] != '\n' && grid->map[i] != '\0'){
        i++;
    }
    grid->width = i;

    // check if width is empty
    if (grid->width == 0){
        fprintf(stderr, "Error: first line of grid is empty\n");
        grid_delete(grid);
        return NULL;
    }

    // loop through to find height (total occurances of '\n')
    int count = 0;
    int line_width = 0;
    i = 0;

    while (grid->map[i] != '\0'){

        // if '\n'
        if (grid->map[i] == '\n'){

            // if lines vary in size
            if (line_width != grid->width){
                fprintf(stderr, "Error: inputted string varies in line width\n");
                grid_delete(grid);
                return NULL;
            }

            // increment
            count++;
            // reset line_width
            line_width = 0;
        }
        // if not '\n'
        else{
            line_width++;
        }
        i++;
    }
    // see if last character not '\n'
    if (line_width > 0){
        // validate width
        if (line_width != grid->width){
            fprintf(stderr, "Error: last line varies in width\n");
            grid_delete(grid);
            return NULL;
        }
        // increment count
        count++;
    }

    // set grid->height
    grid->height = count;

    // return the grid
    return grid;
}


/**************** grid_fromFile ****************/
/* Create new grid from inputted FILE*. 
 * Call grid_new and pass char* from txt file as argument.
 * See file.h for more information.
 * See grid.h for more information.
 */
grid_t* grid_fromFile(FILE* fp)
{
    // validate parameters
    if (fp == NULL) {
        fprintf(stderr, "Error: NULL file pointer provided to grid_fromFile\n");
        return NULL;
    }

    // read entire file into a dynamically allocated string
    char* file_contents = file_readFile(fp);
    if (file_contents == NULL){
        fprintf(stderr, "Error: failed to read file contents in grid_fromFile\n");
        return NULL;
    }

    // create grid from file contents
    grid_t* grid = grid_new(file_contents);
    if (grid == NULL){
        fprintf(stderr, "Error: issue creating grid in grid_fromFile");
        free(file_contents);
        return NULL;
    }

    // free the buffer allocated by file_readFile()
    free(file_contents);

    // return the grid
    return grid;
}


/**************** grid_get ****************/
/* Find and return the char that exists at grid->map[index]. 
 * See grid.h for more information. */
char grid_get(grid_t* grid, int index)
{
    // validate parameters
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid of grid->map in grid_get\n");
        return '\0';
    }
    // validate bounds
    if (index < 0 || index > (strlen(grid->map) - 1)){
        fprintf(stderr, "Error: %d index out of bounds in grid_get\n", index);
        return '\0';
    }
    // return the char at grid->map[index]
    return grid->map[index];
}


/**************** grid_set ****************/
/* Update the char that exists at grid->map[index]. 
 * See grid.h for more information. */
bool grid_set(grid_t* grid, int index, char character)
{
    // validate parameters
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid of grid->map in grid_set\n");
        return false;
    }
    // validate bounds
    if (index < 0 || index > (strlen(grid->map) - 1)){
        fprintf(stderr, "Error: %d index out of bounds in grid_set\n", index);
        return false;
    }
    /* Commenting this out as Nate requested
    // return false if char at index cannot be changed
    char check = grid->map[index];
    if (check == ' ' || check == '|' || check == '-' || check == '+' || check == '#'){
        return false;
    }
    */
    // otherwise change grid->map[index] and return true
    grid->map[index] = character;
    return true;
}


/**************** grid_getWidth ****************/
/* Return width of grid. 
 * See grid.h for more information. */
int grid_getWidth(grid_t* grid)
{
    // validate
    if (grid == NULL){
        fprintf(stderr, "Error: NULL grid in grid_getWidth\n");
        return -1;
    }
    return grid->width;
}


/**************** grid_getHeight ****************/
/* Return height of grid. 
 * See grid.h for more information. */
int grid_getHeight(grid_t* grid)
{
    // validate
    if (grid == NULL){
        fprintf(stderr, "Error: NULL grid in grid_getHeight\n");
        return -1;
    }
    return grid->height;
}


/**************** grid_delete ****************/
/* Delete grid. 
 * See grid.h for more information. */
void grid_delete(grid_t* grid)
{
    // validate
    if (grid == NULL){
        return;
    }
    free(grid->map);
    free(grid);
}


/**************** grid_print ****************/
/* Print the grid. 
 * See grid.h for more information. */
void grid_print(grid_t* grid)
{
    if (grid == NULL || grid->map == NULL){
        printf("(null)\n");
        return;
    }
    printf("%s", grid->map);
}


/**************** grid_makeGold ****************/
/* Place piles of gold in grid.
 * The number of gold piles should fall in between minPiles and maxPiles.
 * The amount of golf placed should be totGold.
 * Return a hashtable mapping goldIndices-goldAmount.
 * See grid.h for more information. */
hashtable_t* grid_makeGold(grid_t* grid, int minPiles, int maxPiles, int totGold)
{

    // count number of available '.' positions
    int periodCount = 0;
    for (int i = 0; grid->map[i] != '\0'; i++) {
        if (grid->map[i] == '.') {
            periodCount++;
        }
    }

    // verify enough positions exist
    if (periodCount < maxPiles){
        fprintf(stderr, "Error: not enough valid chars in grid->map to place all gold\n");
        return NULL;
    }

    // create hashtable goldMap
    hashtable_t* goldMap = hashtable_new(maxPiles);
    if (goldMap == NULL){
        fprintf(stderr, "Error: issue creating hashtable in grid_makeGold\n");
        return NULL;
    }

    int numPiles = 0;
    int goldPlaced = 0;

    // loop through until goldPlace = totGold
    while (goldPlaced < totGold && numPiles < maxPiles){

        // create random index
        int randIndex = rand() % (strlen(grid->map));

        // make sure its a period
        if (grid->map[randIndex] != '.'){
            continue;
        }

        ///// char at randIndex is '.' /////

        // calculate random amount of gold on interval [totGold/maxPiles, totGold/minPiles]
        int randAmount = (totGold/maxPiles) + rand() % ((totGold/minPiles) - (totGold/maxPiles) + 1);

        // validate you won't go over totGold
        if (goldPlaced + randAmount > totGold){
            randAmount = totGold - goldPlaced;
        }

        // convert int -> int to char* -> void* and insert into hashtable
        char key[10];
        sprintf(key, "%d", randIndex);
        int* randAmountPT = malloc(sizeof(int));
        *randAmountPT = randAmount;
        hashtable_insert(goldMap, key, (void*)randAmountPT);

        // update the grid
        if (!grid_set(grid, randIndex, '*')) {
            fprintf(stderr, "Error: Issue updating gold index in grid_makeGold\n");
            hashtable_delete(goldMap, NULL);
            return NULL;
        }

        // update goldPlaced
        goldPlaced += randAmount;
        numPiles++;
    }
    return goldMap;
}


/**************** grid_makeEmpty ****************/
/* Make the grid->map all spaces. 
 * See grid.h for more information. */
void grid_makeEmpty(grid_t* grid)
{
    // validate
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid or grid->map in grid_makeEmpty\n");
        return;
    }
    // loop through grid->map and change char to ' ' if not '\n'
    for (int i = 0; grid->map[i] != '\0'; i++){
        // only change if not '\n'
        if (grid->map[i] != '\n'){
            grid->map[i] = ' ';
        }
    }
}


/**************** grid_numGoldPiles ****************/
/* Return the number of gold piles in a grid->map. 
 * See grid.h for more information. */
int grid_numGoldPiles(grid_t* grid)
{
    // validate
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid or grid-map in grid_numGoldPiles\n");
        return -1;
    }

    // loop through and count '*'
    int goldCount = 0;
    for (int i = 0; grid->map[i] != '\0'; i++){
        if (grid->map[i] == '*'){
            goldCount++;
        }
    }
    return goldCount;
}


/**************** grid_getWalls ****************/
/* Return an int* with the indices of all walls, roofs, corners, and passage ways. 
 * See grid.h for more information. */
int* grid_getWalls(grid_t* grid)
{
    // validate
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid or grid->map in grid_getWalls\n");
        return NULL;
    }

    // count walls
    int count = 0;
    for (int i = 0; grid->map[i] != '\0'; i++){
        if (grid->map[i] == '|' || grid->map[i] == '-' || grid->map[i] == '+' || grid->map[i] == '#') {
            count++;
        }
    }

    // allocate memory for the wall indices
    int* walls = malloc((count+1) * sizeof(int));
    if (walls == NULL) {
        fprintf(stderr, "Error: memory allocation failed in grid_getWalls\n");
        return NULL;
    }

    // if no walls
    if (count == 0){
        walls[0] = -1;
        return walls;
    }

    // build walls
    int index = 0;
    for (int i = 0; grid->map[i] != '\0'; i++){
        if (grid->map[i] == '|' || grid->map[i] == '-' || grid->map[i] == '+' || grid->map[i] == '#') {
            walls[index++] = i;
        }
    }
    // set end point
    walls[index] = -1;

    return walls;
}

/**************** grid_getMap ****************/
/* Return an char* with the grid->map. 
 * See grid.h for more information. */
char* grid_getMap(grid_t* grid)
{
    // validate grid
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid of grid->map in grid_getMap\n");
        return NULL;
    }
    // otherwise return grid->map
    return grid->map;
}


/**************** grid_getLength ****************/
/* Return an int with strlen(grid->map). 
 * See grid.h for more information. */
int grid_getLength(grid_t* grid)
{
    // validate grid
    if (grid == NULL || grid->map == NULL){
        fprintf(stderr, "Error: NULL grid or grid->map in grid_getLength\n");
        return 0;
    }
    // otherwise return strlen(grid->map)
    return strlen(grid->map);
}
