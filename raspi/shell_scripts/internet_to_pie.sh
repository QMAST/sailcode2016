#/bin/bash
# Forward Internet (Host -> Pi)
#
# Performs the reverse of a typical wireless network connection. This assumes
# that you are a client in a wireless network, but you have the internet
# connection, which you are going to give to the wireless "router" (which in this
# case is the RPI)
#
# This script is to be run on the laptop, not the RPI. The RPI script will need
# the local IP address of the computer that runs this script.
#
# Adapters are currently hardcoded for Stephen's laptop only.
#   enp0s25 is the wired adapter that already has an internet connection
#   wlp3s0  is the wireless adapter connected to the Pi's Wireless AP
echo 1 > /proc/sys/net/ipv4/ip_forward

/sbin/iptables -t nat -A POSTROUTING -o enp0s25 -j MASQUERADE
/sbin/iptables -A FORWARD -i enp0s25 -o wlp3s0 -m state \
   --state RELATED,ESTABLISHED -j ACCEPT
/sbin/iptables -A FORWARD -i wlp0s3 -o enp0s25 -j ACCEPT

