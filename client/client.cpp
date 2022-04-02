#include <common.h>

#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

std::string server1;
std::string server2;
std::vector<std::string> localips;
std::string publicip1;
std::string publicip2;
unsigned short publicport1 = 0;
unsigned short publicport2 = 0;

int main(int argc, char *argv[])
{
  int ret = 0;

#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData))
  {
    auto errcode = GetLastError();
    std::cout << "windows net module init fail " << errcode << std::endl;
    WSACleanup();
    return -1;
  }

#endif
  // local ip search
  gethostname(staticbuf, sizeof(staticbuf));
  std::cout << "hostname is " << staticbuf << std::endl;
  struct hostent *host = gethostbyname(staticbuf);
  for (int i = 0; host->h_addr_list != nullptr && host->h_addr_list[i] != nullptr; ++i)
  {
    std::string localip = inet_ntoa(*(struct in_addr *)host->h_addr_list[i]);
    std::cout << "local ip maybe : " << localip << std::endl;
    localips.push_back(localip);
  }
  localips.push_back("0.0.0.0");
  localips.push_back("127.0.0.1");

  server1 = argv[1];
  server2 = argv[2];

  // stun server
  struct sockaddr_in server1_addr;
  socklen_t server1_addr_len = sizeof(server1_addr);
  server1_addr.sin_addr.s_addr = inet_addr(server1.c_str());
  server1_addr.sin_port = htons(serverport);
  server1_addr.sin_family = AF_INET;

  // UDP socket
  SOCKET fd1 = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in myself_addr = {0};
  socklen_t myself_addr_len = sizeof(myself_addr);
  // myself_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.0.142");
  // myself_addr.sin_port = htons(5555);
  // myself_addr.sin_family = AF_INET;
  // ret = bind(fd1, (struct sockaddr*)&myself_addr, myself_addr_len);

  staticbuf[0] = REQUEST;
  ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr *)&server1_addr, server1_addr_len);
  std::cout << ">>> request to server1 " << server1 << std::endl;

  ret = getsockname(fd1, (struct sockaddr *)&myself_addr, &myself_addr_len);
  std::cout << "local ip is " << inet_ntoa(myself_addr.sin_addr) << std::endl;
  std::cout << "local port is " << ntohs(myself_addr.sin_port) << std::endl;

  bool stop = false;
  std::thread th([&]()
                 {
      std::this_thread::sleep_for(std::chrono::seconds(15));
      std::cout << "total time out" << std::endl;
      if (!stop) {
          std::cout << "type : Blocked" << std::endl;
          stop = true;
      }
      staticbuf[0] = P2P;
      ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr*)&server1_addr, server1_addr_len); });
  th.detach();

  do
  {
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    ret = recvfrom(fd1, staticbuf, sizeof(staticbuf), 0, (struct sockaddr *)&server_addr, &server_addr_len);
    if (ret > 0)
    {
      std::cout << "<<< recv data from " << inet_ntoa(server_addr.sin_addr) << ":" << ntohs(server_addr.sin_port) << ", len: " << ret << std::endl;
      auto cmd = staticbuf[0];
      std::cout << "cmd type is " << (int)cmd << std::endl;
      switch (cmd)
      {
      case RESPONSE:
      {
        if (publicip1.empty())
        {
          publicip1 = inet_ntoa(*(struct in_addr *)(staticbuf + 1));
          publicport1 = ntohs(*(unsigned short *)(staticbuf + 5));
          std::cout << "public1 addr is " << publicip1 + ":" << publicport1 << std::endl;
          if (std::find(localips.begin(), localips.end(), publicip1) != localips.end() && !stop)
          {
            std::cout << "nat type : Opened" << std::endl;
            stop = true;
          }
          std::thread th([&]()
                         {
                      std::this_thread::sleep_for(std::chrono::seconds(5));
                      if (!stop) {
                          std::cout << "wait REDIRECT time out" << std::endl;
                          staticbuf[0] = REQUEST;
                          struct sockaddr_in peer_server_addr;
                          peer_server_addr.sin_addr.s_addr = inet_addr(server2.c_str());
                          peer_server_addr.sin_port = htons(serverport);
                          peer_server_addr.sin_family = AF_INET;
                          ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr*)&peer_server_addr, sizeof(peer_server_addr));
                          std::cout << ">>> request to server2 " << server2 << std::endl;
                          std::thread th([&]() {
                              std::this_thread::sleep_for(std::chrono::seconds(5));
                              if (!stop) {
                                  std::cout << "wait server2 response time out" << std::endl;
                                  std::cout << "nat type : Blocked" << std::endl;
                                  stop = true;
                              }
                              });
                          th.detach();
                      } });
          th.detach();
        }
        else
        {
          if (ntohs(server_addr.sin_port) == serverport)
          {
            publicip2 = inet_ntoa(*(struct in_addr *)(staticbuf + 1));
            publicport2 = ntohs(*(unsigned short *)(staticbuf + 5));
            std::cout << "public2 addr is " << publicip2 + ":" << publicport2 << std::endl;
            if (publicip1 != publicip2 || publicport1 != publicport2 && !stop)
            {
              std::cout << "type : Symmetric" << std::endl;
              stop = true;
            }
            else
            {
              staticbuf[0] = REQUESTUNIPORT;
              ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr *)&server1_addr, server1_addr_len);
              std::cout << ">>> request to server1 " << server1 << " wish other port" << std::endl;
              std::thread th([&]()
                             {
                              std::this_thread::sleep_for(std::chrono::seconds(5));
                              if (!stop) {
                                  std::cout << "wait RESPONSEUNIPORT time out" << std::endl;
                                  std::cout << "type : Port Restricted Cone" << std::endl;
                                  stop = true;
                              } });
              th.detach();
            }
          }
          else
          {
            std::cout << "type : Restricted Cone" << std::endl;
            stop = true;
          }
        }
      }
      break;
      case RESPONSERED:
        if (!publicip1.empty() && !stop)
        {
          std::cout << "type : Full Cone" << std::endl;
          stop = true;
        }
        break;
      case RESPONSEUNIPORT:
        if (!stop)
        {
          std::cout << "type : Restricted Cone" << std::endl;
          stop = true;
        }
        break;
      case P2P:
      {
        struct sockaddr_in peer_addr = {0};
        peer_addr.sin_addr = *(struct in_addr *)(staticbuf + 1);
        peer_addr.sin_port = *(unsigned short *)(staticbuf + 5);
        peer_addr.sin_family = AF_INET;
        staticbuf[0] = PING;
        ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
        std::cout << "try to create p2p to " << inet_ntoa(peer_addr.sin_addr) << ":" << ntohs(peer_addr.sin_port) << std::endl;
      }
      break;
      case PING:
      {
        staticbuf[0] = PONG;
        ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr *)&server_addr, server_addr_len);
      }
      break;
      case PONG:
      {
        staticbuf[0] = PING;
        ret = sendto(fd1, staticbuf, 1, 0, (struct sockaddr *)&server_addr, server_addr_len);
      }
      break;
      default:
        break;
      }
    }
    else if (!stop)
    {
      std::cout << "type : Blocked" << std::endl;
      stop = true;
    }
  } while (true);

END:
  stop = true;
  closesocket(fd1);

  std::cout << "run successd" << std::endl;
  return 0;
}