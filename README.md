# Multi-threaded TCP Buffer Server (C)

This project implements a multi-threaded TCP server in C using POSIX sockets and pthreads.

## Features

- TCP socket communication
- Multi-threaded connection handling
- Thread-safe shared circular buffer
- Mutex-based synchronization
- Simple client-server protocol

## How It Works

- Clients send messages to the server.
- The server stores messages in a circular buffer (size = 3).
- If the client sends "R", the server removes and returns a stored message.
- If the buffer is full, the server responds with "Buffer full".
- If empty, it responds with "Buffer empty".

## Compile


gcc bufferServer.c -o server -pthread
gcc client.c -o client


## Run

Server:

./server 5555


Client:

./client localhost 5555


## Concepts Covered

- TCP/IP Networking
- Socket Programming
- Multi-threading
- Mutex Locks
- Circular Buffer Design
