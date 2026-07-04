#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>

using namespace std;

// Helper function to print timing
void print_timing(const string& task_name, chrono::duration<double, milli> elapsed) {
    cout << left << setw(25) << task_name << " : " 
         << fixed << setprecision(3) << elapsed.count() << " ms" << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./sequential_analytics <dataset_size>\n";
        return 1;
    }

    size_t N = stoull(argv[1]);
    cout << "--- Sequential Analytics Baseline ---" << endl;
    cout << "Dataset Size: " << N << " elements\n\n";

    // ---------------------------------------------------------
    // DATA GENERATION (Option A - Synthetic Data)
    // ---------------------------------------------------------
    auto start_total = chrono::high_resolution_clock::now();
    
    vector<double> X(N);
    vector<double> Y(N); // Needed for Pearson Correlation
    
    mt19937_64 rng(42); // Fixed seed for reproducibility
    uniform_real_distribution<double> dist(0.0, 10000.0);

    for (size_t i = 0; i < N; ++i) {
        X[i] = dist(rng);
        Y[i] = dist(rng);
    }

    // ---------------------------------------------------------
    // TASK 1: Basic Statistics (Mean, Variance, StdDev, Min, Max)
    // ---------------------------------------------------------
    auto start_t1 = chrono::high_resolution_clock::now();
    
    double sum = 0.0, min_val = X[0], max_val = X[0];
    for (size_t i = 0; i < N; ++i) {
        sum += X[i];
        if (X[i] < min_val) min_val = X[i];
        if (X[i] > max_val) max_val = X[i];
    }
    double mean = sum / N;

    double sq_diff_sum = 0.0;
    for (size_t i = 0; i < N; ++i) {
        sq_diff_sum += (X[i] - mean) * (X[i] - mean);
    }
    double variance = sq_diff_sum / N;
    double stddev = sqrt(variance);

    cout << "   [Result] Mean: " << mean << ", StdDev: " << stddev << "\n";
    
    auto end_t1 = chrono::high_resolution_clock::now();
    print_timing("1. Basic Statistics", end_t1 - start_t1);

    // ---------------------------------------------------------
    // TASK 2: Histogram Generation (10 Bins)
    // ---------------------------------------------------------
    auto start_t2 = chrono::high_resolution_clock::now();
    
    int num_bins = 10;
    vector<int> histogram(num_bins, 0);
    double bin_width = 10000.0 / num_bins;

    for (size_t i = 0; i < N; ++i) {
        int bin = static_cast<int>(X[i] / bin_width);
        if (bin >= num_bins) bin = num_bins - 1; 
        histogram[bin]++;
    }
    
    auto end_t2 = chrono::high_resolution_clock::now();
    print_timing("2. Histogram Generation", end_t2 - start_t2);

    // ---------------------------------------------------------
    // TASK 3: Sorting (Standard C++ Sort)
    // ---------------------------------------------------------
    auto start_t3 = chrono::high_resolution_clock::now();
    
    vector<double> X_sorted = X; // Copy to avoid altering original data structure
    sort(X_sorted.begin(), X_sorted.end());
    
    auto end_t3 = chrono::high_resolution_clock::now();
    print_timing("3. Sorting", end_t3 - start_t3);

    // ---------------------------------------------------------
    // TASK 4: Pearson Correlation (Between X and Y)
    // ---------------------------------------------------------
    auto start_t4 = chrono::high_resolution_clock::now();
    
    double sum_Y = 0.0;
    for (size_t i = 0; i < N; ++i) sum_Y += Y[i];
    double mean_Y = sum_Y / N;

    double num = 0.0, den_X = 0.0, den_Y = 0.0;
    for (size_t i = 0; i < N; ++i) {
        double diff_X = X[i] - mean;
        double diff_Y = Y[i] - mean_Y;
        num += diff_X * diff_Y;
        den_X += diff_X * diff_X;
        den_Y += diff_Y * diff_Y;
    }
    double pearson_corr = num / sqrt(den_X * den_Y);

    cout << "   [Result] Mean: " << mean << ", StdDev: " << stddev << "\n";
    
    auto end_t4 = chrono::high_resolution_clock::now();
    print_timing("4. Pearson Correlation", end_t4 - start_t4);

    // ---------------------------------------------------------
    // TASK 5: Moving Average (Window Size = 5)
    // ---------------------------------------------------------
    auto start_t5 = chrono::high_resolution_clock::now();
    
    int window = 5;
    vector<double> moving_avg(N - window + 1);
    double window_sum = 0.0;
    
    // Initial window sum
    for (int i = 0; i < window; ++i) window_sum += X[i];
    moving_avg[0] = window_sum / window;

    // Sliding window
    for (size_t i = 1; i <= N - window; ++i) {
        window_sum = window_sum - X[i - 1] + X[i + window - 1];
        moving_avg[i] = window_sum / window;
    }
    
    auto end_t5 = chrono::high_resolution_clock::now();
    print_timing("5. Moving Average", end_t5 - start_t5);

    // ---------------------------------------------------------
    // TASK 6: Outlier Detection (Z-Score > 3)
    // ---------------------------------------------------------
    auto start_t6 = chrono::high_resolution_clock::now();
    
    size_t outlier_count = 0;
    for (size_t i = 0; i < N; ++i) {
        double z_score = abs(X[i] - mean) / stddev;
        if (z_score > 3.0) {
            outlier_count++;
        }
    }
    
    cout << "   [Result] Mean: " << mean << ", StdDev: " << stddev << "\n";

    auto end_t6 = chrono::high_resolution_clock::now();
    print_timing("6. Outlier Detection", end_t6 - start_t6);

    // Total time
    auto end_total = chrono::high_resolution_clock::now();
    cout << "\n-----------------------------------------\n";
    print_timing("Total Execution Time", end_total - start_total);
    cout << "-----------------------------------------\n";

    return 0;
}