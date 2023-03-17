Assignment 2 Writeup
=============

My name: 송문경

My POVIS ID: moonsong98

My student ID (numeric): 20180423

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:

- Wrap

As written in a document, offset from isn is absolute sequence number mod 2^32

- Unwrap

Since we have to find the absolute sequence number which is closest to the checkpoint, we first need to find the offset from the checkpoint.
I used it by comparing sequence number.
There is one exception that if sum of offset and the checkpoint is less than 0, overflow occurs since the lowest index is 0.
I add 2^32 to that situation.

- TCPReceiver

When segment is recieved, first check syn and fin flag in a header. Syn flag tells isn.
Since unwrap is implemented, I used unwrap to receive absolute sequence number and used it to write in a buffer.

Specific details are written in files with comments.

Implementation Challenges:
None

Remaining Bugs:
None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
