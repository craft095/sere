#include <iostream>
#include <sstream>
#include "antlr4-runtime.h"
#include "SereLexer.h"
#include "SereParser.h"
#include "SereBaseVisitor.h"

#include "Language.hpp"
#include "Parser.hpp"

using namespace antlr4;

namespace parser {

  class SereErrorListener : public antlr4::BaseErrorListener {
    virtual void syntaxError(
                             antlr4::Recognizer * /*recognizer*/,
                             antlr4::Token * /*offendingSymbol*/,
                             size_t line,
                             size_t charPositionInLine,
                             const std::string &msg,
                             std::exception_ptr /*e*/) override {
      std::ostringstream s;
      s << "[" << line << ":" << charPositionInLine << "]: " << msg;
      throw std::invalid_argument(s.str());
    }
  };

  /**
   * Parse AST traversal class
   */
  class ExprCollector : public SereBaseVisitor {
    std::map<std::string, size_t> vars;
  public:
    virtual antlrcpp::Any visitSere(SereParser::SereContext *ctx) override {
      return visit(ctx->sereExpr());
    }

    virtual antlrcpp::Any visitSereBoolExpr(SereParser::SereBoolExprContext *ctx) override {
      Ptr<BoolExpr> expr = visit(ctx->boolExpr());
      return Ptr<SereExpr>{expr};
    }

    virtual antlrcpp::Any visitBoolVar(SereParser::BoolVarContext *ctx) override {
      auto r = vars.insert({
          std::string {ctx->NAME()->getText()},
          vars.size() });
      antlrcpp::Any result = RE_VAR(r.first->second);
      return result;
    }

    virtual antlrcpp::Any visitBoolOr(SereParser::BoolOrContext *ctx) override {
      Ptr<BoolExpr> lhs = visit(ctx->lhs);
      Ptr<BoolExpr> rhs = visit(ctx->rhs);
      return RE_OR(lhs, rhs);
    }

    virtual antlrcpp::Any visitBoolAnd(SereParser::BoolAndContext *ctx) override {
      Ptr<BoolExpr> lhs = visit(ctx->lhs);
      Ptr<BoolExpr> rhs = visit(ctx->rhs);
      return RE_AND(lhs, rhs);
    }

    virtual antlrcpp::Any visitBoolValue(SereParser::BoolValueContext *ctx) override {
      return ctx->BOOL_TRUE() != nullptr ? RE_TRUE : RE_FALSE;
    }

    virtual antlrcpp::Any visitBoolNeg(SereParser::BoolNegContext *ctx) override {
      Ptr<BoolExpr> arg = visit(ctx->arg);
      return RE_NOT(arg);
    }

    virtual antlrcpp::Any visitBoolParens(SereParser::BoolParensContext *ctx) override {
      return visit(ctx->boolExpr());
    }
  };

  /**
   * Parse input stream into complete SERE
   *
   * @param [in] stream input stream
   * @return SERE AST root pointer
   * @throws std::invalid_argument if parse/scan errors occur
   */

  Ptr<SereExpr> parse(std::istream& stream) {
    ANTLRInputStream input(stream);
    SereLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    SereParser parser(&tokens);
    SereErrorListener errorListner;
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListner);
    SereParser::SereContext* tree = parser.sere();
    ExprCollector visitor;
    Ptr<SereExpr> expr = visitor.visitSere(tree);

    return expr;
  }

} // namespace parser
