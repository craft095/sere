parser grammar ComputeParser;

options { tokenVocab=ComputeLexer; }

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
    | ident                                               # nameReference
    | INTLIT                                              # intLiteral
    | FLOATLIT                                            # floatLiteral
    | STRINGLIT                                           # stringLiteral
    ;
