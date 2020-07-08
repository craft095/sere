lexer grammar ComputeLexer;

// Whitespace
WHITESPACE   : [ \r\n\t] -> skip ;

// Identifiers
ID                 : [A-Za-z][A-Za-z0-9_]* ;

LET : 'let' ;

BOOL_NOT : '!' ;
BOOL_AND : '&&' ;
BOOL_OR  : '||' ;
BOOL_EQ  : '==' ;
BOOL_NE  : '!=' ;
BOOL_LE  : '<=' ;
BOOL_LT  : '<' ;
BOOL_GE  : '>=' ;
BOOL_GT  : '>' ;
LPAREN : '(' ;
RPAREN : ')' ;
LBRACE : '{' ;
RBRACE : '}' ;
DIVIDE : '/' ;
STAR : '*' ;
PLUS : '+' ;
MINUS : '-' ;
TYPE_ANN : '::' ;
ASSIGN : '=' ;
COMPLEMENT : '~' ;
INTERSECTION : '&' ;
UNION : '|' ;
FUSION : ':' ;
CONCAT : ';' ;
KLEENESTAR : '[*]' ;
KLEENEPLUS : '[+]' ;
COMMA : ',' ;

// Literals
STRINGLIT    : '"' .*? '"' ;
INTLIT       : '0'|[1-9][0-9]* ;
FLOATLIT     : ('0'|[1-9][0-9]*) '.' [0-9]+ ;
BOOL_TRUE    : 'true' ;
BOOL_FALSE   : 'false' ;
EPS          : '()' ;
