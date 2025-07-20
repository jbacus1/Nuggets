/*
 * Created by Nate Abbott and Jacob Bacus
 * Jacob Bacus is writing server protcol
 * Nate Abbott is writing visibility code
 * Date Created: 3/3/25
 * Last Updated: 3/12/25
 * Purpose: Make server work
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "../support/log.h"
#include "grid.h"
#include "../support/message.h"
#include "../libcs50/set.h"
#include <math.h>

/****************** Global Constants *******************/
static const int MAX_PLAYERS = 26;        // max players in game
static const int GOLD_TOTAL = 250;        // amount of gold game has
static const int GOLD_MIN_NUM_PILES = 10; // minmimum gold piles
static const int GOLD_MAX_NUM_PILES = 30; // maximum gold piles

/***************** Player Struct *******************/
typedef struct player
{
  bool isActive;
  char *name;
  char playerChar;
  int gold;
  int location;
  grid_t *visibleMap;
  grid_t *placesSeen;
  addr_t address;
} player_t;


// Calls in order: 1) addDeleteCurrentPlayer, 2) find new player location (newSprintedLocation), 3) 1 line to update Player, 4) updateGold, 5) changeVisibleMaps, 6) sendVisibility
void handleMessageContent(grid_t *entireMap, int *walls, hashtable_t *goldRemaining, hashtable_t *playerLocations, char move, const char *from, addr_t clientAddress);

// Return updated playerLocations
// Checks for 'Q' and adds/removes players (DOES NOT MOVE PLAYER)
void addDeleteCurrentPlayer(hashtable_t *playerLocations, addr_t playerAddress);

// Return updated goldRemaining
void updateGold(hashtable_t *goldRemaining, hashtable_t *playerLocations, addr_t playerADDRAddress, const char *playerCharAddress, int newPlayerLoc);

// Changes player visibleMap and returns the changed value
// Calls in order: 1) twoSidedHash, 2) spaceBetween
void updateVisibility(grid_t *entireMap, player_t *player, int *walls, hashtable_t *goldRemaining, int oldLoc);

// Changes all the players visibleMaps
// Calls hashtable_iterate using 1) iteratePlayerHashtable, and 2) iterateGoldHashtable
void changeAllVisibleMaps(grid_t *entireMap, hashtable_t *goldRemaining, hashtable_t *playerLocations, player_t *curPlayer, int oldLoc, int newLoc);

// Iterates through all the players and updates their visible maps
void iteratePlayerHashtable(void *entireMap, const char *address, void *player);

// Iterate through all the goldRemaining and update entireMap accordingly
void iterateGoldHashtable(void *entireMap, const char *goldLoc, void *goldAmount);

// Iterates through every sprinted through location
// Calls in order: 1) update the player, 2) updateGold, 3) updateVisibility (includes updating spectator), 4) sendVisibility
int newSprintedLocation(grid_t *entireMap, player_t *curPlayer, char move, const char *playerCharAddress, hashtable_t *goldRemaining, hashtable_t *playerLocations, int *walls, int currentPlayerLocation, addr_t spectator);

// Returns true if the character is in a cooridor
bool twoSidedHash(int curPlayerLoc, grid_t *entireMap);

// Returns all the points between the wall and the (new) player location
// IFF there are no " " or "|" between the wall and the (new) player location
set_t *spaceBetween(int curWall, int curPlayerLoc, grid_t *entireMap);

// Checks if all inputs are valid
static void parseArgs(int argc, char *argv[], char **mapFile, FILE **fileAddress, int *seed);

// loops in message_loop to deal with input on socket
bool handleMessage(void *arg, const addr_t from, const char *message);

// attempts to add a new player to playerLocations
// returns true or false based on success
bool addNewPlayer(hashtable_t *playerLocations, const char *username, const char *addr, grid_t *entireMap, addr_t givenAddress);

// attemps to add new spectator or change existing spectator
// returns true of false based on success
bool addNewSpectator(const addr_t oldAddress, const addr_t newAddress);

// helper function that counts keys in hashtable_t
void count_keys(void *arg, const char *key, void *item);

// function that counts items in hashtable based on their key existing
int hashtable_key_count(hashtable_t *ht);

// generate a message for game over
char *generateQuitMessage(hashtable_t *playerLocations);

// helper function to add player stats to game over message
void formatPlayerInfo(void *arg, const char *key, void *value);

// function to send game over to all players
void broadCastQuitMessage(hashtable_t *playerLocations, char *message);

// helper function that sends individual players a game over message
void sendPlayerQuitMessage(void *arg, const char *key, void *value);

// helper function that adds gold in hashtable to a total
void sumGold(void *arg, const char *key, void *value);

// function to check if there is gold left
bool isGoldEmpty(hashtable_t *goldMap);

// function to loop through players and prtin their visible maps
void printAllVisibleMaps(hashtable_t *playerLocations);

// helper function that sends a player their udpated visible map
void send_display_map(void *arg, const char *key, void *item);

// sends the spectator the entireMap
void printSpectatorMap(addr_t spectatorAddr, hashtable_t *playerLocations,
                       grid_t *entireMap);

// helper function that adds players to entireMap
void addPlayersToMap(void *arg, const char *key, void *item);

// helper function to free memory for goldRemaining
void freeGoldEntry(void *item);

// helper function to free memory for playerLocations
void freePlayerEntry(void *item);

// sends a message to every client about a change in gold value
void goldUpdateMessage(hashtable_t *playerLocations, hashtable_t *goldRemaining,
                       addr_t pickedUpAddr, int goldAmount);

// helper function to sum all gold in map
void sumRemainingGold(void *arg, const char *key, void *value);

// helper function to send an update message to every client about gold value change and or pickup
void sendGoldUpdate(void *arg, const char *key, void *value);

// Updates a diagonal - called in updateVisibility for set iterate
void updateDiagonal(void *arg, const char *key, void *item);

// if moved player is visible, then make them visible
void iterateMovedPlayer(void *movedPlayer, const char *address, void *curPlayer);

// swap players if one moves into another
void swapPlayerLocation(void* data, const char* key, void* item);


/**************** main function *************************/
int main(int argc, char *argv[])
{
  FILE *fileAddress = NULL;
  char *mapFile = NULL;
  int seed = 0;

  // define struct locally
  typedef struct messageArgs
  {
    grid_t *entireMap;
    int *walls;
    hashtable_t *goldRemaining;
    hashtable_t *playerLocations;
    addr_t spectator;
  } messageArgs_t;

  // validate the arguments given in command line
  parseArgs(argc, argv, &mapFile, &fileAddress, &seed);

  // create our full grid
  grid_t *grid = grid_fromFile(fileAddress);
  if (grid == NULL)
  {
    fprintf(stderr, "Error: could not load map from file '%s'\n", mapFile);
    exit(3);
  }

  hashtable_t *goldRemaining = grid_makeGold(grid, GOLD_MIN_NUM_PILES,
                                             GOLD_MAX_NUM_PILES, GOLD_TOTAL);
  if (goldRemaining == NULL)
  {
    fprintf(stderr, "Error: could not generate gold in map\n");
    grid_delete(grid);
    exit(4);
  }

  // Make empty player's hashtable
  hashtable_t *playerLocations = hashtable_new(MAX_PLAYERS);
  if (playerLocations == NULL)
  {
    fprintf(stderr, "Error: could not create player location hashtable.\n");
    grid_delete(grid);
    hashtable_delete(goldRemaining, freeGoldEntry);
    exit(5);
  }

  // begin server logging
  log_init(stderr);

  // begins running server (message_init should print port)
  int serverPort = message_init(stderr);
  if (serverPort == 0)
  {
    log_e("failed to intialize server.\n");
    grid_delete(grid);
    hashtable_delete(goldRemaining, freeGoldEntry);
    hashtable_delete(playerLocations, freePlayerEntry);
    log_done();
    exit(6);
  }

  log_d("Server running on port %d", serverPort);

  // create a struct for the extra args in message_loop
  messageArgs_t args;

  int *walls = grid_getWalls(grid);
  if (walls == NULL)
  {
    fprintf(stderr, "Failed to gather grid walls.\n");
    grid_delete(grid);
    hashtable_delete(goldRemaining, freeGoldEntry);
    hashtable_delete(playerLocations, freePlayerEntry);
    log_done();
    message_done();
    exit(8);
  }

  // add in values for args
  args.entireMap = grid;
  args.playerLocations = playerLocations;
  args.goldRemaining = goldRemaining;
  args.walls = walls;
  args.spectator = message_noAddr();

  // continual loop of server running
  message_loop(&args, 0, NULL, NULL, handleMessage);
  // should terminate by returning false after detecting no gold remaining

  // creates a message for quitting with final score
  char *message = generateQuitMessage(playerLocations);
  if (message == NULL)
  {
    fprintf(stderr, "Failed to make final message.\n");
    grid_delete(grid);
    free(args.walls);
    hashtable_delete(goldRemaining, freeGoldEntry);
    hashtable_delete(playerLocations, freePlayerEntry);
    log_done();
    message_done();
    exit(9);
  }

  // iterates over players and sends quit message
  broadCastQuitMessage(playerLocations, message);
  message_send(args.spectator, message);
  free(message);

  // cleanup
  message_done();
  log_done();
  grid_delete(grid);
  hashtable_delete(goldRemaining, freeGoldEntry);
  hashtable_delete(playerLocations, freePlayerEntry);
  free(walls);

  exit(0);
}

// helper function to free allocated mmeory for goldRemaining hashtable
void freeGoldEntry(void *item)
{
  free((int *)item); // free int*
}

// helper function to free allocated memory for playerLocations hashtable
void freePlayerEntry(void *item)
{
  // cast player to right type
  player_t *player = (player_t *)item;

  // delete malloc'd grids
  grid_delete(player->visibleMap);
  grid_delete(player->placesSeen);
  free(player->name);
  free(player); // free
}

// validates arguments and assigns them to variables if they are valid
static void parseArgs(int argc, char *argv[], char **mapFile, FILE **fileAddress, int *seed)
{
  // assign given file to mapFile address
  *mapFile = argv[1];

  // if a 2nd argument is given, read as seed for random number generator
  if (argc == 3)
  {
    // casts the written seed to an int (NEEDS ERROR CHECK)
    *seed = (int)strtol(argv[2], NULL, 10);

    // tries to validate mapFile by opening in read mode
    FILE *fp = fopen(*mapFile, "r");
    if (fp == NULL)
    {
      fprintf(stderr, "Error: cannot open map file '%s'\n", *mapFile);
      exit(2);
    }
    *fileAddress = fp;
  }
  else if (argc == 2)
  {
    // turns seed into the int version of time
    *seed = (int)time(NULL);

    // tries to validate mapFile by opening in read mode
    FILE *fp = fopen(*mapFile, "r");
    if (fp == NULL)
    {
      fprintf(stderr, "Error: cannot open map file '%s'\n", *mapFile);
      exit(2);
    }
    *fileAddress = fp;
  }
  else
  {
    fprintf(stderr, "Usage: %s mapFile [optional] seed\n", argv[0]);
    exit(1);
  }

  // this should seed random number generation across this whole program
  srand(*seed);
}

// creates a quit message that caller must free
char* generateQuitMessage(hashtable_t *playerLocations)
{
  char *message = malloc(2048);
  if (!message)
  {
    return NULL;
  }

  strcpy(message, "QUIT GAME OVER:\n");                          // start with a header
  hashtable_iterate(playerLocations, message, formatPlayerInfo); // add players

  return message; // must be freed
}

void formatPlayerInfo(void *arg, const char *key, void *value)
{
  char* message = (char *)arg;
  player_t* player = (player_t *)value;

  if (player != NULL)
  {
    char line[100]; // temporary buffer for line
    snprintf(line, sizeof(line), "%c %10d %s\n", player->playerChar, player->gold, player->name);
    strcat(message, line); // add line to message
  }
}

// broadcast quit message to all players
void broadCastQuitMessage(hashtable_t *playerLocations, char *message)
{
  hashtable_iterate(playerLocations, message, sendPlayerQuitMessage);
}

// retrieves player address and sends them the quit message
void sendPlayerQuitMessage(void *arg, const char *key, void *value)
{
  char *message = (char *)arg;
  player_t *player = (player_t *)value;

  if (player != NULL)
  {
    message_send(player->address, message);
  }
}

// runs in message_loop, recieving any message from the socket
// handleMessage has a specific format, this is what we are stuck with to my knowledge
bool handleMessage(void *arg, const addr_t from, const char *message)
{

  // we use this to index into our hashtable
  const char *fromString = message_stringAddr(from);

  // local struct for args pashed to this function
  struct
  {
    grid_t *entireMap;
    int *walls;
    hashtable_t *goldRemaining;
    hashtable_t *playerLocations;
    addr_t spectator;
  } *args = arg;

  // convert to proper pointer and check
  if (args == NULL)
  {
    log_e("Failed to convert args from (void*)");
  }

  if (isGoldEmpty(args->goldRemaining))
  {
    return true;
  }

  if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0)
  {
    const char *content = message + strlen("PLAY ");

    // Attempt to add new player
    if (addNewPlayer(args->playerLocations, content, fromString, args->entireMap, from))
    {

      // sends player confirmation of joining with their char
      player_t *newPlayer = hashtable_find(args->playerLocations, fromString);
      char joinMessage[] = "OK X";
      joinMessage[3] = newPlayer->playerChar;
      message_send(from, joinMessage);

      // sends player grid on join
      int width = grid_getWidth(args->entireMap);
      int height = grid_getHeight(args->entireMap);
      char gridMessage[32];
      snprintf(gridMessage, sizeof(gridMessage), "GRID %d %d", height, width);
      message_send(from, gridMessage);

      // Attemping to create an intial print of display message
      updateVisibility(args->entireMap, newPlayer, args->walls, args->goldRemaining, newPlayer->location);

      // finally, send them their current visible map
      printAllVisibleMaps(args->playerLocations);
      printSpectatorMap(args->spectator, args->playerLocations, args->entireMap);

      return false;
    }
    else
    {
      // inform if game is full
      char *gameFullMessage = "QUIT Game is full";
      message_send(from, gameFullMessage);
      return false;
    }
  }
  else if (strncmp(message, "SPECTATE", strlen("SPECTATE")) == 0)
  {

    // assign a new spectator after checking to kick the old one
    if (addNewSpectator(args->spectator, from))
    {
      args->spectator = from;

      // sends player grid on join
      int width = grid_getWidth(args->entireMap);
      int height = grid_getHeight(args->entireMap);
      char gridMessage[32];
      snprintf(gridMessage, sizeof(gridMessage), "GRID %d %d", height, width);
      message_send(from, gridMessage);

      // gives spectator their first map
      printSpectatorMap(from, args->playerLocations, args->entireMap);

      return false;
    }
    else
    {
      // This should not happen
      log_e("Unexpected behavior spectator failed to join\n");
      message_send(from, "QUIT could not join");
      return false;
    }
  }
  else if (strncmp(message, "KEY ", strlen("KEY ")) == 0)
  {
    const char *content = message + strlen("KEY ");

    char messageChar = '\0';
    // validates that we recieved a single key char and turns it into a char
    if (content[0] != '\0' && content[1] == '\0')
    {
      messageChar = content[0];
    }
    else
    {
      log_s("Invalid format of message '%s'", message);
      return false;
    }

    // delete the player from hashtable
    // Q Press

    player_t *p = hashtable_find(args->playerLocations, fromString);

    // if we have a non spectator inactive player ignore
    // if spectator is rejoined inactive player then continue
    if (p != NULL && p->isActive == false && !message_eqAddr(args->spectator, from))
    {
      return false; // ignore becuase this player is inactive (should only happen in testing)
    }

    if (messageChar == 'Q' || messageChar == 'q')
    {
      // check that this is a player (not a spectator)

      if (p != NULL)
      {
        // if it's a real player
        p->isActive = false;
        char *quitMessage = "QUIT Thanks for playing!";
        message_send(from, quitMessage);
      }
      else if (message_eqAddr(args->spectator, from))
      {
        // it's the spectator
        char *spectatorQuitMessage = "QUIT Thanks for spectating!";
        message_send(from, spectatorQuitMessage);
        args->spectator = message_noAddr();
      }
      else
      {
        log_e("Unexpected address for quit message");
      }
      return false;
    }

    // ignore all other spectator key presses
    if (message_eqAddr(args->spectator, from)) {
      return false;
    }

    const char *fromString = message_stringAddr(from);
    if (p == NULL)
    {
      // not a known player so just ignore
      return false;
    }

    // process message content for single char of movement (guarenteed not Q)
    handleMessageContent(args->entireMap, args->walls,
                         args->goldRemaining, args->playerLocations,
                         messageChar, fromString, args->spectator);
    return false;
  }
  else
  {
    log_s("Recieved an unexpected message '%s'", message);
    return false;
  }
}

// Callback function to sum all gold amounts
void sumGold(void *arg, const char *key, void *value)
{
  int *goldAmount = (int *)value;
  int *totalGoldLeft = (int *)arg;

  if (goldAmount != NULL && *goldAmount != -1)
  {
    *totalGoldLeft += *goldAmount;
  }
}

// Function to check if all gold has been collected
bool isGoldEmpty(hashtable_t *goldMap)
{
  if (goldMap == NULL)
  {
    return true; // No gold exists somehow
  }

  int totalGoldLeft = 0;
  hashtable_iterate(goldMap, &totalGoldLeft, sumGold);

  return (totalGoldLeft == 0);
}

// check if we can add a player and if we can, add them
bool addNewPlayer(hashtable_t *playerLocations, const char *username, const char *addr, grid_t *entireMap, addr_t givenAddress)
{
  if (playerLocations == NULL)
  {
    return false;
  }

  player_t *existingPlayer = hashtable_find(playerLocations, addr);

  if (existingPlayer != NULL)
  {
    // key already in table
    if (existingPlayer->isActive)
    {
      log_s("Tried to reactivate active player at %s", addr);
    }
    else
    {
      // copy name otherwise bad pointer reference
      size_t len = strlen(username) + 1;
      char* nameCopy = malloc(len);
      if (nameCopy == NULL) {
        return false;
      }
      strcpy(nameCopy, username);

      // give player new attributes
      existingPlayer->name = nameCopy;
      existingPlayer->isActive = true;
      existingPlayer->gold = 0;

      grid_makeEmpty(existingPlayer->visibleMap);
      grid_makeEmpty(existingPlayer->placesSeen);

      return true;
    }
  }

  // counts players
  int nPlayer = hashtable_key_count(playerLocations);

  // reutrn false if too many players
  if (nPlayer >= 26)
  {
    return false;
  }
  else
  {
    // create empty player struct
    player_t *player = malloc(sizeof(player_t));
    if (player == NULL)
    {
      return false;
    }

    // duplicate so no bad pointer
    size_t len = strlen(username) + 1;
    char* nameCopy = malloc(len);
    if (nameCopy == NULL) {
      return false;
      free(player);
    }
    strcpy(nameCopy, username);

    // fills in player struct attributes
    player->isActive = true;
    player->name = nameCopy;
    player->playerChar = 'A' + nPlayer;
    player->gold = 0;

    // finds a valid starting location to give player
    int startingLocation = 0;
    char locationChar = grid_get(entireMap, startingLocation);
    size_t length = grid_getLength(entireMap); 

    // guesses random locations until one is empty
    while (locationChar != '.')
    {
      startingLocation = rand() % length;

      locationChar = grid_get(entireMap, startingLocation);
    }

    player->location = startingLocation;

    // gives player new empty grids for their visible map and places seen
    grid_t *vMap = grid_new(grid_getMap(entireMap));
    grid_t *pMap = grid_new(grid_getMap(entireMap));
    grid_makeEmpty(vMap);
    grid_makeEmpty(pMap);
    player->visibleMap = vMap;
    player->placesSeen = pMap;

    player->address = givenAddress;

    return hashtable_insert(playerLocations, addr, player);
  }
}

// add new spectator or change existing spectator
bool addNewSpectator(const addr_t oldAddress, const addr_t newAddress)
{
  if (message_eqAddr(oldAddress, message_noAddr()))
  {
    return true;
  }
  else
  {
    // don't forget to kick old spectator
    char *kickMessage = "QUIT You have been replaced by a new spectator.";
    message_send(oldAddress, kickMessage);
    return true;
  }
}

// helper funciton for iteration
void count_keys(void *arg, const char *key, void *item)
{
  if (key != NULL)
  {
    (*(int *)arg)++; // iterate count that is in arg
  }
}

// go through and count items in a hashtable (if key exists)
int hashtable_key_count(hashtable_t *ht)
{
  if (ht == NULL)
  {
    return 0;
  }

  int count = 0;
  hashtable_iterate(ht, &count, count_keys);
  return count;
}

// Give a new message I will update gold and visibility for all players
void handleMessageContent(grid_t *entireMap, int *walls, hashtable_t *goldRemaining, hashtable_t *playerLocations, char move, const char *from, addr_t spectator)
{

  // IF "Q" THEN PRIOR FUNCTION ALREADY TAKE CARE OF THAT
  // Should only be 1 char (ex. h,j,k,l)

  player_t *curPlayer = hashtable_find(playerLocations, from);

  // Check if curPlayer exists
  if (curPlayer == NULL)
  {
    log_e("SOMETHING WENT VERY WRONG -- THERE IS NO PLAYER AT ADDRESS\n");
    return;
  }

  int oldLoc = curPlayer->location;

  int width = grid_getWidth(entireMap) + 1;

  int newLoc = -1;

  // Based off of:
  // y k u
  // h @ l
  // b j n
  // q = quit = return -1
  // anything else return NULL
  if (move == 'h')
  {
    newLoc = oldLoc - 1;
  }
  else if (move == 'y')
  {
    newLoc = oldLoc - 1 - width;
  }
  else if (move == 'k')
  {
    newLoc = oldLoc - width;
  }
  else if (move == 'u')
  {
    newLoc = oldLoc + 1 - width;
  }
  else if (move == 'l')
  {
    newLoc = oldLoc + 1;
  }
  else if (move == 'n')
  {
    newLoc = oldLoc + 1 + width;
  }
  else if (move == 'j')
  {
    newLoc = oldLoc + width;
  }
  else if (move == 'b')
  {
    newLoc = oldLoc - 1 + width;
  }
  else if (move == 'H' || move == 'Y' || move == 'K' || move == 'U' || move == 'L' || move == 'N' || move == 'J' || move == 'B')
  {
    // Look if caps lock therefore sprinting. Loop through all points with samne code as belwo

    newLoc = newSprintedLocation(entireMap, curPlayer, move, from, goldRemaining, playerLocations, walls, oldLoc, spectator);
    // Do everything in here because I already check visibility and wall constraints.
    curPlayer->location = newLoc;
    return;
  }
  else
  {
    return;
  }

  // Check if we're just hitting a wall or out of bounds
  char newLocChar = grid_get(entireMap, newLoc);
  if (newLocChar == '|' || newLocChar == '-' || newLocChar == '+' || newLocChar == ' ')
  {
    // return oldLoc;
    log_e("YOU HIT A WALL\n");
    return;
  }
  else if ((newLoc % width == 0 && oldLoc % width == 1) || (newLoc % width == 1 && oldLoc % width == 0))
  {
    // Would only happen if cooridors are right on the edge of the screen
    log_e("YOU TIRED MOVING OFF THE BOARD (sideways)\n");
    return;
  }
  else if ((newLoc < width + 1 && oldLoc > grid_getLength(entireMap) - width) || (newLoc > grid_getLength(entireMap) - width && oldLoc < width + 1))
  {
    // Would only happen if cooridors are right on the edge of the screen
    log_e("YOU TIRED MOVING OFF THE BOARD (up/down)\n");
    return;
  }

  // Function names here are intuitive.
  // Put the newLoc into player to replace the oldLoc
  curPlayer->location = newLoc;
  updateGold(goldRemaining, playerLocations, curPlayer->address, from, newLoc);

  updateVisibility(entireMap, curPlayer, walls, goldRemaining, oldLoc);

  // Swapping with player if I landed on them
  int data[3] = {(int)curPlayer->playerChar, oldLoc, newLoc};
  hashtable_iterate(playerLocations, data, swapPlayerLocation);

  changeAllVisibleMaps(entireMap, goldRemaining, playerLocations, curPlayer, oldLoc, newLoc);

  printAllVisibleMaps(playerLocations);
  printSpectatorMap(spectator, playerLocations, entireMap);
}

void swapPlayerLocation(void* data, const char* key, void* item){
  // look at all player locations. If they're the same as newLoc then put the playerChar at oldLoc
  int *intData = (int *)data; // Cast to int*
  char playerChar = (char) intData[0];
  int oldLoc = intData[1];
  int newLoc = intData[2];
  
  player_t *player = (player_t *) item;
  if((playerChar != player->playerChar) && newLoc == player->location){
    player->location = oldLoc;
  }
}

// Will work by looping through player hashtable getting their visible maps then sending them to all addresses
void printAllVisibleMaps(hashtable_t *playerLocations)
{
  if (playerLocations == NULL)
  {
    return;
  }

  hashtable_iterate(playerLocations, NULL, send_display_map);
}

void send_display_map(void *arg, const char *key, void *item)
{
  if (item == NULL) return; // check for NULL pointer

  // check that nothing is NULL
  player_t *player = (player_t *)item;
  if (player == NULL) return;
  grid_t* vMap = player->visibleMap;
  grid_t* placesSeen = player->placesSeen;
  if (vMap == NULL || placesSeen == NULL) {
    return;
  }

  // bad address
  if (message_eqAddr(player->address, message_noAddr())) {
    return;
  }

  // get string from grids
  char* vMapString = grid_getMap(vMap);
  char* placesSeenString = grid_getMap(placesSeen);


  if (vMapString == NULL || placesSeenString == NULL) {
    return;
  }

  // add prefix for message_send
  int prefixLen = 8; // GIVEN
  int mapLen = grid_getLength(placesSeen);
  int totalLen = prefixLen + mapLen + 1; // for null terminator

  char *message = malloc(totalLen);
  if (message == NULL) return; // out of memory

  // add on prefix
  snprintf(message, totalLen, "DISPLAY\n%s", placesSeenString);

  for (int i = 0; i < mapLen; i++) {
    char c = vMapString[i];
    if (c == '*' || (c >= 'A' && c <= 'Z')) {
      message[i+prefixLen] = c;
    }
  }

  message[player->location + prefixLen] = '@'; // replace own char with @ sign

  // send message to address stored in player
  message_send(player->address, message);

  // free
  free(message);
}

void printSpectatorMap(addr_t spectatorAddr, hashtable_t *playerLocations, grid_t *entireMap)
{
  // check for bad args
  if (playerLocations == NULL || message_eqAddr(spectatorAddr, message_noAddr()) || entireMap == NULL)
  {
    return;
  }

  size_t prefixLen = 8;
  int mapLen = grid_getLength(entireMap);
  size_t totalLen = prefixLen + mapLen + 1; // for null terminator

  char *message = malloc(totalLen);
  if (message == NULL) return; // out of memory

  // write into the message
  snprintf(message, totalLen, "DISPLAY\n%s", grid_getMap(entireMap));

  // add players' chars
  hashtable_iterate(playerLocations, message, addPlayersToMap);

  message_send(spectatorAddr, message);

  free(message);
}

// a function to add player's char to each player's location on a map
void addPlayersToMap(void *arg, const char *key, void *item)
{
  if (item == NULL || arg == NULL) return;

  // cast arg and item
  player_t *player = (player_t *)item;
  char *mapString = (char *)arg;

  // can guarentee 8 bc of prefixLen
  if (player->location >= 8)
  {
    mapString[player->location + 8] = player->playerChar;
  }
}

// Will go to the 1 before the closest wall or
// if in a corridor will go to farthest '#'
int newSprintedLocation(grid_t *entireMap, player_t *curPlayer, char move, const char *playerCharAddress, hashtable_t *goldRemaining, hashtable_t *playerLocations, int *walls, int currentPlayerLocation, addr_t spectator)
{

  int width = grid_getWidth(entireMap) + 1;
  int prevLoc = curPlayer->location; // 2 back
  int nextLoc = curPlayer->location; // 0 back
  int curLoc = curPlayer->location;  // 1 back
  
  char charMapSpot = grid_get(entireMap, curLoc);

  // Based off of:
  // y k u
  // h @ l
  // b j n

  // Keep going up/down/right/left until you hit a wall or " " (meaning end of cooridor)
  while (charMapSpot != '|' && charMapSpot != '-' && charMapSpot != '+' && charMapSpot != ' ')
  {
    // update new then have old become new
    prevLoc = curLoc; // 1 back = 2 back, will then be
    curLoc = nextLoc; // 2 back = 0 back, will then be 1 back
    if (move == 'H')
    {
      nextLoc = prevLoc - 1; // making 0 back
    }
    else if (move == 'Y')
    {
      nextLoc = prevLoc - width - 1;
    }
    else if (move == 'K')
    {
      nextLoc = prevLoc - width;
    }
    else if (move == 'U')
    {
      nextLoc = prevLoc - width + 1;
    }
    else if (move == 'L')
    {
      nextLoc = prevLoc + 1;
    }
    else if (move == 'N')
    {
      nextLoc = prevLoc + width + 1;
    }
    else if (move == 'J')
    {
      nextLoc = prevLoc + width;
    }
    else if (move == 'B')
    {
      nextLoc = prevLoc + width - 1;
    }
    else
    {
      log_e("Something went very wrong in SPRINT\n");
      return -1;
    }

    

    // set the next location
    charMapSpot = grid_get(entireMap, nextLoc);

    // set player New Location
    curPlayer->location = curLoc;

    // call updateVisibility and updateGold
    updateGold(goldRemaining, playerLocations, curPlayer->address, playerCharAddress, curLoc);
    updateVisibility(entireMap, curPlayer, walls, goldRemaining, curLoc);
    changeAllVisibleMaps(entireMap, goldRemaining, playerLocations, curPlayer, prevLoc, curLoc);
  }

  // Sends visible map to all players
  printAllVisibleMaps(playerLocations);
  printSpectatorMap(spectator, playerLocations, entireMap);
  return curLoc;
}

// updates the goldRemaining set.
// ***ALSO updates the players gold
void updateGold(hashtable_t *goldRemaining, hashtable_t *playerLocations, addr_t playerADDRAddress, const char *playerCharAddress, int newPlayerLoc)
{
  // look if there's a spot of gold in the new location
  // ****************** CHANGE newPlayerLoc INTO A STRING ***********************
  char locKey[20];
  snprintf(locKey, sizeof(locKey), "%d", newPlayerLoc);

  // get gold amount. goldAmount = -1 means its been taken.
  // Therefor if <0 do nothing
  int *goldAmount = hashtable_find(goldRemaining, locKey);
  int justPickedUp = 0;
  if (goldAmount != NULL && *goldAmount > 0)
  {
    justPickedUp = *goldAmount;

    // Setting the gold
    player_t *curPlayer = hashtable_find(playerLocations, playerCharAddress);
    if (curPlayer != NULL)
    {
      int prevGold = curPlayer->gold;
      int curGold = prevGold + *goldAmount;
      curPlayer->gold = curGold;
    }

    // Mskr the seen gold -1 so it is no longer seen
    *goldAmount = -1;
    goldUpdateMessage(playerLocations, goldRemaining, playerADDRAddress, justPickedUp);
  }
}

// In order to give the client display infromation, we must give them a gold update message
// This message has form GOLD n p r (number picked up, purse gold, remaining nuggets)
void goldUpdateMessage(hashtable_t *playerLocations, hashtable_t *goldRemaining, addr_t pickedUpAddr, int goldAmount)
{

  // predefine a struct for passing multiple args into out goldUpdate hashtable_iterate
  typedef struct
  {
    int goldPickedUp;
    addr_t pickedUpAddr;
    int nuggetsRemaining;
  } goldUpdateData_t;


  // check for bad arguments passed or if there is no gold
  if (playerLocations == NULL || goldRemaining == NULL || goldAmount <= 0)
  {
    return;
  }

  // calculate total gold left
  int nuggetsRemaining = 0;
  hashtable_iterate(goldRemaining, &nuggetsRemaining, sumRemainingGold);

  // create struct to give data to iterate
  goldUpdateData_t goldData;
  goldData.goldPickedUp = goldAmount;
  goldData.pickedUpAddr = pickedUpAddr;
  goldData.nuggetsRemaining = nuggetsRemaining;

  hashtable_iterate(playerLocations, &goldData, sendGoldUpdate);

}

// A helper function to find the remaining gold
void sumRemainingGold(void *arg, const char *key, void *value)
{
  int *goldAmount = (int *)value;
  int *totalRemaining = (int *)arg;

  // only add if it is positive since we assign -1 for empty piles picked up
  if (goldAmount != NULL && *goldAmount > 0)
  {
    *totalRemaining += *goldAmount;
  }
}

// a helper function that iterates over a hashtable and sends an update message about gold
void sendGoldUpdate(void *arg, const char *key, void *value)
{

  struct {
    int goldPickedUp;
    addr_t pickedUpAddr;
    int nuggetsRemaining;
  } *goldData = arg;

  player_t *player = (player_t *)value;
  if (player == NULL)
  {
    return; // weird value
  }

  // determine gold picked up which is either 0 or actual value for on picker
  int goldForThisPlayer = message_eqAddr(player->address, goldData->pickedUpAddr) ? goldData->goldPickedUp : 0;

  // send actual message out
  char message[50];
  snprintf(message, sizeof(message), "GOLD %d %d %d", goldForThisPlayer, player->gold, goldData->nuggetsRemaining); // fill in everything

  message_send(player->address, message);
}

// Look through all the players and update their visibleMaps, placesSeen, gold, etc.
void changeAllVisibleMaps(grid_t *entireMap, hashtable_t *goldRemaining, hashtable_t *playerLocations, player_t *curPlayer, int oldLoc, int newLoc)
{

  // first change the new VisibleString
  // Assumes the grids won't be NULL
  // Doing moved Player first
  grid_t *newVisibleString = curPlayer->visibleMap;

  grid_set(newVisibleString, oldLoc, grid_get(entireMap, oldLoc));
  char curPlayerChar = curPlayer->playerChar;
  grid_set(newVisibleString, newLoc, curPlayerChar);

  int data[4] = {(int)curPlayerChar, oldLoc, newLoc, (int)grid_get(entireMap, oldLoc)};

// Now need to change the entireMaps gold standing
  hashtable_iterate(goldRemaining, entireMap, iterateGoldHashtable);

  // Change for all players
  hashtable_iterate(playerLocations, data, iteratePlayerHashtable);

  //ITERATE THE MOVED PLAYER IF THERE ARE OTHER PLAYERS AROUND
  hashtable_iterate(playerLocations, curPlayer, iterateMovedPlayer);
}

// If the moved player is visible then make them moved for all the other players
void iterateMovedPlayer(void *movedPlayer, const char *address, void *curPlayer){
  player_t *mPlayer = (player_t *)movedPlayer;
  player_t *cPlayer = (player_t *)curPlayer;
  grid_t *curVisibleMap = mPlayer->visibleMap;

// if visibile then make moved
  if(grid_get(curVisibleMap, cPlayer->location) != ' '){
    grid_set(curVisibleMap, cPlayer->location, cPlayer->playerChar);
  }
}

// get rid of all taken gold from entireMap
void iterateGoldHashtable(void *entireMap, const char *goldLoc, void *goldAmount)
{
  // Assumes that gold has already been placed on the map
  if ((*(int *)goldAmount) < 0)
  {
    grid_set((grid_t *)entireMap, atoi(goldLoc), '.');
  }
}

void iteratePlayerHashtable(void *data, const char *address, void *player)
{

  // We are looking if the old spot is already visible
  // Can I already see the other player?
  // If I can then show the player moving
  // If not then change nothing.
  player_t *curPlayer = (player_t *)player;
  grid_t *curVisibleMap = curPlayer->visibleMap;
  // grid_t* curPlacesSeen = player -> placesSeen;
  int *intData = (int *)data; // Cast to int*
  char movedPlayerChar = (char)intData[0];
  int oldLoc = intData[1];
  int newLoc = intData[2];
  char newLocInEntireMap = (char)intData[3];

  
  char oldVisible = grid_get(curVisibleMap, oldLoc);
  char newVisible = grid_get(curVisibleMap, newLoc);

  // If the spot is on the visibleMap then show the player moving
  if (oldVisible != ' ')
  {
    grid_set(curVisibleMap, oldLoc, newLocInEntireMap);
  }
  if (newVisible != ' ')
  {
    grid_set(curVisibleMap, newLoc, movedPlayerChar);
  }
  curPlayer->visibleMap = curVisibleMap;
}

/***************** visibility ****************************/
// check if Im in a cooridor
bool twoSidedHash(int curPlayerLoc, grid_t *entireMap)
{
  // look at how many of sides are "#"
  //  only count to 4 (up, down, right, and left)
  int hashCount = 0;
  // Need width to know what to divide by
  int width = grid_getWidth(entireMap) + 1;
  // Need size to know if we're out of bounds
  int mapSize = grid_getLength(entireMap);

  // Check all 4 sides
  if (grid_get(entireMap, curPlayerLoc + 1) == '#')
  {
    hashCount++;
  }
  if (grid_get(entireMap, curPlayerLoc - 1) == '#')
  {
    hashCount++;
  }
  if ((curPlayerLoc + width) < mapSize && grid_get(entireMap, curPlayerLoc + width) == '#')
  {
    hashCount++;
  }
  if ((curPlayerLoc - width) < 0 && grid_get(entireMap, curPlayerLoc - width) == '#')
  {
    hashCount++;
  }

  // if exactly 2 sides are #s then we are in a cooridor
  if (hashCount > 1)
  {
    return true;
  }
  if (hashCount == 1)
  {
    // Look if the cooridor goes to nowhere
    if (grid_get(entireMap, curPlayerLoc + 1) == ' ' || grid_get(entireMap, curPlayerLoc - 1) == ' ' || grid_get(entireMap, curPlayerLoc + width) == ' ' || grid_get(entireMap, curPlayerLoc - width) == ' ')
    {
      return true;
    }
  }
  return false;
}

void freeItem(void *item)
{
  free(item);
}

// lot of memory allocation to insertSet, also called multiple times
void insertSet(set_t *goodDiagonal, grid_t *entireMap, int roundedCurPoint, int startPoint)
{
  // Don't want to add startPoint b/c it might be a hash
  // frees in set_delete
  char *key = malloc(((int)ceil(log(grid_getLength(entireMap))) + 1) * sizeof(char));
  sprintf(key, "%d", roundedCurPoint);
  // Need to put item of heap not stack
  char *item = malloc(sizeof(char) + sizeof(char));
  item[0] = grid_get(entireMap, roundedCurPoint);
  item[1] = '\0';
  // Add to a set that will then be sent back to super function to be used on visibleMap
  set_insert(goodDiagonal, key, item);
}

// Limit visibility if distance between sPt and ePt is >5
bool visLimit(int startPoint, int endPoint, int width){
  int wDiff = abs((endPoint%width) - (startPoint%width));
  double hDiff = round(abs(((double)endPoint)/width) - (((double)startPoint)/width));

  // Using pythagorean theorm
  if((wDiff*wDiff)+(hDiff*hDiff) > 25){
    return true;
  }
  return false;
}

// Return the set of the diagonal between position and wall if none of the spaces between are walls or spaces
// Else return NULL
set_t *spaceBetween(int curWall, int curPlayerLoc, grid_t *entireMap)
{
  int width = grid_getWidth(entireMap) + 1;
  // See if there is a " " between wall and player
  // If so then we won't consider it

  // + therefore player is left of wall.
  // - therfore player is right of wall.
  int widthDifference = (curWall % width) - (curPlayerLoc % width);

  // + therefore player is above of wall.
  // - therfore player is below of wall.
  int heightDifference = ((int)floor(curWall / width)) - ((int)floor(curPlayerLoc / width));

  // Check how many points in between (Pythagorean Theorm)
  double pointsInBetween = sqrt((heightDifference * heightDifference) + (widthDifference * widthDifference));

  // check all the points in between player and wall
  double exactCeilCurPoint = curPlayerLoc;

  double exactFloorCurPoint = curPlayerLoc;

  double exactHeightMove = (heightDifference / pointsInBetween) * width;

  double exactWidthMove = widthDifference / pointsInBetween;

  double curHMove = 0.0;
  double curWMove = 0.0;
  int startPoint = curPlayerLoc;

  int roundedCeilCurPoint = (int)round(exactCeilCurPoint);

  int roundedFloorCurPoint = (int)round(exactFloorCurPoint);

  // USing Ceil and Floor so that we can see both sides of the diagonal (more accurate)
  bool floorHit = false;
  bool ceilHit = false;

  // don't want to consider starting point
  bool isFirst = true;

  // set for storing good indexes
  //  set: key= (int) map index, item= (char) map char at spot
  set_t *goodDiagonal = set_new();

  while (roundedCeilCurPoint != curWall && roundedFloorCurPoint != curWall)
  {
    // most common is " ", but still need to check for |,-,+

    if (!isFirst && (grid_get(entireMap, roundedCeilCurPoint) == ' ' || grid_get(entireMap, roundedCeilCurPoint) == '|' || grid_get(entireMap, roundedCeilCurPoint) == '-' || grid_get(entireMap, roundedCeilCurPoint) == '+' || grid_get(entireMap, roundedCeilCurPoint) == '#'))
    {
      if (floorHit)
      {
        set_delete(goodDiagonal, freeItem);
        
        return NULL;
      }
      ceilHit = true;
    }

    if (!isFirst && (grid_get(entireMap, roundedFloorCurPoint) == ' ' || grid_get(entireMap, roundedFloorCurPoint) == '|' || grid_get(entireMap, roundedFloorCurPoint) == '-' || grid_get(entireMap, roundedFloorCurPoint) == '+' || grid_get(entireMap, roundedFloorCurPoint) == '#'))
    {
      if (ceilHit)
      {
        set_delete(goodDiagonal, freeItem);

        return NULL;
      }
      floorHit = true;
    }

    // Width greater than height than floor and ceil height
    if (abs(heightDifference) < abs(widthDifference))
    {
      if(!ceilHit && !visLimit(startPoint, roundedCeilCurPoint, width)){insertSet(goodDiagonal, entireMap, roundedCeilCurPoint, startPoint);}
      if(!floorHit && !visLimit(startPoint, roundedFloorCurPoint, width)){insertSet(goodDiagonal, entireMap, roundedFloorCurPoint, startPoint);}

      // update the exact and rounded
      curHMove += exactHeightMove;
      curWMove += exactWidthMove;
      exactCeilCurPoint = startPoint + curWMove + ceil(curHMove / width) * width;
      exactFloorCurPoint = startPoint + curWMove + floor(curHMove / width) * width;
      roundedCeilCurPoint = (int)round(exactCeilCurPoint);
      roundedFloorCurPoint = (int)round(exactFloorCurPoint);
      isFirst = false;
    }
    else
    {
      if(!ceilHit && !visLimit(startPoint, roundedCeilCurPoint, width)){insertSet(goodDiagonal, entireMap, roundedCeilCurPoint, startPoint);}
      if(!floorHit && !visLimit(startPoint, roundedFloorCurPoint, width)){insertSet(goodDiagonal, entireMap, roundedFloorCurPoint, startPoint);}

      // else floor and ceil width
      curHMove += exactHeightMove;
      curWMove += exactWidthMove;
      exactCeilCurPoint = startPoint + curWMove + round(curHMove / width) * width;
      exactFloorCurPoint = startPoint + curWMove + round(curHMove / width) * width;
      roundedCeilCurPoint = (int)ceil(exactCeilCurPoint);
      roundedFloorCurPoint = (int)floor(exactFloorCurPoint);
      isFirst = false;
    }
  }
  // Need to add the wall too
  if(!visLimit(startPoint, curWall, width)){
    insertSet(goodDiagonal, entireMap, curWall, startPoint);
  }
  //  return true if no spaces
  return goodDiagonal;
}

// updating the visible map with all seeable points (gold and characters included (chars not here tho))
void updateDiagonalVisibleMap(void *arg, const char *key, void *item)
{
  grid_t *grid = (grid_t *)arg;
  int index = atoi(key);
  char dot = *(char *)item;

  grid_set(grid, index, dot);
}

// updating the placesSeen so that only . will appear
void updateDiagonalPlacesSeen(void *arg, const char *key, void *item)
{

  grid_t *grid = (grid_t *)arg;
  int index = atoi(key);
  char dot = *(char *)item;

  if(dot == '*'){
    dot = '.';
  }

  grid_set(grid, index, dot);
}

// updating visibility in the new Location
void updateVisibility(grid_t *entireMap, player_t *player, int *walls, hashtable_t *goldRemaining, int oldLoc)
{
  // This is the newLoc
  int curPlayerLoc = player->location;
  // This is all the places player has been
  grid_t *placesSeen = player->placesSeen;
  grid_t *visibleMap = player->visibleMap;
  int width = grid_getWidth(entireMap) + 1;

  // See if we're in a coridor
  // If so then just update visible map with hashtags
  if (twoSidedHash(curPlayerLoc, entireMap))
  {
    // update all 4 spots around the current position
    //  they should only be " " or "#"

    // Setting places I've been
    grid_set(placesSeen, curPlayerLoc + 1, grid_get(entireMap, curPlayerLoc + 1));
    grid_set(placesSeen, curPlayerLoc - 1, grid_get(entireMap, curPlayerLoc - 1));
    grid_set(placesSeen, curPlayerLoc + width, grid_get(entireMap, curPlayerLoc + width));
    grid_set(placesSeen, curPlayerLoc - width, grid_get(entireMap, curPlayerLoc - width));

    // Setting the Visible Map (what I can see)
    // This should be all the way down all the cooridors
    // NO!!! You can only see 1 in front of you in a dark cooridor
    grid_makeEmpty(visibleMap);
    grid_set(visibleMap, curPlayerLoc + 1, grid_get(entireMap, curPlayerLoc + 1));
    grid_set(visibleMap, curPlayerLoc - 1, grid_get(entireMap, curPlayerLoc - 1));
    grid_set(visibleMap, curPlayerLoc + width, grid_get(entireMap, curPlayerLoc + width));
    grid_set(visibleMap, curPlayerLoc - width, grid_get(entireMap, curPlayerLoc - width));

    
    player->placesSeen = placesSeen;
    player->visibleMap = visibleMap;
  }
  else
  {
    // else will need to do visibility and other updated
    //  Grid has function to get all the wall pointers
    int wallsIndex = 0;
    int curWall;

    
    // Resetting the visibleMap
    grid_makeEmpty(visibleMap);

    // Look at every wall as long as there are more walls left
    while ((curWall = walls[wallsIndex]) != -1)
    {
      // Only consider walls that are have no " " (therefore no other walls) between
      // the wall and the player (player position)

      // creating a diagonal of good entries for wall to player
      set_t *diagonal;

      if ((diagonal = spaceBetween(curWall, curPlayerLoc, entireMap)) != NULL)
      {
        //  update every spot in the diagonal (maximum of sqrt(width^2+height^2) updates)
        set_iterate(diagonal, visibleMap, updateDiagonalVisibleMap);
        set_iterate(diagonal, placesSeen, updateDiagonalPlacesSeen);
        set_delete(diagonal, freeItem);
      }
      wallsIndex++;
    }
    // Need function that will add the other playerLocs and goldRemaing
    // Should do this in space in updateGold and updatePlayerLocations
    // Otherwise will be too inefficient

    player->placesSeen = placesSeen;
    player->visibleMap = visibleMap;

  }
}