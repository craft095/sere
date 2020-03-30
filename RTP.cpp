#include "RTP.hpp"

namespace rtp {
  class FromBoolExpr : public BoolVisitor {
  private:
    std::vector<uint8_t> data;
  public:
    FromBoolExpr(BoolExpr& expr) {
      writeValue(rtp::magic);
      expr.accept(*this);
    }

    template <typename T>
    void writeValue(T t) {
      uint8_t* eip = reinterpret_cast<uint8_t*>(&t);
      data.insert(data.end(), eip, eip + sizeof(t));
    }

    const std::vector<uint8_t>& getData() const { return data; }

    void visit(Variable& v) override {
      writeValue(rtp::Code::Name);
      writeValue(static_cast<rtp::Offset>(v.getName().ix));
    }

    void visit(BoolValue& v) override {
      if (v.getValue()) {
        writeValue(rtp::Code::True);
      } else {
        writeValue(rtp::Code::False);
      }
    }

    void visit(BoolNot& v) override {
      writeValue(rtp::Code::Not);
      v.getArg()->accept(*this);
    }

    void visit(BoolAnd& v) override {
      writeValue(rtp::Code::And);
      v.getLhs()->accept(*this);
      v.getRhs()->accept(*this);
    }
  };

  void toRTP(BoolExpr& expr,
             std::vector<uint8_t>& data) {
    FromBoolExpr be{expr};
    data = be.getData();
  }

}
