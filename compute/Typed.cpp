#include "Typed.hpp"
#include "Algo.hpp"

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

  void Scalar::constrain(const TypeIds& typs) {
    //std::remove_if(typeIds.begin(), typeIds.end()
  }

  void Scalar::to_json(json& j) const {
    j = json {
      {"node",     "Scalar" },
      {"typeIds",  typeIds },
    };
  }

  void Apply::constrain(const TypeIds& typs) {
  }

  void Apply::to_json(json& j) const {
    j = json {
      {"node",      "Apply" },
      {"funcTypes", funcTypes },
      {"func",      func },
      {"args",      args },
    };
  }

  void Func::constrain(const FuncTypes& typs) {
  }

  void Func::to_json(json& j) const {
    j = json {
      {"node",      "Func" },
      {"funcTypes", funcTypes },
    };
  }

  bool matchArgs(const FuncType& ft, const std::vector<TypeIds>& args) {
    if (ft.args.size() != args.size()) {
      return false;
    }

    auto u = args.begin();
    auto v = ft.args.begin();

    for (; u != args.end(); ++u, ++v) {
      if (!set_member(*u, *v)) {
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
      if (matchArgs(fi, argsT)) {
        funcTypes.insert(fi);
        typeIds.insert(fi.result);
      }
    }
  }

} // namespace compute
