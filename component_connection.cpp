#include "cell.hpp"
#include "cluster.hpp"
#include "shared_definitions.hpp"
#include <array>

#include <stdio.h>

/// Implemementation of SparseCCL, following
/// [DOI: 10.1109/DASIP48288.2019.9049184]
///
/// Requires cells to be sorted in column major
namespace detail {

/// Find root of the tree for entry @param e
///
/// @param L an equivalance table
///
/// @return the root of @param e
inline unsigned int find_root(
    const unsigned int *L, const unsigned int& e) {
    unsigned int r = e;
    while (L[r] != r) {
        r = L[r];
    }
    return r;
}

/// Create a union of two entries @param e1 and @param e2
///
/// @param L an equivalance table
///
/// @return the rleast common ancestor of the entries
inline unsigned int make_union(unsigned int *L,
                                                  const unsigned int& e1,
                                                  const unsigned int& e2) {
    int e;
    if (e1 < e2) {
        e = e1;
        L[e2] = e;
    } else {
        e = e2;
        L[e1] = e;
    }
    return e;
}

/// Helper method to find adjacent cells
///
/// @param a the first cell
/// @param b the second cell
///
/// @return boolan to indicate 8-cell connectivity
inline bool is_adjacent(cell a, cell b) {
    return (a.channel0 - b.channel0) * (a.channel0 - b.channel0) <= 1 and
           (a.channel1 - b.channel1) * (a.channel1 - b.channel1) <= 1;
}

/// Helper method to find define distance,
/// does not need abs, as channels are sorted in
/// column major
///
/// @param a the first cell
/// @param b the second cell
///
/// @return boolan to indicate !8-cell connectivity
inline bool is_far_enough(cell a, cell b) {
    return (a.channel1 - b.channel1) > 1;
}

/// Sparce CCL algorithm
///
/// @param cells is the cell collection
/// @param L is the vector of the output indices (to which cluster a cell
/// belongs to)
/// @param labels is the number of clusters found
/// @param cluster_sizes stores the number of cells of cluster with index i
template <template <typename> class vector_t, typename cell_t>
inline void sparse_ccl(const vector_t<cell_t>& cells,
                                          unsigned int *L, int size,
                                          unsigned int& labels,
                                          unsigned int *cluster_sizes) {

    // first scan: pixel association
    unsigned int start_j = 0;
    for (unsigned int i = 0; i < size; ++i) {
        L[i] = i;
        int ai = i;
        if (i > 0) {
            for (unsigned int j = start_j; j < i; ++j) {
                if (is_adjacent(cells[i], cells[j])) {
                    ai = make_union(L, ai, find_root(L, j));
                } else if (is_far_enough(cells[i], cells[j])) {
                    ++start_j;
                }
            }
        }
    }
    // second scan: transitive closure
    for (unsigned int i = 0; i < size; ++i) {
        unsigned int l = 0;
        if (L[i] == i) {
            l = labels;
            ++labels;
        } else {
            l = L[L[i]];
        }
        L[i] = l;
        cluster_sizes[l]++;
    }
}
}  // namespace detail

/// Implementation for the public cell collection creation operators
template <template <typename> class vector_type>
void sequential_ccl(const cell_collection<vector_type>& cells,
                const cell_module& module,
                cluster_element*& clusters) {
    // Run the algorithm
    unsigned int num_clusters = 0;
    unsigned int* cluster_sizes = new unsigned int[cells.size()]{}; // initialize values at 0
    unsigned int* connected_cells = new unsigned int[cells.size()];
    
    detail::sparse_ccl<vector_type, cell>(cells, connected_cells, cells.size(),
                                                  num_clusters, cluster_sizes);

    clusters = new cluster_element[num_clusters];
    printf("Num cluster before %d\n", num_clusters);
    for(int i = 0; i < num_clusters; i++){
      // initialize the items arrays and store size information
      clusters[i].items = new cell[cluster_sizes[i]];
      clusters[i].items_size = 0; // use it as index when filling the items array later, will correspond at the end to cluster_sizes[i]
      printf("Init cluster %d\n", i);
      printf("Num cluster from within loop %d\n", num_clusters);
    }
    printf("Num cluster after %d\n", num_clusters);

    for(int i = 0; i < num_clusters; i++){
      printf("Cluster %d: size=%d\n", i, cluster_sizes[i]);
    }

    for(int i = 0; i < cells.size(); i++){
      // get the cluster label info for the current cell
      unsigned int k = connected_cells[i]; 
      // assign add the cell to the cluster, and increase the current index for this cluster
      printf("Cell %d should be in cluster %d; current cluster index=%d\n", i, k, clusters[k].items_size);
      clusters[k].items[clusters[k].items_size++] = cells[i];
    }

    delete[] connected_cells;
    delete[] cluster_sizes;
}
