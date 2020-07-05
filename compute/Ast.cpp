#include "Ast.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace compute {
  void to_json(json& j, const Node& a) {
    a.to_json(j);
  }

  const String Node::pretty() const {
    std::ostringstream s;
    constexpr int spaces = 4;
    s << json(*this).dump(spaces) << std::endl;
    return s.str();
  }

  void Ident::to_json(json& j) const {
    j = json {
      {"node",   "Ident" },
      {"name",    name },
    };
  }

  void ArgDecl::to_json(json& j) const {
    j = json {
      {"node",    "ArgDecl" },
      {"name",    name },
      {"typeId",  typeId },
    };
  }

  void NameRef::to_json(json& j) const {
    j = json {
      {"node",    "NameRef" },
      {"name",    name->getName() },
    };
  }

  void FuncCall::to_json(json& j) const {
    j = json {
      {"node",    "FuncCall" },
      {"name",    name->getName() },
      {"args",    args },
    };
  }

  void LetDecl::to_json(json& j) const {
    j = json {
      {"node",    "LetDecl" },
      {"name",    name->getName() },
      {"args",    args },
      {"body",    body },
    };
  }

  void Root::to_json(json& j) const {
    j = json {
      {"node",    "Root" },
      {"args",    argDecls },
      {"lets",    letDecls },
      {"expr",    expression },
    };
  }


} // namespace compute
