#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <vector>
#include <tuple>
#include <list>
#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <utility>
#include <unordered_set>
#include "tree.h"
#include "graph.h"
#include "graphics.h"

template<typename T>
void
hash_combine(std::size_t &seed, T const &key) {
  std::hash<T> hasher;
  seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
  template<typename T1, typename T2>
  struct hash<std::pair<T1, T2>> {
    std::size_t operator()(std::pair<T1, T2> const &p) const {
      std::size_t seed1(0);
      ::hash_combine(seed1, p.first);
      ::hash_combine(seed1, p.second);

      std::size_t seed2(0);
      ::hash_combine(seed2, p.second);
      ::hash_combine(seed2, p.first);

      return std::min(seed1, seed2);
    }
  };

  template<typename T1, typename T2>
  struct equal_to<std::pair<T1, T2>> {
    bool operator()(std::pair<T1, T2> const &lhs, std::pair<T1, T2> const &rhs) const {
      if (lhs.first == rhs.first) {
         if (lhs.second == rhs.second) 
            return true;
      }
      if (lhs.first == rhs.second) {
         if (lhs.second == rhs.first) 
            return true;
      }
      return false;
    }
  };
}
struct Tree;

//graphics related
void parse_test_file           (std::string);
void begin_graphics            (Tree*);
void drawtree                  (void);

#endif

