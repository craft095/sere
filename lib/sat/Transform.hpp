#ifndef SAT_TRANSFORM_HPP
#define SAT_TRANSFORM_HPP

#include "boolean/Expr.hpp"

/**
 * Convert to NNF
 *
 * The result:
 * - either TRUE
 * - or FALSE
 * - or it does not contain neither TRUE nor FALSE
 *   and NEGATION is only applied to variable
 */
extern boolean::Expr nnf(boolean::Expr expr);

/**
 * Substitute expression with a const.
 * @param expr expression must be in NNF
 * @param search expression to search
 * @param replace expression to replace with
 * @returns new expression
 */
extern boolean::Expr subst(boolean::Expr expr, boolean::Expr search, boolean::Expr replace);

#endif // SAT_TRANSFORM_HPP
