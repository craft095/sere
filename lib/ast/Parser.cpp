#include <iostream>
#include <sstream>

#include "boolean/Expr.hpp"

#include "antlr4-runtime.h"
#include "SereLexer.h"
#include "SereParser.h"
#include "SereBaseVisitor.h"

#include "ast/BoolExpr.hpp"
#include "ast/SereExpr.hpp"
#include "ast/Parser.hpp"

using namespace antlr4;
using namespace ast;

namespace parser {

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


  class SereErrorListener : public antlr4::BaseErrorListener {
  public:
    SereErrorListener(const FileName& file_) : file(file_) {}
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
   * Make all possible permutation form a given vector of expressions
   *
   * @param [in] us vector of expressions
   * @returns union of all permutations
   */
  Ptr<SereExpr> permute(const Located& loc, const std::vector<Ptr<SereExpr>>& us) {
    if (us.empty()) {
      return std::make_shared<SereEmpty>(loc);
    } else if (us.size() == 1) {
      return us[0];
    }

    assert(us.size() >= 2);
    Ptr<SereExpr> opt = nullptr;
    for (size_t ix = 0; ix < us.size(); ix++) {
      auto tail{us};
      tail.erase(tail.begin() + ix);
      auto alt = std::make_shared<Concat>(loc, us[ix], permute(loc, tail));
      if (opt == nullptr) {
        opt = alt;
      } else {
        opt = std::make_shared<Union>(loc, opt, alt);
      }
    }
    return opt;
  }
  /**
   * Parse AST traversal class
   */
  class ExprCollector : public SereBaseVisitor {
    std::map<std::string, size_t> vars;
    FileName fileName;
  public:
    ExprCollector(const FileName& file) : fileName(file) {}

    Located toLocated(antlr4::ParserRuleContext& rule) const {
      Located loc;
      parser::toLocated(fileName, rule, loc);
      return loc;
    }

    template<typename T, typename Node, typename... Args>
    Ptr<T> create(Node ctx, Args... args) {
      return std::make_shared<T>(toLocated(*ctx), args...);
    }

    std::map<std::string, size_t> getVars() const { return vars; }

    virtual antlrcpp::Any visitSere(SereParser::SereContext *ctx) override {
      Ptr<SereExpr> result = visit(ctx->sereExpr());
      return result;
    }

    virtual antlrcpp::Any visitSereParens(SereParser::SereParensContext *ctx) override {
      Ptr<SereExpr> result = visit(ctx->sereExpr());
      return result;
    }

    virtual antlrcpp::Any visitSereEps(SereParser::SereEpsContext *ctx) override {
      Ptr<SereExpr> expr = create<SereEmpty>(ctx);
      return expr;
    }

    virtual antlrcpp::Any visitSereIntersection(SereParser::SereIntersectionContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = create<Intersect>(ctx, lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereUnion(SereParser::SereUnionContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = create<Union>(ctx, lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereConcat(SereParser::SereConcatContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = create<Concat>(ctx, lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereFusion(SereParser::SereFusionContext *ctx) override {
      Ptr<SereExpr> lhs = visit(ctx->lhs);
      Ptr<SereExpr> rhs = visit(ctx->rhs);
      Ptr<SereExpr> result = create<Fusion>(ctx, lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitSereKleeneStar(SereParser::SereKleeneStarContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = create<KleeneStar>(ctx, arg);
      return result;
    }

    virtual antlrcpp::Any visitSereKleenePlus(SereParser::SereKleenePlusContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = create<KleenePlus>(ctx, arg);
      return result;
    }

    virtual antlrcpp::Any visitSerePartial(SereParser::SerePartialContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = create<Partial>(ctx, arg);
      return result;
    }

    virtual antlrcpp::Any visitSereComplement(SereParser::SereComplementContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> result = create<Complement>(ctx, arg);
      return result;
    }

    virtual antlrcpp::Any visitSereAbort(SereParser::SereAbortContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      Ptr<SereExpr> err = visit(ctx->err);
      Ptr<SereExpr> result =
        create<Union>(ctx,
                      create<Concat>(ctx,
                                     create<Partial>(ctx, arg), err),
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

      Ptr<SereExpr> result = permute(toLocated(*ctx), es);

      return result;
    }

    virtual antlrcpp::Any visitSereFullRange(SereParser::SereFullRangeContext *ctx) override {
      assert(ctx->begin != nullptr);
      assert(ctx->end != nullptr);
      size_t begin = std::stoi(ctx->begin->getText());
      size_t end = std::stoi(ctx->end->getText());

      if (begin == 0 && end == 0) {
        Ptr<SereExpr> r = create<SereEmpty>(ctx);
        return r;
      }

      Ptr<SereExpr> arg = visit(ctx->arg);

      Ptr<SereExpr> opt = nullptr;
      for (size_t ix = 0; ix < begin; ++ix) {
        if (opt == nullptr) {
          opt = arg;
        } else {
          opt = create<Concat>(ctx, opt, arg);
        }
      }

      // Combine results: opt;(1 + v;(1 + v))

      Ptr<SereExpr> rhs = nullptr;
      for (size_t ix = begin; ix < end; ++ix) {
        if (rhs == nullptr) {
          rhs = create<Union>(ctx, create<SereEmpty>(ctx), arg);
        } else {
          rhs = create<Union>(ctx, create<SereEmpty>(ctx), create<Concat>(ctx, arg, rhs));
        }
      }

      Ptr<SereExpr> result =
        rhs == nullptr
        ? (opt == nullptr
           ? create<SereEmpty>(ctx)
           : opt)
        : (opt == nullptr
           ? rhs
           : create<Concat>(ctx, opt, rhs));
      return result;
    }

    virtual antlrcpp::Any visitSereMinRange(SereParser::SereMinRangeContext *ctx) override {
      Ptr<SereExpr> arg = visit(ctx->arg);
      assert(ctx->begin != nullptr);
      size_t begin = std::stoi(ctx->begin->getText());

      switch (begin) {
      case 0: return Ptr<SereExpr>{create<KleeneStar>(ctx, arg)};
      case 1: return Ptr<SereExpr>{create<KleenePlus>(ctx, arg)};
      }
      Ptr<SereExpr> opt = arg;
      for (size_t ix = 1; ix < begin; ++ix) {
        opt = create<Concat>(ctx, opt, arg);
      }
      opt = create<Concat>(ctx, opt, create<KleeneStar>(ctx, arg));
      return opt;
    }

    virtual antlrcpp::Any visitSereSingleRange(SereParser::SereSingleRangeContext *ctx) override {
      assert(ctx->count != nullptr);
      size_t count = std::stoi(ctx->count->getText());

      if (count == 0) {
        Ptr<SereExpr> r = create<SereEmpty>(ctx);
        return r;
      }

      // traverse the expression only if it is not discarded
      Ptr<SereExpr> arg = visit(ctx->arg);

      if (count == 1) {
        return arg;
      }
      Ptr<SereExpr> opt = arg;
      for (size_t ix = 1; ix < count; ++ix) {
        opt = create<Concat>(ctx, opt, arg);
      }
      return opt;
    }

    virtual antlrcpp::Any visitSereBoolExpr(SereParser::SereBoolExprContext *ctx) override {
      Ptr<BoolExpr> expr = visit(ctx->boolExpr());
      Ptr<SereExpr> result = create<SereBool>(ctx, expr);
      return result;
    }

    virtual antlrcpp::Any visitBoolVar(SereParser::BoolVarContext *ctx) override {
      auto r = vars.insert({
          std::string {ctx->NAME()->getText()},
          vars.size() });
      Ptr<BoolExpr> result = create<Variable>(ctx, VarName {r.first->second});
      return result;
    }

    virtual antlrcpp::Any visitBoolOr(SereParser::BoolOrContext *ctx) override {
      Ptr<BoolExpr> lhs = visit(ctx->lhs);
      Ptr<BoolExpr> rhs = visit(ctx->rhs);
      Ptr<BoolExpr> result = create<BoolOr>(ctx, lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitBoolAnd(SereParser::BoolAndContext *ctx) override {
      Ptr<BoolExpr> lhs = visit(ctx->lhs);
      Ptr<BoolExpr> rhs = visit(ctx->rhs);
      Ptr<BoolExpr> result = create<BoolAnd>(ctx, lhs, rhs);
      return result;
    }

    virtual antlrcpp::Any visitBoolValue(SereParser::BoolValueContext *ctx) override {
      Ptr<BoolExpr> value = create<BoolValue>(ctx, ctx->BOOL_TRUE() != nullptr);
      return value;
    }

    virtual antlrcpp::Any visitBoolNeg(SereParser::BoolNegContext *ctx) override {
      Ptr<BoolExpr> arg = visit(ctx->arg);
      Ptr<BoolExpr> result = create<BoolNot>(ctx, arg);
      return result;
    }

    virtual antlrcpp::Any visitBoolParens(SereParser::BoolParensContext *ctx) override {
      return Ptr<BoolExpr>{visit(ctx->boolExpr())};
    }
  };

  /**
   * Parse input stream into complete SERE
   *
   * @param [in] file stream source name
   * @param [in] stream input stream
   * @return SERE AST root pointer
   * @throws parser::ParseError if parse/scan errors occur
   */

  ParseResult parse(const std::string& file, std::istream& stream) {
    ANTLRInputStream input(stream);
    SereLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    SereParser parser(&tokens);
    SereErrorListener errorListner(file);
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListner);
    SereParser::SereContext* tree = parser.sere();
    ExprCollector visitor(file);
    ParseResult result;
    result.expr = visitor.visitSere(tree);
    result.vars = visitor.getVars();

    return result;
  }

  ParseResult parse(std::istream& stream) {
    return parse("<unknown>", stream);
  }
} // namespace parser
