#include "AST.h"
#include "Builder.h"
#include "lexer.h"
#include "util.h"

namespace Parser {

struct MismatchError {
    Lexeme expected;
    Lexeme found;
};

struct Unexpected {
    Lexeme lex;
};

struct UnexpectedEnd {};

template <typename istream> class Parser {
    Lexer<istream> tokens;
    int idx;

  public:
    Parser(istream& stream) : tokens(stream, 2), idx(0) {}
    AST::Node make_id() { return AST::Node(idx++); }
    LexState operator[](int i) { return tokens[i]; }
    std::string consume(Lexeme lex) {
        if (tokens.empty()) throw UnexpectedEnd{};
        if ((*tokens).first == lex) {
            std::string ans = (*tokens).second;
            ++tokens;
            return ans;
        } else {
            write(std::cerr, "ERROR:");
            std::cerr << "Expected '" << lex << "' and found '"
                      << tokens[0].first << "'\n";
            throw MismatchError{lex, tokens[0].first};
        }
    }
};

struct ContextGuard {
    size_t line;
    std::string label;
    bool active;

    ContextGuard(size_t _line, std::string _label)
        : line(_line), label(_label), active(true) {}
    ~ContextGuard() {
        if (!active) return;
        write(std::cerr, "while parsing", label, "from line", line);
    }
};

template <typename istream> AST::ptr<AST::Exp> Exp(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::ExpList> ExpList(Lexer<istream>&);
template <typename istream> AST::ptr<AST::Stm> Stm(Lexer<istream>&);
template <typename istream> AST::ptr<AST::Type> Type(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::FormalList> FormalList(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::MethodDecl> MethodDecl(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::ClassDecl> ClassDecl(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::MainClass> MainClass(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::Program> Program(Lexer<istream>&);

template <typename istream>
AST::ptr<AST::Program> Program(Parser<istream>& parser) {
    using std::make_unique;
    using std::move;
    auto main = MainClass(parser);
    std::vector<AST::__detail::pClassDecl> classes;
    while (Lexeme(parser[0]) != Lexeme::eof)
        classes.push_back(ClassDecl(parser));
    return make_unique<AST::Program>(AST::ProgramRule{
        parser.make_id(), move(main), move(classes)});
}

template <typename istream>
AST::ptr<AST::MainClass> MainClass(Parser<istream>& parser) {
    using std::make_unique;
    using std::move;

    ContextGuard guard(parser[0].third, "<main class>");
    parser.consume(Lexeme::class_keyword);
    auto name = parser.consume(Lexeme::identifier);
    parser.consume(Lexeme::open_brace);
    parser.consume(Lexeme::public_keyword);
    parser.consume(Lexeme::static_keyword);
    parser.consume(Lexeme::void_keyword);
    if (parser[0].second != std::string("main"))
        throw Unexpected{Lexeme(parser[0])};
    parser.consume(Lexeme::identifier);
    parser.consume(Lexeme::open_paren);
    parser.consume(Lexeme::string_keyword);
    parser.consume(Lexeme::open_bracket);
    parser.consume(Lexeme::close_bracket);
    auto arg = parser.consume(Lexeme::identifier);
    parser.consume(Lexeme::close_paren);
    parser.consume(Lexeme::open_brace);
    auto body = Stm(parser);
    parser.consume(Lexeme::close_brace);
    parser.consume(Lexeme::close_brace);
    guard.active = false;
    return make_unique<AST::MainClass>(
        AST::MainClassRule{parser.make_id(), name, arg, move(body)});
}

template <typename istream>
AST::ptr<AST::ClassDecl> ClassDecl(Parser<istream>& parser) {
    using std::make_unique;
    using std::move;

    ContextGuard guard(parser[0].third, "<class>");
    parser.consume(Lexeme::class_keyword);
    auto name = parser.consume(Lexeme::identifier);

    bool has_superclass =
        (Lexeme(parser[0]) == Lexeme::extends_keyword);

    std::string superclass;
    if (has_superclass) {
        parser.consume(Lexeme::extends_keyword);
        superclass = parser.consume(Lexeme::identifier);
    }

    parser.consume(Lexeme::open_brace);
    std::vector<AST::__detail::pVarDecl> variables;
    while (Lexeme(parser[0]) != Lexeme::close_brace &&
           Lexeme(parser[0]) != Lexeme::public_keyword)
        variables.push_back(VarDecl(parser));

    std::vector<AST::__detail::pMethodDecl> methods;
    while (Lexeme(parser[0]) != Lexeme::close_brace)
        methods.push_back(MethodDecl(parser));
    parser.consume(Lexeme::close_brace);

    guard.active = false;
    if (has_superclass)
        return make_unique<AST::ClassDecl>(AST::ClassDeclInheritance{
            parser.make_id(), name, superclass, move(variables),
            move(methods)});
    else
        return make_unique<AST::ClassDecl>(
            AST::ClassDeclNoInheritance{parser.make_id(), name,
                                        move(variables),
                                        move(methods)});
}

template <typename istream>
AST::ptr<AST::MethodDecl> MethodDecl(Parser<istream>& parser) {
    using std::make_unique;
    using std::move;

    ContextGuard guard(parser[0].third, "<method declaration>");
    parser.consume(Lexeme::public_keyword);
    auto type = Type(parser);
    auto name = parser.consume(Lexeme::identifier);
    auto args = FormalList(parser);
    parser.consume(Lexeme::open_brace);

    std::vector<AST::__detail::pVarDecl> vars;

    bool eating_vars = true;
    while (eating_vars) {
        switch (Lexeme(parser[0])) {
        case Lexeme::boolean_keyword:
        case Lexeme::int_keyword:
            vars.push_back(VarDecl(parser));
            break;
        case Lexeme::open_brace:
        case Lexeme::if_keyword:
        case Lexeme::while_keyword:
        case Lexeme::println_keyword:
        case Lexeme::return_keyword:
            eating_vars = false;
            break;
        case Lexeme::identifier: {
            if (Lexeme(parser[1]) == Lexeme::identifier)
                vars.push_back(VarDecl(parser));
            else
                eating_vars = false;
        } break;
        default:
            throw Unexpected{Lexeme(parser[0])};
        }
    }

    std::vector<AST::__detail::pStm> body;
    while (Lexeme(parser[0]) != Lexeme::return_keyword)
        body.push_back(Stm(parser));
    parser.consume(Lexeme::return_keyword);
    auto ret = Exp(parser);
    parser.consume(Lexeme::semicolon);
    parser.consume(Lexeme::close_brace);
    guard.active = false;
    return make_unique<AST::MethodDecl>(AST::MethodDeclRule{
        parser.make_id(), move(type), name, move(args), move(vars),
        move(body), move(ret)});
}

template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(Parser<istream>& parser) {
    ContextGuard guard(parser[0].third, "<variable declaration>");
    auto type = Type(parser);
    auto word = parser.consume(Lexeme::identifier);
    parser.consume(Lexeme::semicolon);
    guard.active = false;
    return std::make_unique<AST::VarDecl>(
        AST::VarDeclRule{parser.make_id(), std::move(type), word});
}

template <typename istream>
AST::ptr<AST::FormalList> FormalList(Parser<istream>& parser) {
    using std::make_unique;
    using std::move;

    ContextGuard guard(parser[0].third,
                       "<method declaration argument list>");
    parser.consume(Lexeme::open_paren);
    std::vector<AST::FormalDecl> list;
    while (Lexeme(parser[0]) != Lexeme::close_paren) {
        if (list.size()) parser.consume(Lexeme::comma);
        auto type = Type(parser);
        auto word = parser.consume(Lexeme::identifier);
        list.push_back(
            AST::FormalDecl{parser.make_id(), move(type), word});
    }
    parser.consume(Lexeme::close_paren);
    guard.active = false;
    return make_unique<AST::FormalList>(
        AST::FormalListRule{parser.make_id(), move(list)});
}

template <typename istream>
AST::ptr<AST::Type> Type(Parser<istream>& parser) {
    using std::make_unique;

    auto [lex, word, lc] = parser[0];
    ContextGuard guard(lc, "<type>");
    AST::ptr<AST::Type> ans;
    switch (lex) {
    case Lexeme::boolean_keyword: {
        parser.consume(Lexeme::boolean_keyword);
        ans = make_unique<AST::Type>(
            AST::booleanType{parser.make_id()});
    } break;
    case Lexeme::identifier: {
        parser.consume(Lexeme::identifier);
        ans = make_unique<AST::Type>(
            AST::classType{parser.make_id(), word});
    } break;
    case Lexeme::int_keyword: {
        parser.consume(Lexeme::int_keyword);
        if (Lexeme(parser[0]) == Lexeme::open_bracket) {
            parser.consume(Lexeme::open_bracket);
            parser.consume(Lexeme::close_bracket);
            ans = make_unique<AST::Type>(
                AST::integerArrayType{parser.make_id()});
        } else
            ans = make_unique<AST::Type>(
                AST::integerType{parser.make_id()});
    } break;
    default:
        throw Unexpected{lex};
    }
    guard.active = false;
    return ans;
}

template <typename istream>
AST::ptr<AST::Stm> Stm(Parser<istream>& parser) {
    Builder<AST::ptr<AST::Stm>> builder(parser.make_id());
    switch (Lexeme(parser[0])) {
    case Lexeme::open_brace:
        parser.consume(Lexeme::open_brace);
        while (Lexeme(parser[0]) != Lexeme::close_brace)
            builder.keep(Stm(parser));
        parser.consume(Lexeme::close_brace);
        return builder.blockStm();
    case Lexeme::if_keyword:
        parser.consume(Lexeme::if_keyword);
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        builder.keep(Stm(parser));
        parser.consume(Lexeme::else_keyword);
        return builder.keep(Stm(parser)).ifStm();
    case Lexeme::while_keyword:
        parser.consume(Lexeme::while_keyword);
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        return builder.keep(Stm(parser)).whileStm();
    case Lexeme::println_keyword:
        parser.consume(Lexeme::println_keyword);
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        parser.consume(Lexeme::semicolon);
        return builder.printStm();
    case Lexeme::identifier:
        builder.keep(parser.consume(Lexeme::identifier));
        if (Lexeme(parser[0]) == Lexeme::equals_sign) {
            parser.consume(Lexeme::equals_sign);
            builder.keep(Exp(parser));
            parser.consume(Lexeme::semicolon);
            return builder.assignStm();
        }
        parser.consume(Lexeme::open_bracket);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_bracket);
        parser.consume(Lexeme::equals_sign);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::semicolon);
        return builder.indexAssignStm();
    default:
        throw Unexpected{Lexeme(parser[0])};
    }
}

template <typename istream>
AST::ptr<AST::ExpList> ExpList(Parser<istream>& parser) {
    using std::make_unique;
    using std::move;

    std::vector<AST::__detail::pExp> list;
    ContextGuard guard(parser[0].third,
                       "<method call argument list>");
    parser.consume(Lexeme::open_paren);
    while (Lexeme(parser[0]) != Lexeme::close_paren) {
        if (list.size()) parser.consume(Lexeme::comma);
        list.push_back(Exp(parser));
    }
    parser.consume(Lexeme::close_paren);
    guard.active = false;
    return make_unique<AST::ExpList>(
        AST::ExpListRule{parser.make_id(), move(list)});
}

template <typename istream>
AST::ptr<AST::Exp> _Exp(Parser<istream>& parser,
                        AST::ptr<AST::Exp>&& lhs) {
    Builder<AST::ptr<AST::Exp>> builder(parser.make_id());
    builder.keep(std::move(lhs));
    switch (Lexeme(parser[0])) {
    case Lexeme::and_operator:
        parser.consume(Lexeme::and_operator);
        return _Exp(parser, builder.keep(Exp(parser)).andExp());
    case Lexeme::less_operator:
        parser.consume(Lexeme::less_operator);
        return _Exp(parser, builder.keep(Exp(parser)).lessExp());
    case Lexeme::plus_operator:
        parser.consume(Lexeme::plus_operator);
        return _Exp(parser, builder.keep(Exp(parser)).sumExp());
    case Lexeme::minus_operator:
        parser.consume(Lexeme::minus_operator);
        return _Exp(parser, builder.keep(Exp(parser)).minusExp());
    case Lexeme::times_operator:
        parser.consume(Lexeme::times_operator);
        return _Exp(parser, builder.keep(Exp(parser)).prodExp());
    case Lexeme::open_bracket:
        parser.consume(Lexeme::open_bracket);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_bracket);
        return _Exp(parser, builder.indexingExp());
    case Lexeme::period:
        parser.consume(Lexeme::period);
        if (Lexeme(parser[0]) == Lexeme::lenght_keyword) {
            parser.consume(Lexeme::lenght_keyword);
            return _Exp(parser, builder.lengthExp());
        }
        return _Exp(parser,
                    builder.keep(parser.consume(Lexeme::identifier))
                        .keep(ExpList(parser))
                        .methodCallExp());
    default:
        return builder.lhs();
    }
}

template <typename istream>
AST::ptr<AST::Exp> Exp(Parser<istream>& parser) {
    Builder<AST::ptr<AST::Exp>> builder(parser.make_id());
    switch (Lexeme(parser[0])) {
    case Lexeme::integer_literal:
        return _Exp(parser, builder
                                .keep(std::stoi(parser.consume(
                                    Lexeme::integer_literal)))
                                .integerExp());
    case Lexeme::true_keyword:
        parser.consume(Lexeme::true_keyword);
        return _Exp(parser, builder.trueExp());
    case Lexeme::false_keyword:
        parser.consume(Lexeme::false_keyword);
        return _Exp(parser, builder.falseExp());
    case Lexeme::identifier:
        return _Exp(parser,
                    builder.keep(parser.consume(Lexeme::identifier))
                        .identifierExp());
    case Lexeme::this_keyword:
        parser.consume(Lexeme::this_keyword);
        return _Exp(parser, builder.thisExp());
    case Lexeme::new_keyword:
        parser.consume(Lexeme::new_keyword);
        if (Lexeme(parser[0]) == Lexeme::int_keyword) {
            parser.consume(Lexeme::int_keyword);
            parser.consume(Lexeme::open_bracket);
            builder.keep(Exp(parser));
            parser.consume(Lexeme::close_bracket);
            return _Exp(parser, builder.newArrayExp());
        }
        builder.keep(parser.consume(Lexeme::identifier));
        parser.consume(Lexeme::open_paren);
        parser.consume(Lexeme::close_paren);
        return _Exp(parser, builder.newObjectExp());
    case Lexeme::bang:
        parser.consume(Lexeme::bang);
        return _Exp(parser, builder.keep(Exp(parser)).bangExp());
    case Lexeme::open_paren:
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        return _Exp(parser, builder.parenExp());
    default:
        throw Unexpected{Lexeme(parser[0])};
    }
}

} // namespace Parser
