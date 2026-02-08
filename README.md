# High-Performance Non-Blocking HTTP Server
A single-threaded, high-concurrency web server developed from scratch using C++ and the Berkeley Sockets API (WinSock2). This project implements a fully non-blocking architecture to handle multiple concurrent clients within a single execution thread.

## 📂 Project Structure
To keep the networking and application layers separate, the project is organized as follows:

* **Root Directory**: Contains the core server logic, socket management, and entry point.
  * `nonblocking server.cpp`: The main loop and `select()` multiplexing logic.
  * `SocketManager.cpp / .h`: Handles the lifecycle of sockets and inactivity reaps.
  * `SocketData.h`: Defines the state machine and shared data structures.
* **http/**: A dedicated module for protocol-specific logic.
  * `HttpRequest / HttpResponse`: Custom parsers for RFC 2616 compliance.
  * `Endpoints`: Implementation of REST-like services (File I/O, language support).
  * `HttpStatusCodes.h`: Standardized HTTP status response mappings.
* **Testing**:
  * `Web Server Test Collection.json`: A Postman collection for automated API verification.



## 🚀 Key Technical Features
* **I/O Multiplexing:** Uses `select()` to monitor dozens of file descriptors simultaneously, ensuring no single connection blocks the server.
* **Protocol Adherence:** Implements a robust parser for **RFC 2616**, supporting `GET`, `POST`, `PUT`, `DELETE`, `OPTIONS`, `HEAD`, and `TRACE`.
* **Stateful Connections:** A custom state machine tracks every socket from `LISTENING` through `RECEIVING` and `SENDING`.
* **Resource Security:** Implements a 120-second timeout mechanism to drop inactive connections and prevent resource exhaustion.



## 🛠️ How to Compile
This project is built using standard C++17 and requires the `Ws2_32.lib` library for networking.
1. Ensure the `http/` folder is in the same directory as the source files.
2. Compile via your preferred C++ compiler (e.g., `g++` or MSVC).
3. Run the executable; the server listens on port `8080` by default.
