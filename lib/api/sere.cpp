#include "api/sere.hpp"

#include "ast/Parser.hpp"
#include "ast/SereExpr.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"
#include "nfasl/Dfasl.hpp"
#include "rt/RtDfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "boolean/Expr.hpp"
#include "Match.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <vector>

using json = nlohmann::json;

/**
 * Opaque (for clients) structure to
 * keep reference to service objects
 */

class sere_object;

struct sere_ref {
  std::shared_ptr<sere_object> object;
  std::string content;
};

struct sere_context {
  std::shared_ptr<sere_object> object;
  rt::ExecutorPtr context;
  rt::Names vars;
};

class sere_object {
public:
  static std::shared_ptr<sere_object> load(const char* data);

  const std::vector<std::string>& getAtomics() const { return atomics; }
  void setAtomics(const std::map<std::string, size_t>& vars) {
    atomics.resize(vars.size());
    for (auto v : vars) {
      atomics[v.second] = v.first;
    }
  }

  virtual rt::ExecutorPtr createExecutor() const = 0;
  virtual void load(const json& j) = 0;
  virtual void save(json& j) const = 0;
  virtual ~sere_object() {}
protected:
  std::vector<std::string>& getAtomicsRef() { return atomics; }
private:
  std::vector<std::string> atomics;
};

class sere_nfasl : public sere_object {
public:
  rt::ExecutorPtr createExecutor() const override {
    return std::make_shared<rt::NfaslContext>(rt);
  }
  void load(const json& j) override {
    from_json(j, nfa);
    rt = std::make_shared<rt::Nfasl>();
    nfasl::toRt(nfa, *rt);
  }
  void save(json& j) const override {
    j = json {
              { "kind", "nfasl" },
              { "atomics", getAtomics() },
              { "fasl", nfa } };
  }
  void setNfasl(const nfasl::Nfasl& nfa_) { nfa = nfa_; }
  const nfasl::Nfasl& getNfasl() const { return nfa; }

private:
  std::shared_ptr<rt::Nfasl> rt;
  nfasl::Nfasl nfa;
};

class sere_dfasl : public sere_object {
public:
  rt::ExecutorPtr createExecutor() const override {
    return std::make_shared<rt::DfaslContext>(rt);
  }
  void load(const json& j) {
    from_json(j, dfa);
    rt = std::make_shared<rt::Dfasl>();
    dfasl::toRt(dfa, *rt);
  }
  void save(json& j) const override {
    j = json {
              { "kind", "dfasl" },
              { "atomics", getAtomics() },
              { "fasl", dfa } };
  }
  void setDfasl(const dfasl::Dfasl& dfa_) { dfa = dfa_; }
  const dfasl::Dfasl& getDfasl() const { return dfa; }
private:
  std::shared_ptr<rt::Dfasl> rt;
  dfasl::Dfasl dfa;
};

std::shared_ptr<sere_object> sere_object::load(const char* data) {
  std::string kind;
  std::shared_ptr<sere_object> obj;

  json j = json::parse(data);

  j.at("kind").get_to(kind);

  if (kind == "nfasl") {
    obj = std::make_shared<sere_nfasl>();
  } else if (kind == "dfasl") {
    obj = std::make_shared<sere_dfasl>();
  } else {
    assert(false); // TODO: report an error
  }
  j.at("atomics").get_to(obj->getAtomicsRef());
  obj->load(j.at("fasl"));

  return obj;
}

/**
 * Release SERE compilation results
 */
void sere_release(sere_compiled* result) {
  if (result->compiled) {
    delete result->ref;
  }
}

/**
 * Compile SERE expression
 *
 * @param[in] expr NUL terminated SERE expression
 * @param[in] opts options to control compilation
 * @param[out] result compilation results
 */
int sere_compile(const char* expr,
                 const struct sere_options* opts,
                 sere_compiled* result) {
  memset((void*)result, 0, sizeof(result));
  result->ref = new sere_ref;

  try {
    std::istringstream stream(expr);
    parser::ParseResult r = parser::parse("<buffer>", stream);

    Ptr<SereExpr> expr = r.expr;
    const std::map<std::string, size_t>& vars = r.vars;

    nfasl::Nfasl nfa = sereToNfasl(*expr);
    nfasl::Nfasl min;
    nfasl::minimize(nfa, min);

    result->compiled = 1;

    if (opts->target == SERE_TARGET_DFASL) {
      auto ptr = std::make_shared<sere_dfasl>();
      result->ref->object = ptr;
      dfasl::Dfasl dfa;
      dfasl::toDfasl(min, dfa);
      ptr->setDfasl(dfa);
    } else if (opts->target == SERE_TARGET_NFASL) {
      auto ptr = std::make_shared<sere_nfasl>();
      result->ref->object = ptr;
      ptr->setNfasl(min);
    } else {
      assert(false); // TODO: error reporting
    }
    result->ref->object->setAtomics(vars);
    json j;
    result->ref->object->save(j);
    result->ref->content = j.dump(4);
    result->content = result->ref->content.c_str();
    result->content_size = result->ref->content.size();
    return 0;
  } catch(std::exception& ex) {
    result->compiled = 0;
    result->ref->object = nullptr;
    result->ref->content = ex.what();
    result->error = result->ref->content.c_str();
    return -1;
  }
}

int sere_context_load(const char* rt, /** serialized *FASL */
                      size_t, /** serialized *FASL size */
                      void** sere /** loaded SERE */
                      ) {
  struct sere_context* ctx = nullptr;
  try {
    ctx = new sere_context;
    ctx->object = sere_object::load(rt);
    rt::ExecutorPtr context = ctx->object->createExecutor();
    ctx->context = context;
    ctx->vars.resize(ctx->object->getAtomics().size());
    *sere = reinterpret_cast<void*>(ctx);
    return 0;
  } catch(rt::LoadingFailed& ex) {
    throw;
  } catch(std::exception&) {
    delete ctx;
    return -1;
  }
}

void sere_context_atomic_count(void* ctx, size_t* count) {
  *count = reinterpret_cast<sere_context*>(ctx)->object->getAtomics().size();
}

int sere_context_atomic_name(void* ctx, size_t id, const char** name) {
  auto ref = reinterpret_cast<sere_context*>(ctx);
  if (id < ref->vars.size()) {
    *name = ref->object->getAtomics().at(id).c_str();
    return 0;
  }
  return -1;
}

void sere_context_release(void* ctx) {
  delete reinterpret_cast<sere_context*>(ctx);
}

void sere_context_reset(void* ctx) {
  auto ref = reinterpret_cast<sere_context*>(ctx);
  ref->context->reset();
  ref->vars.reset();
}

int sere_context_set_atomic(void* ctx, size_t atomic) {
  auto ref = reinterpret_cast<sere_context*>(ctx);
  if (atomic < ref->vars.size()) {
    ref->vars.set(atomic);
    return 0;
  }
  return -1;
}

void sere_context_advance(void* ctx) {
  auto ref = reinterpret_cast<sere_context*>(ctx);
  ref->context->advance(ref->vars);
  ref->vars.reset();
}

void sere_context_get_result(void* ctx, int* result) {
  *result = reinterpret_cast<sere_context*>(ctx)->context->getResult();
}