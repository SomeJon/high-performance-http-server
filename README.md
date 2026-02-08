# High-Performance Non-Blocking HTTP Server
A single-threaded, high-concurrency web server built from the ground up using C++ and the WinSock2/Berkeley Sockets API. This project was engineered to handle multiple concurrent connections efficiently without the overhead of multi-threading.

## 🚀 Key Systems Features
* **I/O Multiplexing:** Utilizes the `select()` system call to monitor multiple file descriptors simultaneously, allowing a single-threaded process to manage dozens of concurrent connections.
* **Non-Blocking Architecture:** All sockets are configured to be non-blocking, ensuring the server remains responsive and never stalls on a single slow or malicious client.
* **RFC 2616 Compliance:** Implements a custom parser for the HTTP/1.1 protocol, supporting methods including `GET`, `POST`, `PUT`, `DELETE`, `OPTIONS`, `HEAD`, and `TRACE`.
* **Stateful Connection Management:** Tracks the lifecycle of every connection through a robust state machine (`LISTENING` → `RECEIVING` → `PROCESSING` → `SENDING`).



## 🛡️ Security & Robustness
* **Resource Protection:** Includes a custom inactivity timeout mechanism (120 seconds) that automatically reaps "stuck" connections to prevent resource exhaustion attacks.
* **Memory Safety:** Manual buffer management and parsing logic designed to handle raw byte transit safely at the system level.
* **Error Handling:** Implements standardized HTTP status codes (404, 400, 500) with custom error pages and detailed server-side logging.

## 📂 Project Structure
- `http/`: Core HTTP logic including Request/Response objects and Endpoint interfaces.
- `SocketManager`: Manages the lifecycle of active connections and the `select()` loop.
- `Endpoints`: Implementation of REST-like endpoints for file management and language-specific content delivery.
- `Postman Collection`: Includes a test suite for verifying RFC compliance across all supported methods.

## 🛠️ Technical Stack
* **Language:** C++17
* **APIs:** WinSock2 / Berkeley Sockets
* **Testing:** Postman
