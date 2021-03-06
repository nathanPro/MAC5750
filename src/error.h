#ifndef BCC_ASTERROR
#define BCC_ASTERROR
#include "AST.h"
#include "grammar.h"
#include "util.h"

template <typename ostream> class OstreamReporter
{
    ostream& out;

  public:
    OstreamReporter(ostream& __out) : out(__out) {}

    void operator()(const AST::ParsingError& err)
    {
        Util::write(out, "Context:");
        {
            size_t i = 0;
            if (err.ctx.size() && err.lines[0] == -1) {
                Util::write(out,
                            err.ctx[0] + std::string(":") +
                                std::to_string(err.lines.back()));
                i++;
            }
            for (; i < err.ctx.size(); i++)
                Util::write(out, "[", err.lines[i], "]", err.ctx[i]);
        }
        std::visit(*this, err.inner);
    }

    void operator()(const AST::Unexpected& err)
    {
        Util::write(out, "Unexpected lexeme: '", err.lex, "'");
    }

    void operator()(const AST::Mismatch& err)
    {
        Util::write(out, "Mismatch: expected '", err.expected,
                    "', found '", err.found);
    }

    void operator()(const AST::WrongIdentifier& err)
    {
        Util::write(out, "Identifier mismatch: expected '",
                    err.expected, "', found '", err.found);
    }
};

template <typename ostream> class Reporter
{
    ostream&                     out;
    OstreamReporter<ostream>     logger;
    std::vector<AST::ErrorData>& E;
    bool                         ok;

  public:
    Reporter(ostream& __out, std::vector<AST::ErrorData>& err)
        : out(__out), logger(out), E(err), ok(true)
    {
    }

    operator bool() const { return ok; }

    void operator()(const AST::ProgramRule& prog)
    {
        Grammar::visit(*this, prog.main);
        for (auto& c : prog.classes) Grammar::visit(*this, c);
        for (auto& err : E[prog.id]) logger(*err);
        if (E[prog.id].size()) ok = false;
    }

    void operator()(const AST::MainClassRule& main)
    {
        Grammar::visit(*this, main.body);
        for (auto& err : E[main.id]) {
            Util::write(out, "At main class");
            logger(*err);
        }
        if (E[main.id].size()) ok = false;
    }

    void operator()(const AST::ClassDeclNoInheritance& cls)
    {
        for (auto& v : cls.variables) Grammar::visit(*this, v);
        for (auto& m : cls.methods) Grammar::visit(*this, m);
        for (auto& err : E[cls.id]) {
            Util::write(out, "At class ", cls.name);
            logger(*err);
        }
        if (E[cls.id].size()) ok = false;
    }

    void operator()(const AST::ClassDeclInheritance& cls)
    {
        for (auto& v : cls.variables) Grammar::visit(*this, v);
        for (auto& m : cls.methods) Grammar::visit(*this, m);
        for (auto& err : E[cls.id]) {
            Util::write(out, "At class", cls.name);
            logger(*err);
        }
        if (E[cls.id].size()) ok = false;
    }

    void operator()(const AST::VarDeclRule& vd)
    {
        Grammar::visit(*this, vd.type);
        for (auto& err : E[vd.id]) logger(*err);
        if (E[vd.id].size()) ok = false;
    }

    void operator()(const AST::MethodDeclRule& mtd)
    {
        Grammar::visit(*this, mtd.type);
        Grammar::visit(*this, mtd.arguments);
        for (auto& v : mtd.variables) Grammar::visit(*this, v);
        for (auto& s : mtd.body) Grammar::visit(*this, s);
        Grammar::visit(*this, mtd.return_exp);
        for (auto& err : E[mtd.id]) logger(*err);
        if (E[mtd.id].size()) ok = false;
    }

    void operator()(const AST::blockStm& blk)
    {
        for (auto& s : blk.statements) Grammar::visit(*this, s);
        for (auto& err : E[blk.id]) logger(*err);
        if (E[blk.id].size()) ok = false;
    }

    void operator()(const AST::ifStm& ifs)
    {
        Grammar::visit(*this, ifs.condition);
        Grammar::visit(*this, ifs.if_clause);
        Grammar::visit(*this, ifs.else_clause);
        for (auto& err : E[ifs.id]) logger(*err);
        if (E[ifs.id].size()) ok = false;
    }

    void operator()(const AST::whileStm& stm)
    {
        Grammar::visit(*this, stm.condition);
        Grammar::visit(*this, stm.body);
        for (auto& err : E[stm.id]) logger(*err);
        if (E[stm.id].size()) ok = false;
    }

    void operator()(const AST::printStm& stm)
    {
        Grammar::visit(*this, stm.exp);
        for (auto& err : E[stm.id]) logger(*err);
        if (E[stm.id].size()) ok = false;
    }

    void operator()(const AST::assignStm& stm)
    {
        Grammar::visit(*this, stm.value);
        for (auto& err : E[stm.id]) logger(*err);
        if (E[stm.id].size()) ok = false;
    }

    void operator()(const AST::indexAssignStm& stm)
    {
        Grammar::visit(*this, stm.index);
        Grammar::visit(*this, stm.value);
        for (auto& err : E[stm.id]) logger(*err);
        if (E[stm.id].size()) ok = false;
    }

    void operator()(const AST::FormalListRule& lst)
    {
        for (auto& err : E[lst.id]) logger(*err);
        if (E[lst.id].size()) ok = false;
    }

    template <typename T> void operator()(const T& leaf)
    {
        for (auto& err : E[leaf.id]) logger(*err);
        if (E[leaf.id].size()) ok = false;
    }
};

#endif
