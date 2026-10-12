// Лабораторна 2: фільтр Блума. Запуск: lab [input [output]] [--trace]

#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

#include "bloom_filter.h"

// швидке посимвольне читання
#ifdef _WIN32
#define GETC _getc_nolock
#else
#define GETC getc_unlocked
#endif

static const long long N_MAX = 1000000;  // максимальна кількість елементів (умова)
static const double P_FALSE = 0.01;      // ймовірність хибнопозитивного спрацювання (умова)
static const uint64_t SEED = 2026;       // зерно вибору хеш-функцій

// читає наступне слово; повертає довжину або -1 в кінці файлу
static int readToken(FILE* in, char* buf, int cap) {
    int c = GETC(in);
    while (c == ' ' || c == '\n' || c == '\r' || c == '\t') c = GETC(in);
    if (c == EOF) return -1;
    int n = 0;
    while (c != EOF && c != ' ' && c != '\n' && c != '\r' && c != '\t') {
        if (n < cap) buf[n] = (char)c;
        ++n;
        c = GETC(in);
    }
    return n;
}

static bool validWord(const char* w, int len) {
    if (len < 1 || len > BloomFilter::MAX_LEN) return false;
    for (int i = 0; i < len; ++i)
        if (!((w[i] >= 'a' && w[i] <= 'z') || (w[i] >= 'A' && w[i] <= 'Z'))) return false;
    return true;
}

static void printCounters(const BloomFilter& f, const uint64_t* pos) {
    for (int i = 0; i < f.l(); ++i) std::fprintf(stderr, " %d", f.counter(pos[i]));
}

int main(int argc, char** argv) {
    bool trace = false;
    std::vector<const char*> files;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--trace") == 0)
            trace = true;
        else
            files.push_back(argv[i]);
    }

    FILE* in = stdin;
    FILE* out = stdout;
    if (files.size() >= 1 && !(in = std::fopen(files[0], "rb"))) {
        std::fprintf(stderr, "cannot open %s\n", files[0]);
        return 1;
    }
    if (files.size() >= 2 && !(out = std::fopen(files[1], "wb"))) {
        std::fprintf(stderr, "cannot open %s\n", files[1]);
        return 1;
    }
    static char inBuf[1 << 16], outBuf[1 << 16];
    std::setvbuf(in, inBuf, _IOFBF, sizeof inBuf);
    std::setvbuf(out, outBuf, _IOFBF, sizeof outBuf);

    auto start = std::chrono::steady_clock::now();

    BloomFilter filter(N_MAX, P_FALSE, SEED);
    std::fprintf(stderr, "n=%lld p=%.2f m=%llu l=%d\n", N_MAX, P_FALSE,
                 (unsigned long long)filter.m(), filter.l());

    char op[8], word[64];
    uint64_t pos[BloomFilter::MAX_L];
    long long lineNo = 0, yes = 0, no = 0;
    for (;;) {
        int opLen = readToken(in, op, sizeof op);
        if (opLen == -1) break;
        ++lineNo;
        if (opLen == 1 && op[0] == '#') break;  // кінець вхідних даних

        int len = readToken(in, word, sizeof word);
        if (opLen != 1 || len == -1 || !validWord(word, len) ||
            (op[0] != '+' && op[0] != '-' && op[0] != '?')) {
            std::fprintf(stderr, "line %lld: invalid operation, skipped\n", lineNo);
            continue;
        }

        filter.positions(word, len, pos);
        bool present = filter.contains(pos);
        if (trace) {
            std::fprintf(stderr, "\n[%lld] %c %.*s\n    positions:", lineNo, op[0], len, word);
            for (int i = 0; i < filter.l(); ++i)
                std::fprintf(stderr, " %llu", (unsigned long long)pos[i]);
            std::fprintf(stderr, "\n    counters :");
            printCounters(filter, pos);
        }

        const char* result = "";
        switch (op[0]) {
            case '+':
                // множина: повторне додавання не змінює фільтр
                if (!present) filter.add(pos);
                result = present ? "already present, skipped" : "added";
                break;
            case '-':
                // вилучаємо лише те, що фільтр вважає наявним
                if (present) filter.remove(pos);
                result = present ? "removed" : "not present, skipped";
                break;
            case '?':
                std::fputs(present ? "Y\n" : "N\n", out);
                ++(present ? yes : no);
                result = present ? "all counters > 0 -> Y" : "some counter = 0 -> N";
                break;
        }

        if (trace) {
            if ((op[0] == '+' && !present) || (op[0] == '-' && present)) {
                std::fprintf(stderr, "  ->");
                printCounters(filter, pos);
            }
            std::fprintf(stderr, "   (%s)\n", result);
        }
    }
    std::fflush(out);

    double ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - start).count();
    if (trace) std::fputc('\n', stderr);
    std::fprintf(stderr, "lines=%lld answers: Y=%lld N=%lld time_ms=%.2f\n", lineNo, yes, no,
                 ms);

    if (in != stdin) std::fclose(in);
    if (out != stdout) std::fclose(out);
    return 0;
}
