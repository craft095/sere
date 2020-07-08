#include "Typed.hpp"

#include <nlohmann/json.hpp>

namespace compute {
  const String TypedNode::pretty() const {
    std::ostringstream s;
    constexpr int spaces = 4;
    s << json(*this).dump(spaces) << std::endl;
    return s.str();
  }

  void to_json(json& j, const FuncType& ft) {
    j = json {
      {"node",   "FuncType" },
      {"args",   ft.args },
      {"result", ft.result },
    };
  }

  void to_json(json& j, const TypedNode& a) {
    a.to_json(j);
  }

  void to_json(json& j, const Func& a) {
    a.to_json(j);
  }

  void Scalar::to_json(json& j) const {
    j = json {
      {"node",     "Scalar" },
      {"typeIds",  typeIds },
    };
  }

  void Apply::to_json(json& j) const {
    j = json {
      {"node",      "Apply" },
      {"funcTypes", funcTypes },
      {"func",      func },
      {"args",      args },
    };
  }

  void Func::to_json(json& j) const {
    j = json {
      {"node",      "Func" },
      {"funcTypes", funcTypes },
    };
  }

  bool match(const FuncType& ft, const std::vector<TypeIds>& args) {
    if (ft.args.size() != args.size()) {
      return false;
    }
    for (size_t ix = 0; ix < args.size(); ++ix) {
      bool found = false;
      for (auto t : args[ix]) {
        if (t == ft.args[ix]) {
          found = true;
          break;;
        }
      }
      if (!found) {
        return false;
      }
    }
    return true;
  }

  Apply::Apply(Func::Ptr f, TypedNodes& as) : func(f), args(as) {
    std::vector<TypeIds> argsT;
    for (auto node : args) {
      argsT.push_back(node->getTypeIds());
    }

    for (auto& fi : func->getTypes()) {
      if (match(fi, argsT)) {
        funcTypes.push_back(fi);
        typeIds.push_back(fi.result);
      }
    }
  }

} // namespace compute
