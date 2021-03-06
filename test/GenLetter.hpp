#ifndef GENLETTER_HPP
#define GENLETTER_HPP

#include "rt/RtPredicate.hpp"
#include "test/Tools.hpp"

class LetterGenerator : public Catch2::IGenerator <rt::Names> {
private:
  rt::Names names;
public:
  LetterGenerator(size_t atoms) {
    names.resize(atoms);
  }
  const rt::Names& get() const override {
    return names;
  }

  bool next() override {
    make(names.size(), names);
    return true;
  }

  static void make(size_t atoms, rt::Names& names) {
    names.resize(atoms);

    for (size_t k = 0; k < names.size(); ++k) {
      size_t c = Catch2::random(0, 1).get();
      names.set(k, c);
    }
  }
};

inline Catch2::GeneratorWrapper<Letter> genLetter(size_t atoms) {
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
    auto g = [atoms] () { rt::Names names; LetterGenerator::make(atoms, names); return names; };
    return vector_of<rt::Names>(mn, mx, g);
  }
};

inline Catch2::GeneratorWrapper<Word> genWord(size_t atoms, size_t mn, size_t mx) {
  return Catch2::GeneratorWrapper<Word>(std::make_unique<WordGenerator>(atoms, mn, mx));
}

#endif // GENLETTER_HPP
