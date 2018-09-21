# MiniJava Implementation

Compiler implementation for MAC5750 in C++. Following the book
[Modern Compiler Implementaton in Java](https://www.amazon.com.br/Modern-Compiler-Implementation-Andrew-Appel-ebook/dp/B00AHTN2TU?__mk_pt_BR=%C3%85M%C3%85%C5%BD%C3%95%C3%91&keywords=Modern+compiler+implementation+in+Java&qid=1537540596&sr=8-1-fkmrnull&ref=sr_1_fkmrnull_1)

## API and Ideas

I wish to have some freedom in the representation of trees, both
the AST and the IR. The idea then is to have the tree structure represented
in several types, which can be traversed with a visitor interface. Moreover,
the tree itself only keeps the structure, and extra information are keep separated,
using the
[ECS Design Pattern](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system).

To make the parser file agnostic about the representation, it uses
a [Builder patttern](https://en.wikipedia.org/wiki/Builder_pattern).
The idea is that the parser file is as close as possible to the BNF grammar,
with only the additional control flow information. The builder is responsible
with keeping the information and logging errors.

Since the parser is implemented in a recursive descent manner, there is a
need of an object to record pertinent state. That is the role of ParserContext.

## TODOs

* Builder record errors
* Visitor report errors
* Type enhance EC
* Implement IR
* Symbol Table pass

### Builder record errors

ASTBuilder handles the data from the parsing and build the objects.
Whenever an error occurs, there is some node in the resulting AST that
should record it. Hence, this check must be made by the builder. That
is why the builder object consumes the lexemes from the stream.

When we do something like

    builder << Type(parser) << Lexeme::identifier << Lexeme::semicolon;

if there is no semicolon or identifier, the current builder object
records it.

### Visitor report errors

This should be the proof of concept about EC and Visitor as the way
to map over the AST. Should be simple and easy to use.

### Type enhance EC

Once the concept is first developed, I should take some care to allow
the type system to help.

### Implement IR

Once I have finished working on the AST, I can use the tools developed
to work with the IR.
