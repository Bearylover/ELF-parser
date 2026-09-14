#pragma once
#include "ELFTypes.h"
#include <vector>

void print_header_info(const ELFHeader& header);
void print_section_info(const SectionHeader& section);
void print_strtab(const std::vector<char>& strtab);
