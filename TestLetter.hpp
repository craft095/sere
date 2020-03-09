#ifndef TESTLETTER_HPP
#define TESTLETTER_HPP

#include "Letter.hpp"
#include "TestTools.hpp"

class LetterGenerator : public Catch2::IGenerator <Letter> {
private:
  size_t atoms;
  Letter value;
public:
  LetterGenerator(size_t atoms_) : atoms(atoms_), value("","") {
    next();
  }
  Letter const& get() const override {
    return value;
  }

  bool next() override {
    std::string p, n;

    for (size_t i = 0; i < atoms; ++i) {
      size_t c = Catch2::random(0, 1).get();
      if (c == 1) {
        p.push_back(char('a' + i));
      } else {
        n.push_back(char('a' + i));
      }
    }

    value = Letter(p,n);
    return true;
  }
};

Catch2::GeneratorWrapper<Letter> genLetter(size_t atoms) {
  return Catch2::GeneratorWrapper<Letter>(std::make_unique<LetterGenerator>(atoms));
}

#endif // TESTLETTER_HPP
