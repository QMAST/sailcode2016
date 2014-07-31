Using on-board Linux SBC
========================

Configuration

-   Common tasks
    *   Wireless access
-   operating system
    *   Arch Linux ARM
    *   Systemctl configurations
-   network setup
    *   hostapd
    *   dnsmasq
    *   static ip config
-   operating
    *   remote arduino login
        +   uploading code
        +   opening terminal emulator
    *   forwarding an internet connection

The on-board linux Single Board Computer (SBC), currently a Raspbery Pi, is able to make its own wireless network for your computer to connect to, and then interact with the Arduino units and any installed software.

Wireless Access
---------------

When you connect to the SBC through your laptop's wireless card, you are connecting to the SBC's USB-Wireless adapter. The SBC's adapter is performing the same function as the router in your home, but with some differences. Note that there is no "Wireless router" on board, just a usb wifi adapter like you would find in your laptop.

The adapter is configured as a Wireless Access Point (WAP) to the SBC's internal network interfaces. When you're computer connects, a program is set up to provide an IP-address to your computer so that you can communicate. Note that just because you are given an IP-address, you won't get internet access, even if the GSM modem is connected to the internet[1]. As of writing this, the boat will assign your computer an IP of the form "`192.168.0.XXX`". The SBC will always give itself the IP address of "`192.168.0.1`" and you can connect to it over ssh using the root account[2].

    SSH Access: root@192.168.0.1

---
1.  You could configure the boat to give your computer its internet connection. Please resist the urge to turn the boat into a floating wifi access point.
2.  You should feel dirty and uneasy for using the root account, don't ever do this on a real computer. This is the primary reason we back up the root drive of the SBC often.
