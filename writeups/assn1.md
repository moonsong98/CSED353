Assignment 1 Writeup
=============

My name: Moon Kyung Song

My POVIS ID: moonsong98

My student ID (numeric): 20180423

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:

Data Structure
Since the segments could come in different order with overlapping data,
lot's of insertion, modification, and deletion exists anywhere in the data structure necessarily.
BST takes O(log n) for all those operations, so I chose map which is one of BSTs.

Program Structure
Everytime push_substring is called, I created a segment and write segments if possible.
When creating a segment, I considered several cases considering overlapping.
When writing segments to output, I considered remaining capacity and write if possible.

Implementation Challenges:
When I first read the instruciton, I though all the data has a content.
But, when I run a test, empty data existed.
It was not easy to find which code made the error by just running 'make check_lab1'.
Instead, I found that gdb is useful to debug.
I was not used to use gdb, but by this assignment, I could be familiar with gdb.

Remaining Bugs:
I cannot find any bug.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
