# ft_ping

My **ft_ping** implementation for the 42 School cursus, written in C. This project is a reimplementation of the standard `ping` utility, which sends ICMP Echo Request packets to network hosts and measures the round-trip time for messages sent from the originating host to a destination computer.

Final grade : `100/100`

## Getting Started

You need to compile the project with `make`.

### Usage 

```
Usage
  ft_ping [options] <destination>Options:
  <destination>      dns name or ip address
  -c <count>         stop after <count> replies
  -h                 print help and exit
  -V                 print version and exit
  -v                 verbose output
```

## Overview

The `ft_ping` program sends ICMP Echo Request packets to a specified host and waits for ICMP Echo Reply packets. It measures the time taken for the round trip and provides statistics about the packets sent and received.

The ICMP protocol is used for network diagnostics and is a core component of the Internet Protocol Suite. It is primarily used for error messages and operational information queries. The program constructs ICMP packets manually and handles raw sockets to send and receive these packets.
