
#include "hnm.h"

/**
 * print_symbol_entry32 - Prints one symbol entry in ELF 32-bit file.
 * @symbol: the symbol entry
 * @string_table: the string table to resolve symbol name
 * @section_headers: array of section headers
 */
static void print_symbol_entry32(Elf32_Sym symbol, char *string_table,
				 Elf32_Shdr *section_headers)
{
	char *symbol_name = string_table + symbol.st_name;
	char symbol_type = '?';

	if (symbol.st_name == 0 || ELF32_ST_TYPE(symbol.st_info) == STT_FILE)
		return;

	if (ELF32_ST_BIND(symbol.st_info) == STB_WEAK)
	{
		if (symbol.st_shndx == SHN_UNDEF)
			symbol_type = 'w';
		else if (ELF32_ST_TYPE(symbol.st_info) == STT_OBJECT)
			symbol_type = 'V';
		else
			symbol_type = 'W';
	}
	else if (symbol.st_shndx == SHN_UNDEF)
	{
		symbol_type = 'U';
	}
	else if (symbol.st_shndx == SHN_ABS)
	{
		symbol_type = 'A';
	}
	else if (symbol.st_shndx == SHN_COMMON)
	{
		symbol_type = 'C';
	}
	else if (symbol.st_shndx < SHN_LORESERVE)
	{
		Elf32_Shdr sec = section_headers[symbol.st_shndx];

		if (ELF32_ST_BIND(symbol.st_info) == STB_GNU_UNIQUE)
			symbol_type = 'u';
		else if (sec.sh_type == SHT_NOBITS &&
			 sec.sh_flags == (SHF_ALLOC | SHF_WRITE))
			symbol_type = 'B';
		else if (sec.sh_type == SHT_PROGBITS)
		{
			if (sec.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
				symbol_type = 'T';
			else if (sec.sh_flags == SHF_ALLOC)
				symbol_type = 'R';
			else if (sec.sh_flags == (SHF_ALLOC | SHF_WRITE))
				symbol_type = 'D';
		}
		else if (sec.sh_type == SHT_DYNAMIC)
		{
			symbol_type = 'D';
		}
		else
		{
			symbol_type = 't';
		}
	}

	if (ELF32_ST_BIND(symbol.st_info) == STB_LOCAL)
		symbol_type = tolower(symbol_type);

	if (symbol_type != 'U' && symbol_type != 'w')
		printf("%08x %c %s", symbol.st_value, symbol_type, symbol_name);
	else
		printf("         %c %s", symbol_type, symbol_name);
}

/**
 * print_symbol_table32 - Prints all symbol entries from a 32-bit ELF file.
 * @section_header: header of the symbol table section
 * @symbol_table: pointer to the symbol table
 * @string_table: string table for symbol names
 * @section_headers: full list of section headers
 */
void print_symbol_table32(Elf32_Shdr *section_header, Elf32_Sym *symbol_table,
			  char *string_table, Elf32_Shdr *section_headers)
{
	int i;
	int symbol_count = section_header->sh_size / sizeof(Elf32_Sym);

	for (i = 0; i < symbol_count; i++)
		print_symbol_entry32(symbol_table[i], string_table, section_headers);
}

/**
 * validate_elf_file32 - Validates ELF header for 32-bit class and endianness.
 * @file_path: path of the file
 * @file: file pointer
 * @elf_header: pointer to ELF header to validate
 * Return: 1 if valid, 0 otherwise
 */
static int validate_elf_file32(char *file_path, FILE *file,
			       Elf32_Ehdr *elf_header)
{
	fread(elf_header, sizeof(Elf32_Ehdr), 1, file);

	if (elf_header->e_ident[EI_CLASS] != ELFCLASS32 &&
	    elf_header->e_ident[EI_CLASS] != ELFCLASS64)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file format",
			file_path);
		fclose(file);
		return (0);
	}

	if (elf_header->e_ident[EI_DATA] != ELFDATA2LSB &&
	    elf_header->e_ident[EI_DATA] != ELFDATA2MSB)
	{
		fprintf(stderr, "./hnm: %s: unsupported ELF file endianness",
			file_path);
		fclose(file);
		return (0);
	}

	return (1);
}

/**
 * read_section_headers32 - Allocates and reads ELF section headers.
 * @file_path: file path for error reporting
 * @file: file pointer
 * @elf_header: pointer to ELF header
 * Return: pointer to section headers or NULL
 */
static Elf32_Shdr *read_section_headers32(char *file_path, FILE *file,
					  Elf32_Ehdr *elf_header)
{
	Elf32_Shdr *section_headers;

	section_headers = malloc(elf_header->e_shentsize *
				 elf_header->e_shnum);

	if (!section_headers)
	{
		fprintf(stderr,
			"./hnm: %s: memory allocation error for section_headers",
			file_path);
		fclose(file);
		return (NULL);
	}

	fseek(file, elf_header->e_shoff, SEEK_SET);
	fread(section_headers, elf_header->e_shentsize,
	      elf_header->e_shnum, file);

	return (section_headers);
}

/**
 * process_elf_file32 - Processes a 32-bit ELF file at given path.
 * @file_path: file path
 */
void process_elf_file32(char *file_path)
{
	int i;
	int symbol_table_index = -1;
	int string_table_index;
	FILE *file = fopen(file_path, "rb");
	Elf32_Ehdr elf_header;
	Elf32_Shdr *section_headers;
	Elf32_Shdr symbol_table_header, string_table_header;
	Elf32_Sym *symbol_table;
	char *string_table;

	if (!file)
	{
		fprintf(stderr, "./hnm: %s: failed to open file", file_path);
		return;
	}

	if (!validate_elf_file32(file_path, file, &elf_header))
		return;

	section_headers = read_section_headers32(file_path, file, &elf_header);
	if (!section_headers)
		return;

	for (i = 0; i < elf_header.e_shnum; i++)
	{
		if (section_headers[i].sh_type == SHT_SYMTAB)
		{
			symbol_table_index = i;
			break;
		}
	}

	if (symbol_table_index == -1)
	{
		fprintf(stderr, "./hnm: %s: no symbols", file_path);
		fclose(file);
		free(section_headers);
		return;
	}

	symbol_table_header = section_headers[symbol_table_index];
	symbol_table = malloc(symbol_table_header.sh_size);
	fseek(file, symbol_table_header.sh_offset, SEEK_SET);
	fread(symbol_table, symbol_table_header.sh_size, 1, file);

	string_table_index = symbol_table_header.sh_link;
	string_table_header = section_headers[string_table_index];
	string_table = malloc(string_table_header.sh_size);
	fseek(file, string_table_header.sh_offset, SEEK_SET);
	fread(string_table, string_table_header.sh_size, 1, file);

	print_symbol_table32(&symbol_table_header, symbol_table,
			     string_table, section_headers);

	fclose(file);
	free(section_headers);
	free(symbol_table);
	free(string_table);
}
