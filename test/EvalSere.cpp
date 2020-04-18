#include "test/Letter.hpp"
#include "test/EvalBoolExpr.hpp"
#include "test/EvalSere.hpp"

#include "ast/SereExpr.hpp"

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

  void visit(SereBool& v) override {
    calcBool(*v.getExpr());
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

  void visit(Fusion& v) override {
    if (word.empty()) {
      result = Match_Partial;
      return;
    }
    Match r = Match_Failed;
    for (size_t i = 1; i <= word.size(); ++i) {
      Word lhsW(word.begin(), word.begin() + i);
      Word rhsW(word.begin() + i - 1, word.end());

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

  void visit(KleenePlus& v) override {
    result = eval(*RE_CONCAT(v.getArg(), RE_STAR(v.getArg())), word);
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

  /**
   * `Partial` turns any partial result into OK.
   * But this evaluator can yield partial match even for
   * words which have no matched continuation (because of
   * not all states have route to finals).
   * This means that `partial` can make the evaluator
   * almost completely useless as it start to return
   * OK for failing words.
   */
  void visit(Partial& v) override {
    result = eval(*RE_CONCAT(v.getArg(), RE_STAR(v.getArg())), word);
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
};

Match evalSere(SereExpr& expr, const Word& word) {
  return EvalSere::eval(expr, word);
}
