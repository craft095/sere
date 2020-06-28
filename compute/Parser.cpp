#include "Parser.hpp"
#include "Ast.hpp"

#include "antlr4-runtime.h"
#include "ComputeLexer.h"
#include "ComputeParser.h"
#include "ComputeParserVisitor.h"

#include <iostream>
#include <sstream>

using namespace antlr4;

namespace compute {

  ParseError::ParseError(const Located& loc_, const std::string& desc)
    : loc(loc_) {
    std::ostringstream stream{msg};
    stream << loc.pretty() << ": " << desc;
    msg = stream.str();
  }

  const char* ParseError::what() const throw() {
    return msg.c_str();
  }

  void toPosStart(const FileName& file,
                 const antlr4::Token& token,
                 Pos& pos) {
    pos.fileName = file;
    pos.line = token.getLine();
    pos.column = token.getCharPositionInLine();
  }

  void toPosEnd(const FileName& file,
                const antlr4::Token& token,
                Pos& pos) {
    toPosStart(file, token, pos);
    pos.column += token.getText().size();
  }

  void toLocated(const FileName& file,
                 antlr4::ParserRuleContext& rule,
                 Located& loc) {
    const Token* start = rule.getStart();
    const Token* stop = rule.getStop();

    toPosStart(file, *start, loc.from);
    toPosEnd(file, *stop, loc.to);
  }


  class ErrorListener : public antlr4::BaseErrorListener {
  public:
    ErrorListener(const FileName& file_) : file(file_) {}
    virtual void syntaxError(
                             antlr4::Recognizer * /*recognizer*/,
                             antlr4::Token * /*offendingSymbol*/,
                             size_t line,
                             size_t charPositionInLine,
                             const std::string &msg,
                             std::exception_ptr /*e*/) override {
      Pos pos(file, line, charPositionInLine);
      throw ParseError(Located(pos,pos), msg);
    }
  private:
    FileName file;
  };

  /**
   * Parse AST traversal class
   */
  class ExprCollector : public ComputeParserVisitor {
    Parser& parser;
    Root::Ptr root;
    FileName fileName;
  public:
    ExprCollector(const FileName& file, Parser& p) : fileName(file), parser(p) {}

    Located toLocated(antlr4::ParserRuleContext& rule) const {
      Located loc;
      compute::toLocated(fileName, rule, loc);
      return loc;
    }

    template<typename T, typename Node, typename... Args>
    Ptr<T> create(Node ctx, Args... args) {
      return std::make_shared<T>(toLocated(*ctx), args...);
    }

    antlrcpp::Any visitIdent(ComputeParser::IdentContext *context) override {
      std::string name{context->ID()->getText()};
      Ident::Ptr r { create<Ident>(context, name) };
      return r;
    }

    antlrcpp::Any visitCompute(ComputeParser::ComputeContext *context) override {
      ArgDecls args;
      LetDecls lets;
      Expression::Ptr expr = visit(context->payload);

      for (auto a : context->args) {
        ArgDecl::Ptr arg = visit(a);
        args.push_back(arg);
      }

      for (auto l : context->lets) {
        ArgDecl::Ptr let = visit(l);
        args.push_back(let);
      }

      Root::Ptr r = create<Root>(context, args, lets, expr);

      return r;
    }

    static bool resolveTypeId(const Ident::Name& name, TypeId& r) {
      if (name == "Bool") { r = TypeId::Bool; }
      else if (name == "UInt8") { r = TypeId::UInt8; }
      else if (name == "UInt16") { r = TypeId::UInt16; }
      else if (name == "UInt32") { r = TypeId::UInt32; }
      else if (name == "UInt64") { r = TypeId::UInt64; }
      else if (name == "SInt8") { r = TypeId::SInt8; }
      else if (name == "SInt16") { r = TypeId::SInt16; }
      else if (name == "SInt32") { r = TypeId::SInt32; }
      else if (name == "SInt64") { r = TypeId::SInt64; }
      else if (name == "Float") { r = TypeId::Float; }
      else if (name == "String") { r = TypeId::String; }
      else if (name == "Time") { r = TypeId::Time; }
      else { return false; }
      return true;
    }

    antlrcpp::Any visitArgDeclaration(ComputeParser::ArgDeclarationContext *context) override {
      Ident::Ptr name = visit(context->name);
      Ident::Ptr typeIdent = visit(context->type);

      // Resolve type name into type id right here (not a good idea, should be fixed)
      TypeId typeId;
      bool resolved = resolveTypeId(typeIdent->getName(), typeId);

      if (!resolved) {
        parser.notifyErrorListeners(context->type->ID()->getSymbol(), "unknown type name", nullptr);
        return nullptr;
      }

      ArgDecl::Ptr r { create<ArgDecl>(context, typeId, name) };
      return r;
    }

    antlrcpp::Any visitLetDeclaration(ComputeParser::LetDeclarationContext *context) override {
      return visit(context->assignment());
    }

    antlrcpp::Any visitConstAssign(ComputeParser::ConstAssignContext *context) override {
      Ident::Ptr name { visit(context->name) };
      Expression::Ptr expr { visit(context->body) };
      LetDecl::Ptr r { create<LetDecl>(context, name, Idents{}, expr) };
      return r;
    }

    antlrcpp::Any visitFuncAssign(ComputeParser::FuncAssignContext *context) override {
      Ident::Ptr name { visit(context->name) };
      Idents args;
      for (auto i : context->args) {
        Ident::Ptr arg { visit(i) };
        args.push_back(arg);
      }
      Expression::Ptr expr { visit(context->body) };
      LetDecl::Ptr r { create<LetDecl>(context, name, args, expr) };
      return r;
    }

    antlrcpp::Any visitCallExpression(ComputeParser::CallExpressionContext *context) override {
      Ident::Ptr func { visit(context->func) };
      Expressions args;
      for (auto i : context->args) {
        Expression::Ptr arg { visit(i) };
        args.push_back(arg);
      }
      FuncCall::Ptr r { create<FuncCall>(context, func, args) };
      return r;
    }

    antlrcpp::Any visitNameReference(ComputeParser::NameReferenceContext *context) override {
      Ident::Ptr name { visit(context->ident()) };
      NameRef::Ptr r { create<NameRef>(context, name) };
      return r;
    }

    antlrcpp::Any visitUnary(ComputeParser::UnaryContext *context) override {
      Expression::Ptr arg { visit(context->arg) };

      UnaryOp::OpId opId;

      if (context->COMPLEMENT()) { opId = UnaryOp::OpId::SERE_COMPLEMENT; }
      else if (context->MINUS()) { opId = UnaryOp::OpId::MATH_NEG; }
      else if (context->BOOL_NOT()) { opId = UnaryOp::OpId::BOOL_NOT; }
      else if (context->KLEENESTAR()) { opId = UnaryOp::OpId::SERE_KLEENESTAR; }
      else if (context->KLEENEPLUS()) { opId = UnaryOp::OpId::SERE_KLEENEPLUS; }
      else { assert(false); } // unexpected operation

      Expression::Ptr r { create<UnaryOp>(context, opId, arg) };
      return r;
    }

    antlrcpp::Any visitBinary(ComputeParser::BinaryContext *context) override {
      Expression::Ptr lhs { visit(context->lhs) };
      Expression::Ptr rhs { visit(context->rhs) };

      BinaryOp::OpId opId;

      if (context->DIVIDE()) { opId = BinaryOp::OpId::MATH_DIV; }
      else if (context->STAR()) { opId = BinaryOp::OpId::MATH_MUL; }
      else if (context->MINUS()) { opId = BinaryOp::OpId::MATH_SUB; }
      else if (context->PLUS()) { opId = BinaryOp::OpId::MATH_ADD; }
      else if (context->BOOL_AND()) { opId = BinaryOp::OpId::BOOL_AND; }
      else if (context->BOOL_OR()) { opId = BinaryOp::OpId::BOOL_OR; }
      else if (context->BOOL_EQ()) { opId = BinaryOp::OpId::BOOL_EQ; }
      else if (context->BOOL_NE()) { opId = BinaryOp::OpId::BOOL_NE; }
      else if (context->BOOL_LT()) { opId = BinaryOp::OpId::BOOL_LT; }
      else if (context->BOOL_LE()) { opId = BinaryOp::OpId::BOOL_LE; }
      else if (context->BOOL_GT()) { opId = BinaryOp::OpId::BOOL_GT; }
      else if (context->BOOL_GE()) { opId = BinaryOp::OpId::BOOL_GE; }
      else if (context->INTERSECTION()) { opId = BinaryOp::OpId::SERE_INTERSECT; }
      else if (context->UNION()) { opId = BinaryOp::OpId::SERE_UNION; }
      else if (context->FUSION()) { opId = BinaryOp::OpId::SERE_FUSION; }
      else if (context->CONCAT()) { opId = BinaryOp::OpId::SERE_CONCAT; }
      else { assert(false); } // unexpected operation

      Expression::Ptr r { create<BinaryOp>(context, opId, lhs, rhs) };
      return r;
    }

    antlrcpp::Any visitParenExpression(ComputeParser::ParenExpressionContext *context) override {
      return visit(context->expression());
    }

    antlrcpp::Any visitStringLiteral(ComputeParser::StringLiteralContext *context) override {
      std::string string {context->STRINGLIT()->getText()};
      StringLit::Ptr r { create<StringLit>(context, string) };
      return r;
    }

    antlrcpp::Any visitIntLiteral(ComputeParser::IntLiteralContext *context) override {
      uint64_t value { std::stoull(context->INTLIT()->getText()) };
      IntLit::Ptr r { create<IntLit>(context, value) };
      return r;
    }

    antlrcpp::Any visitFloatLiteral(ComputeParser::FloatLiteralContext *context) override {
      double value { std::stod(context->FLOATLIT()->getText()) };
      FloatLit::Ptr r { create<FloatLit>(context, value) };
      return r;
    }

  };

  /**
   * Parse input stream into complete AST
   *
   * @param [in] file stream source name
   * @param [in] stream input stream
   * @return AST root pointer
   * @throws ParseError if parse/scan errors occur
   */

  Root::Ptr parse(const std::string& file, std::istream& stream) {
    ANTLRInputStream input(stream);
    ComputeLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    ComputeParser parser(&tokens);
    ErrorListener errorListner(file);
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListner);
    ComputeParser::ComputeContext* tree = parser.compute();
    ExprCollector visitor(file, parser);
    Root::Ptr result = visitor.visitCompute(tree);

    return result;
  }

  Root::Ptr parse(std::istream& stream) {
    return parse("<unknown>", stream);
  }
} // namespace compute
