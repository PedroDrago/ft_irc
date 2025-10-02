// // ft_irc_server.cpp
// #include <iostream>
// #include <string>
// #include <vector>
// #include <map>
// #include <cerrno>
// #include <cstring>
// #include <cstdlib>
// #include <unistd.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <poll.h>
// #include <arpa/inet.h>
// #include <fcntl.h>
//
// #define BUFFER_SIZE 512
//
// class IRCServer {
// private:
//     int                 _listenSock;
//     int                 _port;
//     std::vector<pollfd> _pfds;
//
// public:
//     IRCServer(int port) : _listenSock(-1), _port(port) {}
//
//     void init() {
//         // Create listening socket
//         _listenSock = socket(AF_INET, SOCK_STREAM, 0);
//         if (_listenSock == -1) {
//             std::cerr << "socket() failed: " << strerror(errno) << std::endl;
//             std::exit(EXIT_FAILURE);
//         }
//
//         // Allow quick reuse of the port
//         int yes = 1;
//         if (setsockopt(_listenSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
//             std::cerr << "setsockopt() failed: " << strerror(errno) << std::endl;
//             close(_listenSock);
//             std::exit(EXIT_FAILURE);
//         }
//
//         // Bind to the given port
//         struct sockaddr_in addr;
//         std::memset(&addr, 0, sizeof(addr));
//         addr.sin_family = AF_INET;
//         addr.sin_port = htons(_port);
//         addr.sin_addr.s_addr = INADDR_ANY;
//
//         if (bind(_listenSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//             std::cerr << "bind() failed: " << strerror(errno) << std::endl;
//             close(_listenSock);
//             std::exit(EXIT_FAILURE);
//         }
//
//         // Listen for incoming connections
//         if (listen(_listenSock, SOMAXCONN) == -1) {
//             std::cerr << "listen() failed: " << strerror(errno) << std::endl;
//             close(_listenSock);
//             std::exit(EXIT_FAILURE);
//         }
//
//         // Add listening socket to poll
//         pollfd pfd;
//         pfd.fd = _listenSock;
//         pfd.events = POLLIN;
//         _pfds.push_back(pfd);
//
//         std::cout << "Server listening on port " << _port << std::endl;
//     }
//
//     void run() {
//         char buffer[BUFFER_SIZE];
//
//         while (true) {
//             int pollCount = poll(&_pfds[0], _pfds.size(), -1);
//             if (pollCount == -1) {
//                 std::cerr << "poll() failed: " << strerror(errno) << std::endl;
//                 break;
//             }
//
//             for (size_t i = 0; i < _pfds.size(); ++i) {
//                 if (_pfds[i].revents & POLLIN) {
//                     if (_pfds[i].fd == _listenSock) {
//                         // Accept new connection
//                         struct sockaddr_in clientAddr;
//                         socklen_t clientLen = sizeof(clientAddr);
//                         int clientSock = accept(_listenSock, (struct sockaddr*)&clientAddr, &clientLen);
//                         if (clientSock == -1) {
//                             std::cerr << "accept() failed: " << strerror(errno) << std::endl;
//                             continue;
//                         }
//
//                         pollfd newPfd;
//                         newPfd.fd = clientSock;
//                         newPfd.events = POLLIN;
//                         _pfds.push_back(newPfd);
//
//                         std::cout << "New connection: fd " << clientSock << std::endl;
//                     } else {
//                         // Receive data from existing client
//                         std::memset(buffer, 0, BUFFER_SIZE);
//                         int bytesRecv = recv(_pfds[i].fd, buffer, BUFFER_SIZE - 1, 0);
//
//                         if (bytesRecv <= 0) {
//                             if (bytesRecv == 0) {
//                                 std::cout << "Client disconnected: fd " << _pfds[i].fd << std::endl;
//                             } else {
//                                 std::cerr << "recv() failed: " << strerror(errno) << std::endl;
//                             }
//                             close(_pfds[i].fd);
//                             _pfds.erase(_pfds.begin() + i);
//                             --i; // adjust index
//                         } else {
//                             buffer[bytesRecv] = '\0';
//                             std::cout << "Received from fd " << _pfds[i].fd << ": " << buffer;
//
//                             // Echo back to client for now
//                             send(_pfds[i].fd, buffer, bytesRecv, 0);
//                         }
//                     }
//                 }
//             }
//         }
//     }
//
//     ~IRCServer() {
//         for (size_t i = 0; i < _pfds.size(); ++i) {
//             close(_pfds[i].fd);
//         }
//     }
// };
//
// int main(int argc, char **argv) {
//     if (argc != 2) {
//         std::cerr << "Usage: ./ircserv <port>" << std::endl;
//         return 1;
//     }
//
//     int port = std::atoi(argv[1]);
//     IRCServer server(port);
//
//     server.init();
//     server.run();
//
//     return 0;
// }
//
