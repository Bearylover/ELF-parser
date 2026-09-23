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

    auto parsed_shstrtab = read_shstrtab(file, header, sections);
    if (!parsed_shstrtab) return 1;
    std::vector<char> shstrtab = *parsed_shstrtab;

    auto parsed_program_header = parse_program_headers(file, header);
    if (!parsed_program_header) return 1;
    std::vector<ProgramHeader> programs = *parsed_program_header;

    SectionHeader symtab_header = {}, dynsym_header = {};
    for (SectionHeader i : sections) {
        if (i.type == 3) {
            symtab_header = i;
        } else if (i.type == 11) {
            dynsym_header = i;
        }
    }
    if (symtab_header.link >= header.section_count) {
        std::cerr << "Invalid symtab location\n";
    }
    if (symtab_header.link >= header.section_count) {
        std::cerr << "Invalid dynamic symtab location\n";
    }
    SectionHeader symtab_strtab_header = sections[symtab_header.link];
    SectionHeader dynsym_strtab_header = sections[dynsym_header.link];
    //symtab
    std::vector<char> strtab;
    

    std::cout << "Header Info: \n";
    print_header_info(header);
    std::cout << "\nString Table Section Info: \n";
    print_section_header(sections[header.section_string_index]);
    std::cout << "\nString Table: \n";
    print_strtab(shstrtab);
    std::cout << "\nSegment list: \n";
    print_sections_in_segments(sections, programs, shstrtab);

    return 0;
}