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
    typeIds = set_intersects(typeIds, typs);
  }

  void Scalar::to_json(json& j) const {
    j = json {
      {"node",     "Scalar" },
      {"typeIds",  typeIds },
    };
  }

  void Apply::constrain(const TypeIds& typs) {
    TypeIds ts;
    FuncTypes fts;
    std::vector<TypeIds> as{args.size()};
    for (auto const& funcType : funcTypes) {
      if (set_member(typs, funcType.result)) {
        fts.insert(funcType);
        ts.insert(funcType.result);

        for (size_t ix = 0; ix < as.size(); ++ix) {
          as[ix].insert(funcType.args[ix]);
        }
      }
    }
    func->constrain(fts);
    for (size_t ix = 0; ix < as.size(); ++ix) {
      args[ix]->constrain(as[ix]);
    }
    std::swap(funcTypes, fts);
    std::swap(typeIds, ts);
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
    funcTypes = set_intersects(funcTypes, typs);
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
    constrain(typeIds);
  }

} // namespace compute
