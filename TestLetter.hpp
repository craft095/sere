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
    value = make(atoms);
    return true;
  }

  static Letter make(size_t atoms) {
    std::string p, n;

    for (size_t i = 0; i < atoms; ++i) {
      size_t c = Catch2::random(0, 1).get();
      if (c == 1) {
        p.push_back(char('a' + i));
      } else {
        n.push_back(char('a' + i));
      }
    }

    // return Letter(std::move(p), std::move(n));
    return Letter(p, n);
  }
};

Catch2::GeneratorWrapper<Letter> genLetter(size_t atoms) {
  return Catch2::GeneratorWrapper<Letter>(std::make_unique<LetterGenerator>(atoms));
}

class WordGenerator : public Catch2::IGenerator <Word> {
private:
  size_t atoms;
  size_t mn;
  size_t mx;
  Word value;
public:
  WordGenerator(size_t atoms_, size_t mn_, size_t mx_)
    : atoms(atoms_), mn(mn_), mx(mx_) {
    next();
  }

  Word const& get() const override {
    return value;
  }

  bool next() override {
    value = make(atoms, mn, mx);
    return true;
  }

  static Word make(size_t atoms, size_t mn, size_t mx) {
    auto g = [atoms] () { return LetterGenerator::make(atoms); };
    return vector_of<Letter>(mn, mx, g);
  }
};

Catch2::GeneratorWrapper<Word> genWord(size_t atoms, size_t mn, size_t mx) {
  return Catch2::GeneratorWrapper<Word>(std::make_unique<WordGenerator>(atoms, mn, mx));
}

#endif // TESTLETTER_HPP
