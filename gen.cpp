// Генератор вхідних даних (параметри — у README).

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

static std::mt19937_64 rng;

static long long rnd(long long k) { return (long long)(rng() % (unsigned long long)k); }

// слово довжиною 1..15 з великих і малих латинських літер
static std::string randomWord() {
    static const char LETTERS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string w(1 + rnd(15), ' ');
    for (char& c : w) c = LETTERS[rnd(52)];
    return w;
}

static void addLine(std::string& out, char op, const std::string& w) {
    out += op;
    out += ' ';
    out += w;
    out += '\n';
}

static void usage() {
    std::fprintf(stderr, "usage: gen mix N seed [add del ask] [pool]\n"
                         "       gen fp ADD ASK seed\n");
}

int main(int argc, char** argv) {
    if (argc < 2) return usage(), 1;
    std::string out;

    if (std::strcmp(argv[1], "mix") == 0 && argc >= 4) {
        long long n = std::atoll(argv[2]);
        rng.seed(std::strtoull(argv[3], nullptr, 10));
        int wAdd = 50, wDel = 20, wAsk = 30;
        if (argc >= 7) {
            wAdd = std::atoi(argv[4]);
            wDel = std::atoi(argv[5]);
            wAsk = std::atoi(argv[6]);
        }
        long long pool = argc >= 8 ? std::atoll(argv[7]) : n / 3;
        if (pool < 1) pool = 1;
        if (wAdd + wDel + wAsk <= 0) {
            std::fprintf(stderr, "operation weights must not all be zero\n");
            return 1;
        }
        std::vector<std::string> words(pool);
        for (std::string& w : words) w = randomWord();

        out.reserve((size_t)n * 12 + 4);
        int total = wAdd + wDel + wAsk;
        for (long long i = 0; i < n; ++i) {
            int r = (int)rnd(total);
            addLine(out, r < wAdd ? '+' : r < wAdd + wDel ? '-' : '?', words[rnd(pool)]);
        }
    } else if (std::strcmp(argv[1], "fp") == 0 && argc >= 5) {
        long long nAdd = std::atoll(argv[2]);
        long long nAsk = std::atoll(argv[3]);
        rng.seed(std::strtoull(argv[4], nullptr, 10));

        // усі слова різні: перші nAdd додаємо, решту лише запитуємо
        std::unordered_set<std::string> used;
        used.reserve((size_t)(nAdd + nAsk) * 2);
        std::vector<std::string> words;
        words.reserve((size_t)(nAdd + nAsk));
        while ((long long)words.size() < nAdd + nAsk) {
            std::string w = randomWord();
            if (used.insert(w).second) words.push_back(w);
        }
        out.reserve((size_t)(nAdd + nAsk) * 12 + 4);
        for (long long i = 0; i < nAdd; ++i) addLine(out, '+', words[i]);
        for (long long i = 0; i < nAsk; ++i) addLine(out, '?', words[nAdd + i]);
    } else {
        return usage(), 1;
    }

    out += "#\n";
    std::fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
