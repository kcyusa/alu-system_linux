#include "hnm.h"

/**
 * get_symbol_type32 - Determines symbol type based on ELF attributes.
 * @symbol: ELF symbol.
 * @section_headers: ELF section headers.
 * Return: Symbol type character.
 */
char get_symbol_type32(Elf32_Sym symbol, Elf32_Shdr *section_headers)
{
	Elf32_Shdr section;
	char type = '?';

	if (ELF32_ST_BIND(symbol.st_info) == STB_WEAK)
	{
		if (symbol.st_shndx == SHN_UNDEF)
			return ('w');
		else if (ELF32_ST_TYPE(symbol.st_info) == STT_OBJECT)
			return ('V');
		else
			return ('W');
	}
	if (symbol.st_shndx == SHN_UNDEF)
		type = 'U';
	else if (symbol.st_shndx == SHN_ABS)
		type = 'A';
	else if (symbol.st_shndx == SHN_COMMON)
		type = 'C';
	else if (symbol.st_shndx < SHN_LORESERVE)
	{
		section = section_headers[symbol.st_shndx];
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
			else
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
 * print_symbol_table32 - Prints the symbol table for a 32-bit ELF file
 * @section_header: pointer to the section header containing the symbol table
 * @symbol_table: pointer to the array of symbol entries
 * @string_table: pointer to the string table for symbol names
 * @section_headers: pointer to the full array of section headers
 *
 * This function interprets symbol information including type, binding,
 * and section to display human-readable ELF symbol data.
 */
void print_symbol_table32(Elf32_Shdr *section_header, Elf32_Sym *symbol_table,
						  char *string_table, Elf32_Shdr *section_headers)
{
	int i, count = section_header->sh_size / sizeof(Elf32_Sym);

	for (i = 0; i < count; i++)
	{
		Elf32_Sym sym = symbol_table[i];

		if (sym.st_name && ELF32_ST_TYPE(sym.st_info) != STT_FILE)
		{
			char type = get_symbol_type32(sym, section_headers);

			printf((type != 'U' && type != 'w') ? "%08x %c %s\n" : "         %c %d\n",
				   sym.st_value, type, string_table + sym.st_name);
		}
	}
}

/**
 * load_section_headers - Loads section headers from a 32-bit ELF file
 * @file: pointer to the opened ELF file
 * @hdr: pointer to the ELF32 header structure
 *
 * Return: pointer to an array of section headers, or NULL on failure
 */

Elf32_Shdr *load_section_headers(FILE *file, Elf32_Ehdr *hdr)
{
	Elf32_Shdr *sections = malloc(hdr->e_shentsize * hdr->e_shnum);

	if (!sections)
	{
		return (NULL);
	}
	fseek(file, hdr->e_shoff, SEEK_SET);
	fread(sections, hdr->e_shentsize, hdr->e_shnum, file);
	return (sections);
}

/**
 * handle_symbol_output32 - Reads and prints
 * the symbol table, then frees resources.
 * @file: pointer to the opened ELF file
 * @sym_hdr: section header for the symbol table
 * @str_hdr: section header for the string table
 * @sections: pointer to the array of section headers
 * Return: nothing (void)
 */
void handle_symbol_output32(FILE *file, Elf32_Shdr sym_hdr,
							Elf32_Shdr str_hdr, Elf32_Shdr *sections)
{
	Elf32_Sym *symbols = malloc(sym_hdr.sh_size);
	char *strings = malloc(str_hdr.sh_size);

	if (!symbols || !strings)
	{
		fprintf(stderr, "Memory allocation failed\n");
		free(symbols);
		free(strings);
		free(sections);
		fclose(file);
		return;
	}

	fseek(file, sym_hdr.sh_offset, SEEK_SET);
	fread(symbols, sym_hdr.sh_size, 1, file);

	fseek(file, str_hdr.sh_offset, SEEK_SET);
	fread(strings, str_hdr.sh_size, 1, file);

	print_symbol_table32(&sym_hdr, symbols, strings, sections);

	free(sections);
	free(symbols);
	free(strings);
	fclose(file);
}

/**
 * process_elf_file32 - Processes
 * and prints symbol information from a 32-bit ELF file
 * @path: path to the ELF file to be processed
 *
 * This function opens the ELF file, verifies it, loads section headers,
 * reads symbol and string tables, and prints the symbol table.
 */
void process_elf_file32(char *path)
{
	int i, sym_index = -1, str_index;
	Elf32_Ehdr hdr;
	FILE *file = fopen(path, "rb");
	Elf32_Shdr *sections, sym_hdr, str_hdr;

	if (!file)
	{
		(void)fprintf(stderr, "./hnm: %s: open fail\n", path);
		return;
	}
	sections = load_section_headers(file, &hdr);
	if (!sections)
	{
		fclose(file);
		(void)fprintf(stderr, "./hnm: %s: alloc fail\n", path);
		return;
	}

	for (i = 0; i < hdr.e_shnum; i++)
	{
		if (sections[i].sh_type == SHT_SYMTAB)
		{
			sym_index = i;
			break;
		}
	}
	if (sym_index == -1)
	{
		fclose(file);
		free(sections);
		(void)fprintf(stderr, "./hnm: %s: no symbols\n", path);
		return;
	}
	sym_hdr = sections[sym_index];
	str_index = sym_hdr.sh_link;
	str_hdr = sections[str_index];

	handle_symbol_output32(file, sym_hdr, str_hdr, sections);
}
