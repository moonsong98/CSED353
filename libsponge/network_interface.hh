#ifndef SPONGE_LIBSPONGE_NETWORK_INTERFACE_HH
#define SPONGE_LIBSPONGE_NETWORK_INTERFACE_HH

#include "ethernet_frame.hh"
#include "tcp_over_ip.hh"
#include "tun.hh"

#include <map>
#include <optional>
#include <queue>

//! \brief A "network interface" that connects IP (the internet layer, or network layer)
//! with Ethernet (the network access layer, or link layer).

//! This module is the lowest layer of a TCP/IP stack
//! (connecting IP with the lower-layer network protocol,
//! e.g. Ethernet). But the same module is also used repeatedly
//! as part of a router: a router generally has many network
//! interfaces, and the router's job is to route Internet datagrams
//! between the different interfaces.

//! The network interface translates datagrams (coming from the
//! "customer," e.g. a TCP/IP stack or router) into Ethernet
//! frames. To fill in the Ethernet destination address, it looks up
//! the Ethernet address of the next IP hop of each datagram, making
//! requests with the [Address Resolution Protocol](\ref rfc::rfc826).
//! In the opposite direction, the network interface accepts Ethernet
//! frames, checks if they are intended for it, and if so, processes
//! the the payload depending on its type. If it's an IPv4 datagram,
//! the network interface passes it up the stack. If it's an ARP
//! request or reply, the network interface processes the frame
//! and learns or replies as necessary.
class NetworkInterface {
  private:
    //! Ethernet (known as hardware, network-access-layer, or link-layer) address of the interface
    EthernetAddress _ethernet_address;

    //! IP (known as internet-layer or network-layer) address of the interface
    Address _ip_address;

    //! outbound queue of Ethernet frames that the NetworkInterface wants sent
    std::queue<EthernetFrame> _frames_out{};

    /*
     * Private Variables
     */

    // amount of time passed
    size_t _time_passed{0};

    // A map that connects IP address - EthernetAddress & time when mapping started
    // key: IP address, value: Ethernet address
    std::map<uint32_t, std::pair<EthernetAddress, size_t>> _ip2eth{};

    // A map that contains pair of IP address and last time ARP request sent
    // key: IP address, value: time
    std::map<uint32_t, size_t> _not_mapped_ip_address{};

    // A map that contains unsent datagram payloads for each IP address
    // key: IP address, value: vector of unsent datagram payloads
    std::map<uint32_t, std::vector<BufferList>> _not_sent_dgrams{};

    /*
     * Private Methods
     */

    // Send a IPv4 frame using given destination address and payload
    void _send_IP_v4_frame(const EthernetAddress &dst_addr, const BufferList &payload);

    // Send a ARP request to all the hosts in LAN
    // to know ethernet address of given target IP address
    void _send_ARP_request(const uint32_t &target_addr);

    // Send a ARP reply to target using its IP address and ethernet address
    void _send_ARP_reply(const uint32_t &target_ip_addr, const EthernetAddress &target_eth_addr);

    // Send not sent datagrams when received ARP reply
    void _send_not_sent_datagrams(const uint32_t &ip_addr, const EthernetAddress &eth_addr);

  public:
    //! \brief Construct a network interface with given Ethernet (network-access-layer) and IP (internet-layer) addresses
    NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address);

    //! \brief Access queue of Ethernet frames awaiting transmission
    std::queue<EthernetFrame> &frames_out() { return _frames_out; }

    //! \brief Sends an IPv4 datagram, encapsulated in an Ethernet frame (if it knows the Ethernet destination address).

    //! Will need to use [ARP](\ref rfc::rfc826) to look up the Ethernet destination address for the next hop
    //! ("Sending" is accomplished by pushing the frame onto the frames_out queue.)
    void send_datagram(const InternetDatagram &dgram, const Address &next_hop);

    //! \brief Receives an Ethernet frame and responds appropriately.

    //! If type is IPv4, returns the datagram.
    //! If type is ARP request, learn a mapping from the "sender" fields, and send an ARP reply.
    //! If type is ARP reply, learn a mapping from the "sender" fields.
    std::optional<InternetDatagram> recv_frame(const EthernetFrame &frame);

    //! \brief Called periodically when time elapses
    void tick(const size_t ms_since_last_tick);
};

#endif  // SPONGE_LIBSPONGE_NETWORK_INTERFACE_HH
