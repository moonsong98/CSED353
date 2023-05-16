#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    _routing_table.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    DUMMY_CODE(dgram);
    // Your code here.
    if (dgram.header().ttl <= 1)
        return;

    --dgram.header().ttl;

    bool is_matched = false;
    uint8_t matched_length = 0;
    size_t interface_num = 0;
    std::optional<Address> next_hop;

    for (auto el : _routing_table) {
        if ((not el.prefix_length or
             (dgram.header().dst & (0xFFFFFFFF << (32 - el.prefix_length))) == el.route_prefix) and
            matched_length <= el.prefix_length) {
            is_matched = true;
            matched_length = el.prefix_length;
            interface_num = el.interface_num;
            next_hop = el.next_hop;
        }
    }

    if (not is_matched)
        return;

    if (next_hop.has_value())
        interface(interface_num).send_datagram(dgram, next_hop.value());
    else
        interface(interface_num).send_datagram(dgram, Address::from_ipv4_numeric(dgram.header().dst));
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
