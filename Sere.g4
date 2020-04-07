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
/*          | '(' sereExpr ')'                        # sereParens
          | EPS                                     # sereEps
          | PERMUTE '(' (elements += sereExpr)
                    (',' elements += sereExpr)* ')' # serePermute
          | sereExpr INTERSECTION sereExpr          # sereIntersection
          | sereExpr UNION sereExpr                 # sereUnion
          | sereExpr KLEENESTAR                     # sereKleenStar
          | sereExpr KLEENEPLUS                     # sereKleenePlus
          | sereExpr '{' begin = NUM
                    (',' end = NUM)? '}'            # sereRange
  */        ;

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
PERMUTE : 'PERMUTE' ;
INTERSECTION : ';' ;
FUSION : ':' ;
UNION : '|' ;
KLEENESTAR : '[*]' ;
KLEENEPLUS : '[+]' ;
STRING      : '"' .*? '"' ;
NUM         : [0-9]+ ;
NAME        :  [a-zA-Z][a-zA-Z0-9\-_]* ;
NEWLINE     : ('\r'? '\n' | '\r')+ ;
TAB         : ('\t' | '        ' | '    ' ) ;
WHITESPACE  : ' ' -> skip ;
