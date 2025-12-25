#pragma once

#include <sstream>
#include <string>
#include <cstddef>

namespace utl {

    struct field_description {
        const char* const name;
        const size_t offset;
        const size_t size;
    };

    /*-------------------------------------------------------------*
    *  Dump Bin                                                    *
    *-------------------------------------------------------------*/
    template<typename T>
    static inline std::string dump_bin(const T& obj) {
        const size_t size_in_bytes{ sizeof(T) };
        //const size_t alignment_in_bytes{ alignof(T) };
        //const uintptr_t address{ reinterpret_cast<uintptr_t>(&obj) };

        const char* const nibbles[] = {
            "0000", "0001", "0010", "0011",
            "0100", "0101", "0110", "0111",
            "1000", "1001", "1010", "1011",
            "1100", "1101", "1110", "1111"
        };

        std::string str{};
        //str.resize((address & 0xF) * 8, ' ');

        for (size_t ind{ size_in_bytes - 1 }; ind >= 0 && ind <= size_in_bytes; ind--) {
            const unsigned char* const byte{ reinterpret_cast<const unsigned char* const>(&obj) + ind };

            str += nibbles[*byte >> 4]; str += " ";
            str += nibbles[*byte & 0xF]; if (ind > 0) str += " ";
        }

        //if (!str.empty() && str.back() == ' ') { str.pop_back(); }

        return str;
    }

    /*-------------------------------------------------------------*
    *  Dump Hex                                                    *
    *-------------------------------------------------------------*/
    template<typename T>
    static inline std::string dump_hex(const T& obj) {
        const size_t size_in_bytes{ sizeof(T) };
        //const size_t alignment_in_bytes{ alignof(T) };
        //const uintptr_t address{ reinterpret_cast<uintptr_t>(&obj) };

        const char* const nibbles[] = {
            "0", "1", "2", "3",
            "4", "5", "6", "7",
            "8", "9", "A", "B",
            "C", "D", "E", "F"
        };

        std::string str{};
        //str.resize((address & 0xF), ' ');

        for (size_t ind{ size_in_bytes - 1 }; ind >= 0 && ind <= size_in_bytes; ind--) {
            const unsigned char* const byte{ reinterpret_cast<const unsigned char* const>(&obj) + ind };

            str += nibbles[*byte >> 4];
            str += nibbles[*byte & 0xF]; if (ind > 0) str += " ";
        }

        //if (!str.empty() && str.back() == ' ') { str.pop_back(); }

        return str;
    }

}