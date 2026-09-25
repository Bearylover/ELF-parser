#include "ELFTypes.h"
#include "ELFPrint.h"
#include "ELFParse.h"

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <vector>

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

    auto parsed_header = parse_header(file);
    if (!parsed_header) return 1;
    ELFHeader header = *parsed_header;

    auto parsed_section_header = parse_section_headers(file, header);
    if (!parsed_section_header) return 1;
    std::vector<SectionHeader> sections = *parsed_section_header;

    if (header.section_string_index >= sections.size()) {
        std::cerr << "Invalid section string table index\n";
        return 1;
    }
    auto parsed_shstrtab = read_section(file, sections[header.section_string_index]);
    if (!parsed_shstrtab) {
        std::cerr << "Failed to read shstrtab\n";
        return 1;
    }
    std::vector<char> shstrtab = *parsed_shstrtab;

    auto parsed_program_header = parse_program_headers(file, header);
    if (!parsed_program_header) return 1;
    std::vector<ProgramHeader> programs = *parsed_program_header;

    std::optional<SectionHeader> temp_symtab_header = {}, temp_dynsym_header = {};
    SectionHeader symtab_header, dynsym_header;
    for (const SectionHeader& i : sections) {
        if (i.type == 2) {
            temp_symtab_header = i;
        } else if (i.type == 11) {
            temp_dynsym_header = i;
        }
    }

    if (temp_symtab_header) {
        symtab_header = *temp_symtab_header;
        if (symtab_header.link >= header.section_count) {
            std::cerr << "Invalid symtab strtab location\n";
            return 1;
        }

        if (symtab_header.entsize == 0) {
            std::cerr << "Malformed symtab header (entsize is 0)\n";
            return 1; 
        } else if (symtab_header.entsize < 24) {
            std::cerr << "Malformed symtab header (entsize is too small)\n";
            return 1; 
        } else if (symtab_header.entsize > 24) {
            std::cerr << "Malformed symtab header (entsize is too large)\n";
            return 1; 
        }

        if (symtab_header.size % symtab_header.entsize != 0) {
            std::cerr << "Warning: partial symbol record in symtab (will not be output)\n";
        }

        uint64_t symbol_count = symtab_header.size/symtab_header.entsize;
        std::vector<Symbol> symtab_symbols(symbol_count);

        for (uint64_t i = 0; i < symbol_count; ++i) {
            file.seekg(symtab_header.offset + i * symtab_header.entsize);
            if (!file) {
                std::cerr << "Failed to seek to symtab entry\n";
                return 1;
            }
            if (!read_bytes(file, &symtab_symbols[i].name, sizeof(symtab_symbols[i].name))) return 1;
            if (!read_bytes(file, &symtab_symbols[i].info, sizeof(symtab_symbols[i].info))) return 1;
            if (!read_bytes(file, &symtab_symbols[i].other, sizeof(symtab_symbols[i].other))) return 1;
            if (!read_bytes(file, &symtab_symbols[i].shndx, sizeof(symtab_symbols[i].shndx))) return 1;
            if (!read_bytes(file, &symtab_symbols[i].value, sizeof(symtab_symbols[i].value))) return 1;
            if (!read_bytes(file, &symtab_symbols[i].size, sizeof(symtab_symbols[i].size))) return 1;
        }

        auto parsed_symtab_strtab = read_section(file, sections[symtab_header.link]);
        if (!parsed_symtab_strtab) {
            std::cerr << "Failed to read symtab strtab\n";
            return 1;
        }
        std::vector<char> symtab_strtab = *parsed_symtab_strtab;

        std::cout << "\nSymtab: \n";
        print_section(symtab_strtab);
    } else {
        std::cout << "\nNo symbol table found.\n";
    }

    if (temp_dynsym_header) {
        dynsym_header = *temp_dynsym_header;
        if (dynsym_header.link >= header.section_count) {
            std::cerr << "Invalid dynsym strtab location\n";
            return 1;
        }

        auto parsed_dynsym_strtab = read_section(file, sections[dynsym_header.link]);
        if (!parsed_dynsym_strtab) {
            std::cerr << "Failed to read dynsym\n";
            return 1;
        }
        std::vector<char> dynsym_strtab = *parsed_dynsym_strtab;

        std::cout << "\nDynsym: \n";
        print_section(dynsym_strtab);
    } else {
        std::cout << "\nNo dynamic symbol table found.\n";
    }

    std::cout << "Header Info: \n";
    print_header_info(header);
    std::cout << "\nSection Header String Table Section: \n";
    print_section_header(sections[header.section_string_index]);
    std::cout << "\nSection Header String Table: \n";
    print_section(shstrtab);
    std::cout << "\nSegment list: \n";
    print_sections_in_segments(sections, programs, shstrtab);

    return 0;
}