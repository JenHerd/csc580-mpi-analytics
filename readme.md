# Distributed Analytics Engine (MPI Cluster)

## Project Overview
This project demonstrates a distributed computing cluster built across physical nodes using the Message Passing Interface (MS-MPI) in C++. It processes large datasets (up to 10 million elements) and calculates basic statistics, histograms, and data sorting in parallel.

## Hardware & Network Architecture
* **Framework:** MS-MPI (C++)
* **Nodes:** Physical Laptops/PCs (Master/Worker architecture)
* **Network Infrastructure:** Wireless Mobile Hotspot

## Execution & Security Bypass (Error 5: Access Denied)
When running `mpiexec` across physical Windows machines, Windows Defender and local account security policies often block remote execution (Error 5). We bypassed this by establishing a unified cluster identity across all physical nodes.

**1. On all Worker nodes (Run Command Prompt as Administrator):**
`net user MpiCluster P@ssw0rd2026! /add`
`net localgroup administrators MpiCluster /add`

**2. On the Master node (Normal Command Prompt):**
`runas /user:MpiCluster cmd`
*Execute all `mpiexec` commands from this newly spawned authenticated terminal.*

## How to Run

1. Compile the `.cpp` files using Visual Studio with MS-MPI linked (`msmpi.lib`).

2. **CRITICAL: Distribute the Executable**
   Make sure that the `mpi_analytics.exe` file you just compiled is copied to the exact same folder path on your Master laptop and all Worker laptops. The MPI service needs to find the file locally on all machines to execute it.
   **Recommended path:** `C:\MPI_Project\`

3. Test the baseline sequential performance:
   `C:\MPI_Project\sequential_analytics.exe 10000000`

4. Verify cluster connectivity from the Master node:
   `mpiexec -hosts 4 [IP_1] 1 [IP_2] 1 [IP_3] 1 [IP_4] 1 hostname`

5. After the terminal lists every hostname, execute the parallel analytics from the Master node (scaling up to test speedup):
   `mpiexec -hosts 2 [IP_1] 1 [IP_2] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
   `mpiexec -hosts 3 [IP_1] 1 [IP_2] 1 [IP_3] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
   `mpiexec -hosts 4 [IP_1] 1 [IP_2] 1 [IP_3] 1 [IP_4] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
