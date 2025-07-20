# Nuggets Game

## Overview

This project implements a real-time multiplayer dungeon exploration game with client-server architecture. The Nuggets game supports up to 26 concurrent players exploring a dungeon map to collect gold nuggets, featuring  visibility mechanics, real-time communication protocols, and dynamic game state management. Built through collaborative development, the system demonstrates  network programming, efficient data structures, and real-time game mechanics with client-server synchronization.

---

## Key Features

### Server Architecture (`server`)

- **Concurrent Client Handling**: Manages up to 26 simultaneous players plus one spectator using UDP-based messaging protocols.
- **Real-Time Communication Protocol**: Custom message-passing system enabling game state updates and player interactions.
- **Dynamic Visibility System**: Line-of-sight calculations with radius-limited visibility for gameplay mechanics.
- **Gold Distribution Engine**: Placement of 250 gold nuggets across 10-30 randomized piles with configurable distribution algorithms.

### Grid Engine (`grid`)

- **Dynamic Map Management**: Grid system supporting various dungeon layouts with walls, corridors, and open spaces.
- **Memory-Efficient Storage**: Hashtable-based tracking of gold locations and amounts with memory allocation.
- **Collision Detection**: Wall and obstacle detection system preventing invalid player movements.
- **Map Validation**: Parsing and validation of map files ensuring consistent game boundaries.

### Network Communication

- **UDP Protocol Implementation**: Low-latency message passing optimized for real-time game interactions.
- **Message Queue Management**: Handling of player inputs, game state updates, and spectator communications.
- **Address Management**: Client address tracking enabling player reconnection and state persistence.
- **Protocol Validation**: Message parsing with error handling for malformed client communications.

### Game Mechanics

- **Sprint Movement System**: Movement mechanics allowing players to traverse multiple spaces in single keystrokes.
- **Player State Tracking**: Player profiles including location, gold collection, visibility maps, and activity status.
- **Spectator Mode**: Read-only observer functionality providing game overview without player interaction.
- **Game Over Coordination**: Synchronized game termination with scoring and statistics reporting.

### Design Principles

- **Modular Architecture**: Separation between server logic, grid management, and communication protocols.
- **Concurrent Programming**: Thread-safe operations enabling multi-player interactions without race conditions.
- **Error Handling**: Validation and graceful failure modes throughout the system.
- **Scalable Design**: Architecture supporting extension of player limits and game features.

---

## System Components

### Server Core
- Manages game state, player coordination, and real-time message processing.
- Implements visibility calculations with radius-based sight limitations.
- Coordinates gold collection, player movement, and game termination sequences.

### Grid Management System
- Processes dungeon maps from text files into internal representations.
- Manages dynamic game elements including gold placement and player positioning.
- Provides collision detection and boundary validation for player actions.

### Communication Protocol
- Custom UDP-based messaging system for low-latency game interactions.
- Handles player authentication, movement commands, and game state synchronization.
- Supports both active players and passive spectators with differentiated message types.

### Client Integration
- Interfaces with provided client binary for gameplay experience.
- Supports multiple client instances for multi-player testing.
- Maintains compatibility with standard client implementations.

---

## How It Works

1. **Server Initialization**:
   - Load and validate dungeon map from specified text file.
   - Initialize UDP communication socket and announce available port.
   - Distribute gold nuggets randomly throughout accessible map locations.

2. **Player Connection Phase**:
   - Accept up to 26 player connections with unique character assignments (A-Z).
   - Support single spectator connection for game observation.
   - Initialize individual visibility maps and player state tracking.

3. **Real-Time Game Loop**:
   - Process player movement commands and validate against map boundaries.
   - Update visibility calculations based on line-of-sight algorithms and radius limitations.
   - Broadcast appropriate map views to each player and spectator.
   - Handle gold collection events and update player scores.

4. **Game Termination**:
   - Monitor for all gold collection or player departure conditions.
   - Generate game over reports with player rankings.
   - Disconnect all clients with final score information.

---

## Environment Setup

### Build Requirements
- GCC compiler with C99 standard support
- CS50 library (libcs50) for fundamental data structures
- Unix-like environment with UDP networking capabilities
- Make build system for automated compilation

### Usage

1. **Build the system**:
   ```bash
   make
   ```

2. **Start the server**:
   ```bash
   ./server [mapfile] [seed]
   ```
   Example:
   ```bash
   ./server maps/main.txt 42
   ```

3. **Connect clients using the provided binary**:
   ```bash
   ./clientbinary hostname port [playername]
   ```
   Example:
   ```bash
   ./clientbinary localhost 45678 Alice
   ```

4. **Connect spectator**:
   ```bash
   ./clientbinary hostname port
   ```

### Available Maps
- Multiple pre-designed dungeon layouts in the `maps/` directory
- Support for custom map files following the specified format
- Maps range from simple layouts to complex multi-room dungeons

### Testing
- Server testing with multiple concurrent connections
- Grid module validation with various map configurations
- Memory management verification (note: known memory leak in visibility system)
- Protocol testing with provided client binary

---

## Known Issues

- **Memory Management**: There is a known memory leak in the visibility calculation system that occurs during complex line-of-sight computations. This does not affect gameplay but should be addressed in future versions.

---

## Technical Achievements

This project demonstrates systems programming concepts including:
- **Real-time networking** with UDP protocol implementation
- **Concurrent programming** supporting multiple simultaneous clients  
- **Algorithms** for visibility and line-of-sight calculations
- **Memory management** with dynamic data structures and allocation
- **Protocol design** for client-server communication
- **Game engine architecture** with modular, extensible design

---

## Acknowledgments

This project was developed by a collaborative team of 3 developers, with contributions across server architecture, networking protocols, game mechanics, and system integration.

---

## License

This project is distributed under the MIT License. Refer to the LICENSE file for detailed terms and conditions.
