#include "ELFParse.h"

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

bool read_bytes(std::ifstream& file, void* destination, std::size_t size) {
    if (!file.read(reinterpret_cast<char*>(destination), size)) {
        std::cerr << "Unexpected EOF\n";
        return false;
    }
    return true;
}

std::optional<ELFHeader> parse_header(std::ifstream& file) {
    ELFHeader header;
    if (!read_bytes(file, header.ident, sizeof(header.ident))) return std::nullopt;
    if (header.ident[0] != 0x7F || header.ident[1] != 'E' || header.ident[2] != 'L' || header.ident[3] != 'F') {
        std::cerr << "Not an ELF file\n";
        return std::nullopt;
    }

    auto elf_class = parse_class(header.ident[4]);
    if (!elf_class) {
        std::cerr << "Invalid ELF Class\n";
        return std::nullopt;
    }
    header.elf_class = *elf_class;

    auto endian = parse_endian(header.ident[5]);
    if (!endian) {
        std::cerr << "Invalid Endianness\n";
        return std::nullopt;
    }
    header.endian = *endian;

    if (!read_bytes(file, &header.type, sizeof(header.type))) return std::nullopt;
    if (!read_bytes(file, &header.machine, sizeof(header.machine))) return std::nullopt;
    if (!read_bytes(file, &header.version, sizeof(header.version))) return std::nullopt;

    if (header.version == static_cast<uint32_t>(Version::Invalid)) {
        std::cerr << "Invalid ELF Version\n";
        return std::nullopt;
    } else if (header.version != static_cast<uint32_t>(Version::Current)) {
        std::cerr << "Unsupported ELF Version \n";
        return std::nullopt;
    }

    uint32_t e_entry32;
    if (header.elf_class == ELFClass::ELF32) {
        if (!read_bytes(file, &e_entry32, sizeof(e_entry32))) return std::nullopt;
        header.entry = e_entry32;
    } else {
        if (!read_bytes(file, &header.entry, sizeof(header.entry))) return std::nullopt;
    }

    uint32_t e_phoff32;
    if (header.elf_class == ELFClass::ELF32) {
        if (!read_bytes(file, &e_phoff32, sizeof(e_phoff32))) return std::nullopt;
        header.program_offset = e_phoff32;
    } else {
        if (!read_bytes(file, &header.program_offset, sizeof(header.program_offset))) return std::nullopt;
    }

    uint32_t e_shoff32;
    if (header.elf_class == ELFClass::ELF32) {
        if (!read_bytes(file, &e_shoff32, sizeof(e_shoff32))) return std::nullopt;
        header.section_offset = e_shoff32;
    } else {
        if (!read_bytes(file, &header.section_offset, sizeof(header.section_offset))) return std::nullopt;
    }

    if (!read_bytes(file, &header.flags, sizeof(header.flags))) return std::nullopt;
    if (!read_bytes(file, &header.header_size, sizeof(header.header_size))) return std::nullopt;
    if (!read_bytes(file, &header.program_entry_size, sizeof(header.program_entry_size))) return std::nullopt;
    if (!read_bytes(file, &header.program_count, sizeof(header.program_count))) return std::nullopt;
    if (!read_bytes(file, &header.section_entry_size, sizeof(header.section_entry_size))) return std::nullopt;
    if (!read_bytes(file, &header.section_count, sizeof(header.section_count))) return std::nullopt;
    if (!read_bytes(file, &header.section_string_index, sizeof(header.section_string_index))) return std::nullopt;

    return header;
}

std::optional<std::vector<SectionHeader>> parse_section_headers(std::ifstream& file, const ELFHeader& header) {
    uint64_t offset;
    std::vector<SectionHeader> sections(header.section_count);

    for (uint16_t i = 0; i < header.section_count; ++i) {
        offset = header.section_offset + i * header.section_entry_size;
        file.seekg(offset);
        if (!file) {
            std::cerr << "Failed to seek section " << i << " header\n";
            return std::nullopt;
        }
        
        if (!read_bytes(file, &sections[i].name, sizeof(sections[i].name))) return std::nullopt;
        if (!read_bytes(file, &sections[i].type, sizeof(sections[i].type))) return std::nullopt;
        if (!read_bytes(file, &sections[i].flags, sizeof(sections[i].flags))) return std::nullopt;
        if (!read_bytes(file, &sections[i].addr, sizeof(sections[i].addr))) return std::nullopt;
        if (!read_bytes(file, &sections[i].offset, sizeof(sections[i].offset))) return std::nullopt;
        if (!read_bytes(file, &sections[i].size, sizeof(sections[i].size))) return std::nullopt;
        if (!read_bytes(file, &sections[i].link, sizeof(sections[i].link))) return std::nullopt;
        if (!read_bytes(file, &sections[i].info, sizeof(sections[i].info))) return std::nullopt;
        if (!read_bytes(file, &sections[i].addralign, sizeof(sections[i].addralign))) return std::nullopt;
        if (!read_bytes(file, &sections[i].entsize, sizeof(sections[i].entsize))) return std::nullopt;
    }
    return sections;
}

std::optional<std::vector<char>> read_shstrtab(std::ifstream& file, const ELFHeader& header, const std::vector<SectionHeader>& sections) {
    if (header.section_string_index >= sections.size()) {
        std::cerr << "Invalid section string table index\n";
        return std::nullopt;
    }
    
    const SectionHeader& strtab = sections[header.section_string_index];
    std::vector<char> shstrtab(strtab.size);

    file.seekg(strtab.offset);
    if (!read_bytes(file, shstrtab.data(), shstrtab.size())) return std::nullopt;

    return shstrtab;
}

std::optional<std::vector<ProgramHeader>> parse_program_headers(std::ifstream& file, const ELFHeader& header) {
    uint64_t offset;
    std::vector<ProgramHeader> programs(header.program_count);

    for (uint16_t i = 0; i < header.program_count; ++i) {
        offset = header.program_offset+ i * header.program_entry_size;
        file.seekg(offset);
        if (!file) {
            std::cerr << "Failed to seek program " << i << " header\n";
            return std::nullopt;
        }
        
        if (!read_bytes(file, &programs[i].type, sizeof(programs[i].type))) return std::nullopt;
        if (!read_bytes(file, &programs[i].flags, sizeof(programs[i].flags))) return std::nullopt;
        if (!read_bytes(file, &programs[i].offset, sizeof(programs[i].offset))) return std::nullopt;
        if (!read_bytes(file, &programs[i].vaddr, sizeof(programs[i].vaddr))) return std::nullopt;
        if (!read_bytes(file, &programs[i].paddr, sizeof(programs[i].paddr))) return std::nullopt;
        if (!read_bytes(file, &programs[i].filesz, sizeof(programs[i].filesz))) return std::nullopt;
        if (!read_bytes(file, &programs[i].memsz, sizeof(programs[i].memsz))) return std::nullopt;
        if (!read_bytes(file, &programs[i].align, sizeof(programs[i].align))) return std::nullopt;
    }

    return programs;
}

std::optional<std::string> get_section_name(const SectionHeader& section, const std::vector<char>& strtab) {
    uint32_t init_idx = section.name;
    std::string resolved_name;
    if (init_idx >= strtab.size()) {
        std::cout << "Section name out of bounds\n";
        return std::nullopt;
    }
    while (strtab[init_idx] != 0 && (init_idx < strtab.size())) {
        resolved_name.push_back(strtab[init_idx]);
        init_idx++;
    }
    if (init_idx == strtab.size()) {
        std::cout << "Section name out of bounds\n";
        return std::nullopt;
    }
    return resolved_name;
}

std::string get_program_flags(uint32_t flags) {
    std::string result;
    if (flags & 4) {
        result += 'R';
    } else {
        result += '-';
    }
    if (flags & 2) {
        result += 'W';
    } else {
        result += '-';
    }
    if (flags & 1) {
        result += 'X';
    } else {
        result += '-';
    }
    return result;
}

std::vector<size_t> sections_in_segment(const ProgramHeader& segment, const std::vector<SectionHeader>& sections) {
    std::vector<size_t> section_index;
    size_t idx = 0;
    for (SectionHeader i : sections) {
        if (segment.offset < i.offset && (segment.offset + segment.filesz) > (i.offset + i.size)) {
            section_index.push_back(idx);
        }
        idx++;
    }
    return section_index;
}