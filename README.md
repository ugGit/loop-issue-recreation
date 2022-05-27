# nvc++: A variable with a variable value?

Clone the repo:

```
git clone git@github.com:ugGit/loop-issue-recreation.git
```

## Requirements
The issue has been encountered in an environment using the following modules, which must be available during the compilation:

* gcc/11.2.0
* nvhpc/22.5 (using CUDA 11.7)

## Information about the setup
The code has been tested on an Nvidia A6000 and a GeForce RTX 2080.

This project contains the minimal required modules and code to recreate the issue encountered.

## Issue description
The issue has been observed in the context of a connected component labeling algorithm.
A variable keeps track of the number of clusters created within a function, and is then used to initialize a container for each cluster.
Now, the value of this variable changes without being modified by the program.

Since not all containers (arrays) get initialized, we encounter a (correct) `cudaErrorIllegalAddress` error when trying to access an array that has not been initialized.

The next subsections elaborate the code that causes the problem, and suggest a possible cause for the issue.

### Hypothesis
We assume, that the compiler optimizes the variable `num_clusters`, as it seems to stay unmodified within the scope of the function `sequential_ccl(...)`. 
However, the variable is passed as lvalue reference to `sparse_ccl`, and increased several times within that function.
This might be considered a rather weak hypothesis, since the loop still gets executed one time, although `num_clusters` is initialized to `0`.

### Relevant Code
Note that some lines are stripped from the code to be as concise as possible.

From component_connection.cpp, function `void sequential_ccl(...)`, [lines 119-134](https://github.com/ugGit/loop-issue-recreation/blob/main/component_connection.cpp#L119-L134):
```
// Run the algorithm
unsigned int num_clusters = 0;
unsigned int* cluster_sizes = new unsigned int[cells.size()]{}; // initialize values at 0
unsigned int* connected_cells = new unsigned int[cells.size()];

detail::sparse_ccl<vector_type, cell>(cells, connected_cells, cells.size(),
                                      num_clusters, cluster_sizes);

clusters = new cluster_element[num_clusters];
printf("Num cluster %d\n", num_clusters);
for(int i = 0; i < num_clusters; i++){
  // initialize the items arrays and store size information
  clusters[i].items = new cell[cluster_sizes[i]];
  clusters[i].items_size = 0; // use it as index when filling the items array later, will correspond at the end to cluster_sizes[i]
  printf("Init cluster %d\n", i);
  printf("Num cluster from within loop %d\n", num_clusters);
}
```

From component_connection.cpp, function `detail::sparse_ccl`:
```
template <template <typename> class vector_t, typename cell_t>
inline void sparse_ccl(const vector_t<cell_t>& cells,
                       unsigned int *L, 
                       int size,
                       unsigned int& labels,
                       unsigned int *cluster_sizes) {
  ...
  ++labels;
  ...
}
```

## Execute the code
```
nvc++ -stdpar=gpu -o main main.cpp && ./main
```

Resulting output:
```
Num cluster 3
Init cluster 0
Num cluster from within loop 0
Cluster 0: size=2
Cluster 1: size=1
Cluster 2: size=6
Cell 0 should be in cluster 0; current cluster index=0
Cell 1 should be in cluster 0; current cluster index=1
Cell 2 should be in cluster 1; current cluster index=0
terminate called after throwing an instance of 'thrust::system::system_error'
  what():  for_each: failed to synchronize: cudaErrorIllegalAddress: an illegal memory access was encountered
Aborted (core dumped)
```


## Observations
The following list resumes the changes made that solve the issue:
* Changing the execution policy to `std::execution::seq` in main.cpp (removing the policy works as well).
* Using nvhpc 22.3 for compilation instead of 22.5.
* Applying the flags `-g`, `-O0`, `-O1`, `-O2`, `-O3` during compilation.

And here things that did not help:
* Removing the `inline` tag from function `sparce_ccl(...)`.
* Appending `-gpu=managed:intercept` during compilation.
