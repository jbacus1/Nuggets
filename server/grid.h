/* 
 * grid.h - header file for grid module
 *
 * A grid is a stucture to save a map for the nuggest project
 * 
 * Todd Rosenbaum
 * CS 50 Nuggets
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "../libcs50/hashtable.h"
#include "../libcs50/file.h"


/**************** global types ****************/
typedef struct grid grid_t;


/**************** functions ****************/

/**************** grid_new ****************/
/* Create a new grid from inputted string.
 *
 * Assumptions:
 *   there is a '\n' at the end of every line
 *
 * create and allocate memory for a grid object
 * create and allocate memory for string
 *   copy inputted string parameter into memory-allocated string
 *   set grid->map = string
 * find the grid’s width by looping through characters until you reach a ‘\n'
 *   set as grid->width
 * find the grid’s height by looping through the grid by factors of width and counting the number of ‘\n’
 *   consider final line (might need to add 1)
 *   validate map has constant dimensions
 *     free variables and return NULL if not
 *   set as grip->height
 *
 * Notes:
 *   caller is responsible for calling grid_delete
 */
grid_t* grid_new(char* string);


/**************** grid_fromFile ****************/
/* Create new grid from inputted FILE*. 
 * Call grid_new and pass char* from txt file as argument.
 * 
 * validate fp
 * read file contents into dynamically allocated char*
 * call grid_new() with contents from file
 *   return NULL if grid_new() returns NULL
 * free contents of file (grid_new makes a copy)
 * return grid
 * 
 */
grid_t* grid_fromFile(FILE* fp);


/**************** grid_get ****************/
/* Find and return the char that exists at grid->map[index]. 
 * 
 * Notes:
 *   returned char is an int
 *     use ASCII to convert
 *   returns '\0' if error
 *
 * validate grid
 * validate index
 * return char at grid->map[index]
 */
char grid_get(grid_t* grid, int index);


/**************** grid_set ****************/
/* Update the char that exists at grid->map[index]. 
 *
 * validate grid
 * validate index
 * if trying to update wall, roof, corner, or tunnel - UPDATED THIS FUNCTION TO CHANGE ALL CHARS
 *   return false and do nothing - USER RESPONSIBLE FOR ENSURING THEY DONT CHANGE AN UNCHANGABLE CHAR
 * otherwise
 *   update char at index and return true
 */
bool grid_set(grid_t* grid, int index, char character);


/**************** grid_getWidth ****************/
/* Return width of grid. 
 * 
 * Notes:
 *   return -1 if error
 *
 * validate grid
 * return grid->width
 */
int grid_getWidth(grid_t* grid);


/**************** grid_getHeight ****************/
/* Return height of grid. 
 * 
 * Notes:
 *   return -1 if error
 *
 * validate grid
 * return grid->height
 */
int grid_getHeight(grid_t* grid);


/**************** grid_delete ****************/
/* Delete grid. 
 * 
 * validate grid
 * free grid->map
 * free grid
 */
void grid_delete(grid_t* grid);


/**************** grid_print ****************/
/* Print the grid. 
 *
 * Notes:
 *   print "(null)" if NULL grid or grid->map 
 *
 * validate grid
 * print grid->map to stdout
 */
void grid_print(grid_t* grid);


/**************** grid_makeGold ****************/
/* Place piles of gold in grid.
 * The number of gold piles should fall in between minPiles and maxPiles.
 * The amount of golf placed should be totGold.
 * Return a hashtable mapping goldIndices-goldAmount.
 * 
 * Notes:
 *   caller responsible for deleting created hashtable
 *     entails calling hashtable_delete with helping function that will free each void* in the (char* -> void*) pair
 * 
 * validate parameters
 * count number of valid spots to put gold
 *   '.'
 * verify there are at least maxPiles valid spots to place gold in grid->map
 * loop until all gold placed
 *   create random index
 *   if grid->map[randIndex] is not a '.' continue
 *   create random amount
 *   ensure you wont go over totGold
 *     place totGold - goldPlaced if you will
 *   insert (char* -> void*) representation of (int randIndex -> int randAmount) into hashtable
 *   update grid->map
 *   update goldPlaced
 * return hashtable
 */
hashtable_t* grid_makeGold(grid_t* grid, int minPiles, int maxPiles, int totGold);


/**************** grid_makeEmpty ****************/
/* Make the grid->map all spaces. 
 * 
 * validate grid
 * loop through grid->map
 *   make all non '\n' char into spaces
 */
void grid_makeEmpty(grid_t* grid);


/**************** grid_numGoldPiles ****************/
/* Return the number of gold piles in a grid->map. 
 *
 * validate grid
 * return number of goldPiles in grid
 */
int grid_numGoldPiles(grid_t* grid);


/**************** grid_getWalls ****************/
/* Return an int* with the indices of all walls, roofs, corners, and passage ways. 
 * 
 * Notes:
 *   caller responsible for freeing int* returned
 *
 * validate grid
 * count number of walls, roofs, corners, and passage ways
 * create dynamically allocated int* with size for all obstructions
 * loop through and populate int* with indices of obstructions
 * return int*
 */
int* grid_getWalls(grid_t* grid);


/**************** grid_getMap ****************/
/* Return an char* with the grid->map. 
 *
 * Notes:
 *   char* points to the grid->map from the grid passed as an argument
 *   if the caller updates this char* it will be reflected in the grid
 *
 * validate grid
 * return grid->map
 */
char* grid_getMap(grid_t* grid);


/**************** grid_getLength ****************/
/* Return an int with strlen(grid->map). 
 *
 * validate grid
 * return strlen(grid->map)
 */
int grid_getLength(grid_t* grid);
