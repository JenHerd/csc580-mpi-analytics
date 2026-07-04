# Distributed Analytics Engine (MPI Cluster)

**Course:** CSC580 Parallel Computing
**Institution:** Universiti Teknologi MARA (UiTM)

## Team Members

- **ALIF DANIAL BIN AMIN NASIR** (2024540075) - Node Master / Architect
- **MUHAMMAD AMIRUL HUSNI BIN KAMAL ARIFIN** (2024757817) - Algorithm Developer
- **MUHAMAD FARIS BIN MOHD ROZI** (2024962977) - Data Engineer
- **NIK NASYAD BIN NIK SHAUKI** (2024752633) - Performance Analyst

## Project Overview

This project demonstrates a distributed computing cluster built across physical nodes using the Message Passing Interface (MS-MPI) in C++. It processes large datasets (up to 10 million elements) and calculates basic statistics, histograms, and data sorting in parallel to measure speedup and communication overhead.

## Hardware & Network Architecture

- **Framework:** MS-MPI (C++)
- **Nodes:** 4x Physical Laptops (Master/Worker architecture)
- **Network Infrastructure:** Wireless Mobile Hotspot 

## Prerequisites

To compile and run this project, all nodes must have the following installed:

1. **Microsoft MPI (MS-MPI):** Both the SDK (`msmpisdk.msi`) and Runtime (`msmpisetup.exe`).
2. **Visual Studio Build Tools:** Specifically the "Desktop development with C++" workload.

## Execution & Security Bypass (Error 5: Access Denied)

When running `mpiexec` across physical Windows machines, Windows Defender and local account security policies often block remote execution (Error 5). We bypassed this by establishing a unified cluster identity across all physical nodes.

**1. On all Worker nodes (Run Command Prompt as Administrator):**
`net user MpiCluster P@ssw0rd2026! /add`
`net localgroup administrators MpiCluster /add`

**2. On the Master node (Normal Command Prompt):**
`runas /user:MpiCluster cmd`
_Execute all `mpiexec` commands from this newly spawned authenticated terminal._

## How to Compile

Open the **x64 Native Tools Command Prompt for VS** and run the following commands:

**For the Sequential Baseline:**
`cl /EHsc /O2 sequential_analytics.cpp /link /out:sequential_analytics.exe`

**For the MPI Distributed Program:**
`cl /EHsc /O2 mpi_analytics.cpp /I "%MSMPI_INC%." /link "%MSMPI_LIB64%msmpi.lib" /out:mpi_analytics.exe`

## How to Run

1. **CRITICAL: Distribute the Executable**
   Make sure that the `mpi_analytics.exe` file you just compiled is copied to the exact same folder path on your Master laptop and all Worker laptops. The MPI service needs to find the file locally on all machines to execute it.
   **Recommended path:** `C:\MPI_Project\`

2. Test the baseline sequential performance:
   `C:\MPI_Project\sequential_analytics.exe 10000000`

3. Verify cluster connectivity from the Master node:
   `mpiexec -hosts 4 [IP_1] 1 [IP_2] 1 [IP_3] 1 [IP_4] 1 hostname`

4. After the terminal lists every hostname, execute the parallel analytics from the Master node (scaling up to test node failure and efficiency):
   `mpiexec -hosts 1 [IP_1] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
   `mpiexec -hosts 2 [IP_1] 1 [IP_2] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
   `mpiexec -hosts 3 [IP_1] 1 [IP_2] 1 [IP_3] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
   `mpiexec -hosts 4 [IP_1] 1 [IP_2] 1 [IP_3] 1 [IP_4] 1 C:\MPI_Project\mpi_analytics.exe 10000000`
