#pragma once
#include "ELFTypes.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <vector>

bool read_bytes(std::ifstream& file, void* destination, std::size_t size);
std::optional<ELFClass> parse_class(uint8_t val);
std::optional<Endian> parse_endian(uint8_t val);
std::optional<ELFHeader> parse_header(std::ifstream& file);
std::optional<std::vector<SectionHeader>> parse_section_headers(std::ifstream& file, const ELFHeader& header);
std::optional<std::vector<char>> read_strtab(std::ifstream& file, const ELFHeader& header, const std::vector<SectionHeader>& sections);
std::optional<std::vector<ProgramHeader>> parse_program_headers(std::ifstream& file, const ELFHeader& header);
std::optional<std::string> get_section_name(const SectionHeader& section, const std::vector<char>& strtab);