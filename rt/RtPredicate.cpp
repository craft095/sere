#include "RtPredicate.hpp"

namespace rt {
  class FromBoolExpr {
  private:
    std::vector<uint8_t> data;
  public:
    FromBoolExpr(expr::Expr expr) {
      // writeValue(rt::magic);
      write(expr);
    }

    template <typename T>
    void writeValue(T t) {
      uint8_t* eip = reinterpret_cast<uint8_t*>(&t);
      data.insert(data.end(), eip, eip + sizeof(t));
    }

    const std::vector<uint8_t>& getData() const { return data; }

    void write(expr::Expr expr) {
      bool tf;
      uint32_t v;
      expr::Expr lhs, rhs;

      if (expr.get_value(tf)) {
        writeValue(tf ? rt::Code::True : rt::Code::False);
        return;
      }
      if (expr.get_var(v)) {
        writeValue(rt::Code::Name);
        writeValue(static_cast<rt::Offset>(v));
        return;
      }
      if (expr.not_arg(lhs)) {
        writeValue(rt::Code::Not);
        write(lhs);
        return;
      }
      if (expr.and_args(lhs, rhs)) {
        writeValue(rt::Code::And);
        write(lhs);
        write(rhs);
        return;
      }
      if (expr.or_args(lhs, rhs)) {
        writeValue(rt::Code::Or);
        write(lhs);
        write(rhs);
        return;
      }
      assert(false); // unreachable code
    }
  };

  void toRtPredicate(expr::Expr expr,
                     std::vector<uint8_t>& data) {
    FromBoolExpr be{expr};
    data = be.getData();
  }

  void toRtPredicate(BoolExpr& expr,
                     std::vector<uint8_t>& data) {
    toRtPredicate(boolExprToExpr(expr), data);
  }

}
