Assignment 5 Writeup
=============

My name: Moon Kyung Song

My POVIS ID: moonsong98

My student ID (numeric): 20180423

This assignment took me about 6 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:

1) Data Structures

1-1) IP Address - Ethernet Address Mapping

I mapped IP address to Ethernet Address using std::map.
As mentioned in assignment instruction, it also contained when it has been mapped
to expire mapping 30s after it is mapped.

1-2) IP Address Watting for ARP Reply
To save IP addresses to be mapped, I saved them in std::map with time
when ARP request sent.

1-3) Not sent datagrams
To save not sent datagrams, I saved them in std::map with its IP address.

The reason I used std::map for all above is that It takes O(log n)
for insert, access, delete.

2) Methods

2-1) Send Datagram

Before sending a datagram, we need to find Ethernet address of next hop.
If the router has the mapping, it just need to send it using the mapped
Ethernet address.
Otherwise, it needs to send ARP request to hosts in LAN to retrieve Ethernet address.
It also needs to consider whether ARP request has sent for given next hop's IP Address.

2-2) Receive Frame

When a frame is received, it has to take care of whether the destination is valid.
Valid destination is Broadcast or router's ethernet address

2-2-1) ARP

If type is ARP, then map the IP address and Ethernet Address,
and send not sent datagrams.
If it is ARP request, send reply that contains router's IP address & Ethernet address.

2-2-2) IPv4

If it's valid(which is equivalent to parsable), return the Datagram which parses the frame.

2-3) Tick

When time passes, expires the mapping which has passed 30s after it has mapped.

Implementation Challenges:

None

Remaining Bugs:

None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
