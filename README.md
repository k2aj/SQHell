# SQHell

SQHell is a video game written in SQL.

Why? _Because I can_.

## How to build and run it

```sh
cmake -S . -B build
cmake --build build
./build/sqhell sql/game.sql     # or use any other script from sql/ directory
```

## How it works

- The game is basically a single big SQL script, which I run repeatedly against an SQLite database in a while loop.

- The entire state of the game is stored inside database tables and manipulated exclusively with SQL queries.

- The database is stored in RAM to make the performance somewhat decent.

- Graphics/window management are done by exposing native OpenGL/GLFW functions to SQL.

    (Yes, this means I often have to do stupid nonsense such as storing C++ pointers in database tables and retrieving them later. _Please don't do this in real code._)