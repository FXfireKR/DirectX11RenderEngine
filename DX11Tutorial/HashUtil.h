#pragma once

#include <cstdint>
#include <string_view>

// hash-base
constexpr uint64_t FNV_OFFSET_BASIS_64 = 14695981039346656037ull;
constexpr uint64_t FNV_PRIME_64 = 1099511628211ull;

constexpr uint64_t fnv1a_64(std::string_view s)
{
    uint64_t hash = FNV_OFFSET_BASIS_64;
    for (unsigned char c : s) {
        hash ^= c;
        hash *= FNV_PRIME_64;
    }
    return hash;
}

constexpr uint64_t operator""_sid(const char* s, size_t n) 
{
    uint64_t hash = FNV_OFFSET_BASIS_64;

    for (size_t i = 0; i < n; ++i) 
    {
        hash ^= static_cast<unsigned char>(s[i]);
        hash *= FNV_PRIME_64;
    }
    return hash;
}