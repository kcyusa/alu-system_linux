#include "hnm.h"

#include <ctype.h> /* for tolower() */

/**
 * print_symbol_info32 - Determines and prints ELF32 symbol information.
 * @symbol: the symbol to inspect
 * @symbol_name: the name of the symbol (from string table)
 * @section_headers: the section headers array
 *
 * Return: void
 */
void print_symbol_info32(Elf32_Sym symbol, char *symbol_name,
						 Elf32_Shdr *section_headers, char symbol_type)
{
	if (ELF32_ST_BIND(symbol.st_info) == STB_WEAK)
		if (symbol.st_shndx == SHN_UNDEF)
			symbol_type = 'w';
		else if (ELF32_ST_TYPE(symbol.st_info) == STT_OBJECT)
			symbol_type = 'V';
		else
			symbol_type = 'W';
	else if (symbol.st_shndx == SHN_UNDEF)
		symbol_type = 'U';
	else if (symbol.st_shndx == SHN_ABS)
		symbol_type = 'A';
	else if (symbol.st_shndx == SHN_COMMON)
		symbol_type = 'C';
	else if (symbol.st_shndx < SHN_LORESERVE)
	{
		Elf32_Shdr symbol_section = section_headers[symbol.st_shndx];

		if (ELF32_ST_BIND(symbol.st_info) == STB_GNU_UNIQUE)
			symbol_type = 'u';
		else if (symbol_section.sh_type == SHT_NOBITS &&
				 symbol_section.sh_flags == (SHF_ALLOC | SHF_WRITE))
			symbol_type = 'B';
		else if (symbol_section.sh_type == SHT_PROGBITS)
			if (symbol_section.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
				symbol_type = 'T';
			else if (symbol_section.sh_flags == SHF_ALLOC)
				symbol_type = 'R';
			else if (symbol_section.sh_flags == (SHF_ALLOC | SHF_WRITE))
				symbol_type = 'D';
			else if (symbol_section.sh_type == SHT_DYNAMIC)
				symbol_type = 'D';
			else
				symbol_type = 't';
	}
	if (ELF32_ST_BIND(symbol.st_info) == STB_LOCAL)
		symbol_type = tolower(symbol_type);
	if (symbol_type != 'U' && symbol_type != 'w')
		printf("%08x %c %s\n", symbol.st_value, symbol_type, symbol_name);
	else
		printf("         %c %s\n", symbol_type, symbol_name);
}

/**
 * print_symbol_table32 - program that prints
 * the symbol table for a 32-bit ELF file
 * considering special section indices and visibility attributes
 * @section_header: a pointer to the section header of the symbol table
 * @symbol_table: a pointer to the beginning of the symbol table
 * @string_table: a pointer to the beginning of the string table,
 *                which contains the names of the symbols
 * @section_headers: a pointer to the array of section headers for the ELF file
 * Return: nothing (void)
 */

void print_symbol_table32(Elf32_Shdr *section_header, Elf32_Sym *symbol_table,
						  char *string_table, Elf32_Shdr *section_headers)
{
	int i;
	int symbol_count = section_header->sh_size / sizeof(Elf32_Sym);
	char *symbol_name;

	for (i = 0; i < symbol_count; i++)
	{
		Elf32_Sym symbol = symbol_table[i];
		char symbol_type = "?";

		symbol_name = string_table + symbol.st_name;
		if (symbol.st_name != 0 && ELF32_ST_TYPE(symbol.st_info) != STT_FILE)
		{
			print_symbol_info32(symbol, symbol_name, section_headers, symbol_type);
		}
	}
}

/**
 * process_elf_file32 - program that processes a 32-bit ELF file
 * located at the given file path
 * this function opens the file, reads the ELF header, and verifies
 * the ELF format and endianness;
 * it then reads the section headers to locate the symbol table
 * and the string table;
 * afterward, it reads the symbol table and string table from the file
 * and calls 'print_symbol_table32' to print the symbol information
 * @file_path: a pointer to a string that contains the path
 *             to the ELF file to be processed
 * Return: nothing (void)
 */

void process_elf_file32(char *file_path)
{
	int symbol_table_index = -1;
	int i;
	int is_little_endian, is_big_endian;
	int string_table_index;

	FILE *file = fopen(file_path, "rb");

	if (file == NULL)
	{
		fprintf(stderr, "./hnm: %s: failed to open file\n", file_path);
		return;
	}

	Elf32_Ehdr elf_header;

	fread(&elf_header, sizeof(Elf32_Ehdr), 1, file);

	/* Il faut vérifier le type de fichier ELF */
	if (elf_header.e_ident[EI_CLASS] != ELFCLASS32 && elf_header.e_ident[EI_CLASS] != ELFCLASS64)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file format\n", file_path);
		fclose(file);
		return;
	}

	/* Pour l'endianness */
	is_little_endian = (elf_header.e_ident[EI_DATA] == ELFDATA2LSB);
	is_big_endian = (elf_header.e_ident[EI_DATA] == ELFDATA2MSB);

	if (!is_little_endian && !is_big_endian)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file endianness\n", file_path);
		fclose(file);
		return;
	}

	/* Pour la table des sections */
	Elf32_Shdr *section_headers = malloc(elf_header.e_shentsize * elf_header.e_shnum);

	if (section_headers == NULL)
	{
		fprintf(stderr, "./hnm: %s: memory allocation error for section_headers\n", file_path);
		fclose(file);
		return;
	}

	fseek(file, elf_header.e_shoff, SEEK_SET);
	fread(section_headers, elf_header.e_shentsize, elf_header.e_shnum, file);

	/* Trouver l'index de la section de la table des symboles */
	for (i = 0; i < elf_header.e_shnum; i++)
	{
		if (i < elf_header.e_shnum && section_headers[i].sh_type == SHT_SYMTAB)
		{
			symbol_table_index = i;
			break;
		}
	}

	if (symbol_table_index == -1)
	{
		fprintf(stderr, "./hnm: %s: no symbols\n", file_path);
		fclose(file);
		free(section_headers);
		return;
	}

	/* Lire la table des symboles */
	Elf32_Shdr symbol_table_header = section_headers[symbol_table_index];
	Elf32_Sym *symbol_table = malloc(symbol_table_header.sh_size);

	fseek(file, symbol_table_header.sh_offset, SEEK_SET);
	fread(symbol_table, symbol_table_header.sh_size, 1, file);

	string_table_index = symbol_table_header.sh_link;

	Elf32_Shdr string_table_header = section_headers[string_table_index];

	char *string_table = malloc(string_table_header.sh_size);

	fseek(file, string_table_header.sh_offset, SEEK_SET);
	fread(string_table, string_table_header.sh_size, 1, file);

	/* Afficher la table des symboles */
	print_symbol_table32(&symbol_table_header, symbol_table, string_table, section_headers);

	fclose(file);

	free(section_headers);
	free(symbol_table);
	free(string_table);
}
