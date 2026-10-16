# WATCH OS — CLI & Analysis Console Guide

## 1. Startup & Main Menu Navigation
Launch WATCH OS:
```powershell
.\build\watchos.exe [database_path]
```
Main Menu options:
1. **Create New Session**: Configure processes, review workload, and proceed to Analysis Console.
2. **Resume Existing Session**: Select by row number or Session ID to load previous session and enter Analysis Console.
3. **View Session History**: View all saved sessions in SQLite database.
4. **Compare Previous Sessions**: Side-by-side comparison of workloads and parameters.
5. **Settings & Configuration**: Configure default quantum, memory frames, aging settings, and toggle ANSI color.
6. **Exit**: Gracefully close and preserve committed data.

---

## 2. Kernel Analysis Console (Options 01–18)

When a session is finalized or resumed, the Kernel Analysis Console becomes active:

```
============================================================
            WATCH OS — KERNEL ANALYSIS CONSOLE
============================================================
 01  Find Best CPU Scheduling Algorithm
 02  Run All Scheduling Algorithms
 03  Display Gantt Charts
 04  Show Process Lifecycle
 05  Analyze Context Switching
 06  Detect Starvation
 07  Apply Aging and Compare
 08  Run Deadlock Analysis
 09  Estimate Deadlock Frequency
 10  Virtual Memory Analysis
 11  Disk Usage & I/O Analysis
 12  Process Synchronization & IPC
 13  Permissions & Protection
 14  Run Complete Kernel Analysis
 15  Compare Previous Runs
 16  View Session Logs
 17  Generate Final Report
 18  Return to Main Menu
```

### Supported CLI Commands
In addition to typing numbers `01` through `18`, the command prompt accepts:
- `schedule all`: Runs all CPU scheduling algorithms.
- `show lifecycle`: Displays process lifecycle transition history.
- `deadlock analyze`: Renders RAG and runs Banker's algorithm safe-state analysis.
- `compare runs`: Lists and compares simulation runs from SQLite.
- `report generate`: Generates Markdown (`reports/report_<session>.md`) and CSV (`reports/report_<session>.csv`) reports.
- `back` / `quit` / `exit`: Returns to Main Menu.
