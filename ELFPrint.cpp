#include "ELFPrint.h"
#include <iostream>

void print_header_info(const ELFHeader& header) {
    switch (header.elf_class) {
        case ELFClass::ELF32:
            std::cout << "Class: ELF32\n";
            break;
        case ELFClass::ELF64:
            std::cout << "Class: ELF64\n";
            break;
    }
    switch (header.endian) {
        case Endian::Little:
            std::cout << "Endian: Little Endian\n";
            break;
        case Endian::Big: //unused for now
            std::cout << "Endian: Big Endian\n";
            break;
    }
    switch (header.ident[7]) {
        case 0:
            std::cout << "OS/ABI: System V\n";
            break;
        case 1:
            std::cout << "OS/ABI: HP-UX\n";
            break;
        case 2:
            std::cout << "OS/ABI: NetBSD\n";
            break;
        case 3:
            std::cout << "OS/ABI: Linux\n";
            break;
        case 6:
            std::cout << "OS/ABI: Solaris\n";
            break;
        case 9:
            std::cout << "OS/ABI: FreeBSD\n";
            break;
        default:
            std::cout << "OS/ABI: Unknown (" << header.ident[7] << ")\n";
    }

    switch (header.type) {
        case 0:
            std::cout << "File Type: None\n";
            break;
        case 1:
            std::cout << "File Type: Relocatable\n";
            break;
        case 2:
            std::cout << "File Type: Executable\n";
            break;
        case 3:
            std::cout << "File Type: Shared object\n";
            break;
        case 4:
            std::cout << "File Type: Core\n";
            break;
        default:
            std::cout << "File Type: Special (" << header.type << ")\n";
    }

    switch (header.machine) {
        case 0x03:
            std::cout << "Machine: Intel 80386\n";
            break;
        case 0x08:
            std::cout << "Machine: MIPS\n";
            break;
        case 0x14:
            std::cout << "Machine: PowerPC\n";
            break;
        case 0x28:
            std::cout << "Machine: ARM\n";
            break;
        case 0x3e:
            std::cout << "Machine: x86-64\n";
            break;
        case 0xb7:
            std::cout << "Machine: 64-bit ARM\n";
            break;
        default:
            std::cout << "Machine: Unknown\n";
    }

    std::cout << "Entry Point Address: 0x" << std::hex << header.entry << '\n';
    std::cout << "Program Offset: 0x" << header.program_offset << " bytes\n";
    std::cout << "Section Offset: 0x" << header.section_offset << " bytes\n";
}

void print_section_info(const SectionHeader& section) {
    std::cout << "Name offset: " << section.name << '\n';
    std::cout << "Type: " << section.type << '\n';
    std::cout << "Flags: " << section.flags << '\n';
    std::cout << "Address: " << section.addr << '\n';
    std::cout << "File Offset: " << section.offset << '\n';
    std::cout << "Size: " << section.size << '\n';
    std::cout << "Link: " << section.link << '\n';
    std::cout << "Info: " << section.info << '\n';
    std::cout << "Address Alignment: " << section.addralign << '\n';
    std::cout << "Entry Size: " << section.entsize << '\n';
}