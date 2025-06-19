#ifndef HNM_H
#define HNM_H

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <ctype.h>

void print_symbol_table64(Elf64_Shdr *section_header, Elf64_Sym *symbol_table,
			  char *string_table, Elf64_Shdr *section_headers);
void process_elf_file64(char *file_path);
void print_symbol_table32(Elf32_Shdr *section_header, Elf32_Sym *symbol_table,
			  char *string_table, Elf32_Shdr *section_headers);
void process_elf_file32(char *file_path);

#endif /* HNM_H */