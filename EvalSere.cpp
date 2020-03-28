#include "Letter.hpp"
#include "Language.hpp"
#include "EvalSere.hpp"

#include "Z3.hpp"

class EvalBool : public BoolVisitor {
private:
  Letter letter;
  bool result;
public:
  EvalBool(BoolExpr& expr, Letter letter_) : letter(letter_) {
    expr.accept(*this);
  }

  bool getResult() const { return result; }

  void visit(Variable& v) override {
    if (letter.pos.find(v.getName()) != letter.pos.end()) {
      result = true;
    } else if (letter.neg.find(v.getName()) != letter.neg.end()) {
      result = false;
    } else {
      assert(false);
    }
  }

  void visit(BoolValue& v) override {
    result = v.getValue();
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    result = !result;
  }

  void visit(BoolAnd& v) override {
    bool lhs, rhs;

    v.getLhs()->accept(*this);
    lhs = result;

    v.getRhs()->accept(*this);
    rhs = result;

    result = lhs && rhs;
  }
};

class EvalSere : public SereVisitor {
private:
  Word word;
  Match result;
public:
  EvalSere(SereExpr& expr, const Word& word_) : word(word_) {
    expr.accept(*this);
  }

  Match getResult() const {
    return result;
  }

  void visit(Variable& v) override {
    calcBool(v);
  }

  void visit(BoolValue& v) override {
    calcBool(v);
  }

  void visit(BoolNot& v) override {
    calcBool(v);
  }

  void visit(BoolAnd& v) override {
    calcBool(v);
  }

  void visit(SereEmpty& ) override {
    if (word.empty()) {
      result = Match_Ok;
    } else {
      result = Match_Failed;
    }
  }

  void visit(Union& v) override {
    Match lhs = eval(*v.getLhs(), word);
    Match rhs = eval(*v.getRhs(), word);

    if (lhs == Match_Ok || rhs == Match_Ok) {
      result = Match_Ok;
    } else if (lhs == Match_Partial || rhs == Match_Partial) {
      result = Match_Partial;
    } else {
      assert(lhs == Match_Failed && rhs == Match_Failed);
      result = Match_Failed;
    }
  }

  void visit(Intersect& v) override {
    Match lhs = eval(*v.getLhs(), word);
    Match rhs = eval(*v.getRhs(), word);

    if (lhs == Match_Ok && rhs == Match_Ok) {
      result = Match_Ok;
    } else if (lhs != Match_Failed && rhs != Match_Failed) {
      result = Match_Partial;
    } else {
      assert(lhs == Match_Failed || rhs == Match_Failed);
      result = Match_Failed;
    }
  }

  void visit(Concat& v) override {
    Match r = Match_Failed;
    for (size_t i = 0; i <= word.size(); ++i) {
      Word lhsW(word.begin(), word.begin() + i);
      Word rhsW(word.begin() + i, word.end());

      Match lhsR0 = eval(*v.getLhs(), lhsW);
      Match rhsR0 = eval(*v.getRhs(), rhsW);

      if (lhsR0 == Match_Ok && rhsR0 == Match_Ok) {
        r = Match_Ok;
        break;
      }

      if (lhsR0 == Match_Ok && rhsR0 == Match_Partial) {
        r = Match_Partial;
      }

      if (lhsR0 == Match_Partial && rhsW.size() == 0) {
        r = Match_Partial;
      }
    }

    result = r;
  }

  void visit(KleeneStar& v) override {
    if (word.size() == 0) {
      result = Match_Ok;
      return;
    }

    // word is not empty, so it is safe to start with 1
    Match r = Match_Failed;
    for (size_t i = 1; i <= word.size(); ++i) {
      Word w1(word.begin(), word.begin() + i);
      Word w2(word.begin() + i, word.end());

      Match w1R = eval(*v.getArg(), w1);
      Match w2R = eval(v, w2);

      if (w1R == Match_Ok && w2R == Match_Ok) {
        r = Match_Ok;
        break;
      }

      if (w1R == Match_Ok && w2R == Match_Partial) {
        r = Match_Partial;
      }

      if (w1R == Match_Partial && w2.size() == 0) {
        r = Match_Partial;
      }
    }

    result = r;
  }

  static Match eval(SereExpr& expr, const Word& word) {
    return EvalSere(expr, word).getResult();
  }

private:
  void calcBool(BoolExpr& boolExpr) {
    switch (word.size()) {
    case 0:
      result = Match_Partial;
      break;
    case 1:
      if (evalBool(boolExpr, word[0])) {
        result = Match_Ok;
      } else {
        result = Match_Failed;
      }
      break;
    default:
      result = Match_Failed;
    }
  }

  void calcBool0(BoolExpr& boolExpr) {
    switch (word.size()) {
    case 0:
      result = Match_Partial;
      break;
    case 1:
      {
        if (evalBoolZ3(boolExpr, word[0])) {
          result = Match_Ok;
        } else {
          result = Match_Failed;
        }
      }
      break;
    default:
      result = Match_Failed;
    }
  }

};

Match evalSere(SereExpr& expr, const Word& word) {
  return EvalSere::eval(expr, word);
}

bool evalBool(BoolExpr& expr, const Letter& letter) {
  return EvalBool(expr, letter).getResult();
}

bool evalBoolZ3(BoolExpr& expr, const Letter& letter) {
  z3::expr l = letterToZex(letter);
  z3::expr b = boolSereToZex(expr);

  bool r = evalWithImply(l,b);
  return r;
}
