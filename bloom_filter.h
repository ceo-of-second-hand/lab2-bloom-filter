// Фільтр Блума з лічильниками; хеш-функції з класу H_pm.
#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

class BloomFilter {
public:
    static const int MAX_LEN = 15;
    static const int MAX_L = 32;

    BloomFilter(long long n, double p, uint64_t seed) {
        const double ln2 = std::log(2.0);
        m_ = (uint64_t)std::ceil(-n * std::log(p) / (ln2 * ln2));    // (4.1)
        l_ = (int)std::lround((double)m_ / n * ln2);                 // (4.2)
        if (l_ < 1) l_ = 1;
        if (l_ > MAX_L) l_ = MAX_L;
        cnt_.assign(m_, 0);

        // випадковий вибір функцій з H_pm: a_i з 1..P-1, b_i з 0..P-1
        std::mt19937_64 rng(seed);
        base_ = 53 + rng() % (P - 53);  // основа більша за кількість символів (52)
        for (int i = 0; i < l_; ++i) {
            a_[i] = 1 + rng() % (P - 1);
            b_[i] = rng() % P;
        }
    }

    // l позицій рядка у масиві лічильників
    void positions(const char* s, int len, uint64_t* pos) const {
        uint64_t k = key(s, len);
        for (int i = 0; i < l_; ++i) pos[i] = mulAddMod(a_[i], k, b_[i]) % m_;
    }

    // InsBlF: +1 на кожній з l позицій
    void add(const uint64_t* pos) {
        for (int i = 0; i < l_; ++i)
            if (cnt_[pos[i]] < MAX_COUNT) ++cnt_[pos[i]];
    }

    // ScinBlF: є нуль хоча б на одній позиції -> рядка точно немає
    bool contains(const uint64_t* pos) const {
        for (int i = 0; i < l_; ++i)
            if (cnt_[pos[i]] == 0) return false;
        return true;
    }

    // вилучення: -1 на кожній позиції
    void remove(const uint64_t* pos) {
        for (int i = 0; i < l_; ++i)
            if (cnt_[pos[i]] < MAX_COUNT) --cnt_[pos[i]];
    }

    int counter(uint64_t pos) const { return cnt_[pos]; }
    uint64_t m() const { return m_; }
    int l() const { return l_; }

private:
    static const uint64_t P = (1ULL << 61) - 1;
    static const uint8_t MAX_COUNT = 255;

    uint64_t m_;
    int l_;
    uint64_t base_;
    uint64_t a_[MAX_L], b_[MAX_L];
    std::vector<uint8_t> cnt_;

    // (a * x + b) mod P; 2^61 ≡ 1 (mod P), тому замість ділення — додавання частин
    static uint64_t mulAddMod(uint64_t a, uint64_t x, uint64_t b) {
        unsigned __int128 t = (unsigned __int128)a * x + b;
        uint64_t r = (uint64_t)(t & P) + (uint64_t)(t >> 61);
        r = (r & P) + (r >> 61);
        return r >= P ? r - P : r;
    }

    // код символу: 'a'..'z' -> 1..26, 'A'..'Z' -> 27..52 (регістр розрізняємо)
    static uint64_t code(char c) {
        return c >= 'a' ? (uint64_t)(c - 'a' + 1) : (uint64_t)(c - 'A' + 27);
    }

    // поліноміальний хеш за схемою Горнера: (...(c0 * base + c1) * base + ...) mod P
    uint64_t key(const char* s, int len) const {
        uint64_t h = 0;
        for (int i = 0; i < len; ++i) h = mulAddMod(h, base_, code(s[i]));
        return h;
    }
};
