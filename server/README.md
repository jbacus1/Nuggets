# serverGrid subdirectory

## Overview
The `serverGrid` subdirectory contains the core server-side components for managing the game grid and player interactions in the Nuggets project. It includes modules for grid representation, player actions, gold placement, visibility handling, and server communication.

## Files

### `grid.c`
This module implements the grid structure used to represent the game map. It stores the following variables:
- char* map
- int width
- int height

It provides functionalities such as:
- **Creating a grid** from a string or a file.
- **Retrieving and modifying grid cells** using `grid_get()` and `grid_set()`.
- **Placing gold piles** in random locations while ensuring constraints on the number of piles and total gold.
- **Checking grid attributes**, including width, height, and walls.
- **Printing and clearing the grid** when necessary.
---

### `gridtest.c`
This is a test file for validating the functionality of the `grid.c` module.
These tests consider:
- **Grid initialization tests** to ensure the map is loaded correctly from a file.
- **Functionality tests** for retrieving, modifying, and printing grid contents.
- **Gold placement validation** to check if gold is distributed properly within constraints.
- **Wall detection tests** to verify accurate wall indexing.
- **Visibility tests** ensuring that the player's vision is correctly updated based on obstacles.
---

### `server.c`
The main server module, responsible for game logic, player interactions, and message processing. Key functionalities include:
- **Handling player connections and disconnections**:
  - Players can join with a `PLAY <name>` message.
  - Players can quit the game (`Q` command).
  - A spectator can join and view the entire map.
- **Processing player moves and updating the grid**:
  - Players move using `h`, `j`, `k`, `l`, `b`, `n`, `y`, `u` for directional movement.
  - Sprinting is enabled with capitalized movement keys (`H`, `J`, `K`, `L`, `B`, `N`, `Y`, `U`).
  - The server prevents movement through walls and enforces game boundaries.
- **Managing gold collection**:
  - The server tracks gold remaining in the game and updates players’ gold.
  - If all gold is collected, the game ends, and final scores are broadcast.
- **Updating player visibility dynamically**:
  - The server calculates which grid cells are visible to each player.
  - Visibility is affected by walls and corridors.
- **Broadcasting game state updates**:
  - Players receive updates on their visible map after every move.
  - The spectator receives a full-map update, including all player positions.
---

### `serverTesting.md`
Provides an extensive documentation of how the server was testing
- **Unit Testing**
  - Tested invidual functionality of server inialization
  - Verified server protocol using `miniclient` provided in support
- **Integration Testing**
  - Ensured that server had all expected behaviors when integrated with `client`

## Starting the server
To start the server with a specific map file:
```bash
./server path/to/map.txt [optional_seed]
```
- `path/to/map.txt` should be a filepath to a valid game map
- `[optional_seed]` is an optional random integer seed for reproducible gold placement
---

## Gameplay
Players and spectators join using the provided client executable file. 

### Player commands
- Q & q = quit the game
- h = move left, H = sprint left
- l = move right, L = sprint right
- j = move down, J = sprint down
- k = move up, K = sprint up
- y = move diagonally up and left, Y = sprint diagonally up and left
- u = move diagonally up and right, U = sprint diagonally up and right
- b = move diagonally down and left, B = sprint diagonally down and left
- n = move diagonally down and right, N = move diagonally down and right
---

### Gold collection
- Stepping on gold updates the player’s purse.
- Gold pickups are broadcast to all players.
---

### Specator mode
- A spectator can join with the SPECTATE command.
- They see the entire grid, including player positions.
---

### Ending the game
- If all gold is collected, the game ends.
- A final scoreboard is sent to all players.
- Players are disconnected.
---

## Dependencies
- `libcs50`: Utility functions for hash table and set operations.
- `support`: Provides client, logging, and message-passing utilities.
