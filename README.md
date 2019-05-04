
# Dehprox

Dehprox is a transparent DNS and TCP to HTTP proxy redirector (for Linux). It is designed to run in environments guarded with a heavy restricted HTTP proxy, where it is not possible to resolve DNS queries using a DNS to TCP or HTTP tunnel. DNS queries are answered with surrogate addresses that are mapped back to the hostname when the transparent proxy tries to establish a connection via the HTTP proxy.

If your network is not that restricted, you should probably look at other TCP to proxy redictores like [redsocks2](https://github.com/semigodking/redsocks).

## Features

* TCP traffic to HTTP proxy server redirection
* DNS resolution and resolution faking
* Automatic detection of routes without the HTTP proxy

## Motivation

Some companies use HTTP proxies. It is always a time consuming struggle configure your system (or systems) and your tools (that may ignore your system settings) to use them and some tools might not support proxy servers at all. Hence, it is potentially easier to set up a little router that allows you to use your network like an ordinary network without having to spend countless hours on configuring proxy settings.

## Build Instructions

* Clone the repository and initialize submodules.
* Build the project with `cmake`.

## Router Setup

* Use a machine (which may be virtual) with two network interfaces.
* The first of the interfaces has to be connected to the network where you can reach the HTTP proxy.
* The second interface will act as your gateway. Set it up with a somewhat static IPv4 address, which may or may not be in same network.
* Configure your `iptables` to redirect incoming DNS and TCP traffic from the second interface to the transparent proxy:
```
iptables -t nat -A PREROUTING -i <second_interface> -p udp --dport 53 -j DNAT --to <ip_of_second_interface>:62124
iptables -t nat -A PREROUTING -i <second_interface> -p tcp -j DNAT --to <ip_of_second_interface>:62124
```
* Start the `dehprox` server.
* Configure your clients to use the IP address of the second interface as gateway and DNS server.

(It might be possible to set up the proxy in different ways, but I have not yet tried anything else.)

