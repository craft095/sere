#include "rt/RtPredicate.hpp"
#include "boolean/Expr.hpp"

namespace boolean {

  Context Expr::context;

  /**
   * Serialize expression into runtime representation
   */
  class FromExpr {
  public:
    FromExpr(Expr expr) {
      // writeValue(rt::magic);
      write(expr);
    }

    const std::vector<uint8_t>& getData() const { return data; }

  private:
    std::vector<uint8_t> data;

    template <typename T>
    void writeValue(T t) {
      uint8_t* eip = reinterpret_cast<uint8_t*>(&t);
      data.insert(data.end(), eip, eip + sizeof(t));
    }

    void write(Expr expr) {
      bool tf;
      uint32_t v;
      Expr lhs, rhs;

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

  void toRtPredicate(Expr expr,
                     std::vector<uint8_t>& data) {
    FromExpr be{expr};
    data = be.getData();
  }

} // namespace boolean
