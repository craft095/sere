#include <iostream>
#include <memory>

#include "gtest/gtest.h"

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

TEST(Sere, empty) {
  ASSERT_EQ(evalSere(*RE_EMPTY, {}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_EMPTY, {{"",""}}), Match_Failed);
}

TEST(Sere, boolean) {
  ASSERT_EQ(evalSere(*RE_TRUE, {{"y", "x"}}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_FALSE, {{"y", "x"}}), Match_Failed);
  ASSERT_EQ(evalSere(*RE_NOT(RE_TRUE), {{"y", "x"}}), Match_Failed);
  ASSERT_EQ(evalSere(*RE_VAR("y"), {{"y", "x"}}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_VAR("x"), {{"y", "x"}}), Match_Failed);

  SereChildPtr tc = RE_AND(RE_TRUE, RE_NOT(RE_VAR("x")));
  ASSERT_EQ(evalSere(*tc, {{"y", "x"}}), Match_Ok);
  ASSERT_EQ(evalSere(*tc, {{"x", "y"}}), Match_Failed);
  ASSERT_EQ(evalSere(*tc, {{"y", "x"}, {"y", "x"}}), Match_Failed);
  ASSERT_EQ(evalSere(*tc, {}), Match_Partial);
  // ASSERT_EQ(evalSere(*tc, {{"z", "y"}}), Match_Failed);
}

TEST(Sere, concat) {
  ASSERT_EQ(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {}), Match_Partial);
  ASSERT_EQ(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {{"x",""},{"y",""}}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {{"xy",""},{"x","y"}}), Match_Failed);
  ASSERT_EQ(evalSere
         (*RE_CONCAT
          (RE_CONCAT
           (RE_VAR("x"),
            RE_VAR("y")
            ),
           RE_VAR("x")),
          {{"x","y"}}), Match_Partial);
  ASSERT_EQ(evalSere
         (*RE_CONCAT
          (RE_CONCAT
           (RE_VAR("x"),
            RE_VAR("y")
            ),
           RE_VAR("x")),
          {{"x","y"},{"x","y"}}), Match_Failed);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
