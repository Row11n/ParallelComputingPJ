#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

int partition(int* data, int start, int end)   
{
    if(end - start >= 2)
    {
        if(data[start + 1] > data[start])
            swap(data[start + 1], data[start]);
        if(data[start + 1] > data[start + 2])
            swap(data[start + 1], data[start + 2]);
        if(data[start+2] > data[start])
            swap(data[start+2], data[start]);
    }
    int temp = data[start];   
    while (start < end) 
    {
        while (start < end && data[end] >= temp)
            end--;   
        data[start] = data[end];

        while (start < end && data[start] <= temp)
            start++;  
        data[end] = data[start];
    }
    data[start] = temp;   
    return start;
}

void quickSort(int* data, int start, int end)  
{
    if (start < end) 
    {
        int pos = partition(data, start, end);        
        quickSort(data, start, pos - 1);        
        quickSort(data, pos + 1, end);
    }
}

void quickSort_omp(int* data, int start, int end)  
{
    if (end - start >= 0 && end - start < 8000)
    {
        quickSort(data, start, end);
        return;
    }
    if (start < end) 
    {
        int pos = partition(data, start, end);
        int pos_left = partition(data, start, pos - 1);
        int pos_right = partition(data, pos + 1, end);
        #pragma omp parallel shared(data, start, end)
        {
            #pragma omp sections   
            {
                #pragma omp section        
                quickSort_omp(data, start, pos_left - 1);
                #pragma omp section        
                quickSort_omp(data, pos_left + 1, pos - 1);
                #pragma omp section        
                quickSort_omp(data, pos + 1, pos_right - 1);
                #pragma omp section         
                quickSort_omp(data, pos_right + 1, end);
            }
            #pragma omp barrier
        }
    }
}


int main(int argc, char* argv[])
{
    int n = 4;
    int sizes[6] = {1000, 5000, 10000, 100000, 1000000, 10000000};
    omp_set_num_threads(n); 
    cout << "Input the number of parallel threads: ";
    cin >> n;

    for(int i = 0; i < 6; i++)
    {
        int size = sizes[i];
        int* num1 = (int*)malloc(sizeof(int) * size);
        int* num2 = (int*)malloc(sizeof(int) * size);
        srand(time(NULL));
        
        double total_time_p = 0, total_time_s = 0;
        for(int j = 0; j < 500; j++)
        {
            for (int i = 0; i < size; i++)
                num1[i] = num2[i] = rand();
            
            double start_s = omp_get_wtime();
            quickSort(num2, 0, size - 1);
            double end_s = omp_get_wtime();

            double start_p = omp_get_wtime();
            quickSort_omp(num1, 0, size - 1);
            double end_p = omp_get_wtime();

            total_time_s += end_s - start_s;
            total_time_p += end_p - start_p;
        }
        double average_time_p = total_time_p / 50;
        double average_time_s = total_time_s / 50;

        cout << "scale: " << size << "  runtime_parallel: " << average_time_p << "  runtime_single: " << average_time_s << endl;
        cout << "Speedup: " << average_time_s / average_time_p << endl;
    }
    return 0;
}
