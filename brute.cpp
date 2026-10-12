// Точний розв'язок на std::unordered_set для перевірки.

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

static bool validWord(const std::string& w) {
    if (w.empty() || w.size() > 15) return false;
    for (char c : w)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return false;
    return true;
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::ifstream fin;
    std::ofstream fout;
    if (argc >= 2) fin.open(argv[1], std::ios::binary);
    if (argc >= 3) fout.open(argv[2], std::ios::binary);
    std::istream& in = argc >= 2 ? static_cast<std::istream&>(fin) : std::cin;
    std::ostream& out = argc >= 3 ? static_cast<std::ostream&>(fout) : std::cout;

    std::unordered_set<std::string> set;
    std::string op, word;
    while (in >> op) {
        if (op == "#") break;
        if (!(in >> word)) break;
        if (op.size() != 1 || !validWord(word)) continue;
        if (op[0] == '+')
            set.insert(word);
        else if (op[0] == '-')
            set.erase(word);
        else if (op[0] == '?')
            out << (set.count(word) ? "Y\n" : "N\n");
    }
    return 0;
}
