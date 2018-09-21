# MinJava Implementation

TODOs

* Builder record errors
* Visitor report errors
* Type enhance EC
* Implement IR
* Symbol Table pass

## Builder record errors

Keep in parser stack context, with guards given by ASTBuilder.
Moreover, keep the error component in the Parser, so that it can
be later accessed.

Migrate .consume method from parser to builder, so that builder record
errors. Be sure that

    builder.consume(Lexeme::identifier);

behaves as

    builder.keep(parser.consume(Lexeme::identifier));


## Visitor report errors

This should be the proof of concept about EC and Visitor as the way
to map over the AST. Should be simple and easy to use.

## Type enhance EC

Once the concept is first developed, I should take some care to allow
the type system to help.

## Implement IR

Once I have finished working on the AST, I can use the tools developed
to work with the IR.
