#include <common.h>

#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

// server otherserverip
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "invalid params" << std::endl;
    return -1;
  }
  std::string peerserver = argv[1];
  std::cout << "peerserver is " << peerserver << std::endl;

  int ret = 0;

#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData))
  {
    auto errcode = GetLastError();
    std::cerr << "windows net module init fail " << errcode << std::endl;
    WSACleanup();
    return -2;
  }
#endif

  // UDP socket
  SOCKET fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in myself_addr = {0};
  socklen_t myself_addr_len = sizeof(myself_addr);
  myself_addr.sin_addr.s_addr = INADDR_ANY;
  myself_addr.sin_port = htons(serverport);
  myself_addr.sin_family = AF_INET;
  ret = bind(fd, (struct sockaddr *)&myself_addr, myself_addr_len);
  CHECKRETRETURN(ret, 0, -3);

  SOCKET otherfd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in other_addr = {0};
  socklen_t other_addr_len = sizeof(other_addr);
  other_addr.sin_addr.s_addr = INADDR_ANY;
  other_addr.sin_port = htons(serverport + 1);
  other_addr.sin_family = AF_INET;
  ret = bind(otherfd, (struct sockaddr *)&other_addr, other_addr_len);

  ret = getsockname(fd, (struct sockaddr *)&myself_addr, &myself_addr_len);
  CHECKRETRETURN(ret, 0, -4);
  std::cout << "local ip is " << inet_ntoa(myself_addr.sin_addr) << std::endl;
  std::cout << "local port is " << ntohs(myself_addr.sin_port) << std::endl;

  staticbuf[0] = PING;
  struct sockaddr_in peerserver_addr;
  socklen_t peerserver_addr_len = sizeof(peerserver_addr);
  peerserver_addr.sin_addr.s_addr = inet_addr(peerserver.c_str());
  peerserver_addr.sin_port = htons(serverport);
  peerserver_addr.sin_family = AF_INET;
  ret = sendto(fd, staticbuf, 1, 0, (sockaddr *)&peerserver_addr, peerserver_addr_len);
  while (true)
  {
    std::cout << "waiting for client" << std::endl;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int recvlen = recvfrom(fd, staticbuf, sizeof(staticbuf), 0, (sockaddr *)&client_addr, &client_addr_len);
    if (recvlen > 0)
    {
      std::cout << "recv data from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << ", len: " << recvlen << std::endl;
      auto cmd = staticbuf[0];
      std::cout << "cmd type is " << (int)cmd << std::endl;
      switch (cmd)
      {
      case PING:
      {
        staticbuf[0] = PONG;
        ret = sendto(fd, staticbuf, 1, 0, (sockaddr *)&client_addr, client_addr_len);
      }
      break;
      case REQUEST:
      {
        staticbuf[0] = RESPONSE;
        memcpy(staticbuf + 1, &client_addr.sin_addr, 4);
        memcpy(staticbuf + 5, &client_addr.sin_port, 2);
        ret = sendto(fd, staticbuf, 7, 0, (sockaddr *)&client_addr, client_addr_len);

        staticbuf[0] = REDIRECT;
        ret = sendto(fd, staticbuf, 7, 0, (sockaddr *)&peerserver_addr, peerserver_addr_len);
        std::cout << "redirect to server " << peerserver << std::endl;
      }
      break;
      case REDIRECT:
      {
        staticbuf[0] = RESPONSERED;
        memcpy(&client_addr.sin_addr, staticbuf + 1, 4);
        memcpy(&client_addr.sin_port, staticbuf + 5, 2);
        sendto(fd, staticbuf, 1, 0, (sockaddr *)&client_addr, client_addr_len);
        std::cout << "redirect to client " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
      }
      break;
      case REQUESTUNIPORT:
      {
        staticbuf[0] = RESPONSEUNIPORT;
        memcpy(staticbuf + 1, &client_addr.sin_addr, 4);
        memcpy(staticbuf + 5, &client_addr.sin_port, 2);
        ret = sendto(otherfd, staticbuf, 7, 0, (sockaddr *)&client_addr, client_addr_len);
        ret = getsockname(otherfd, (struct sockaddr *)&other_addr, &other_addr_len);
        std::cout << "other ip is " << inet_ntoa(other_addr.sin_addr) << std::endl;
        std::cout << "other port is " << ntohs(other_addr.sin_port) << std::endl;
      }
      break;
      case P2P:
      {
        static struct sockaddr_in p2p_addr = {0};
        static socklen_t p2p_addr_len = 0;
        if (p2p_addr_len == 0)
        {
          p2p_addr = client_addr;
          p2p_addr_len = client_addr_len;
          std::cout << "save peer for p2p" << std::endl;
        }
        else
        {
          staticbuf[0] = P2P;
          memcpy(staticbuf + 1, &p2p_addr.sin_addr, 4);
          unsigned short port = ntohs(p2p_addr.sin_port);
          for (int i = 0; i < 300; ++i)
          {
            p2p_addr.sin_port = htons(port + i);
            memcpy(staticbuf + 5, &p2p_addr.sin_port, 2);
            ret = sendto(fd, staticbuf, 7, 0, (sockaddr *)&client_addr, client_addr_len);
            std::cout << "peer 1 ip is " << inet_ntoa(p2p_addr.sin_addr) << std::endl;
            std::cout << "peer 1 port is " << ntohs(p2p_addr.sin_port) << std::endl;
          }
          p2p_addr.sin_port = htons(port);

          memcpy(staticbuf + 1, &client_addr.sin_addr, 4);
          port = ntohs(client_addr.sin_port);
          for (int i = 0; i < 300; ++i)
          {
            client_addr.sin_port = htons(port + i);
            memcpy(staticbuf + 5, &client_addr.sin_port, 2);
            ret = sendto(fd, staticbuf, 7, 0, (sockaddr *)&p2p_addr, sizeof(p2p_addr));
            std::cout << "peer 2 ip is " << inet_ntoa(client_addr.sin_addr) << std::endl;
            std::cout << "peer 2 port is " << ntohs(client_addr.sin_port) << std::endl;
          }

          p2p_addr_len = 0;
        }
      }
      break;
      default:
        break;
      }
    }
  }

  return 0;
}