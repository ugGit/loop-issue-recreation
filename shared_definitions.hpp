#pragma once

#include <vector>
#include <algorithm>
#include <initializer_list>
#include <array>

namespace vecmem{
  template <typename T>
  using polymorphic_allocator = std::pmr::polymorphic_allocator<T>;
  
  // alternative is just aliasing std::vector without pmr
  template <typename T>
  // using vector = std::vector<T, polymorphic_allocator<T>>;
  using vector = std::vector<T>;
}

using event_id = int;
using geometry_id = int;
using channel_id = int;
using scalar = double;
using transform3 = int; // random placeholder

using point2 = std::array<scalar, 2>;
using variance2 = std::array<scalar, 2>;
using vector2 = std::array<scalar, 2>;

variance2 operator+(const variance2 v, const point2 p){
  return {v[0]+p[0], v[1]+p[1]};
}
point2 operator-(const point2 a, const point2 b){
  return {a[0]-b[0], a[1]-b[1]};
}
point2 operator*(double scalar, const point2 a){
  return {a[0]*scalar, a[1]*scalar};
}

/// A very basic pixel segmentation with
/// a minimum corner and ptich x/y
///
/// No checking on out of bounds done
struct pixel_data {

    scalar min_center_x = 0.;
    scalar min_center_y = 0.;
    scalar pitch_x = 1.;
    scalar pitch_y = 1.;

    vector2 get_pitch() const { return {pitch_x, pitch_y}; };
};
