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

    auto parsed_strtab = read_strtab(file, header, sections);
    if (!parsed_strtab) return 1;
    std::vector<char> section_strtab = *parsed_strtab;

    auto parsed_program_header = parse_program_headers(file, header);
    if (!parsed_program_header) return 1;
    std::vector<ProgramHeader> programs = *parsed_program_header;

    std::cout << "Header Info: \n";
    print_header_info(header);
    std::cout << "\nString Table Section Info: \n";
    print_section_header(sections[header.section_string_index]);
    std::cout << "\nString Table: \n";
    print_strtab(section_strtab);

    size_t idx = 0;
    for (auto i : programs) {
        std::vector<size_t> section_index = sections_in_segment(i, sections);
        std::cout << "Segment " << idx << ":\n";
        for (auto j : section_index) {
            auto parsed_name = get_section_name(sections[j], section_strtab);
            if (!parsed_name) return 1;
            std::cout << "|  " << *parsed_name << "\n";
        }
        idx++;
    }

    return 0;
}