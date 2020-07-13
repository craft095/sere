grammar Compute;

compute : (args += argDeclaration)* (lets += letDeclaration)* payload=expression ;

ident : ID ;

argDeclaration : name=ident TYPE_ANN type=ident ';' ;

letDeclaration : LET assignment ';' ;

assignment
    : name=ident ASSIGN body=expression        # constAssign
    | name=ident LPAREN (args += ident (',' args += ident)*)
              RPAREN
              ASSIGN body=expression           # funcAssign
    ;

expression
    : LPAREN expression RPAREN                            # parenExpression
    | EPS                                                 # sereLiteral
    | func=ident LPAREN (args += expression)
                     (',' (args += expression))*
              RPAREN                                      # callExpression
    | (COMPLEMENT | MINUS | BOOL_NOT) arg=expression      # unary
    | arg=expression (KLEENESTAR | KLEENEPLUS)            # unary
    | lhs=expression (DIVIDE | STAR | INTERSECTION)
        rhs=expression                                    # binary
    | lhs=expression (MINUS | PLUS)
        rhs=expression                                    # binary
    | lhs=expression BOOL_AND
        rhs=expression                                    # binary
    | lhs=expression BOOL_OR
        rhs=expression                                    # binary
    | lhs=expression (BOOL_EQ | BOOL_NE | BOOL_LT | BOOL_LE | BOOL_GT | BOOL_GE)
        rhs=expression                                    # binary
    | lhs=expression UNION
        rhs=expression                                    # binary
    | lhs=expression (CONCAT | FUSION)
        rhs=expression                                    # binary
    | (BOOL_TRUE | BOOL_FALSE)                            # boolLiteral
    | ident                                               # nameReference
    | INTLIT                                              # intLiteral
    | FLOATLIT                                            # floatLiteral
    | STRINGLIT                                           # stringLiteral
    ;

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

STRINGLIT    : '"' .*? '"' ;
INTLIT       : '0'|[1-9][0-9]* ;
FLOATLIT     : ('0'|[1-9][0-9]*) '.' [0-9]+ ;

BOOL_TRUE    : 'true' ;
BOOL_FALSE   : 'false' ;

EPS          : '()' ;

WHITESPACE   : [ \r\n\t] -> skip ;

//  Identifiers
ID                 : [A-Za-z][A-Za-z0-9_]* ;
