# 🌲 prc24s.c — Process Tree Control and Signals Handling

**prc24s.c** is a Unix-based utility program designed to interact with and control process trees. It provides tools to send signals, retrieve detailed process relationship data (siblings, children, orphans, zombies), and manage process states across the tree rooted at a given process.

---

## 📌 Overview

This utility helps:

- Explore the hierarchical **process tree** of any root process.
- Control processes by sending **Unix signals** like `SIGKILL`, `SIGSTOP`, and `SIGCONT`.
- Identify **zombie**, **orphan**, and **defunct** processes.
- Print **descendants**, **siblings**, and **grandchildren** of any process.

---

## 🧠 Features

- ✅ **Process Tree Navigation**  
  Identify if a process belongs to a tree and fetch its `PID` and `PPID`.

- 🔁 **Signal Control**  
  Send signals to descendants for kill, pause, or continue operations.

- 🔎 **Relationship Discovery**  
  List descendants, immediate children, siblings, and grandchildren.

- ☠️ **Zombie Detection**  
  Identify and manage zombie (defunct) processes.

- 👻 **Orphan Identification**  
  Detect orphaned processes (whose parent is no longer alive).

---

## ⚙️ Command-Line Options

| Option  | Description |
|---------|-------------|
| `-dx`   | Kill all **descendants** of `root_process` using `SIGKILL`. |
| `-dt`   | Send `SIGSTOP` to all **descendants** of `root_process`. |
| `-dc`   | Send `SIGCONT` to **paused descendants** of `root_process`. |
| `-rp`   | Kill the `process_id` **if** it belongs to the tree rooted at `root_process`. |
| `-nd`   | List all **non-direct descendants** of `process_id`. |
| `-dd`   | List all **immediate children** of `process_id`. |
| `-sb`   | List all **siblings** of `process_id`. |
| `-bz`   | List all **zombie siblings** of `process_id`. |
| `-zd`   | List all **zombie descendants** of `process_id`. |
| `-od`   | List all **orphan descendants** of `process_id`. |
| `-gc`   | List all **grandchildren** of `process_id`. |
| `-sz`   | Show whether `process_id` is **Defunct** or **Not Defunct**. |
| `-so`   | Show whether `process_id` is **Orphan** or **Not Orphan**. |
| `-kz`   | Kill the **parents** of all zombie descendants of `process_id` (including itself if applicable). |

---

## 🛠️ Compilation

Use the following command to compile the source code:

```bash
gcc -o prc24s prc24s.c
```

---

##  🚀 Usage

To run the compiled executable:

```bash
./prc24s [Option] [root_process] [process_id]
```
📌 Note: Some options do not require both root_process and process_id.


---

##  🧬 Technical Concepts Used

- Process Trees: Visualizing the parent-child hierarchy in Unix systems.

- Signals: Handling SIGKILL, SIGSTOP, SIGCONT using kill().

- Zombie Processes: Processes that have finished execution but still exist in the process table.

- Orphan Processes: Child processes with a terminated parent.

- Process Relationship Identification: Exploring direct and indirect process connections via /proc filesystem.
