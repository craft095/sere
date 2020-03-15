#include "Language.hpp"

class GetAtomics : public BoolVisitor {
private:
  std::set<VarName> vars;
public:
  GetAtomics(BoolExpr& expr) {
    expr.accept(*this);
  }

  const std::set<VarName>&  getResult() const { return vars; }

  void visit(Variable& v) override {
    vars.insert(v.getName());
  }

  void visit(BoolValue&) override {
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }
};

std::set<VarName> boolExprGetAtomics(BoolExpr& expr) {
  GetAtomics ga{expr};
  return ga.getResult();
}
