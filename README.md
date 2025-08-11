# Operating System Process & Resource Manager

## Overview

This project implements a simulation of an operating system’s process and resource manager. It manages process creation, destruction, scheduling, and resource allocation using a combination of Process Control Blocks (PCBs), Resource Control Blocks (RCBs), and multi-level ready lists.

## Features

* Process Management
  * Create processes with priorities 0, 1, or 2.
  * Destroy processes (including their children recursively).
  * Maintain a hierarchical process tree.
* Resource Management
  *  Four resource types (R0–R3) with different unit capacities.
  *  Request and release resources with unit constraints.
  *  Waitlists for blocked processes.
* Scheduling
  * Multi-level ready list with priorities (highest priority ready process runs first).
  * Preemption via timeout.
  * Blocked processes moved to waitlists until resources become available.
* Linked List Utilities
  * Custom singly linked list implementation for ready lists, children lists, and resource lists.
  * Support for inserting, removing, searching, and popping nodes.
 
## How to Run

This project reads an input.txt file containing the actions for the process & resource manage to execute and prints the output to an output.txt file. There is a sample input.txt provided. 

**Compile:** g++ -g resourceManager.cpp -o f <br>
**Run:** ./f < input.txt
