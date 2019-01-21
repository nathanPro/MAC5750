# MiniJava Implementation

Compiler implementation for MAC5750 in C++. Following the book
[Modern Compiler Implementaton in Java](https://www.amazon.com.br/Modern-Compiler-Implementation-Andrew-Appel-ebook/dp/B00AHTN2TU?__mk_pt_BR=%C3%85M%C3%85%C5%BD%C3%95%C3%91&keywords=Modern+compiler+implementation+in+Java&qid=1537540596&sr=8-1-fkmrnull&ref=sr_1_fkmrnull_1)


## Building

The project itself has no dependenies -- except googletest for testing --
but its build system has. You need to have the python libs
[ninja](https://ninja-build.org/manual.html) and
[meson](https://mesonbuild.com/). Both can be installed from pip. However,
to avoid messing up existing python libraries, I decided to install them
in virtual environments (i.e., locally). If you have python 3.6 or later,
you can just run

    ./configure.sh

and all the build system dependencies will be installed locally. If all goes
well, you can then run

    make

and then

    ./bcc input/sample.miniJava
    ./a.out

and execute a miniJava progam.

However, if things do not work, you will have to install ninja and meson with

    pip install ninja meson

and empty the PYTHON_ENV variable in the Makefile

    .PHONY: test clean

    SHELL = /bin/bash

    PYTHON_VENV = 

    (rest of the file)

after that, you may be able to run make and compile code.


## Gotchas

### Error reporting is bad

The compiler just crashes if the error is anything but a syntax error.
However, if this is the case, it may produce a helpful error message.
All the other errors just kill the program.

### There is no operator precedence

I implemented the parser and lexer without any tools. It was really interesting 
and I learned a lot. But a few features went missing. One that I just noticed
by the end of the project is that there is no operator precedence in the
parser. Hence, you will probably want to use several parenthesis.

### There is no array functionality

None of the statements from the language about arrays are implemented.
There is no `new int [Exp]` nor `a[Exp]`.

## API and Ideas

The front end of the project is really good. The [parser file](src/parser.cpp)
is remarkably close to the grammar itself, which is quite satisfying and was
quite hard to achieve.

Seriously, take a look at the [parser](src/parser.cpp). Some methods,
like the one that parses the main function, are basically a copy of
the grammar defined in the book. The one that departs the most from
that are the methods that parse expressions, since I had to change
the grammar to avoid left recursion. The `_E` method is the one that
books usually represent as `E'`, which is the artifact from removing
left recursion.

The main idea in this part is to have builder objects that hold the
necessary info and record errors, which can then be used to instantiate
the AST nodes. They are the builder objects, and are defined in
[builder.h](src/builder.h).

Every node in the AST is given a unique index, and this index is used as
a vector position for storing aditional information on the nodes. This is
how the syntax errors are recorded and then reported back -- see 
[error.h](src/error.h).

When I noticed it was a lot of work to code a recursion in the tree, I
decided to implement a tool to help me in it. I did this for the [IR](src/IR.h),
which I implemented from scratch, without C++17 help. Since I had a 
[builder class](src/IRBuilder.h) for the class, I was able to easily keep
the invariant that the label of every node is greater than the labels
of its children, so that recursion on the tree could be done in a single pass.
This is what the template Catamorphism, in [IR.h](src/IR.h) does. It takes
a function that can handle every type of node in the IR and "lifts" it into a
function of the tree as a whole. In the same file, the template classes
ShallowFormat and DeepFormat are examples.

The current implementation ships inheritance, but no virtual tables. It
does a "inheritance via overloading" in a way similar to C++ inheritance
without virtual functions. The correct method to be called can be
determined in compile time, so it suffices to determine which function
to call and to keep the memory layout compatible. The input file
[inheritance.miniJava](input/inheritance.miniJava) is an example that
compiles and produces the same output as [sample.miniJava](input/sample.miniJava).

The file [helper.h](src/helper.h) is the symbol table of the language.
I build it from the AST and use it in translation. It keeps all the informations
about classes and method stack layout. Moreover, since I did not implement
class member variables as pointers, but in a way similar to C structs,
_I had to find a topological sort of the classes in order to process them_.
Basically, if a class `A` has a member variable of class `B`, the size of
`A` depends on the size of `B`. I decided not to have forward declarations
in the language, and implemented a topological sort of the classes, that
actually complains when there are cyclic dependencies. This utility
can be found [here](src/class_graph.h).

The translation is done in [translation.h](src/translation.h). It translates
the tree into a first IR representation. This IR is then adapted until
it is really close to assembly, and then it is actually translated. This
second step is done in [codegen.h](src/codegen.h).
