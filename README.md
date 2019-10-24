
# Dehprox

Dehprox is a transparent DNS and TCP to HTTP proxy redirector (for Linux). It is designed to run in environments guarded with a heavy restricted HTTP proxy, where it is not possible to resolve DNS queries using a DNS to TCP or HTTP tunnel. DNS queries are answered with surrogate addresses that are mapped back to the hostname when the transparent proxy tries to establish a connection using the HTTP proxy.

If your network is not that restricted, you should probably look at other TCP to proxy server redirector like [redsocks2](https://github.com/semigodking/redsocks).

## Features

* TCP traffic (including HTTPS) to HTTP proxy server redirection
* DNS resolution and resolution faking
* Automatic detection of a faster route without the HTTP proxy

## Motivation

It is sometimes a very time consuming struggle to configure your system (or systems) and your tools (that may ignore your system settings) to use a HTTP proxy and some tools might not support proxy servers at all. Hence, it is potentially easier to set up a little router that allows you to use your network like an ordinary network with gateway and DNS server.

## Build Instructions

* Clone the repository and initialize submodules.
* Build the project with `cmake`.
* You can build a `deb` package using the target `package` in CMake.

## Router Setup

* Use a machine (which may be virtual) with two network interfaces.
* The first interface has to be connected to the network where you can reach the HTTP proxy.
* The second interface will act as your gateway. Set it up with a somewhat static IPv4 address and it may or may not be in same network.
* Configure your `iptables` to redirect incoming DNS and TCP traffic from the second interface (except TCP traffic directly directed to your router) to the transparent proxy:
```
iptables -t nat -A PREROUTING -i <second_interface> -p udp --dport 53 -j DNAT --to <ip_of_second_interface>:62124
iptables -t nat -A PREROUTING -i <second_interface> -p tcp ! -d <ip_of_second_interface> -j DNAT --to <ip_of_second_interface>:62124
```
* Start the `dehprox` server.
* Configure your clients to be in the same network as the second interface and to use the IP address of the second interface as gateway and DNS server.

(It might be possible to set up the proxy in different ways (like using it locally without a second machine), but I have not yet tried anything else.)

## How Does it Work?

Dehprox acts as DNS server and as a transparent proxy server if traffic from a network interface is redirected to dehprox using `iptables` rules and if clients are configured to use that interface as gateway and DNS server.

If the system receives a DNS resolution request (on the configured interface) it is redirected to UDP port 62124. Dehproxy listens on this port and receives the DNS resolution request. It tries to resolve the request using the default DNS resolution settings of the system. If it can be resolved, the answer is redirected to requester. If it cannot be resolved, dehprox generates and returns a fake address in the `100.64.0.0/10` reserved IP address range.

 If a client tries to establish a connection to such a address (or any address), it is redirected to TCP port 62124 of the router system because of the `iptables` rules. Dehprox listens an this port and accepts the connection.
 The original destination of the connection can be determined with the socket option `SO_ORIGINAL_DST`. If it is one of the fake addresses in the `100.64.0.0/10` range, it is mapped back to the hostname and the HTTP proxy server is used to establish a connection to the host (if possible). If it is not a fake address, dehprox tries to connect to that address with and without the proxy server. The connection that is established first is the one that is used and the other one is discarded.
