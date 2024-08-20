# prc24.c: Process Tree Control and Signals Handling

## Overview
The `prc24.c` program is a powerful utility designed to manage and interact with processes in a Unix-like operating system. It allows users to explore and control process trees rooted at a specified process, send signals to control process behavior, and retrieve detailed information about the status of processes, including their descendants, siblings, and whether they are orphaned or zombie processes.

## Features
- **Search Process Trees**: Identify if a given process belongs to a tree rooted at a specified process and print its PID and PPID.
- **Process Control via Signals**: Use options to send signals like `SIGKILL`, `SIGSTOP`, and `SIGCONT` to processes within the tree.
- **Detailed Process Information**: List descendants, siblings, and grandchildren, and check if processes are defunct or orphaned.
- **Orphan and Zombie Detection**: Identify and manage orphan and zombie processes.

## Options
The program supports a variety of command-line options:

- `-dx`: Kill all descendants of the `root_process` using `SIGKILL`.
- `-dt`: Send `SIGSTOP` to all descendants of the `root_process`.
- `-dc`: Send `SIGCONT` to all descendants of the `root_process` that have been paused.
- `-rp`: Kill the specified `process_id` if it belongs to the tree rooted at `root_process`.
- `-nd`: List the PIDs of all non-direct descendants of the `process_id`.
- `-dd`: List the PIDs of all immediate descendants of the `process_id`.
- `-sb`: List the PIDs of all sibling processes of the `process_id`.
- `-bz`: List the PIDs of all sibling processes of the `process_id` that are defunct (zombie).
- `-zd`: List the PIDs of all descendants of the `process_id` that are defunct (zombie).
- `-od`: List the PIDs of all descendants of the `process_id` that are orphans.
- `-gc`: List the PIDs of all grandchildren of the `process_id`.
- `-sz`: Print the status of the `process_id` as Defunct or Not Defunct.
- `-so`: Print the status of the `process_id` as Orphan or Not Orphan.
- `-kz`: Kill the parents of all zombie processes that are descendants of the `process_id` (including the `process_id` itself if applicable).

## Compilation and Usage
### Compilation
To compile the program, use the following command:
```bash
gcc -o prc24s prc24s.c
```

### Running the Program
To run the program, use the following syntax:
```bash
./prc24s [Option] [root_process] [process_id]
```

## Technical Concepts
This project leverages several important Unix process management concepts:

- Process Trees: Understanding and navigating the hierarchical structure of processes.
- Signals: Sending control signals like SIGKILL, SIGSTOP, and SIGCONT to processes.
- Zombie Processes: Managing processes that have completed execution but remain in the process table.
- Orphan Processes: Identifying processes whose parent has exited.
