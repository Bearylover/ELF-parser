#pragma once
#include <cstdint>

enum class ELFClass : uint8_t {
    ELF32 = 1,
    ELF64 = 2
};

enum class Endian : uint8_t {
    Little = 1,
    Big = 2
};

enum class Version : uint32_t {
    Current = 1,
    Invalid = 0
};

struct ELFHeader {
    ELFClass elf_class;
    Endian endian;
    uint8_t ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t program_offset;
    uint64_t section_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_entry_size;
    uint16_t program_count;
    uint16_t section_entry_size;
    uint16_t section_count;
    uint16_t section_string_index;
};

struct SectionHeader {
    
};