// Vita3K emulator project
// Copyright (C) 2024 Vita3K team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#pragma once

#include <mem/atomic.h>
#include <mem/functions.h>
#include <mem/state.h>

template <class T>
class Ptr {
public:
    Ptr()
        : addr(0) {
    }

    explicit Ptr(Address address)
        : addr(address) {
    }

    template <class U>
    Ptr(const Ptr<U> &other)
        : addr(other.address()) {
        static_assert(std::is_convertible_v<U *, T *>, "Ptr is not convertible.");
    }

    template <class U>
    [[deprecated]] Ptr(U *pointer, const MemState &mem) {
        const uint8_t *const pointer_bytes = reinterpret_cast<const uint8_t *>(pointer);
        if (pointer_bytes == nullptr) {
            addr = 0;
        } else { /*
             if (mem.use_page_table) {
                 auto new_addr = static_cast<Address>(pointer_bytes - &mem.memory[0]);
                 if (mem.page_table[new_addr / KiB(4)] + new_addr != pointer_bytes) {
                     size_t pointer_value = bit_cast<size_t>(pointer);
                     // auto idx = pointer_value / KiB(4); // next line needs check and fix
                     //  if (mem.page_table[idx] != &mem.memory[0] + idx) {
                     for (auto &[ptr, mapping] : mem.external_mapping) {
                         if (pointer_value >= ptr && pointer_value < ptr + mapping.size) {
                             addr = static_cast<Address>(pointer_value - ptr + mapping.address);
                             return;
                         }
                     }
                     //  }
                 } else {
                     addr = static_cast<Address>(pointer_bytes - &mem.memory[0]);
                 }*/
            //            } else {
            addr = static_cast<Address>(pointer_bytes - &mem.memory[0]);
            //          }
        }
    }

    Address address() const {
        return addr;
    }

    template <class U>
    Ptr<U> cast() const {
        return Ptr<U>(addr);
    }

    T *get(const MemState &mem) const {
        if (addr == 0) {
            return nullptr;
        } else if (mem.use_page_table) {
            return reinterpret_cast<T *>(mem.page_table[addr / KiB(4)] + addr);
        } else {
            return reinterpret_cast<T *>(&mem.memory[addr]);
        }
    }

    template <class U>
    bool atomic_compare_and_swap(MemState &mem, U value, U expected) {
        static_assert(std::is_arithmetic_v<U>);
        static_assert(std::is_same_v<U, T>);
        uint8_t *mem_ptr = mem.use_page_table ? mem.page_table[addr / KiB(4)] : mem.memory.get();
        const auto ptr = reinterpret_cast<volatile U *>(&mem_ptr[addr]);
        return ::atomic_compare_and_swap(ptr, value, expected);
    }

    bool valid(const MemState &mem) const {
        return is_valid_addr(mem, addr);
    }

    void reset() {
        addr = 0;
    }

    explicit operator bool() const {
        return addr != 0;
    }

    Ptr &operator=(const Address &address) {
        addr = address;
        return *this;
    }

    Ptr &operator=(std::nullptr_t) {
        addr = 0;
        return *this;
    }

private:
    Address addr;
};

static_assert(sizeof(Ptr<const void>) == 4, "Size of Ptr isn't 4 bytes.");

template <class T>
Ptr<T> operator+(const Ptr<T> &base, int32_t offset) {
    return Ptr<T>(base.address() + (offset * sizeof(T)));
}

template <class T, class U>
bool operator<(const Ptr<T> &a, const Ptr<U> &b) {
    return a.address() < b.address();
}

template <class T>
bool operator==(const Ptr<T> &a, const Ptr<T> &b) {
    return a.address() == b.address();
}

template <class T>
Ptr<T> alloc(MemState &mem, const char *name) {
    const Address address = alloc(mem, sizeof(T), name);
    const Ptr<T> ptr(address);
    if (!ptr) {
        return ptr;
    }

    T *const memory = ptr.get(mem);
    new (memory) T;

    return ptr;
}

template <class T>
void free(MemState &mem, const Ptr<T> &ptr) {
    ptr.get(mem)->~T();
    free(mem, ptr.address());
}
