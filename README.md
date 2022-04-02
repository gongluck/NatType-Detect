# NatType-Detect

Simply detect the NAT type and implement P2P connection

## Hardware dependent

- two servers with public addresses

## How to use

- use ```cmake.*``` script to build applications
- start server A with ```server [server B's address]```
- start server B with ```server [server A's address]```
- detect client's NAT type with ```client [server A's address] [server B's address]```
- detect another client's NAT type with ```client [server A's address] [server B's address]``` will start P2P connection
- enjoy it

## Other tools

- [NatTypeTester](https://github.com/gongluck/tools/blob/master/NatTypeTester3.2.exe)

## To learn more

[NAT基本原理及穿透详解(打洞)](https://juejin.cn/post/6844904098572009485)
[NAT详解：基本原理、穿越技术(P2P打洞)、端口老化等](http://www.52im.net/article-64-1.html)