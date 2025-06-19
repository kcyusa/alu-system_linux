#include "hnm.h"
#include <stdio.h>
#include <elf.h>
#include <ctype.h>

/**
 * get_symbol_type32 - Get the symbol type character for a 32-bit symbol
 * @sym: the symbol entry
 * @sections: section headers of the ELF file
 *
 * Return: a character representing the symbol type
 */
char get_symbol_type32(Elf32_Sym sym, Elf32_Shdr *sections)
{
	Elf32_Shdr sec;
	char type = '?';

	if (ELF32_ST_BIND(sym.st_info) == STB_WEAK)
	{
		if (sym.st_shndx == SHN_UNDEF)
			return ('w');
		return ((ELF32_ST_TYPE(sym.st_info) == STT_OBJECT) ? 'V' : 'W');
	}
	if (sym.st_shndx == SHN_UNDEF)
		return ('U');
	if (sym.st_shndx == SHN_ABS)
		return ('A');
	if (sym.st_shndx == SHN_COMMON)
		return ('C');
	if (sym.st_shndx < SHN_LORESERVE)
	{
		sec = sections[sym.st_shndx];
		if (ELF32_ST_BIND(sym.st_info) == STB_GNU_UNIQUE)
			type = 'u';
		else if (sec.sh_type == SHT_NOBITS &&
				 sec.sh_flags == (SHF_ALLOC | SHF_WRITE))
			type = 'B';
		else if (sec.sh_type == SHT_PROGBITS)
		{
			if (sec.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
				type = 'T';
			else if (sec.sh_flags == SHF_ALLOC)
				type = 'R';
			else if (sec.sh_flags == (SHF_ALLOC | SHF_WRITE))
				type = 'D';
		}
		else if (sec.sh_type == SHT_DYNAMIC)
			type = 'D';
		else
			type = 't';
	}
	if (ELF32_ST_BIND(sym.st_info) == STB_LOCAL)
		type = tolower(type);
	return (type);
}

/**
 * print_symbol_table32 - Print the symbol table of a 32-bit ELF file
 * @section_header: the section header of the symbol table
 * @symbol_table: the array of symbols
 * @string_table: the symbol names string table
 * @section_headers: the array of section headers
 */
void print_symbol_table32(Elf32_Shdr *section_header, Elf32_Sym *symbol_table,
						  char *string_table, Elf32_Shdr *section_headers)
{
	int i, count = section_header->sh_size / sizeof(Elf32_Sym);
	char type;
	char *name;

	for (i = 0; i < count; i++)
	{
		Elf32_Sym sym = symbol_table[i];

		name = string_table + sym.st_name;
		if (sym.st_name != 0 && ELF32_ST_TYPE(sym.st_info) != STT_FILE)
		{
			type = get_symbol_type32(sym, section_headers);
			if (type != 'U' && type != 'w')
				printf("%08x %c %s\n", sym.st_value, type, name);
			else
				printf("         %c %s\n", type, name);
		}
	}
}

/**
 * handle_symbol_tables32 - Load and print ELF 32-bit symbol info
 * @file: Open file pointer
 * @file_path: Path to ELF file (for error messages)
 * @section_headers: Array of section headers
 * @symbol_table_index: Index of symbol table
 * @elf_header: ELF header
 */
void handle_symbol_tables32(FILE *file, char *file_path,
			    Elf32_Shdr *section_headers,
			    int symbol_table_index)
{
	if (symbol_table_index == -1)
	{
		fprintf(stderr, "./hnm: %s: no symbols\n", file_path);
		free(section_headers);
		fclose(file);
		return;
	}

	Elf32_Shdr symbol_table_header = section_headers[symbol_table_index];
	Elf32_Sym *symbol_table = malloc(symbol_table_header.sh_size);
	int string_table_index = symbol_table_header.sh_link;
	Elf32_Shdr string_table_header = section_headers[string_table_index];
	char *string_table = malloc(string_table_header.sh_size);

	fseek(file, symbol_table_header.sh_offset, SEEK_SET);
	fread(symbol_table, symbol_table_header.sh_size, 1, file);

	fseek(file, string_table_header.sh_offset, SEEK_SET);
	fread(string_table, string_table_header.sh_size, 1, file);

	print_symbol_table32(&symbol_table_header, symbol_table,
			     string_table, section_headers);

	free(section_headers);
	free(symbol_table);
	free(string_table);
	fclose(file);
}

/**
 * process_elf_file32 - Entry point to process a 32-bit ELF file
 * @file_path: Path to ELF file
 */
void process_elf_file32(char *file_path)
{
	int i, symbol_table_index = -1;
	size_t size;
	FILE *file = fopen(file_path, "rb");
	Elf32_Ehdr elf_header;
	Elf32_Shdr *section_headers;

	if (!file)
	{
		fprintf(stderr, "./hnm: %s: failed to open file\n", file_path);
		return;
	}

	fread(&elf_header, sizeof(Elf32_Ehdr), 1, file);
	if (elf_header.e_ident[EI_CLASS] != ELFCLASS32 ||
	    (elf_header.e_ident[EI_DATA] != ELFDATA2LSB &&
	     elf_header.e_ident[EI_DATA] != ELFDATA2MSB))
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file\n", file_path);
		fclose(file);
		return;
	}
	size = elf_header.e_shentsize * elf_header.e_shnum;
	section_headers = malloc(size);
	if (!section_headers)
	{
		fprintf(stderr, "./hnm: %s: allocation error\n", file_path);
		fclose(file);
		return;
	}
	fseek(file, elf_header.e_shoff, SEEK_SET);
	fread(section_headers, elf_header.e_shentsize, elf_header.e_shnum, file);
	for (i = 0; i < elf_header.e_shnum; i++)
		if (section_headers[i].sh_type == SHT_SYMTAB)
		{
			symbol_table_index = i;
			break;
		}
	handle_symbol_tables32(file, file_path, section_headers,
			       symbol_table_index);
}
