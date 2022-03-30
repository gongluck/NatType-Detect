# NatType-Detect

Simply check the NAT type

## Hardware dependent

- two servers with public addresses

## How to use

- use ```cmake.*``` script to build applications
- start server A with ```server [server B's address]```
- start server B with ```server [server A's address]```
- detect client's NAT type with ```client [server A's address] [server B's address]```
- enjoy it

## Other tools

- [NatTypeTester](https://github.com/gongluck/tools/blob/master/NatTypeTester3.2.exe)