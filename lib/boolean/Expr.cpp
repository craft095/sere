#include "rt/RtPredicate.hpp"
#include "boolean/Expr.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace boolean {

  Context Expr::context;

  void from_json(const json& j, Expr& e) {
    std::string kind;
    j.at("kind").get_to(kind);
    if (kind == "const") {
      bool v;
      j.at("value").get_to(v);
      e = Expr::value(v);
      return;
    }
    if (kind == "var") {
      uint32_t v;
      j.at("variable").get_to(v);
      e = Expr::var(v);
      return;
    }
    if (kind == "not") {
      Expr arg;
      j.at("arg").get_to(arg);
      e = !arg;
      return;
    }
    if (kind == "and") {
      Expr arg0, arg1;
      j.at("arg0").get_to(arg0);
      j.at("arg1").get_to(arg1);
      e = arg0 && arg1;
      return;
    }
    if (kind == "or") {
      Expr arg0, arg1;
      j.at("arg0").get_to(arg0);
      j.at("arg1").get_to(arg1);
      e = arg0 || arg1;
      return;
    }
    assert(false); // unreachable code
  }

  void to_json(json& j, const Expr& e) {
    bool v;
    uint32_t var;
    Expr arg0, arg1;

    if (e.get_value(v)) {
      j = json {
                { "kind", "const" },
                { "value", v } };
      return;
    }
    if (e.get_var(var)) {
      j = json {
                { "kind", "var" },
                { "variable", var } };
      return;
    }
    if (e.not_arg(arg0)) {
      j = json {
                { "kind", "not" },
                { "arg", arg0 } };
      return;
    }
    if (e.and_args(arg0, arg1)) {
      j = json {
                { "kind", "and" },
                { "arg0", arg0 },
                { "arg1", arg1 } };
      return;
    }
    if (e.or_args(arg0, arg1)) {
      j = json {
                { "kind", "or" },
                { "arg0", arg0 },
                { "arg1", arg1 } };
      return;
    }
    assert(false); // unreachable code
  }

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
