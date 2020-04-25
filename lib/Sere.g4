grammar Sere ;

sere : sereExpr EOF ;

boolExpr : '(' boolExpr ')'                         # boolParens
          | var=NAME                                # boolVar
          | value=(BOOL_TRUE|BOOL_FALSE)            # boolValue
          | BOOL_NOT arg=boolExpr                   # boolNeg
          | lhs=boolExpr op=BOOL_AND rhs=boolExpr   # boolAnd
          | lhs=boolExpr op=BOOL_OR rhs=boolExpr    # boolOr
          ;

sereExpr : boolExpr                                 # sereBoolExpr
          | '(' sereExpr ')'                        # sereParens
          | EPS                                     # sereEps
          | PARTIAL '(' arg=sereExpr ')'            # serePartial
          | ABORT '(' arg=sereExpr
                  ',' err=sereExpr ')'              # sereAbort
          | PERMUTE '(' (elements += sereExpr)
                    (',' elements += sereExpr)* ')' # serePermute
          | arg=sereExpr KLEENESTAR                 # sereKleeneStar
          | arg=sereExpr KLEENEPLUS                 # sereKleenePlus
          | arg=sereExpr '{' begin = NUM
                         ',' end = NUM '}'          # sereFullRange
          | arg=sereExpr '{' begin = NUM ',' '}'    # sereMinRange
          | arg=sereExpr '{' count = NUM '}'        # sereSingleRange
          | lhs=sereExpr INTERSECTION rhs=sereExpr  # sereIntersection
          | lhs=sereExpr UNION rhs=sereExpr         # sereUnion
          | lhs=sereExpr CONCAT rhs=sereExpr        # sereConcat
          | lhs=sereExpr FUSION rhs=sereExpr        # sereFusion
          ;

BOOL_TRUE : 'true' ;
BOOL_FALSE : 'false' ;
BOOL_NOT : '!' ;
BOOL_AND : '&&' ;
BOOL_OR  : '||' ;
LPAREN : '(' ;
RPAREN : ')' ;
LBRACE : '{' ;
RBRACE : '}' ;
EPS : '()' ;
ABORT : 'ABORT' ;
PARTIAL : 'PARTIAL' ;
PERMUTE : 'PERMUTE' ;
INTERSECTION : '&' ;
UNION : '|' ;
FUSION : ':' ;
CONCAT : ';' ;
KLEENESTAR : '[*]' ;
KLEENEPLUS : '[+]' ;
STRING      : '"' .*? '"' ;
NUM         : [0-9]+ ;
NAME        :  [a-zA-Z][a-zA-Z0-9\-_]* ;
WHITESPACE  : [ \r\n\t] -> skip ;
