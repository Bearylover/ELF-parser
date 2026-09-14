#include "ELFTypes.h"
#include "ELFPrint.h"

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <vector>

bool read_bytes(std::ifstream& file, void* destination, std::size_t size) {
    if (!file.read(reinterpret_cast<char*>(destination), size)) {
        std::cerr << "Unexpected EOF\n";
        return false;
    }
    return true;
}

std::optional<ELFClass> parse_class(uint8_t val) {
    if (val != static_cast<uint8_t>(ELFClass::ELF32) && val != static_cast<uint8_t>(ELFClass::ELF64)) {
        return std::nullopt;
    }
    return static_cast<ELFClass>(val);
}

std::optional<Endian> parse_endian(uint8_t val) {
    if (val != static_cast<uint8_t>(Endian::Little)) {
        return std::nullopt;
    }
    return static_cast<Endian>(val);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: elfinspect <file>\n";
        return 1;
    }
    std::ifstream file (argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file\n";
        return 1;
    }

    ELFHeader header;

    if (!read_bytes(file, header.ident, sizeof(header.ident))) return 1;
    if (header.ident[0] != 0x7F || header.ident[1] != 'E' || header.ident[2] != 'L' || header.ident[3] != 'F') {
        std::cerr << "Not an ELF file\n";
        return 1;
    }

    auto elf_class = parse_class(header.ident[4]);
    if (!elf_class) {
        std::cerr << "Invalid ELF Class\n";
        return 1;
    }
    header.elf_class = *elf_class;

    auto endian = parse_endian(header.ident[5]);
    if (!endian) {
        std::cerr << "Invalid Endianness\n";
        return 1;
    }
    header.endian = *endian;

    if (!read_bytes(file, &header.type, sizeof(header.type))) return 1;
    if (!read_bytes(file, &header.machine, sizeof(header.machine))) return 1;
    if (!read_bytes(file, &header.version, sizeof(header.version))) return 1;

    if (header.version == static_cast<uint32_t>(Version::Invalid)) {
        std::cerr << "Invalid ELF Version\n";
        return 1;
    } else if (header.version != static_cast<uint32_t>(Version::Current)) {
        std::cerr << "Unsupported ELF Version \n";
        return 1;
    }

    uint32_t e_entry32;
    if (header.elf_class == ELFClass::ELF32) {
        if (!read_bytes(file, &e_entry32, sizeof(e_entry32))) return 1;
        header.entry = e_entry32;
    } else {
        if (!read_bytes(file, &header.entry, sizeof(header.entry))) return 1;
    }

    uint32_t e_phoff32;
    if (header.elf_class == ELFClass::ELF32) {
        if (!read_bytes(file, &e_phoff32, sizeof(e_phoff32))) return 1;
        header.program_offset = e_phoff32;
    } else {
        if (!read_bytes(file, &header.program_offset, sizeof(header.program_offset))) return 1;
    }

    uint32_t e_shoff32;
    if (header.elf_class == ELFClass::ELF32) {
        if (!read_bytes(file, &e_shoff32, sizeof(e_shoff32))) return 1;
        header.section_offset = e_shoff32;
    } else {
        if (!read_bytes(file, &header.section_offset, sizeof(header.section_offset))) return 1;
    }

    if (!read_bytes(file, &header.flags, sizeof(header.flags))) return 1;
    if (!read_bytes(file, &header.header_size, sizeof(header.header_size))) return 1;
    if (!read_bytes(file, &header.program_entry_size, sizeof(header.program_entry_size))) return 1;
    if (!read_bytes(file, &header.program_count, sizeof(header.program_count))) return 1;
    if (!read_bytes(file, &header.section_entry_size, sizeof(header.section_entry_size))) return 1;
    if (!read_bytes(file, &header.section_count, sizeof(header.section_count))) return 1;
    if (!read_bytes(file, &header.section_string_index, sizeof(header.section_string_index))) return 1;

    uint64_t offset;
    std::vector<SectionHeader> sections(header.section_count);

    for (uint16_t i = 0; i < header.section_count; ++i) {
        offset = header.section_offset + i * header.section_entry_size;
        file.seekg(offset);
        if (!file) {
            std::cerr << "Failed to seek section " << i << " header\n";
            return 1;
        }
        
        if (!read_bytes(file, &sections[i].name, sizeof(sections[i].name))) return 1;
        if (!read_bytes(file, &sections[i].type, sizeof(sections[i].type))) return 1;
        if (!read_bytes(file, &sections[i].flags, sizeof(sections[i].flags))) return 1;
        if (!read_bytes(file, &sections[i].addr, sizeof(sections[i].addr))) return 1;
        if (!read_bytes(file, &sections[i].offset, sizeof(sections[i].offset))) return 1;
        if (!read_bytes(file, &sections[i].size, sizeof(sections[i].size))) return 1;
        if (!read_bytes(file, &sections[i].link, sizeof(sections[i].link))) return 1;
        if (!read_bytes(file, &sections[i].info, sizeof(sections[i].info))) return 1;
        if (!read_bytes(file, &sections[i].addralign, sizeof(sections[i].addralign))) return 1;
        if (!read_bytes(file, &sections[i].entsize, sizeof(sections[i].entsize))) return 1;
    }

    print_header_info(header);

    const SectionHeader& string_table = sections[header.section_string_index];
    
    if (header.section_string_index >= sections.size()) {
        std::cerr << "Invalid section string table index\n";
        return 1;
    }

    std::vector<char> section_string_table(string_table.size);
    file.seekg(string_table.offset);
    if (!read_bytes(file, section_string_table.data(), section_string_table.size())) return 1;

    for (auto c : section_string_table) {
        if (c == 0) {
            std::cout << '\n';
        } else {
            std::cout << c;
        }
    }

    return 0;
}