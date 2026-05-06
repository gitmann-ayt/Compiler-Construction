#include "ast.h"
#include "c_emitter.h"
#include "lexer.h"
#include "optimizer.h"
#include "parser.h"
#include "semantics.h"
#include "tac.h"
#include "tac_interpreter.h"
#include "util.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::string read_all(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("failed to open input file: " + path);
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return s;
}

void write_all(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("failed to write file: " + path);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

bool has_flag(const std::vector<std::string>& args, const std::string& f) {
    for (auto const& a : args) if (a == f) return true;
    return false;
}

std::string get_arg_value(const std::vector<std::string>& args, const std::string& key) {
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == key) return args[i + 1];
    }
    return {};
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

        std::string inputPath = get_arg_value(args, "--input");
        if (inputPath.empty()) {
            std::cerr << "Usage: compiler.exe --input <file> [--dump-tokens] [--dump-ast] [--dump-tac] [--opt] [--perf] [--emit-c out.c]\n";
            return 2;
        }

        std::string source = read_all(inputPath);

        if (has_flag(args, "--dump-tokens")) {
            cc::Lexer lx(source);
            while (true) {
                cc::Token t = lx.next();
                std::cout << cc::to_string(t.loc) << "\t" << cc::token_kind_name(t.kind);
                if (!t.lexeme.empty()) std::cout << "\t" << t.lexeme;
                if (t.kind == cc::TokenKind::IntLiteral || t.kind == cc::TokenKind::FloatLiteral) {
                    std::cout << "\t" << t.numberValue;
                }
                std::cout << "\n";
                if (t.kind == cc::TokenKind::End) break;
                if (t.kind == cc::TokenKind::Invalid) {
                    throw std::runtime_error(cc::to_string(t.loc) + ": invalid token: '" + t.lexeme + "'");
                }
            }
        }

        cc::Parser parser{cc::Lexer(source)};
        auto program = parser.parse_program();

        cc::SemanticAnalyzer sema;
        sema.analyze(*program);

        if (has_flag(args, "--dump-ast")) {
            std::cout << "\n=== AST ===\n";
            std::cout << cc::dump_ast(*program);
        }

        cc::TacGenerator gen;
        cc::TacProgram tac = gen.generate(*program);

        if (has_flag(args, "--dump-tac")) {
            std::cout << "\n=== TAC (before) ===\n";
            for (auto const& ins : tac.code) std::cout << ins.to_string() << "\n";
        }

        cc::TacProgram optTac = tac;
        if (has_flag(args, "--opt")) {
            cc::Optimizer opt;
            optTac = opt.optimize(tac, cc::OptimizerOptions{});

            std::cout << "\n=== TAC (after opt) ===\n";
            for (auto const& ins : optTac.code) std::cout << ins.to_string() << "\n";
        }

        if (has_flag(args, "--perf")) {
            // Measure TAC interpreter runtime (5 runs), before vs after optimisations.
            constexpr int runs = 5;
            cc::TacInterpreter interp;

            cc::Stopwatch sw;
            double r1 = 0.0;
            sw.reset();
            for (int i = 0; i < runs; ++i) r1 = interp.run(tac);
            auto t1 = sw.elapsed_ms();

            double r2 = 0.0;
            sw.reset();
            for (int i = 0; i < runs; ++i) r2 = interp.run(optTac);
            auto t2 = sw.elapsed_ms();

            std::cout << "\n=== Performance (TAC interpreter) ===\n";
            std::cout << "result(before) = " << r1 << ", time = " << t1 << " ms for " << runs << " run(s)\n";
            std::cout << "result(after)  = " << r2 << ", time = " << t2 << " ms for " << runs << " run(s)\n";
            if (t2 > 0) {
                double speedup = static_cast<double>(t1) / static_cast<double>(t2);
                std::cout << "speedup = " << speedup << "x\n";
            }
        }

        std::string outC = get_arg_value(args, "--emit-c");
        if (!outC.empty()) {
            cc::CEmitter emitter;
            std::string csrc = emitter.emit(*program);
            write_all(outC, csrc);
            std::cout << "\nWrote C to: " << outC << "\n";
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
