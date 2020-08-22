#include "Typed.hpp"
#include "Error.hpp"
#include "Algo.hpp"

#include <nlohmann/json.hpp>

namespace compute {
  TypeIds anyType {
                   TypeId::UInt8,
                   TypeId::UInt16,
                   TypeId::UInt32,
                   TypeId::UInt64,
                   TypeId::SInt8,
                   TypeId::SInt16,
                   TypeId::SInt32,
                   TypeId::SInt64,
                   TypeId::Float,
                   TypeId::Time,
                   TypeId::Bool,
                   TypeId::String,
  };

  const String TypedNode::pretty() const {
    return compute::pretty(*this);
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
    TypeIds ids {set_intersects(typeIds, typs)};
    if (ids.empty()) {
      throw ScalarTypeMismatch(getNode()->getLoc(), typeIds, typs);
    }
    std::swap(typeIds, ids);
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

    if (fts.empty()) {
      throw ScalarTypeMismatch(getNode()->getLoc(), typeIds, typs);
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
    FuncTypes fts {set_intersects(funcTypes, typs)};
    if (fts.empty()) {
      throw FuncTypeMismatch(getNode()->getLoc(), funcTypes, typs);
    }
    std::swap(funcTypes, fts);
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

  Apply::Apply(const Expression* node, Func::Ptr f, TypedNodes& as)
    : TypedNode(node), func(f), args(as) {
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

    if (typeIds.empty()) {
      throw BadApplication(getNode()->getLoc(), f, as);
    }

    constrain(typeIds);
  }

} // namespace compute
