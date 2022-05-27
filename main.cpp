
#include "shared_definitions.hpp"
#include "cell.hpp"
#include "cluster.hpp"

#include "component_connection.cpp"

#include "CountingIterator.h"

#include <tuple>
#include <stdio.h>
#include <algorithm>
#include <execution>

#include <array>


int main(){
  /*
   * Format the input data (only works for a single detector module,
   * otherwise not generic enough).
   */
  // fields: geometry_id,hit_id,channel0,channel1,timestamp,value
  using row_type = std::tuple<int, int, int, int, int, double>;  
  std::vector<row_type> raw_input{
    row_type{0,0,23,34,0,0.499423710761263},
    row_type{0,0,24,34,0,0.9664007758177955},
    row_type{0,1,35,39,0,0.2248546897281531},
    row_type{0,1,38,38,0,0.3300625315595242},
    row_type{0,1,36,39,0,0.7873422712754605},
    row_type{0,1,37,38,0,0.5652962370516108},
    row_type{0,1,38,37,0,0.5883027614619736},
    row_type{0,1,38,39,0,0.7959729342742885},
    row_type{0,2,24,37,0,0.20687748856071822}
  };
  
  // transform the raw input into cell objects
  cell_collection<vecmem::vector> cells;
  cells.reserve(raw_input.size());
  for(int i=0; i<raw_input.size(); i++){
    cell c {};
    c.channel0 = std::get<2>(raw_input[i]);
    c.channel1 = std::get<3>(raw_input[i]);
    c.time = std::get<4>(raw_input[i]);
    c.activation = std::get<5>(raw_input[i]);
    cells.push_back(c);
  }

  // sort the vector based on channel1, if equal by channel0
  std::sort(cells.begin(), cells.end(), [](const cell& a, const cell& b){
    return a.channel1 != b.channel1 ? a.channel1 < b.channel1 : a.channel0 < b.channel0;
  });  

  // prepare the additional function input data
  host_cell_container data;
  data.push_back(cell_module{0}, cells);
  cell_module* data_header_array = data.get_headers().data();
  vecmem::vector<cell>* data_items_array = data.get_items().data();

  /*
   * Execute the CCA algorithm
   */
  std::for_each_n(std::execution::par, counting_iterator(0), data.size(), [=](unsigned int i){
    // prepare container to store results
    cluster_element* cluster_container; // pointer to array, init in sequential_ccl

    auto module = data_header_array[i];

    // The algorithmic code part: start
    sequential_ccl<vecmem::vector>(data_items_array[i], module, cluster_container);
  });

  return 0;
}
