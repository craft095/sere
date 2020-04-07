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

    virtual antlrcpp::Any visitSereParens(SereParser::SereParensContext *ctx) override {
      return visit(ctx->sereExpr());
    }

    virtual antlrcpp::Any visitSereEps(SereParser::SereEpsContext *) override {
      Ptr<SereExpr> expr = RE_EMPTY;
      return expr;
    }

    virtual antlrcpp::Any visitSereIntersection(SereParser::SereIntersectionContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = RE_INTERSECT(lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereUnion(SereParser::SereUnionContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = RE_UNION(lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereConcat(SereParser::SereConcatContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = RE_CONCAT(lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereFusion(SereParser::SereFusionContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      return antlrcpp::Any {}; //RE_FUSION(lhs, rhs);
    }

    virtual antlrcpp::Any visitSereKleeneStar(SereParser::SereKleeneStarContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = RE_STAR(arg);
      return result;
    }

    virtual antlrcpp::Any visitSereKleenePlus(SereParser::SereKleenePlusContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      return antlrcpp::Any{}; //RE_PLUS(arg);
    }

    virtual antlrcpp::Any visitSerePermute(SereParser::SerePermuteContext *ctx) override {
      std::vector<Ptr<SereExpr>> es;
      es.reserve(ctx->elements.size());
      for (auto& e : ctx->elements) {
        Ptr<SereExpr> expr = visit(e);
        es.push_back(expr);
      }
      return antlrcpp::Any {}; // std::RE_PLUS(arg);
    }

    virtual antlrcpp::Any visitSereRange(SereParser::SereRangeContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      assert(ctx->begin != nullptr);
      size_t begin = std::stoi(ctx->begin->getText());
      size_t end;
      if (ctx->end != nullptr) {
        end = std::stoi(ctx->end->getText());
      } else {
        end = begin;
      }
      return antlrcpp::Any {};
    }

    virtual antlrcpp::Any visitSereBoolExpr(SereParser::SereBoolExprContext *ctx) override {
      Ptr<BoolExpr> expr = visit(ctx->boolExpr());
      return Ptr<SereExpr>{expr};
    }

    virtual antlrcpp::Any visitBoolVar(SereParser::BoolVarContext *ctx) override {
      auto r = vars.insert({
          std::string {ctx->NAME()->getText()},
          vars.size() });
      Ptr<BoolExpr> result = RE_VAR(r.first->second);
      return result;
    }

    virtual antlrcpp::Any visitBoolOr(SereParser::BoolOrContext *ctx) override {
      Ptr<BoolExpr> lhs = visit(ctx->lhs);
      Ptr<BoolExpr> rhs = visit(ctx->rhs);
      Ptr<BoolExpr> result = RE_OR(lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitBoolAnd(SereParser::BoolAndContext *ctx) override {
      Ptr<BoolExpr> lhs = visit(ctx->lhs);
      Ptr<BoolExpr> rhs = visit(ctx->rhs);
      Ptr<BoolExpr> result = RE_AND(lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitBoolValue(SereParser::BoolValueContext *ctx) override {
      Ptr<BoolExpr> value = ctx->BOOL_TRUE() != nullptr ? RE_TRUE : RE_FALSE;
      return value;
    }

    virtual antlrcpp::Any visitBoolNeg(SereParser::BoolNegContext *ctx) override {
      Ptr<BoolExpr> arg = visit(ctx->arg);
      Ptr<BoolExpr> result = RE_NOT(arg);
      return result;
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
