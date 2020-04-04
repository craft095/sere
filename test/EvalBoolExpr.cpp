#include "Letter.hpp"
#include "Language.hpp"
#include "EvalBoolExpr.hpp"

#include "Z3.hpp"
#include "ToolsZ3.hpp"

class EvalBool : public BoolVisitor {
private:
  rt::Names letter;
  bool result;
public:
  EvalBool(BoolExpr& expr, const rt::Names& letter_) : letter(letter_) {
    expr.accept(*this);
  }

  bool getResult() const { return result; }

  void visit(Variable& v) override {
    result = letter.test(v.getName().ix);
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

bool evalBool(BoolExpr& expr, const rt::Names& letter) {
  return EvalBool(expr, letter).getResult();
}

bool evalBoolZ3(BoolExpr& expr, const rt::Names& letter) {
  z3::expr l = letterToZex(letter);
  z3::expr b = boolSereToZex(expr);

  bool r = evalWithImply(l,b);
  return r;
}
