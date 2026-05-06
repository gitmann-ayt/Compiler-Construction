#include <cctype>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

static void print_stack(std::stack<long long> st) {
    std::vector<long long> v;
    while (!st.empty()) {
        v.push_back(st.top());
        st.pop();
    }
    std::cout << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[v.size() - 1 - i];
    }
    std::cout << "]";
}

static bool is_int(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

int main() {
    std::string line;
    if (!std::getline(std::cin, line)) return 0;

    std::istringstream iss(line);
    std::stack<long long> st;

    std::string tok;
    while (iss >> tok) {
        if (is_int(tok)) {
            long long v = std::stoll(tok);
            st.push(v);
            std::cout << "push " << v << "\t";
            print_stack(st);
            std::cout << "\n";
            continue;
        }

        if (tok == "+" || tok == "-" || tok == "*") {
            if (st.size() < 2) {
                std::cerr << "error: not enough operands for operator '" << tok << "'\n";
                return 1;
            }
            long long b = st.top(); st.pop();
            long long a = st.top(); st.pop();
            long long r = 0;
            if (tok == "+") r = a + b;
            else if (tok == "-") r = a - b;
            else r = a * b;
            std::cout << "pop " << a << ", pop " << b << ", apply " << tok << ", push " << r << "\t";
            st.push(r);
            print_stack(st);
            std::cout << "\n";
            continue;
        }

        std::cerr << "error: invalid token in postfix expression: '" << tok << "'\n";
        return 1;
    }

    if (st.size() != 1) {
        std::cerr << "error: invalid postfix expression (stack has " << st.size() << " items)\n";
        return 1;
    }

    std::cout << "result = " << st.top() << "\n";
    return 0;
}
