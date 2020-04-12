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
   * Make all possible permutation form a given vector of expressions
   *
   * @param [in] us vector of expressions
   * @returns union of all permutations
   */
  Ptr<SereExpr> permute(const std::vector<Ptr<SereExpr>>& us) {
    if (us.empty()) {
      return RE_EMPTY;
    } else if (us.size() == 1) {
      return us[0];
    }

    assert(us.size() >= 2);
    Ptr<SereExpr> opt = nullptr;
    for (size_t ix = 0; ix < us.size(); ix++) {
      auto tail{us};
      tail.erase(tail.begin() + ix);
      auto alt = RE_CONCAT(us[ix], permute(tail));
      if (opt == nullptr) {
        opt = alt;
      } else {
        opt = RE_UNION(opt, alt);
      }
    }
    return opt;
  }
  /**
   * Parse AST traversal class
   */
  class ExprCollector : public SereBaseVisitor {
    std::map<std::string, size_t> vars;
  public:
    std::map<std::string, size_t> getVars() const { return vars; }

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
      Ptr<SereExpr> result = RE_FUSION(lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereKleeneStar(SereParser::SereKleeneStarContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = RE_STAR(arg);
      return result;
    }

    virtual antlrcpp::Any visitSereKleenePlus(SereParser::SereKleenePlusContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = RE_PLUS(arg);
      return result;
    }

    virtual antlrcpp::Any visitSerePartial(SereParser::SerePartialContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = RE_PARTIAL(arg);
      return result;
    }

    virtual antlrcpp::Any visitSereAbort(SereParser::SereAbortContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> err = visit(ctx->err);
      Ptr<SereExpr> result =
        RE_UNION(
                 RE_CONCAT(RE_PARTIAL(arg), err),
                 arg);

      return result;
    }

    virtual antlrcpp::Any visitSerePermute(SereParser::SerePermuteContext *ctx) override {
      std::vector<Ptr<SereExpr>> es;
      es.reserve(ctx->elements.size());
      for (auto& e : ctx->elements) {
        Ptr<SereExpr> expr = visit(e);
        es.push_back(expr);
      }

      Ptr<SereExpr> result = permute(es);

      return result;
    }

    virtual antlrcpp::Any visitSereFullRange(SereParser::SereFullRangeContext *ctx) override {
      assert(ctx->begin != nullptr);
      assert(ctx->end != nullptr);
      size_t begin = std::stoi(ctx->begin->getText());
      size_t end = std::stoi(ctx->end->getText());

      if (begin == 0 && end == 0) {
        return Ptr<SereExpr>{RE_EMPTY};
      }

      Ptr<SereExpr> arg = visit(ctx->arg);

      Ptr<SereExpr> opt = nullptr;
      for (size_t ix = 0; ix < begin; ++ix) {
        if (opt == nullptr) {
          opt = arg;
        } else {
          opt = RE_CONCAT(opt, arg);
        }
      }

      // Combine results: opt;(1 + v;(1 + v))

      Ptr<SereExpr> rhs = nullptr;
      for (size_t ix = begin; ix < end; ++ix) {
        if (rhs == nullptr) {
          rhs = RE_UNION(RE_EMPTY, arg);
        } else {
          rhs = RE_UNION(RE_EMPTY, RE_CONCAT(arg, rhs));
        }
      }

      Ptr<SereExpr> result =
        rhs == nullptr
        ? (opt == nullptr
           ? RE_EMPTY
           : opt)
        : (opt == nullptr
           ? rhs
           : RE_CONCAT(opt, rhs));
      return result;
    }

    virtual antlrcpp::Any visitSereMinRange(SereParser::SereMinRangeContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      assert(ctx->begin != nullptr);
      size_t begin = std::stoi(ctx->begin->getText());

      switch (begin) {
      case 0: return Ptr<SereExpr>{RE_STAR(arg)};
      case 1: return Ptr<SereExpr>{RE_PLUS(arg)};
      }
      Ptr<SereExpr> opt = arg;
      for (size_t ix = 1; ix < begin; ++ix) {
        opt = RE_CONCAT(opt, arg);
      }
      opt = RE_CONCAT(opt, RE_STAR(arg));
      return opt;
    }

    virtual antlrcpp::Any visitSereSingleRange(SereParser::SereSingleRangeContext *ctx) override {
      assert(ctx->count != nullptr);
      size_t count = std::stoi(ctx->count->getText());

      if (count == 0) {
        return Ptr<SereExpr>{RE_EMPTY};
      }

      // traverse the expression only if it is not discarded
      Ptr<SereExpr> arg = visit(ctx->arg);

      if (count == 1) {
        return arg;
      }
      Ptr<SereExpr> opt = arg;
      for (size_t ix = 1; ix < count; ++ix) {
        opt = RE_CONCAT(opt, arg);
      }
      return opt;
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

  ParseResult parse(std::istream& stream) {
    ANTLRInputStream input(stream);
    SereLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    SereParser parser(&tokens);
    SereErrorListener errorListner;
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListner);
    SereParser::SereContext* tree = parser.sere();
    ExprCollector visitor;
    ParseResult result;
    result.expr = visitor.visitSere(tree);
    result.vars = visitor.getVars();

    return result;
  }

} // namespace parser
