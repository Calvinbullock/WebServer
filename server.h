// Simple threaded AF_INET server.

#ifndef OBSEQUI_SERVER_H
#define OBSEQUI_SERVER_H

#include <arpa/inet.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace obsequi {

struct TcpConnection {
  int fd;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
};

typedef void (*ConnectionHandler)(const TcpConnection &conn);

class TcpServer {
public:
  TcpServer(ConnectionHandler handler);

  // Never returns.
  void Run(int port, int num_worker_threads);

  // TODO: Add some way to cleanly shutdown. A drain function that stops
  // accepting new functions and waits for all worker threads to join.

private:
  void HandleConnections();

  void Queue(const TcpConnection &conn);
  TcpConnection Dequeue();

  ConnectionHandler handler_;
  std::queue<TcpConnection> queue_;
  std::mutex mutex_;
  std::condition_variable notifier_;
};

} // namespace obsequi
#endif // OBSEQUI_SERVER_H
