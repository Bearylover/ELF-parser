#pragma once
#include "ELFTypes.h"
#include <vector>

void print_header_info(const ELFHeader& header);
void print_section_header(const SectionHeader& section);
void print_section(const std::vector<char>& strtab);
void print_program_header(const ProgramHeader& program);
void print_sections_in_segments(const std::vector<SectionHeader>& sections, const std::vector<ProgramHeader>& programs, const std::vector<char>& section_strtab);
