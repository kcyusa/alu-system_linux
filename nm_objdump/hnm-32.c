#include "hnm.h"

/**
 * get_symbol_type - determines the symbol type character based on ELF symbol properties
 * @symbol: the ELF symbol structure
 * @section_headers: array of section headers
 * Return: character representing the symbol type
 */
char get_symbol_type(Elf32_Sym symbol, Elf32_Shdr *section_headers)
{
	char symbol_type = '?';

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
		{
			if (symbol_section.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
				symbol_type = 'T';
			else if (symbol_section.sh_flags == SHF_ALLOC)
				symbol_type = 'R';
			else if (symbol_section.sh_flags == (SHF_ALLOC | SHF_WRITE))
				symbol_type = 'D';
		}
		else if (symbol_section.sh_type == SHT_DYNAMIC)
			symbol_type = 'D';
		else
			symbol_type = 't';
	}
	return (symbol_type);
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
			symbol_type = get_symbol_type(symbol, section_headers);

			if (ELF32_ST_BIND(symbol.st_info) == STB_LOCAL)
				symbol_type = tolower(symbol_type);
			
			if (symbol_type != 'U' && symbol_type != 'w')
				printf("%08x %c %s\n", symbol.st_value, symbol_type, symbol_name);
			else
				printf("         %c %s\n", symbol_type, symbol_name);
		}
	}
}

/**
 * read_elf_data - reads and validates ELF header and section headers
 * @file: file pointer
 * @elf_header: pointer to store ELF header
 * @section_headers: pointer to store section headers pointer
 * @file_path: path to the file being processed
 * Return: 1 on success, 0 on failure
 */
int read_elf_data(FILE *file, Elf32_Ehdr *elf_header, Elf32_Shdr **section_headers, char *file_path)
{
	int is_little_endian, is_big_endian;

	fread(elf_header, sizeof(Elf32_Ehdr), 1, file);

	if (elf_header->e_ident[EI_CLASS] != ELFCLASS32 && elf_header->e_ident[EI_CLASS] != ELFCLASS64)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file format\n", file_path);
		return 0;
	}

	is_little_endian = (elf_header->e_ident[EI_DATA] == ELFDATA2LSB);
	is_big_endian = (elf_header->e_ident[EI_DATA] == ELFDATA2MSB);

	if (!is_little_endian && !is_big_endian)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file endianness\n", file_path);
		return 0;
	}

	*section_headers = malloc(elf_header->e_shentsize * elf_header->e_shnum);
	if (*section_headers == NULL)
	{
		fprintf(stderr, "./hnm: %s: memory allocation error for section_headers\n", file_path);
		return 0;
	}

	fseek(file, elf_header->e_shoff, SEEK_SET);
	fread(*section_headers, elf_header->e_shentsize, elf_header->e_shnum, file);

	return 1;
}

/**
 * find_and_read_tables - finds symbol table and reads symbol/string tables
 * @file: file pointer
 * @section_headers: array of section headers
 * @elf_header: ELF header structure
 * @symbol_table: pointer to store symbol table pointer
 * @string_table: pointer to store string table pointer
 * @symbol_table_header: pointer to store symbol table header
 * @file_path: path to the file being processed
 * Return: 1 on success, 0 on failure
 */
int find_and_read_tables(FILE *file, Elf32_Shdr *section_headers, Elf32_Ehdr *elf_header,
			 Elf32_Sym **symbol_table, char **string_table, 
			 Elf32_Shdr *symbol_table_header, char *file_path)
{
	int i, symbol_table_index = -1, string_table_index;
	Elf32_Shdr string_table_header;

	for (i = 0; i < elf_header->e_shnum; i++)
	{
		if (i < elf_header->e_shnum && section_headers[i].sh_type == SHT_SYMTAB)
		{
			symbol_table_index = i;
			break;
		}
	}

	if (symbol_table_index == -1)
	{
		fprintf(stderr, "./hnm: %s: no symbols\n", file_path);
		return 0;
	}

	*symbol_table_header = section_headers[symbol_table_index];
	*symbol_table = malloc(symbol_table_header->sh_size);

	fseek(file, symbol_table_header->sh_offset, SEEK_SET);
	fread(*symbol_table, symbol_table_header->sh_size, 1, file);

	string_table_index = symbol_table_header->sh_link;
	string_table_header = section_headers[string_table_index];

	*string_table = malloc(string_table_header.sh_size);

	fseek(file, string_table_header.sh_offset, SEEK_SET);
	fread(*string_table, string_table_header.sh_size, 1, file);

	return 1;
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
	FILE *file = fopen(file_path, "rb");
	Elf32_Ehdr elf_header;
	Elf32_Shdr *section_headers = NULL;
	Elf32_Sym *symbol_table = NULL;
	char *string_table = NULL;
	Elf32_Shdr symbol_table_header;

	if (file == NULL)
	{
		fprintf(stderr, "./hnm: %s: failed to open file\n", file_path);
		return;
	}

	if (!read_elf_data(file, &elf_header, &section_headers, file_path))
	{
		fclose(file);
		return;
	}

	if (!find_and_read_tables(file, section_headers, &elf_header, &symbol_table, 
				  &string_table, &symbol_table_header, file_path))
	{
		fclose(file);
		free(section_headers);
		return;
	}

	print_symbol_table32(&symbol_table_header, symbol_table, string_table, section_headers);

	fclose(file);
	free(section_headers);
	free(symbol_table);
	free(string_table);
}
