#include "Ast.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace compute {
  void to_json(json& j, TypeId t) {
    const char* n = nullptr;
    switch (t) {
    case TypeId::None:
      n = "None";
      break;
    case TypeId::Bool:
      n = "Bool";
      break;
    case TypeId::Sere:
      n = "Sere";
      break;
    case TypeId::UInt8:
      n = "UInt8";
      break;
    case TypeId::UInt16:
      n = "UInt16";
      break;
    case TypeId::UInt32:
      n = "UInt32";
      break;
    case TypeId::UInt64:
      n = "UInt64";
      break;
    case TypeId::SInt8:
      n = "SInt8";
      break;
    case TypeId::SInt16:
      n = "SInt16";
      break;
    case TypeId::SInt32:
      n = "SInt32";
      break;
    case TypeId::SInt64:
      n = "SInt64";
      break;
    case TypeId::Float:
      n = "Float";
      break;
    case TypeId::String:
      n = "String";
      break;
    case TypeId::Time:
      n = "Time";
      break;
    }
    j = n;
  }

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
