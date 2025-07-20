# CS50 Nuggets
## Implementation Spec
### Team 7, Winter, 2025

Our design also includes a grid module.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Client

### Data structures

The client will not implement any major data structures, as neither the design nor the requirement specs do not require that we store any information. 

### Definition of function prototypes

Function which handles our primary client logic. 
```c
int main(const int argc, char* argv[]);
```

Function for validating input arguments to ./client, and extracting them into params.
```c
static void parseArgs(const int argc, char* argv[],  char** serverHostName, char** serverPort, char** playerName, addr_t* server);
```

Function which reads a line of input and sends it to the server.
```c
static bool handleInput(void* arg);
```

Function for handling any message we receive from the server.
```c 
static bool handleMessage(void* arg, const addr_t from, const char* message); 
```

Function for verifying if the grid we’ve received from the server will fit our display. 
```c
static bool gridFitsDisplay(int nrows, const int ncols); 
```

### Detailed pseudo code

#### `main`:

	Take a hostname or ip address, a port, and an optional playername,
	Call ParseArgs on the given inputs to check for validity and player-type
	Initialize the message module 
	Initialize the display
	If client provided a player name,
		“Tell the server “PLAY [playerName]”
	Else,
		“Tell the server “SPECTATOR”
	Call message_loop, parsing over handleInput and handleMessage
	Shut down the message module
	Return status as result of message_loop 

#### `parseArgs`:

	Check for invalid # of arguments
	If no playerType is provided, assume player is a “spectator”
	Initialize the parameters 
	Set up the server address to communicate with
	If no errors, return nothing

#### `handleInput`:

	While inputs are still being read, 
		If player types “Q” 
			Return true 
		Else.
			Strip trailing newline from input
			Send keystrokes to server as “KEY k”
			Return false to keep looping

#### `handleMessage`:

	If message starts with “QUIT GAME OVER ”
		Print the provided scoreboard 
	Else if message starts with “QUIT ”
		Print the rest of the message to nCurses
	Else if message starts with “GRID\n  “
		If verifiedDisplay = false;
			call gridFitsDisplay to verify grid will fit
			verifiedDisplay = true
		Update nCurses to display the updated grid
	Else if message starts with “GOLD ”
		Update nCurses to display the new gold amount
	Else if message starts with “OK “ 
		Let the player know their playertype and that they joined successfully
	Return false if successful, true on any fatal error 

#### `gridFitsDisplay`:

	Use getmaxyx(stdscr, drows, dcols) to get the size of our display
	Check if nrows++ > drows 
	Check if ncols > dcols 
	Return false if either one fails, otherwise return true 

---

## Server

### Data structures

 1. Utilization of the grid module
 2. Hashtable mapping index to gold location
 3. Hashtable mapping player address to play struct
 4. A struct containing all information for a single player (besides address)
 	typedef struct player {
		bool isActive;
		char* name;
		char playerChar;
		int gold;
		int location;
		bool isSpectator;
		grid_t* visibleMap;
		grid_t* placesSeen;
	} player_t;
 5. A struct containing extra Arguments for handleMessage Call
	typedef struct messageArgs {
		grid_t* entireMap;
		int* walls
		hashtable_t* goldRemaining
		hashtable_t* playerLocations
 	} messageArgs_t;

### Definition of function prototypes

```c
int main(int argc, char* argv[]);
```

```c 
static void parseArgs(int argc, char* argv[], char** mapFile, int* seed);
```

```c
void handleMessage(grid_t* entireMap, grid_t* visibleMap, int* walls, hashtable_t* goldRemaining, hashtable_t* playerLocations, char theMessage, addr_t* clientAddress);
```

```c
void addDeleteCurrentPlayer(hashtable_t* playerLocations, addr_t* playerAddress);
```

```c
void updateGold(hashtable_t* goldRemaining, hashtable_t* playerLocations, addr_t* playerAddress, int newPlayerLoc);
```

```c
char* updateVisibility(grid_t* entireMap, grid_t* visibleMap, int* walls, hashtable_t* goldRemaining, hashtable_t* playerLocations, int curPlayerLoc);
```

```c
void changeAllVisibleMaps(grid_t* entireMap, char* newVisibleString, hashtable_t* playerLocations, player_t* curPlayer, int oldLoc, int newLoc);
```

```c
void iteratePlayerHashtable(void* entireMap, const char* goldLoc, void* goldAmount);
```

```c
void iterateGoldHashtable(void* entireMap, const char* goldLoc, void* goldAmount);
```

```c
int newSprintedLocation(grid_t* entireMap, player_t* curPlayer, char theMessage, hashtable_t* playerLocations, int currentPlayerLocation, char currentPlayerName);
```

```c
bool twoSidedHash(int curPlayerLoc, grid_t* entireMap);
```

```c
set_t* spaceBetween(int curWall, int curPlayerLoc, grid_t* entireMap);
```

### Detailed pseudo code

#### `main`:

	Call parseArgs
	Intialize and validate entire map from file
	Populate map with gold
	Create a new empty player locations hashtable
	Intialize the server and declare the port
	Create struct to hold arguments for message_loop
	Begin listening on socket for for messages from clients

#### `parseArgs`:

	If 2 given arguments
		Converts 2nd to int and make that seed
		Verify that given file is valid file
	If given 1 argument
		Uses time as seed
		Validates given file is valid file
	Seeds random number generator

#### `handleMessage`:

	If begins with PLAY
		Creates a player Struct if there is room
		Adds to player hashtable
	If begins with SPECTATE
		Loop through players to check if spectator exists
		Add a spectator player if not present
	If begins with KEY
		Check for q to quit and remove player
		Otherwise send to handleMessageContent

#### `handleMessageContent`:

	Takes from char
	If Sprint
		Call newSprintedLocation
	Else Gets new location from message key
	Check if new location is wall
	Call updateGold
	Call updateVisibility
	Call changeAllVisibleMaps
	Call printAllVisibleMaps

#### `updateGold`:

	Look for gold at player’s new location
	If gold amount is more than 0
		Player’s new gold is old gold + new gold
		Set gold amount at player location to -1

#### `updateVisibility`:

	If twoSidedHash
		Update up down right left from entire map to visible map
	Else
		Loop through all walls
			If no space or wall between given wall and new player location
				Create set of points between given wall and player location
				Iterate over set and replace char at visible location with char at entire map location
	Return new visible Map string

#### `changeAllVisibleMaps`:

	Call hashtable iterate on player locations using iteratePlayerHashtable
	Call hashtable iterate on gold remaining using iterateGoldHashtable

#### `iteratePlayerHashtable`:

	If location at visible map (not seen map) is not a space
		Make visible map at location the entire map at that location

#### `iterateGoldHashtable`:

	If gold is less than 0
		Replace entire map at location with ‘.’

#### `newSprintedLocation`:

	While next spot over is not a wall or empty space
		Move to next location
		Call updateGold
		Call updateVisibility
	Call changeAllVisibleMaps
	Call printAllVisibleMaps
	Return new location

#### `twoSidedHash`:

	Look at all 4 sides of new location
	If 2 of sides are hashtags
		Return true
	Else
		Return false

#### `spaceBetween`:

	Get the width difference of wall and player
	Get the height difference of wall and player
	Find slope between wall and player
	Loop through all points on diagonal between wall and player
		If one of these points is a space or a wall
			Return NULL
		Else
			Insert all points into set and return set

---

## Grid module

### Data structures

 1. hashtable: store (char*, void*) for (int goldIndex, int goldAmount)
 2. char*: dynamically allocated so that it is stored on the heap - caller of grid_new() or grid_fromFile() responsible for calling grid_delete()
 3. int*: returned in grid_getWalls() - caller responsible for freeing created int*

### Definition of function prototypes

Create new grid from char*.
#### `grid_new`:
```c
grid_t* grid_new(char* string);
```

Create new grid from .txt file.
#### `grid_fromFile`:
```c
grid_t* grid_fromFile(FILE* fp);
```

Get a char in grid at index.
#### `grid_get`:
```c
char grid_get(grid_t* grid, int index);
```

Set a char in grid at index.
#### `grid_set`:
```c
bool grid_set(grid_t* grid, int index, char character);
```

Get grid width.
#### `grid_getWidth`:
```c
int grid_getWidth(grid_t* grid);
```

Get grid height.
#### `grid_getHeight`:
```c
int grid_getHeight(grid_t* grid);
```

Free memory allocated for grid.
#### `grid_delete`:
```c
void grid_delete(grid_t* grid);
```

Print grid.
#### `grid_print`:
```c
void grid_print(grid_t* grid);
```

Make the gold in a grid. Gold total should be totGold and it should be distributed across random indices in piles that vary randomly in amount. The total number of gold piles should fall between minPiles and maxPiles.
#### `grid_makeGold`:
```c
hashtable_t* grid_makeGold(grid_t* grid, int minPiles, int maxPiles, int totGold);
```

Make the grid map empty.
#### `grid_makeEmpty`:
```c
void grid_makeEmpty(grid_t* grid);
```

Count the number of gold piles.
#### `grid_numGoldPiles`:
```c
int grid_numGoldPiles(grid_t* grid);
```

Return int* with all indices of obstructions.
#### `grid_getWalls`:
```c
int* grid_getWalls(grid_t* grid);
```

### Detailed pseudo code

#### `grid_new`:

	Validate parameter
	Create and allocate memory for grid object (NULL check)
	Create and allocate memory for grid->map (NULL check)
		Copy contents of inputted string into grid->map
	Loop through and find first occurrence of ‘\n’
		Set this as grid->width
	Loop through and count number of ‘\n’
		Set this a grid->height
	Deal with case where last char is not ‘\n’
	Return grid

#### `grid_fromFile`:

	Validate parameter
	Read file contents into a dynamically allocated char*
	Call grid_new() with the contents of open file
		Return NULL if created grid is NULL
	Free the contents of the file
		This won’t cause an issue because grid_new() copies the inputted char*
	Return the created grid

#### `grid_get`:

	Validate inputted grid
	Validate the inputted index
	Return the char at grid->map[index]

#### `grid_set`:

	Validate the grid
	Validate the inputted index
	Return false and do nothing if caller is trying to modify a char that can’t be modified
		Space, wall, roof, corner, tunnel
	Update grid->map[index] to new character and return true

#### `grid_getWidth`:

	Validate grid
	Return grid->width

#### `grid_getHeight`:

	Validate grid
	Return grid->height

#### `grid_delete`:

	Validate grid
	Free grid->map
	Free grid

#### `grid_print`:

	Validate grid
	Print grid->map to stdout

#### `grid_makeGold`:

	Count number of changeable chars in grid
		Char only valid to make gold if ‘.’
	Verify that there are enough valid positions to place the gold in the worst case (when maxPiles) are placed
	Create hashtable to store (goldIndex -> goldAmount)
	Loop until all gold placed
		Create random index
		If grid->map[randIndex] is not a ‘.’
			Continue
		Create randomAmount of golf to place
		Validate you won’t place too much gold if you place randAmount
			Place totGold - goldPlaced if you will go over
		Create char* representation of randIndex
		Create void* representation of randomAmount
		Insert (randIndex -> randomAmount) into hashtable
		Update the grid (place a ‘*’ at randIndex)
		Update goldPlaced
	Return hashtable goldMap

#### `grid_makeEmpty`:

	Validate grid
	Loop through grid->map and turn every character that isn’t ‘\n’ to a space

#### `grid_numGoldPiles`:

	Validate grid
	Loop through grid->map and count number of gold piles (‘*’)

#### `grid_getWalls`:

	Validate grid
	Count all walls, roofs, corners
	Create int* with memory to hold all walls, roofs, corners
	Loop through grid->map and populate int* with indices of all walls, roofs, corners
	Return int*

---

## Testing plan

### Unit testing
The programs clientTest, serverTest, and gridTest will serve as unit tests for the client, server, and grid modules, respectively.  Below we describe the general testing for each: 

#### Client
Write a testing.sh file to verify that:
 1. all arguments get parsed correctly and nothing is incorrectly sent to the server
 2. Verify that the client doesn’t unexpectedly close 
 3. Verify that after everything is initialized, the client is only ever sending chars 

#### Server
 1. Verify server can run and create a port
 2. Verify client properly initialized
 3. Ensure grid generation and initialization of hashtables
 4. Check that memory is properly freed
 5. Ensure that server properly listens on socket


#### Grid
Write testing.c file to thoroughly test and verify functionality of gird module:
 1. Build grids from inputted char*
 2. Build grids from inputted .txt file
 3. Ensure grid_new() returns NULL where appropriate - Varying line widths, Invalid dimensions
 4. Ensure chars can be manipulated at appropriate indices - Can only change a char if it is ‘.’ or ‘*’
 5. Ensure printing the map works smoothly and without formatting errors
 6. Ensure grid can make gold piles that randomly vary between [10,30] nuggets and total to 250 nuggets - Flag error if not enough space to do this
 7. Ensure grid can find all barriers - This includes ‘|’, ‘+’, ‘-’
Make sure usage and applicability are according to assumptions, and those assumptions are well documented

Run with valgrind to ensure memory allocation for grid and char* map is done and freed properly

### integration and system testing

The nugget game, as a complete program, will be tested by building a server for the game to run on, and then testing how a normal game would play out. 
 1. Create a simple version of the client and server which connect and give/send simple messages
 2. Utilize the provided client.exe and server.exe to test the other (e.g use server.exe to verify that our client implementation works as expected)
 3. Play the game ourselves and see how it runs 

---

## Limitations

 1. Grid only works with maps that follow a specific protocol
 2. Grid only deals with specific characters - ' ', '.', '|', '-', '+', '*', '#'
 3. Assume valid sockets for client/server communication
 4. Client/server communication only deals with specific keys
