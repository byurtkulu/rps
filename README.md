# RPS

## Description

This is a simple rock-paper-scissors game.

A player can play against the computer or against another player. The game can result in a win, a loss or a draw.

## Installation
This program is written using mainly C++ (20), python (3.9) and google protobuf for messaging.
It uses the following libraries:
- CMake (3.21.3) for building the project.
- Boost (1.81.0) for async websocket communication. Specifically boost asio and boost beast.
- Google protobuf (libprotoc 3.21.12) for message serialization.
- Google test (1.11.0) for unit testing. 
- Google benchmark (1.6.0) for benchmarking. **!!! todo: add benchmarks !!!**

## Build
- create a build directory in the root of the project named `build`.
- Inside the build directory run the following commands:
```cmake
$ cmake ../
$ make -j
```
This should build the project and create the executable `rps` in the build directory along with protobuf headers for both cpp and python inside the `build` folder.

Inside the `test` directory there are python scripts that can be used to test the game and simulate gameplay for two different players (`gameplayPlayer1.py` and `gameplayPlayer2.py`).

## Usage
After building the project, run the executable `rps`. This will start the server.

After that you can run the python scripts to simulate gameplay and see the messaging as output.

## Design
- It is designed as server client architecture. The server is the main component and the client is there to interact with the player(s).
- The server is responsible for managing the game state and the client is responsible for the user interface and the game flow.
- The server and the client communicate using serialized google protobuf messages over a websocket.
- The server is implemented using boost asio and boost beast and communication is websocket based that is implemented asynchronously. Uses `boost::asio::thread_pool` for async execution.
- The client is implemented using python and the websocket library. It is not the focus of the project and is not implemented in a very robust way.
- The server is implemented in a way that it can be extended to support multiple games. Currently it only supports rock-paper-scissors but it can be extended to support other games as well.
- The server is also implemented in a way that it can be extended to support multiple players. Currently it only supports two players but it can be extended to support more players if it is a different game.

Server architecture
- It consists of 3 main components:
  - **Listener**: This is the main component that listens for incoming connections and accepts them. Runs with `thread_pool`
  - **Session**: This is the component that handles the communication with the client. It is responsible for sending and receiving messages. New one is created for each client.
  - **Game management**: This is the component that manages the game sessions.
    It is responsible for creating new game sessions and managing the existing ones.
    It is also responsible for managing the game state.
    Keeps track of games, players and the game state.
    There is only one instance of this component. Shared between sessions as a `shared_ptr` Access is protected by a mutex.

## Testing
- Tested if the server is working as expected using the python scripts that can be found under `test/scripts` directory.
- Created a load test. It is a python script that creates given number of clients and each client creates(write operation) games in a loop.
- Tested the server with the following scenarios:
  - Server `thread_pool` size = 4:
    - I tried to create until `410685` games with `10` clients and it was able to create all of them without any problem.
    - I tried to create until `408830` games with `100` clients and it was able to create all of them without any problem.
    - I tried to create until `437611` games with `1000` clients and it was able to create all of them without any problem.
    - I tried to create until `529753` games with `10000` clients and it was able to create all of them without any problem.
  - Server `thread_pool` size = 64:
    - I tried to create until `218123` games with `10` clients and it was able to create all of them without any problem.
    - I tried to create until `216268` games with `100` clients and it was able to create all of them without any problem.
    - I tried to create until `504050` games with `1000` clients and it was able to create all of them without any problem.
    - I tried to create until `481011` games with `10000` clients and it was able to create all of them without any problem.
- During this load test running, I tried to play the game with the python scripts and it was working as expected.
- Attacted the profiler output as an image for the load test with `10000` clients and `thread_pool` `size = 16`.
- All the threads are sharing the load efficiently and there is no bottleneck.
- Python scripts cover most of the testing. Also, added some unit tests for the server component `gameSessionManager`. Can be found under `test` directory.

## Todo's
Couldn't finish everything I had in mind in the given time. Here are some things that I would have done:
- Add benchmarks
- Add more robust error handling
- Add more robust logging
- Add more comments to the code
- Add more robust client implementation
- Current design works similar to REST despite it is websocket implementation.
  Adding functionality to keep each connection as session
  (which is already done in websocket session component but game session manager is not using it yet.).
  So, we can use communication bidirectionally any direction at any time and not as only as a request response mechanism. This would be ideal for a game like this.









