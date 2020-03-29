#include "RTP.hpp"

namespace rtp {
  class FromBoolExpr : public BoolVisitor {
  private:
    std::map<VarName, rtp::Offset> remap;
    std::vector<uint8_t> data;
  public:
    FromBoolExpr(BoolExpr& expr) {
      std::set<VarName> vars = boolExprGetAtomics(expr);
      rtp::Offset c = 0;
      for (auto const& v : vars) {
        remap[v] = c++;
      }
      expr.accept(*this);
      writeValue(rtp::magic);
    }

    template <typename T>
    void writeValue(T t) {
      uint8_t* eip = reinterpret_cast<uint8_t*>(&t);
      data.insert(data.end(), eip, eip + sizeof(t));
    }

    const std::vector<uint8_t>& getData() const { return data; }
    const std::map<VarName, rtp::Offset>& getRemap() const { return remap; }

    void visit(Variable& v) override {
      writeValue(rtp::Code::Name);
      writeValue(remap[v.getName()]);
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
             std::vector<uint8_t>& data,
             std::map<VarName, rtp::Offset>& remap) {
    FromBoolExpr be{expr};
    data = be.getData();
    remap = be.getRemap();
  }

}
