/** TRACCC library, part of the ACTS project (R&D line)
 *
 * (c) 2021-2022 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once
#include <cassert>

#include "shared_definitions.hpp"

// System include(s).
#include <limits>

/// @name Types to use in algorithmic code
/// @{

/// A cell definition:
///
/// maximum two channel identifiers
/// and one activiation value, such as a time stamp
struct cell {
    channel_id channel0 = 0;
    channel_id channel1 = 0;
    scalar activation = 0.;
    scalar time = 0.;
};

inline bool operator<(const cell& lhs, const cell& rhs) {
    if (lhs.channel0 < rhs.channel0) {
        return true;
    } else if (lhs.channel0 == rhs.channel0) {
        if (lhs.channel1 < rhs.channel1) {
            return true;
        } else if (lhs.channel1 == rhs.channel1) {
            if (lhs.activation < rhs.activation) {
                return true;
            }
            return false;
        }
        return false;
    }
    return false;
}

inline bool operator==(const cell& lhs, const cell& rhs) {
    if (lhs.channel0 == rhs.channel0 && lhs.channel1 == rhs.channel1 &&
        lhs.activation == rhs.activation && lhs.time == rhs.time) {
        return true;
    }
    return false;
}

/// Container of cells belonging to one detector module
template <template <typename> class vector_t>
using cell_collection = vector_t<cell>;

/// Convenience declaration for the cell collection type to use in host code
using host_cell_collection = cell_collection<vecmem::vector>;

/// Container of cells belonging to one detector module (const)
template <template <typename> class vector_t>
using cell_const_collection = vector_t<const cell>;

/// Convenience declaration for the cell collection type to use in host code
/// (const)
using host_cell_const_collection = cell_const_collection<vecmem::vector>;

/// Header information for all of the cells in a specific detector module
///
/// It is handled separately from the list of all of the cells belonging to
/// the detector module, to be able to lay out the data in memory in a way
/// that is more friendly towards accelerators.
///
struct cell_module {

    event_id event = 0;
    geometry_id module = 0;
    transform3 placement = transform3{};

    channel_id range0[2] = {std::numeric_limits<channel_id>::max(), 0};
    channel_id range1[2] = {std::numeric_limits<channel_id>::max(), 0};

    pixel_data pixel{-8.425, -36.025, 0.05, 0.05};
};  // struct cell_module

/// Container of cells belonging to one detector module
template <template <typename> class vector_t>
using cell_collection = vector_t<cell>;

/// Container of cell modules
template <template <typename> class vector_t>
using cell_module_collection = vector_t<cell_module>;

/// Convenience declaration for the cell module collection type to use in host
/// code
using host_cell_module_collection = cell_module_collection<vecmem::vector>;

namespace n_cell{
using header_t = cell_module;
using items_t = vecmem::vector<cell>;
using header_vector = vecmem::vector<header_t>;
using item_vector = vecmem::vector<items_t>;
struct element_view{
  header_t header;
  items_t items;
};
}

struct host_cell_container{
    n_cell::header_vector m_headers;
    n_cell::item_vector m_items; 

    /**
     * @brief Bounds-checking mutable element accessor.
     */
    n_cell::element_view at(int i) const {
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
    void push_back(cell_module&& new_header, vecmem::vector<cell>& new_items) {
        this->m_headers.push_back(new_header);
        this->m_items.push_back(new_items);
    }

    /**
     * @brief Accessor method for the internal header vector.
     */
    const n_cell::header_vector& get_headers() const { return m_headers; }

    /**
     * @brief Non-const accessor method for the internal header vector.
     *
     * @warning Do not use this function! It is dangerous, and risks breaking
     * invariants!
     */
    n_cell::header_vector& get_headers() { return m_headers; }

    /**
     * @brief Accessor method for the internal item vector-of-vectors.
     */
    const n_cell::item_vector& get_items() const { return m_items; }

    /**
     * @brief Non-const accessor method for the internal item vector-of-vectors.
     *
     * @warning Do not use this function! It is dangerous, and risks breaking
     * invariants!
     */
    n_cell::item_vector& get_items() { return m_items; }
};
