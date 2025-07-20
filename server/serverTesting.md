# Server Testing
## Authors: Jacob and Nate

**Extra Credit Included**

### Unit Testing

Unit testing was performed at a basic level by running server and internally viewing added print statements and logs at major steps in operation. Later ```miniclient``` was used to test protocol.

- Ensured proper generation of server by running with a variety of test arguments and verifying results
  1. Printed different grid outputs from various map files checking for proper grid module map generation
  2. Ensured that server would load with and without seed argument
  3. Printed grid output with random gold generation to verify both that gold generated and that it randomized correctly based on the seed
- Did extensive testing to verify protocol with ```miniclient``` provided within ```support``` directory
  1. Verified that server generated port to which miniclient could connect
  2. Sent a variety of basic messages to verify that client worked:
    - Utilized basic ```PLAY "username"``` message to verify that players were able to join the server. Verified this by server sending back ```OK "playerChar"``` on successful join.
    - Checked that on any given ```KEY "char"``` server would either remove a player broadcasting ```QUIT``` message, move a player sending a new ```DISPLAY``` message, or ignore an invalid keystroke without error.
    - Verified (via internal server logging) that upon recieving ```SPECTATE``` message, server stores the sender's address as a spectator and continously sends them ```DISPLAY``` messages while ignoring all key messages besides ```KEY Q```.
  3. Checked that server could deal with the logic of multiple ```minclient``` connections
    - Checked that as multiple clients joined, the ```OK "playerChar"``` messages continued to work and also the char incremented
    - Verified that a message resulting in an update in the map from one player (any valid keypress) would broadcast an updated map to all connected miniclients
    - Ensured that ```DISPLAY``` messages sent to ```miniclient``` was an accurate visual grid populated with gold and players
    - Checked that upon any player activity (and on join) spectator client would recieve proper ```DISPLAY``` messages

### Integration Testing

Integration testing was performed via connections with premade client binary files. These files were connected to a server to provide a wide range of integration testing through extensive testing gameplay.

- Began with simple gameplay of a single ```client```
  1. Ensured proper connection and immediate printing of map on client end
  2. Checked that as each room was entered and explored it was added to player memory
  3. Check that hallways are slowly revealed as player moves
  4. Check that visibility only allowed player to see gold within its range
  5. Verified sprinting capabilites
  6. Verified gold colleciton messages with proper display and gold remaining calculation
  7. Checked that after all gold is collected the game ends
- Tested with an additional spectator
  1. Tested that a spectator which joins can immmediately see map
  2. Checked that a spectator would see other players moved across map
  3. Checked that spectator could quit and do nothing else
  4. Verified that another spectator joining kicks other spectator while perserving functionality for new spectator
- Finished testing with multiple clients and gameplay runs
  1. Checked that on player could watch another move as long as they were within visibility
  2. Verified that two players hitting each other would swap their places
  3. Checked that gold remaining updated for all players whenever one player collects gold
  4. Checked that when a game ends, the scores and names of all players are printed properly 
  5. Verfied expected behavior when a player disconnects and that they can rejoin later
  6. Ensured that a spectator would constantly see all players move during the game.
- Verified that our extra credit feature of limiting visiblity to a radius of 5 worked
  1. Ran multiple ```client``` and ensured they could only see each other if they were within 5 tiles
  2. Ensured gold was only visible within a distance of 5 tiles
  3. Verified that rooms were added to the recorded places visited when you went within a radius of 5 tiles.