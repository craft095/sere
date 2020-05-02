#ifndef RTSAVER_HPP
#define RTSAVER_HPP

#include "rt/Format.hpp"

#include <vector>

namespace rt {

  class Saver {
  private:
    std::vector<uint8_t>& data;

  protected:
    void writeData(const uint8_t* d, size_t len) {
      data.insert(data.end(), d, d + len);
    }

    template <typename T>
    void writeValue(const T& t) {
      writeData((const uint8_t*)(&t), sizeof(t));
    }

    void savePredicate(const Predicate& phi) {
      writeValue(phi.size());
      writeData(&phi[0], sizeof(phi[0])*phi.size());
    }

    Saver(std::vector<uint8_t>& data_) : data(data_) {}
  };

} //namespace rt

#endif // RTSAVER_HPP
