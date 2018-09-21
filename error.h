#ifndef BCC_ASTERROR
#define BCC_ASTERROR
#include "Builder.h"
#include "grammar.h"

template <typename ostream> class OstreamReport {
    ostream& out;

  public:
    OstreamReport(ostream& __out) : out(__out) {}

    void operator()(const ParsingError& err) {
        out << "Context:\n";
        for (auto& c : err.ctx) out << c << '\n';
        out << '\n';
        std::visit(*this, err.inner);
    }

    void operator()(const Unexpected& err) {
        out << "Unexpected lexeme: " << err.lex << "\n";
    }

    void operator()(const Mismatch& err) {
        out << "Mismatch: expected " << err.expected << ", found "
            << err.found << "\n";
    }

    void operator()(const WrongIdentifier& err) {
        out << "Identifier mismatch: expected " << err.expected
            << ", found " << err.found << "\n";
    }
};

template <typename ostream> class Reporter {
    ostream& out;
    std::vector<ASTErrorData>& E;

  public:
    Reporter(ostream& __out, std::vector<ASTErrorData>& err)
        : out(__out), E(err) {}

    void operator()(const AST::ProgramRule& prog) {
        Grammar::visit(*this, *prog.main);
        for (auto& c : prog.classes) Grammar::visit(*this, *c);
        for (auto& err : E[prog.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::MainClassRule& main) {
        Grammar::visit(*this, *main.body);
        for (auto& err : E[main.id.get()]) {
            out << "At main class\n";
            OstreamReport(out)(*err);
        }
    }

    void operator()(const AST::ClassDeclNoInheritance& cls) {
        for (auto& v : cls.variables) Grammar::visit(*this, *v);
        for (auto& m : cls.methods) Grammar::visit(*this, *m);
        for (auto& err : E[cls.id.get()]) {
            out << "At class " << cls.name << "\n";
            OstreamReport(out)(*err);
        }
    }

    void operator()(const AST::ClassDeclInheritance& cls) {
        for (auto& v : cls.variables) Grammar::visit(*this, *v);
        for (auto& m : cls.methods) Grammar::visit(*this, *m);
        for (auto& err : E[cls.id.get()]) {
            out << "At class " << cls.name << "\n";
            OstreamReport(out)(*err);
        }
    }

    void operator()(const AST::VarDeclRule& vd) {
        Grammar::visit(*this, *vd.type);
        for (auto& err : E[vd.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::MethodDeclRule& mtd) {
        Grammar::visit(*this, *mtd.type);
        Grammar::visit(*this, *mtd.arguments);
        for (auto& v : mtd.variables) Grammar::visit(*this, *v);
        for (auto& s : mtd.body) Grammar::visit(*this, *s);
        Grammar::visit(*this, *mtd.return_exp);
        for (auto& err : E[mtd.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::blockStm& blk) {
        for (auto& s : blk.statements) Grammar::visit(*this, *s);
        for (auto& err : E[blk.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::ifStm& ifs) {
        Grammar::visit(*this, *ifs.condition);
        Grammar::visit(*this, *ifs.if_clause);
        Grammar::visit(*this, *ifs.else_clause);
        for (auto& err : E[ifs.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::whileStm& stm) {
        Grammar::visit(*this, *stm.condition);
        Grammar::visit(*this, *stm.body);
        for (auto& err : E[stm.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::printStm& stm) {
        Grammar::visit(*this, *stm.exp);
        for (auto& err : E[stm.id.get()]) OstreamReport(out)(*err);
    }

    void operator()(const AST::indexAssignStm& stm) {
        Grammar::visit(*this, *stm.index);
        Grammar::visit(*this, *stm.value);
        for (auto& err : E[stm.id.get()]) OstreamReport(out)(*err);
    }

    template <typename T> void operator()(const T& leaf) {
        for (auto& err : E[leaf.id.get()]) OstreamReport(out)(*err);
    }
};

#endif
