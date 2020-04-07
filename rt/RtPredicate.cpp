#include "RtPredicate.hpp"

namespace rt {
  class FromBoolExpr : public BoolVisitor {
  private:
    std::vector<uint8_t> data;
  public:
    FromBoolExpr(BoolExpr& expr) {
      // writeValue(rt::magic);
      expr.accept(*this);
    }

    template <typename T>
    void writeValue(T t) {
      uint8_t* eip = reinterpret_cast<uint8_t*>(&t);
      data.insert(data.end(), eip, eip + sizeof(t));
    }

    const std::vector<uint8_t>& getData() const { return data; }

    void visit(Variable& v) override {
      writeValue(rt::Code::Name);
      writeValue(static_cast<rt::Offset>(v.getName().ix));
    }

    void visit(BoolValue& v) override {
      if (v.getValue()) {
        writeValue(rt::Code::True);
      } else {
        writeValue(rt::Code::False);
      }
    }

    void visit(BoolNot& v) override {
      writeValue(rt::Code::Not);
      v.getArg()->accept(*this);
    }

    void visit(BoolAnd& v) override {
      writeValue(rt::Code::And);
      v.getLhs()->accept(*this);
      v.getRhs()->accept(*this);
    }

    void visit(BoolOr& v) override {
      writeValue(rt::Code::Or);
      v.getLhs()->accept(*this);
      v.getRhs()->accept(*this);
    }
  };

  void toRtPredicate(BoolExpr& expr,
                     std::vector<uint8_t>& data) {
    FromBoolExpr be{expr};
    data = be.getData();
  }

}
