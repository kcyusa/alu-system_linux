#include "hnm.h"

/**
 * get_symbol_type32 - Determines the type character of an ELF32 symbol.
 * @symbol: The ELF32 symbol to inspect.
 * @section_headers: The section headers array for the ELF file.
 *
 * Return: The character representing the symbol type.
 */
char get_symbol_type32(Elf32_Sym symbol, Elf32_Shdr *section_headers)
{
	char type = '?';

	if (ELF32_ST_BIND(symbol.st_info) == STB_WEAK)
		if (symbol.st_shndx == SHN_UNDEF)
			type = 'w';
		else if (ELF32_ST_TYPE(symbol.st_info) == STT_OBJECT)
			type = 'V';
		else
			type = 'W';
	else if (symbol.st_shndx == SHN_UNDEF)
		type = 'U';
	else if (symbol.st_shndx == SHN_ABS)
		type = 'A';
	else if (symbol.st_shndx == SHN_COMMON)
		type = 'C';
	else if (symbol.st_shndx < SHN_LORESERVE)
	{
		Elf32_Shdr section = section_headers[symbol.st_shndx];

		if (ELF32_ST_BIND(symbol.st_info) == STB_GNU_UNIQUE)
			type = 'u';
		else if (section.sh_type == SHT_NOBITS &&
			section.sh_flags == (SHF_ALLOC | SHF_WRITE))
			type = 'B';
		else if (section.sh_type == SHT_PROGBITS)
			if (section.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
				type = 'T';
			else if (section.sh_flags == SHF_ALLOC)
				type = 'R';
			else if (section.sh_flags == (SHF_ALLOC | SHF_WRITE))
				type = 'D';
		else if (section.sh_type == SHT_DYNAMIC)
			type = 'D';
		else
			type = 't';
	}
	if (ELF32_ST_BIND(symbol.st_info) == STB_LOCAL)
		type = tolower(type);
	return (type);
}

/**
 * print_symbol32 - Prints information for a single ELF32 symbol.
 * @symbol: The symbol to print.
 * @section_headers: The section headers array for the ELF file.
 * @symbol_name: The name of the symbol as a string.
 *
 * This function calls get_symbol_type32() to determine the type character
 * and prints the symbol's value, type, and name in the nm -p format.
 */
void print_symbol32(Elf32_Sym symbol, Elf32_Shdr *section_headers, char *symbol_name)
{
	char type = get_symbol_type32(symbol, section_headers);

	if (type != 'U' && type != 'w')
		printf("%08x %c %s\n", symbol.st_value, type, symbol_name);
	else
		printf("         %c %s\n", type, symbol_name);
}


/**
 * print_symbol_table32 - program that prints the symbol table for a 32-bit ELF file
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
	char *symbol_name, symbol_type;

	for (i = 0; i < symbol_count; i++)
	{
		Elf32_Sym symbol = symbol_table[i];
		symbol_name = string_table + symbol.st_name;

		if (symbol.st_name != 0 && ELF32_ST_TYPE(symbol.st_info) != STT_FILE)
		{
			print_symbol32(symbol, section_headers, symbol_name);
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

	if (elf_header.e_ident[EI_CLASS] != ELFCLASS32 && elf_header.e_ident[EI_CLASS] != ELFCLASS64)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file format\n", file_path);
		fclose(file);
		return;
	}

	is_little_endian = (elf_header.e_ident[EI_DATA] == ELFDATA2LSB);
	is_big_endian = (elf_header.e_ident[EI_DATA] == ELFDATA2MSB);

	if (!is_little_endian && !is_big_endian)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file endianness\n", file_path);
		fclose(file);
		return;
	}

	Elf32_Shdr *section_headers = malloc(elf_header.e_shentsize * elf_header.e_shnum);

	if (section_headers == NULL)
	{
		fprintf(stderr, "./hnm: %s: memory allocation error for section_headers\n", file_path);
		fclose(file);
		return;
	}

	fseek(file, elf_header.e_shoff, SEEK_SET);
	fread(section_headers, elf_header.e_shentsize, elf_header.e_shnum, file);

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

	Elf32_Shdr symbol_table_header = section_headers[symbol_table_index];
	Elf32_Sym *symbol_table = malloc(symbol_table_header.sh_size);

	fseek(file, symbol_table_header.sh_offset, SEEK_SET);
	fread(symbol_table, symbol_table_header.sh_size, 1, file);

	string_table_index = symbol_table_header.sh_link;

	Elf32_Shdr string_table_header = section_headers[string_table_index];

	char *string_table = malloc(string_table_header.sh_size);

	fseek(file, string_table_header.sh_offset, SEEK_SET);
	fread(string_table, string_table_header.sh_size, 1, file);

	print_symbol_table32(&symbol_table_header, symbol_table, string_table, section_headers);

	fclose(file);

	free(section_headers);
	free(symbol_table);
	free(string_table);
}