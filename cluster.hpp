/** TRACCC library, part of the ACTS project (R&D line)
 *
 * (c) 2021-2022 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once
#include <cassert>
#include <array>

#include "shared_definitions.hpp"

struct cluster_id {
    event_id event = 0;
    std::size_t module_idx = 0;
    geometry_id module = 0;
    scalar threshold = 0.;
    transform3 placement = transform3{};
    pixel_data pixel;
};

namespace n_cluster{
using header_t = cluster_id;
using items_t = vecmem::vector<cell>;
using header_vector = vecmem::vector<header_t>;
using item_vector = vecmem::vector<items_t>;
struct element_view{
  header_t header;
  items_t items;
};
}
struct cluster_element{
  cluster_id header;
  cell* items;
  unsigned int items_size;
};
struct host_cluster_container{
    n_cluster::header_vector m_headers;
    n_cluster::item_vector m_items; 

    /**
     * @brief Bounds-checking mutable element accessor.
     */
    n_cluster::element_view at(int i) const {
        return {this->m_headers.at(i), this->m_items.at(i)};
    }

    /**
     * @brief Return the size of the container.
     *
     * In principle, the size of the two internal vectors should always be
     * equal, but we can assert this at runtime for debug builds.
     */
    int size(void) const {
        assert(this->m_headers.size() == this->m_items.size());
        return this->m_headers.size();
    }
    
    /**
     * @brief Resize space in both vectors.
     */
    void resize(int s) {
        this->m_headers.resize(s);
        this->m_items.resize(s);
    }

    /**
     * @brief Push a header and a vector into the container.
     */
    void push_back(cluster_id&& new_header, vecmem::vector<cell>&& new_items) {
        this->m_headers.push_back(
            std::forward<cluster_id>(new_header));
        this->m_items.push_back(
            std::forward<vecmem::vector<cell>>(
                new_items));
    }

    /**
     * @brief Accessor method for the internal header vector.
     */
    const n_cluster::header_vector& get_headers() const { return m_headers; }

    /**
     * @brief Non-const accessor method for the internal header vector.
     *
     * @warning Do not use this function! It is dangerous, and risks breaking
     * invariants!
     */
    n_cluster::header_vector& get_headers() { return m_headers; }

    /**
     * @brief Accessor method for the internal item vector-of-vectors.
     */
    const n_cluster::item_vector& get_items() const { return m_items; }

    /**
     * @brief Non-const accessor method for the internal item vector-of-vectors.
     *
     * @warning Do not use this function! It is dangerous, and risks breaking
     * invariants!
     */
    n_cluster::item_vector& get_items() { return m_items; }
};
