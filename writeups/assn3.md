Assignment 3 Writeup
=============

My name: Moon Kyung Song

My POVIS ID: moonsong98

My student ID (numeric): 20180423

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:

1. Data structure for outstanding segments

To keep outstanding segments in order of absolute sequence number,
I choose to use priority queue which can get the lowest key in O(1), and insert/delete takes O(log N).

2. Methods

2-1. fill_window
1) When a segment with a SYN flag isn't sent, send it with an empty payload.
2) Calculate number of bytes which need to be sent considering
    2-1) Receiver's window
    2-2) Gap between next absolute sequence number and recent ackno
3) Create a segment
    3-1) If buffer is empty and not eof, do not create a segment
    3-2) Chop a segment if it exceeds maximum segment size
    3-3) If it's a last segment, turn on a fin flag
4) Send a segment via _send_segment()

2-2. ack_received
1) Calculate a absolute ackno using ackno, isn, and checkpoint which is a recent ackno
2)  Check validity of ackno. If condition below is satisfied, it's an unvalid ackno
    2-1) If it's greater than next_seqno, it hasn't been sent yet.
    2-2) If it's less than checkpoint, it already have been acknowledged.
3) Iterate through outstanding segments and remove if right edge of the window is less than or equal to absolute ackno
4) Reset the rto to the initial value, and if there is no outstanding segment, reset the timer.
5) Call fill_window to create and send segment

2-3. tick
1) Calculate a time spent.
2) If timer is expired,
    2-1) Retransmit the earliest segment that hsn't been fully acknolweged.
    2-2) _n_consecutive_retransmissions := _n_consecutive_retransmissions
    2-3) _rto := _rto * 2
    2-4) Reset the timer

2-4. send_empty_segment
Create a segment with no header and payload

2-5. _send_segment

1) Send a new segment by adding it in _segments_out
2) Track a segment in flight
3) Update seqno
4) Track a number of bytes in flight
5) If timer is not running, run it


Implementation Challenges:
None

Remaining Bugs:
None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
