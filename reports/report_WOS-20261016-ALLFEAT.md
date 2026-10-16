# WATCH OS — SIMULATION REPORT

**Session ID:** `WOS-20261016-ALLFEAT`  
**Run ID:** `RUN-000004`  
**Generated At:** 2026-10-16 18:24:13 UTC  
**Simulation Status:** COMPLETED  

---

## 1. Executive Summary
A full-system operating simulator analysis was conducted on workload of 4 configured processes. Subsystems evaluated include CPU Scheduling, Virtual Memory, Deadlock/Resource Allocation, Disk Arm Motion, and Access Protection.

## 2. Workload Configuration
| PID | Arrival Time | CPU Burst | Priority | Memory Demand |
| :--- | :--- | :--- | :--- | :--- |
| 101 | 0 | 6 | 1 | 64 units |
| 102 | 1 | 8 | 2 | 128 units |
| 103 | 2 | 4 | 3 | 32 units |
| 104 | 4 | 3 | 4 | 16 units |

## 3. CPU Scheduling Results
| Algorithm | Avg Waiting Time | Avg Turnaround Time | Avg Response Time | Context Switches | CPU Utilization |
| :--- | :--- | :--- | :--- | :--- | :--- |
| FCFS | 7.75 | 13.00 | 7.75 | 3 | 100.00% |
| SJF | 5.25 | 10.50 | 5.25 | 3 | 100.00% |
| SRTF | 5.25 | 10.50 | 5.25 | 3 | 100.00% |
| Priority (Non-Preemptive) | 7.75 | 13.00 | 7.75 | 3 | 100.00% |
| Priority (Preemptive) | 7.75 | 13.00 | 7.75 | 3 | 100.00% |
| Round Robin (Q=2) | 10.00 | 15.25 | 1.75 | 9 | 100.00% |

## 4. Gantt Charts
```text
FCFS Gantt Chart:
| P101 | P102 | P103 | P104 |
0      6     14     18     21

SJF Gantt Chart:
| P101 | P104 | P103 | P102 |
0      6      9     13     21

SRTF Gantt Chart:
| P101 | P104 | P103 | P102 |
0      6      9     13     21

Priority (Non-Preemptive) Gantt Chart:
| P101 | P102 | P103 | P104 |
0      6     14     18     21

Priority (Preemptive) Gantt Chart:
| P101 | P102 | P103 | P104 |
0      6     14     18     21

Round Robin (Q=2) Gantt Chart:
| P101 | P102 | P103 | P101 | P104 | P102 | P103 | P101 | P104 | P102 | P102 |
0      2      4      6      8     10     12     14     16     17     19     21

```

## 5. Memory Management & Page Replacement
| Algorithm | Total References | Page Faults | Page Hits | Fault Ratio | Thrashing Risk |
| :--- | :--- | :--- | :--- | :--- | :--- |
| FIFO | 60 | 26 | 34 | 43.33% | NORMAL |
| LRU | 60 | 25 | 35 | 41.67% | NORMAL |
| Optimal | 60 | 18 | 42 | 30.00% | NORMAL |

## 6. Deadlock & Banker's Algorithm Safe State
- **Banker's Algorithm State:** SAFE
- **Details:** Safe state verified. A valid safe execution sequence exists: <P101, P102, P103, P104>
- **Deadlock Detection State:** DEADLOCK FREE

## 7. Disk Head Scheduling
- **Algorithm:** SSTF
- **Initial Head Position:** 53
- **Total Head Movement:** 236 cylinders
- **Average Movement:** 29.50 cylinders/request

## 8. Key Findings & Trade-Offs
1. Shortest remaining time / SJF algorithms optimize average waiting times at the cost of preemption overhead.
2. Round Robin provides bounded response time ideal for interactive systems.
3. Optimal page replacement establishes the theoretical minimum fault ceiling for comparison against FIFO and LRU.
4. Banker's state checks prevent resource allocation deadlocks.

## 9. Limitations & Assumptions
- Simulates user-space OS algorithms under controlled workloads.
- Integer simulated clock time units represent idealized execution cycles.
- Access control rules and memory protection boundaries are educational and separate from host OS security.
