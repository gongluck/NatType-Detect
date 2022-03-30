#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef _WIN32
#include <WinSock2.h>
#define GETERROR GetLastError
#define socklen_t int
#else
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SOCKET int
#define GETERROR() errno
#define closesocket close
#endif

#define CHECKRETRETURN(ret, success, returnvalue)                    \
  do                                                                 \
  {                                                                  \
    if (ret != success)                                              \
    {                                                                \
      std::cerr << ret << " != " << success << std::endl;            \
      std::cerr << __LINE__ << " error " << GETERROR() << std::endl; \
      return returnvalue;                                            \
    }                                                                \
  } while (false)

static unsigned short serverport = 3478;
static char staticbuf[1024] = {0};

typedef enum CMD
{
  NONE = 0,
  PING,
  PONG,
  REQUEST,
  RESPONSE,
  REDIRECT,
  RESPONSERED,
  REQUESTUNIPORT,
  RESPONSEUNIPORT
} cmd;

#endif //__COMMON_H__