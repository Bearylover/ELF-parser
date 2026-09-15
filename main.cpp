#include "ELFTypes.h"
#include "ELFPrint.h"
#include "ELFParse.h"

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <vector>

std::optional<std::string> get_section_name(const SectionHeader& section, const std::vector<char>& strtab) {
    uint32_t init_idx = section.name;
    std::string resolved_name;
    if (init_idx >= strtab.size()) return std::nullopt;
    while (strtab[init_idx] != 0 && (init_idx < strtab.size())) {
        resolved_name.push_back(strtab[init_idx]);
        init_idx++;
    }
    if (init_idx == strtab.size()) return std::nullopt;
    return resolved_name;
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

    //Header reading
    //Reads all the info from the header (while verifying)
    //This has to come first, since you need the section/program header offset to do anything else

    auto parsed_header = parse_header(file);
    if (!parsed_header) return 1;
    ELFHeader header = *parsed_header;

    //Section reading
    //Loops using header parameters, seeks to every section, reads their info.
    //All section info is saved in a vector of SectionHeader.

    auto parsed_section_header = parse_section_headers(file, header);
    if (!parsed_section_header) return 1;
    std::vector<SectionHeader> sections = *parsed_section_header;

    //Fetches the string table. VERY IMPORTANT.
    //The get_section_name function relies on this. Always use strtab as the second argument (unless you find another strtab somewhere)

    auto parsed_strtab = read_strtab(file, header, sections);
    if (!parsed_strtab) return 1;
    std::vector<char> section_strtab = *parsed_strtab;

    //Program reading
    //Same concept as section reading

    auto parsed_program_header = parse_program_headers(file, header);
    if (!parsed_program_header) return 1;
    std::vector<ProgramHeader> programs = *parsed_program_header;

    std::cout << "Header Info: \n";
    print_header_info(header);
    std::cout << "\nString Table Section Info: \n";
    print_section_header(sections[header.section_string_index]); //strtab as example, function can print any section
    std::cout << "\nString Table: \n";
    print_strtab(section_strtab);
    std::cout << "Sample Program Header Info: \n";
    print_program_header(programs[0]);

    return 0;
}