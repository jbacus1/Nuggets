# CS50 Nuggets
## Design Spec
### Team 7, Winter, 2025
### Nate, Jacob, Todd, Axel

> **Extra Credit:**
> Limited players' visibility to a radius of 5

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes a grid module and a player struct locally defined in the server file.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Client

The *client* acts in one of two modes:

 1. *spectator*, the passive spectator mode described in the requirements spec.
 2. *player*, the interactive game-playing mode described in the requirements spec.

### User interface

See the requirements spec for both the command-line and interactive UI.

The client’s only interface with the user is on the command-line; it must have at least two arguments, no more than three. 
```bash
./client <hostName> <port> [playerMode]
```

For example:
```bash
./client player 52872 [player]
```

### Inputs and outputs

Input: grid to display from server.

Output: Displays the updated map received by server, and any messages the server wants to give to the player. Also displays the “Game Over” screen at the end. 

### Functional decomposition into modules

We anticipate the following modules or functions for the client:
 1. Main, which parses arguments and initializes other modules
 2. parseArgs, which verifies arguments and returns error message if fails
 3. printGrid, for displaying our grid
 4. sendMessage, for communicating with the server using our protocol
 5. printMessage, for printing any updates from the server that aren’t our gri
 6. quitGame, for when the client wants to stop playing
 
### Pseudo code for logic/algorithmic flow

The client will run as follows:

	Take a hostname or ip address, a port, and an optional playername
	ParseArgs the given inputs to check for validity and player-type
		Initialize the network and join the server; let player know their player-type
		Initialize the display
		Wait for grid from server and verify that the display will fit our grid (NR+1 x NC+1)
		If yes
			Print status and grid
			While inputs are still being read
				Send keystrokes to server
				Update display whenever server sends an update
	Print game over message


### Major data structures

None.

---

## Server
### User interface

See the requirements spec for the command-line interface.

The server’s only interface with the user is on the command line; it must have at least one argument, no more than two.
```bash
./server <map.txt> [seed]
```

For example:
```bash
./server ../maps/dungeon.txt [155022]
```

### Inputs and outputs

Input: Keystrokes from connected client. Prints logging and any issues to stderr.

Output: A visualization of the map and exits 0 on success.

### Functional decomposition into modules

We anticipate the following modules or functions for the server:
 1. Main, runs processes of server
 2. parseArgs, validates arguments and assigns them to variable
 3. Handle messages, updates map based on input from client
 4. Send messages to client, broadcasts a string representing their visible map

### Pseudo code for logic/algorithmic flow

The server will run as follows:

	Takes a MAP and a SEED
	ParseArgs the MAP and SEED to check valid arguments
		If SEED provided use SEED
		Else generate a random SEED
	Load the map (assume valid)
	Drops the gold and records in int array
	Initialize and announce port
	Put in While Loop (player activity) + React
		Wait until a player joins (take up to MAXPLAYERS)
		Accept up to 1 spectator
		UPDATE player (quit and move)
		UPDATE gold
	Server-Client Protocol
		Server receives key inputs from client
		Server sends string 

pseudocode:

	Validate the arguments
	Initialize Server and Log for messaging
	Create strings for entire map, visible map, and spectator map
	Create sets of gold remaining, player locations
	Listen for Messages on port
		Check if player has previously connected
			If not add them to list of players
		Update gold remaining
		Update playerLocations
		Update visible map
		Update spectator map
		Send visible map to only the client
		Send spectator map to spectator if applicable
	Close Server and Log


### Major data structures

 1. Grid: stores a dynamic char* map, int width, and int height
 2. Player: Stores isActive, address, username, char representation on map, gold collected, location in map, personal visible map
 3. Hashtable: stores goldIndex-goldAmount pairs

---

## Grid module
A grid stores a dynamic char* map, int width, and int height.

### Functional decomposition

#### grid_new
Creates and returns grit_t* from string input. Will store char* map, int width, int height.
Assume there is a '\n' at the end of every line

#### grid_fromFile
Creates and returns grid_t* from txt file input. Assume there is a '\n' at the end of every line.

#### grid_get
Returns the char at a specific index.

#### grid_set
Updates char at an index. Returns true if successful and false if error.

#### grid_getWidth
Returns the grid width

#### grid_getHeight
Returns the grid height

#### grid_delete
Deletes the grid

#### grid_makeGold
Places gold around the grid in piles that vary randomly on the interval [10,30]. A total of 250 gold will be placed.
Updates char at goldIndex to '*'. Returns a set mapping goldIndex-goldAmount pairs.

#### grid_findWalls
Return as int[] with indices of all walls, roofs, and corners.

### Pseudo code for logic/algorithmic flow

#### grid_new

	create and allocate memory for a grid object
	create and allocate memory for string
	copy inputted string parameter into memory-allocated string
		set grid->map = string
	find the grid’s width by looping through characters until you reach a ‘\n'
		set as grid->width
	find the grid’s height by looping through the grid by factors of width and counting the number of ‘\n’
		consider final line (might need to add 1)
	validate map has constant dimensions
		free variables and return NULL if not
	set as grip->height
	return grid

#### grid_fromFile

	ensure txt file has '\n' at the end of every line
	call grid_new and pass char* from txt file as argument

#### grid_makeGold

	STILL WORKING ON THIS

#### grid_findWalls

	loop through all chars in grid->map
	if char at index is '|', '-', '+'
		store that index in int[]
	return int[]

### Major data structures

 1. hashtable
 2. dynamically allocated char*: used to store string on the heap so that it can be manipulated

---

## Testing Plan
_Unit Testing:_ The programs clientTest, serverTest, and gridTest will serve as unit tests for the client, server, and grid modules, respectively.  Below we describe the general testing for each: 

### Client
 1. Verify that all arguments get parsed correctly and nothing is incorrectly sent to server
 2. Verify that the client doesn’t unexpectedly close 
 3. Verify that after everything is initialized, the client is only ever sending chars

### Server
 1. Verify server can run and create a port
 2. Verify client properly initialized

### Grid
 1. Write testing.c file to thoroughly test and verify functionality of gird module
 2. Build grids from inputted char*
 3. Build grids from inputted .txt file
 4. Ensure grid_new() returns NULL where appropriate - Varying line widths - Invalid dimensions
 5. Ensure chars can be manipulated at appropriate indices - Can only change a char if it is ‘.’ or ‘*’
 6. Ensure printing the map works smoothly and without formatting errors
 7. Ensure grid can make gold piles that randomly vary between [10,30] nuggets and total to 250 nuggets - Flag error if not enough space to do this
 8. Ensure grid can find all barriers - This includes ‘|’, ‘+’, ‘-’
 9. Make sure usage and applicability are according to assumptions, and those assumptions are well documented
 10. Run with valgrind to ensure memory allocation for grid and char* map is done and freed properly

_Integration Testing:_ The nugget game, as a complete program, will be tested by building a server for the game to run on, and then testing how a normal game would play out.
 1. Create a simple version of the client and server which connect and give/send simple messages
 2. Utilize the provided client.exe and server.exe to test the other (e.g use server.exe to verify that our client implementation works as expected)
 3. Play the game ourselves and see how it runs 

