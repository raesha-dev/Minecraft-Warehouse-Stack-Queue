# Minecraft Warehouse Inventory System

**Implementation of Stack and Queue Data Structures**

## Overview

This project implements a **warehouse inventory management system** inspired by Minecraft-style storage mechanics. 
It demonstrates the **practical application of core data structures**, specifically **Stack (LIFO)** and **Queue (FIFO)**, in a realistic systems context.

The emphasis is on **correctness, clarity, and real-world modeling**, rather than toy examples.

---

## Problem Statement

Efficient warehouse systems require:

* Structured storage of items
* Orderly dispatch based on arrival time
* Predictable and efficient operations

This project models these requirements using foundational data structures, showing how abstract DSA concepts translate into real systems.

---

>> Design and Data Structures

### Stack (LIFO)

Used for **inventory storage** where items are placed and removed from the top.

Key operations:

* Push: Add item to storage
* Pop: Remove most recently stored item
* Peek: Inspect current inventory top

Use case: Storage slots where recent items are accessed first.

---

### Queue (FIFO)

Used for **order and dispatch management** to preserve arrival order.

Key operations:

* Enqueue: Add order to dispatch queue
* Dequeue: Process next order
* Front: View upcoming dispatch

Use case: Ensuring fair and sequential order processing.

---

## Features

* Modular implementation of stack and queue
* Menu-driven console interface
* Input validation and overflow/underflow handling
* Clear separation of concerns across source files
* Clean version-controlled project structure

---

## Example Workflow

1. Items arrive and are stored using stack operations
2. Orders are placed and added to the queue
3. Expired batches are detected and ensures easy tracking and removal
4. Dispatch system processes orders in FIFO order
5. Inventory state updates dynamically

---


## Engineering Concepts Demonstrated

* Stack and Queue implementation from scratch
* Memory and state management in C
* Modular programming practices
* Translating abstract DSA into system-level behavior
* Clean Git workflow and repository hygiene

---

## Scope for Improvement

* Persistent storage using files
* Priority-based dispatching
* Visualization or GUI layer
* Performance analysis with time and space complexity metrics

---

## Why This Project

This project demonstrates:

* Strong grasp of **DSA fundamentals**
* Ability to **apply theory to real-world scenarios**
* Clean coding and repository practices expected in professional environments


