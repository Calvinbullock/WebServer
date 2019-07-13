#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PENDING_CONNECTIONS 20

namespace obsequi {

TcpServer::TcpServer(ConnectionHandler handler)
    : handler_(handler), queue_(), mutex_(), notifier_() {}

void TcpServer::Run(uint16_t port, int num_worker_threads) {
  // Create streaming socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Server - socket");
    exit(errno);
  }

  // Initialize address/port structure
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    perror("Server - bind");
    exit(errno);
  }

  printf("%s:%d port open\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

  // Make it a "listening socket"
  if (listen(sockfd, PENDING_CONNECTIONS) != 0) {
    perror("Server - listen");
    exit(errno);
  }

  for (int i = 0; i < num_worker_threads; i++) {
    std::thread t1(&TcpServer::HandleConnections, this);
    t1.detach();
  }

  // Handle each connection by pushing it onto a work queue
  while (true) {
    TcpConnection connection;

    // Accept a connection (creating a data pipe)
    connection.fd = accept(sockfd, (struct sockaddr *)&connection.client_addr,
                           &connection.client_addr_len);

    printf("%s:%d connected\n", inet_ntoa(connection.client_addr.sin_addr),
           ntohs(connection.client_addr.sin_port));

    Queue(connection);
  }
}

void TcpServer::HandleConnections() {
  while (1) {
    TcpConnection conn = Dequeue();
    handler_(conn);
    close(conn.fd);
  }
}

void TcpServer::Queue(const TcpConnection &hc) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(hc);
  notifier_.notify_one();
}

TcpConnection TcpServer::Dequeue() {
  std::unique_lock<std::mutex> lock(mutex_);
  std::chrono::milliseconds timeout(1000);

  while (queue_.empty()) {
    notifier_.wait_for(lock, timeout);
  }

  TcpConnection ret = queue_.front();
  queue_.pop();
  return ret;
}

} // namespace obsequi
