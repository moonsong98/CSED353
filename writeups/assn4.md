Assignment 4 Writeup
=============

My name: 송문경

My POVIS ID: moonsong98

My student ID (numeric): 20180423

This assignment took me about 48 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.30, 0.34]

Program Structure and Design of the TCPConnection:

TCPConnection is a Class that wires up all the tcp implementation I've done before.

1. When segment is received

1) If RST bit is set, kill the connection
2) If ACK bit is set before the connection is established, ignore it
3) Otherwise, receive the segment and reply if necessary.

2. Send segments

To send a segment, sender should create segments by calling fill_window() method.
Then, attach appropriate header to each segments.
Finlly, push them in _segments_out to send to opponent.

3. Tick

When tick() is called, notice it to sender and cound how many time has passed.
If number of consecutive retransmission exceeds given number, do unclean shutdown.
If SYN bit is sent, send segment

4. Unclean Shutdown

When number of consecutive retransmission exceeds given number
or deconstructor is called even the connection is active, unclean shutdown is called. 
It sets error on bytestreams and make the connection inactive.
Then, it sends a segment with RST bit set.

5. Clean Shutdown.

If the inbound stream ends before the TCPConnection has reached EOF on its outbound stream,
TCPConnection no longer needs to be lingered.
If three prerequisites that is written in instruction is met
and linger is not needed or time since last segment received exceeds certain time,
make connection inactive.


Implementation Challenges:

Some of given tests were failed.
By analyzing my code, I could found that StreamReassembler had some problem.
After I reimplement StreamReassembler, I could pass all the tests.

Remaining Bugs:

None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
