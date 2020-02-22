#include <iostream>
#include <memory>

#include "Language.hpp"
#include "EvalSere.hpp"

#define RE_LOC Located(Pos(__FILE__, __LINE__, 0))
#define RE_EMPTY std::make_shared<SereEmpty>(RE_LOC)
#define RE_TRUE std::make_shared<BoolValue>(RE_LOC, true)
#define RE_FALSE std::make_shared<BoolValue>(RE_LOC, false)
#define RE_VAR(n) std::make_shared<Variable>(RE_LOC, n)
#define RE_NOT(n) std::make_shared<BoolNot>(RE_LOC, n)
#define RE_AND(u,v) std::make_shared<BoolAnd>(RE_LOC, u ,v)
#define RE_INTERSECT(u,v) std::make_shared<Intersect>(RE_LOC, u,v)
#define RE_UNION(u,v) std::make_shared<Union>(RE_LOC, u,v)
#define RE_CONCAT(u,v) std::make_shared<Concat>(RE_LOC, u,v)
#define RE_STAR(u) std::make_shared<KleeneStar>(RE_LOC, u)

// std::shared_ptr<SereExpr> test0() {
//   return
//     RE_INTERSECT
//     (RE_STAR
//      (RE_UNION
//       (RE_EMPTY,
//        RE_TRUE)),
//      (RE_AND
//       (RE_FALSE,
//        (RE_NOT
//         (RE_VAR("x"))))));
// }

void testcase_empty() {
  assert(evalSere(*RE_EMPTY, {}) == Match_Ok);
  assert(evalSere(*RE_EMPTY, {{"",""}}) == Match_Failed);
}

void testcase_bool() {
  assert(evalSere(*RE_TRUE, {{"y", "x"}}) == Match_Ok);
  assert(evalSere(*RE_FALSE, {{"y", "x"}}) == Match_Failed);
  assert(evalSere(*RE_NOT(RE_TRUE), {{"y", "x"}}) == Match_Failed);
  assert(evalSere(*RE_VAR("y"), {{"y", "x"}}) == Match_Ok);
  assert(evalSere(*RE_VAR("x"), {{"y", "x"}}) == Match_Failed);

  SereChildPtr tc = RE_AND(RE_TRUE, RE_NOT(RE_VAR("x")));
  assert(evalSere(*tc, {{"y", "x"}}) == Match_Ok);
  assert(evalSere(*tc, {{"x", "y"}}) == Match_Failed);
  assert(evalSere(*tc, {{"y", "x"}, {"y", "x"}}) == Match_Failed);
  assert(evalSere(*tc, {}) == Match_Partial);
  // assert(evalSere(*tc, {{"z", "y"}}) == Match_Failed);
}

void testcase_concat() {
  assert(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {}) == Match_Partial);
  assert(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {{"x",""},{"y",""}}) == Match_Ok);
  assert(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {{"xy",""},{"x","y"}}) == Match_Failed);
  assert(evalSere
         (*RE_CONCAT
          (RE_CONCAT
           (RE_VAR("x"),
            RE_VAR("y")
            ),
           RE_VAR("x")),
          {{"x","y"}}) == Match_Partial);
  assert(evalSere
         (*RE_CONCAT
          (RE_CONCAT
           (RE_VAR("x"),
            RE_VAR("y")
            ),
           RE_VAR("x")),
          {{"x","y"},{"x","y"}}) == Match_Failed);
}

int main() {
  /*
  SereChildPtr tc0 = test0();
  std::cout << tc0->pretty() << std::endl;
  std::cout << "at " << tc0->getLoc().pretty() << std::endl;

  Letter l0("hello", "world");
  Letter l1("ello", "world");
  std::cout << prettyWord({l0,l1}) << std::endl;
  */

  testcase_empty();
  testcase_bool();
  testcase_concat();

  return 0;
}
