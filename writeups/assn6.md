Assignment 6 Writeup
=============

My name: 송문경

My POVIS ID: moonsong98

My student ID (numeric): 20180423

This assignment took me about [1] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:

According to the instruction, it is okay to take O(n) for looking up the routing table.
Since then, I chose a std::vector as a data structure for a routing table,
since vector takes O(n) for iterating all elements.

Each element in a routing table consist of

- route\_prefix
- prefix\_length
- next\_hop
- interface\_num

which can be retrieved from add\_route().

To implement longest prefix match, I used OxFFFFFFFF << (32 - prefix\_length) as a mask.
If prefix\_length is 0 or masked destination address equals to route\_prefix of routing table,  
it means prefix matches.
Then, previous longest prefix length should be compared with the prefix length which matched.
If, current one is longer than the previous one, update it.

If not matched, or ttl is less than or equal to 1, drop the datagram.

Otherwise, send to next hop if next hop has value.
If it doesn't, send directly to the destination address saved in datagram's header.

Implementation Challenges:
None

Remaining Bugs:
None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
