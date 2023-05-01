#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    if(_ip2eth.count(next_hop_ip))
        _send_IP_v4_frame(_ip2eth[next_hop_ip].first, dgram.serialize());

    else {
        auto it = _not_mapped_ip_address.find(next_hop_ip);
        
        // If no mapping exists or time after ARP Request sent passed 5s resend it
        if(it == _not_mapped_ip_address.end() || it->second + 5000 < _time_passed)
            _send_ARP_request(next_hop_ip);

        // If no mapping exists save it as not mapped
        if(it == _not_mapped_ip_address.end())
            _not_mapped_ip_address[next_hop_ip] = _time_passed;

        _not_sent_dgrams[next_hop_ip].push_back(BufferList(dgram.serialize()));
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if(frame.header().dst == ETHERNET_BROADCAST || frame.header().dst == _ethernet_address) {
        if(frame.header().type == EthernetHeader::TYPE_ARP){
            ARPMessage arp_msg;
            if(arp_msg.parse(frame.payload()) == ParseResult::NoError) {
                const uint32_t& sender_ip_address = arp_msg.sender_ip_address;
                const EthernetAddress& sender_eth_address = arp_msg.sender_ethernet_address;

                // Map IP address to Ethernet Address
                _map_ethernet_address(sender_ip_address, sender_eth_address);

                // Received ARP Request, Send ARP Reply
                if(arp_msg.opcode == ARPMessage::OPCODE_REQUEST and arp_msg.target_ip_address == _ip_address.ipv4_numeric())
                    _send_ARP_reply(sender_ip_address, sender_eth_address);

                auto it = _not_sent_dgrams.find(sender_ip_address);
                if(it == _not_sent_dgrams.end())
                    return nullopt;
                for (BufferList bl: it->second)
                    _send_IP_v4_frame(sender_eth_address, bl);
                _not_sent_dgrams.erase(it);
                _not_mapped_ip_address.erase(_not_mapped_ip_address.find(sender_ip_address));
            }
        }
        else if(frame.header().type == EthernetHeader::TYPE_IPv4){
            InternetDatagram dgram;
            if(dgram.parse(frame.payload()) == ParseResult::NoError)
                return dgram;
        }
    }
    return nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    _time_passed += ms_since_last_tick;
    
    // Unmap expired mapping
    for(auto it = _ip2eth.begin(); it != _ip2eth.end();) {
        if(it->second.second + 30000 < _time_passed)
            _ip2eth.erase(it++);
        else
            ++it;
    }

    // Resend if didn't receive ARP reply within 5 seconds
    for(auto it = _not_mapped_ip_address.begin(); it != _not_mapped_ip_address.end();) {
        if(it->second + 5000 < ms_since_last_tick) {
            _send_ARP_request(it->first);
            it->second = ms_since_last_tick;
        }
        else
            ++it;
    }
}

void NetworkInterface::_map_ethernet_address(const uint32_t& ip_addr, const EthernetAddress& eth_addr) {
        auto it = _ip2eth.find(ip_addr);
        if(it != _ip2eth.end() && it->second.first != eth_addr) {
            it->second.first = eth_addr;
            it->second.second = _time_passed;
        }
        else
            _ip2eth[ip_addr] = {eth_addr, _time_passed};
}

void NetworkInterface::_send_IP_v4_frame(const EthernetAddress& dst_addr, const BufferList& payload) {
    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    frame.header().dst = dst_addr;
    frame.header().type = EthernetHeader::TYPE_IPv4;
    frame.payload() = payload;

    _frames_out.push(frame);
}

void NetworkInterface::_send_ARP_request(const uint32_t target_addr) {
    EthernetFrame frame;

    frame.header().src = _ethernet_address;
    frame.header().dst = ETHERNET_BROADCAST;
    frame.header().type = EthernetHeader::TYPE_ARP;

    ARPMessage arp_msg;
    arp_msg.sender_ip_address = _ip_address.ipv4_numeric();
    arp_msg.sender_ethernet_address = _ethernet_address;
    arp_msg.target_ip_address = target_addr;
    arp_msg.opcode = ARPMessage::OPCODE_REQUEST;

    frame.payload() = BufferList(arp_msg.serialize());

    _frames_out.push(frame);
}

void NetworkInterface::_send_ARP_reply(const uint32_t target_ip_addr, const EthernetAddress& target_eth_addr) {
    EthernetFrame frame;

    frame.header().src = _ethernet_address;
    frame.header().dst = target_eth_addr;
    frame.header().type = EthernetHeader::TYPE_ARP;

    ARPMessage arp_msg;
    arp_msg.sender_ip_address = _ip_address.ipv4_numeric();
    arp_msg.sender_ethernet_address = _ethernet_address;
    arp_msg.target_ip_address = target_ip_addr;
    arp_msg.target_ethernet_address = target_eth_addr;
    arp_msg.opcode = ARPMessage::OPCODE_REPLY;

    frame.payload() = BufferList(arp_msg.serialize());

    _frames_out.push(frame);

}
