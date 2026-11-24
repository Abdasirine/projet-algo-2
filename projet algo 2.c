
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <psapi.h>

#define RUNS 5
#define MAX_SIZE 100000
#define SMALL_LIMIT 10000

/* ---------- Utility functions ---------- */
void copyArray(int dest[], int src[], int n)
{
    for (int i = 0; i < n; i++)
        dest[i] = src[i];
}

void generateArray(int a[], int n)
{
    for (int i = 0; i < n; i++)
        a[i] = rand() % 100000;
}

int isSorted(int a[], int n)
{
    for (int i = 1; i < n; i++)
        if (a[i] < a[i - 1])
            return 0;

    return 1;
}

/* ---------- 1. Bubble Sort ---------- */

void bubbleSort(int a[], int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int swapped = 0;

        for (int j = 0; j < n - 1 - i; j++)
        {
            if (a[j] > a[j + 1])
            {
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
                swapped = 1;
            }
        }

        if (swapped == 0)
            break;
    }
}

/* ---------- 2. Selection Sort ---------- */

void selectionSort(int a[], int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int min = i;

        for (int j = i + 1; j < n; j++)
        {
            if (a[j] < a[min])
                min = j;
        }

        int temp = a[i];
        a[i] = a[min];
        a[min] = temp;
    }
}

/* ---------- 3. Insertion Sort ---------- */

void insertionSort(int a[], int n)
{
    for (int i = 1; i < n; i++)
    {
        int key = a[i];
        int j = i - 1;

        while (j >= 0 && a[j] > key)
        {
            a[j + 1] = a[j];
            j--;
        }

        a[j + 1] = key;
    }
}

/* ---------- 4. Merge Sort ---------- */

void merge(int a[], int temp[], int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right)
    {
        if (a[i] <= a[j])
            temp[k++] = a[i++];
        else
            temp[k++] = a[j++];
    }

    while (i <= mid)
        temp[k++] = a[i++];

    while (j <= right)
        temp[k++] = a[j++];

    for (i = left; i <= right; i++)
        a[i] = temp[i];
}

void mergeRec(int a[], int temp[], int left, int right)
{
    if (left >= right)
        return;

    int mid = left + (right - left) / 2;

    mergeRec(a, temp, left, mid);
    mergeRec(a, temp, mid + 1, right);

    merge(a, temp, left, mid, right);
}

void mergeSort(int a[], int n)
{
    if (n <= 1)
        return;

    int *temp = malloc(n * sizeof(int));

    if (temp == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    mergeRec(a, temp, 0, n - 1);

    free(temp);
}

/* ---------- 5. Quick Sort ---------- */
/* Pivot: last element */

int partition(int a[], int low, int high)
{
    int pivot = a[high];
    int i = low - 1;

    for (int j = low; j < high; j++)
    {
        if (a[j] <= pivot)
        {
            i++;

            int temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }

    int temp = a[i + 1];
    a[i + 1] = a[high];
    a[high] = temp;

    return i + 1;
}

void quickRec(int a[], int low, int high)
{
    if (low < high)
    {
        int p = partition(a, low, high);

        quickRec(a, low, p - 1);
        quickRec(a, p + 1, high);
    }
}

void quickSort(int a[], int n)
{
    if (n > 1)
        quickRec(a, 0, n - 1);
}

/* ---------- Memory measurement ---------- */

double getMemoryMB(void)
{
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(pmc);

    if (GetProcessMemoryInfo(GetCurrentProcess(),
            &pmc, sizeof(pmc)))
    {
        return pmc.PeakWorkingSetSize /
               (1024.0 * 1024.0);
    }

    return -1.0;
}

/* ---------- Time measurement ---------- */

typedef void (*SortFunction)(int[], int);

double measureTime(SortFunction sort, int original[],
                   int n, double *memory)
{
    int *a = malloc(n * sizeof(int));

    if (a == NULL)
    {
        *memory = -1.0;
        return -1.0;
    }

    double total = 0.0;
    *memory = -1.0;

    for (int r = 0; r < RUNS; r++)
    {
        copyArray(a, original, n);

        clock_t start = clock();
        sort(a, n);
        clock_t end = clock();

        if (start == (clock_t)-1 ||
            end == (clock_t)-1)
        {
            free(a);
            return -1.0;
        }

        total += 1000.0 * (end - start) /
                 CLOCKS_PER_SEC;

        double m = getMemoryMB();

        if (m > *memory)
            *memory = m;

        if (!isSorted(a, n))
        {
            printf("Sorting error!\n");
            free(a);
            return -1.0;
        }
    }

    free(a);

    return total / RUNS;
}

/* ---------- Run one algorithm ---------- */

void runOne(char name[], SortFunction sort,
            int original[], int n)
{
    double memory;
    double time = measureTime(sort, original, n, &memory);

    if (time < 0)
    {
        printf("Measurement failed.\n");
        return;
    }

    printf("\nAlgorithm: %s\n", name);
    printf("Array size: %d\n", n);
    printf("Average time: %.3f ms\n", time);

    if (memory >= 0)
        printf("Process peak memory: %.2f MB\n", memory);
    else
        printf("Memory measurement failed.\n");
}

/* ---------- Compare all algorithms ---------- */

void compareAll(int original[], int n)
{
    SortFunction functions[5] = {
        bubbleSort, selectionSort, insertionSort,
        mergeSort, quickSort
    };

    char *names[5] = {
        "Bubble Sort", "Selection Sort", "Insertion Sort",
        "Merge Sort", "Quick Sort"
    };

    printf("\n%-18s %12s %15s\n",
           "Algorithm", "Time (ms)", "Memory (MB)");

    for (int i = 0; i < 5; i++)
    {
        if (n > SMALL_LIMIT && i < 3)
        {
            printf("%-18s %12s %15s\n",
                   names[i], "Skipped", "Skipped");
            continue;
        }

        double memory;
        double time = measureTime(functions[i],
                                  original, n, &memory);

        if (time < 0)
            printf("%-18s %12s %15s\n",
                   names[i], "Error", "Error");
        else
            printf("%-18s %12.3f %15.2f\n",
                   names[i], time, memory);
    }
}

/* ---------- Main program ---------- */

int main(void)
{
    int choice, n;
    int *a;

    srand(42);

    do
    {
        printf("\n====== SORTING ALGORITHMS ======\n");
        printf("1. Bubble Sort\n");
        printf("2. Selection Sort\n");
        printf("3. Insertion Sort\n");
        printf("4. Merge Sort\n");
        printf("5. Quick Sort\n");
        printf("6. Compare all algorithms\n");
        printf("7. Large array test (100000)\n");
        printf("0. Exit\n");
        printf("Your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input!\n");

            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}

            continue;
        }

        if (choice == 0)
            break;

        if (choice < 1 || choice > 7)
        {
            printf("Invalid choice!\n");
            continue;
        }

        if (choice == 7)
            n = MAX_SIZE;
        else
        {
            printf("Enter array size (1-%d): ", MAX_SIZE);

            if (scanf("%d", &n) != 1 ||
                n < 1 || n > MAX_SIZE)
            {
                printf("Invalid size!\n");

                int c;
                while ((c = getchar()) != '\n' && c != EOF) {}

                continue;
            }
        }

        a = malloc(n * sizeof(int));

        if (a == NULL)
        {
            printf("Memory allocation failed!\n");
            continue;
        }

        generateArray(a, n);

        switch (choice)
        {
            case 1:
                if (n <= SMALL_LIMIT)
                    runOne("Bubble Sort", bubbleSort, a, n);
                else
                    printf("Use at most %d elements.\n",
                           SMALL_LIMIT);
                break;

            case 2:
                if (n <= SMALL_LIMIT)
                    runOne("Selection Sort", selectionSort, a, n);
                else
                    printf("Use at most %d elements.\n",
                           SMALL_LIMIT);
                break;

            case 3:
                if (n <= SMALL_LIMIT)
                    runOne("Insertion Sort", insertionSort, a, n);
                else
                    printf("Use at most %d elements.\n",
                           SMALL_LIMIT);
                break;

            case 4:
                runOne("Merge Sort", mergeSort, a, n);
                break;

            case 5:
                runOne("Quick Sort", quickSort, a, n);
                break;

            case 6:
                compareAll(a, n);
                break;

            case 7:
                runOne("Merge Sort", mergeSort, a, n);
                runOne("Quick Sort", quickSort, a, n);
                break;
        }

        free(a);

    } while (1);

    printf("Program terminated.\n");

    return 0;
}
