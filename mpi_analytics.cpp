#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <mpi.h>

using namespace std;

int main(int argc, char* argv[]) {
    // 1. Initialize the MPI Environment
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    size_t N = 0;
    
    if (rank == 0) {
        if (argc != 2) {
            cerr << "Usage: mpiexec -n 4 mpi_analytics.exe <dataset_size>\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        N = stoull(argv[1]);
    }

    // Broadcast the total dataset size (N) to all worker nodes
    MPI_Bcast(&N, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    size_t local_N = N / num_procs;
    vector<double> local_X(local_N);
    vector<double> local_Y(local_N);
    vector<double> global_X;
    vector<double> global_Y;

    if (rank == 0) {
        global_X.resize(N);
        global_Y.resize(N);
        mt19937_64 rng(42);
        uniform_real_distribution<double> dist(0.0, 10000.0);
        for (size_t i = 0; i < N; ++i) {
            global_X[i] = dist(rng);
            global_Y[i] = dist(rng);
        }
        cout << "--- MPI Distributed Analytics Baseline ---\n";
        cout << "Dataset Size: " << N << " elements across " << num_procs << " nodes.\n\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_total = MPI_Wtime();

    // Scatter the parallel arrays
    MPI_Scatter(global_X.data(), local_N, MPI_DOUBLE, local_X.data(), local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(global_Y.data(), local_N, MPI_DOUBLE, local_Y.data(), local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // ---------------------------------------------------------
    // TASK 1: Basic Statistics (Distributed)
    // ---------------------------------------------------------
    double start_t1 = MPI_Wtime();
    double local_sum = 0.0, local_min = local_X[0], local_max = local_X[0];
    for (double val : local_X) {
        local_sum += val;
        if (val < local_min) local_min = val;
        if (val > local_max) local_max = val;
    }
    
    double global_sum, global_min, global_max;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_min, &global_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Calculate Global Mean and broadcast to workers (needed for variance and outliers)
    double global_mean = 0.0;
    if (rank == 0) global_mean = global_sum / N;
    MPI_Bcast(&global_mean, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Calculate Variance
    double local_sq_diff = 0.0;
    for (double val : local_X) local_sq_diff += (val - global_mean) * (val - global_mean);
    double global_sq_diff;
    MPI_Reduce(&local_sq_diff, &global_sq_diff, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double global_stddev = 0.0;
    if (rank == 0) {
        double variance = global_sq_diff / N;
        global_stddev = sqrt(variance);
        cout << left << setw(25) << "1. Basic Statistics" << " : " << fixed << setprecision(3) << (MPI_Wtime() - start_t1) * 1000.0 << " ms\n";
        cout << "   [Result] Mean: " << global_mean << ", StdDev: " << global_stddev << "\n";
    }
    // Broadcast StdDev for Task 6
    MPI_Bcast(&global_stddev, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // ---------------------------------------------------------
    // TASK 2: Histogram Generation (Distributed)
    // ---------------------------------------------------------
    double start_t2 = MPI_Wtime();
    int num_bins = 10;
    vector<int> local_hist(num_bins, 0);
    double bin_width = 10000.0 / num_bins;
    
    for (double val : local_X) {
        int bin = static_cast<int>(val / bin_width);
        if (bin >= num_bins) bin = num_bins - 1;
        local_hist[bin]++;
    }
    
    vector<int> global_hist(num_bins, 0);
    MPI_Reduce(local_hist.data(), global_hist.data(), num_bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) cout << left << setw(25) << "2. Histogram Generation" << " : " << (MPI_Wtime() - start_t2) * 1000.0 << " ms\n";

    // ---------------------------------------------------------
    // TASK 3: Distributed Sort
    // ---------------------------------------------------------
    double start_t3 = MPI_Wtime();
    // Use a copy to prevent destroying the paired order required by Task 4
    vector<double> local_X_sorted = local_X; 
    sort(local_X_sorted.begin(), local_X_sorted.end());
    
    if (rank == 0) {
        MPI_Gather(MPI_IN_PLACE, local_N, MPI_DOUBLE, global_X.data(), local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        sort(global_X.begin(), global_X.end());
        cout << left << setw(25) << "3. Sorting" << " : " << (MPI_Wtime() - start_t3) * 1000.0 << " ms\n";
    } else {
        MPI_Gather(local_X_sorted.data(), local_N, MPI_DOUBLE, nullptr, local_N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // ---------------------------------------------------------
    // TASK 4: Pearson Correlation (Distributed)
    // ---------------------------------------------------------
    double start_t4 = MPI_Wtime();
    double local_sum_Y = 0.0;
    for(double val : local_Y) local_sum_Y += val;
    double global_sum_Y;
    MPI_Reduce(&local_sum_Y, &global_sum_Y, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double mean_Y = 0.0;
    if (rank == 0) mean_Y = global_sum_Y / N;
    MPI_Bcast(&mean_Y, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local_num = 0.0, local_den_Y = 0.0;
    for(size_t i = 0; i < local_N; ++i) {
        double dx = local_X[i] - global_mean;
        double dy = local_Y[i] - mean_Y;
        local_num += dx * dy;
        local_den_Y += dy * dy;
    }
    
    double global_num, global_den_Y;
    MPI_Reduce(&local_num, &global_num, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_den_Y, &global_den_Y, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        double pearson = global_num / sqrt(global_sq_diff * global_den_Y);
        cout << left << setw(25) << "4. Pearson Correlation" << " : " << (MPI_Wtime() - start_t4) * 1000.0 << " ms\n";
        cout << "   [Result] Correlation: " << pearson << "\n";
    }

    // ---------------------------------------------------------
    // TASK 5: Moving Average (Halo Exchange)
    // ---------------------------------------------------------
    double start_t5 = MPI_Wtime();
    int window = 5;
    int overlap = window - 1;
    vector<double> prev_elements(overlap, 0.0);

    // Each node requests the tailing elements from the node strictly before it
    if (rank > 0) {
        MPI_Recv(prev_elements.data(), overlap, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (rank < num_procs - 1) {
        MPI_Send(local_X.data() + local_N - overlap, overlap, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
    }

    vector<double> local_mavg(local_N);
    for(size_t i = 0; i < local_N; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = 0; j < window; ++j) {
            int idx = i - j;
            if (idx >= 0) {
                sum += local_X[idx];
                count++;
            } else {
                int prev_idx = overlap + idx;
                if (rank > 0 && prev_idx >= 0) {
                    sum += prev_elements[prev_idx];
                    count++;
                }
            }
        }
        local_mavg[i] = sum / count;
    }
    MPI_Barrier(MPI_COMM_WORLD); 
    if (rank == 0) cout << left << setw(25) << "5. Moving Average" << " : " << (MPI_Wtime() - start_t5) * 1000.0 << " ms\n";

    // ---------------------------------------------------------
    // TASK 6: Outlier Detection
    // ---------------------------------------------------------
    double start_t6 = MPI_Wtime();
    unsigned long long local_outliers = 0;
    for (double val : local_X) {
        if (abs(val - global_mean) / global_stddev > 3.0) {
            local_outliers++;
        }
    }
    
    unsigned long long global_outliers = 0;
    MPI_Reduce(&local_outliers, &global_outliers, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        cout << left << setw(25) << "6. Outlier Detection" << " : " << (MPI_Wtime() - start_t6) * 1000.0 << " ms\n";
        cout << "   [Result] Outliers: " << global_outliers << "\n";
    }

    // ---------------------------------------------------------
    // Final Execution Calculation
    // ---------------------------------------------------------
    double end_total = MPI_Wtime();
    if (rank == 0) {
        cout << "\n-----------------------------------------\n";
        cout << left << setw(25) << "Total MPI Execution" << " : " << (end_total - start_total) * 1000.0 << " ms\n";
        cout << "-----------------------------------------\n";
    }

    MPI_Finalize();
    return 0;
}