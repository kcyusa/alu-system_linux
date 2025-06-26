#include "hnm.h"

/**
 * print_symbol_table64 - program that prints the symbol table for a 64-bit ELF file
 * this function iterates over the symbols in the given symbol table and prints
 * details about each symbol, such as its name, type, and associated section
 * @section_header: a pointer to the section header of the symbol table
 * @symbol_table: a pointer to the beginning of the symbol table
 * @string_table: a pointer to the beginning of the string table,
 *                which contains the names of the symbols
 * @section_headers: a pointer to the array of section headers for the ELF file
 * Return: nothing (void)
 */

void print_symbol_table64(Elf64_Shdr *section_header, Elf64_Sym *symbol_table,
			  char *string_table, Elf64_Shdr *section_headers)
{
	int i;
	int symbol_count = section_header->sh_size / sizeof(Elf64_Sym);
	char *symbol_name, symbol_type;

	for (i = 0; i < symbol_count; i++)
	{
		Elf64_Sym symbol = symbol_table[i];
		symbol_name = string_table + symbol.st_name;
		/*
		 * Le symbole doit avoir un nom (!= 0),
		 * et le symbole ne doit pas être le nom d'un fichier source
		 */
		if (symbol.st_name != 0 && ELF64_ST_TYPE(symbol.st_info) != STT_FILE)
		{
			symbol_type = '?';
			/*
			 * On s'assure que les symboles faibles non définis sont correctement
			 * marqués comme 'w' avant d'attribuer 'U' aux symboles indéfinis
			 */
			if (ELF64_ST_BIND(symbol.st_info) == STB_WEAK)
			{
				if (symbol.st_shndx == SHN_UNDEF)
				{
					/* Symbole weak indéfini */
					symbol_type = 'w';
				}
				else if (ELF64_ST_TYPE(symbol.st_info) == STT_OBJECT)
				{
					/* Symbole weak objet */
					symbol_type = 'V';
				}
				else
				{
					/* Symbole weak défini */
					symbol_type = 'W';
				}
			}
			/* Indices de sections speciales */
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
			/*
			 * Cette condition dans le code vérifie si le symbole est associé à une section normale
			 * (et non à une section spéciale)
			 */
			else if (symbol.st_shndx < SHN_LORESERVE)
			{
				/*
				 * S'il ne s'agit pas d'une section spéciale,
				 * récupérer alors l'en-tête de la section
				 */
				Elf64_Shdr symbol_section = section_headers[symbol.st_shndx];

				/* Vérifier les symboles faibles et uniques */
				if (ELF64_ST_BIND(symbol.st_info) == STB_GNU_UNIQUE)
				{
					symbol_type = 'u';
				}
				/* Vérifier les types de section et les flags */
				else if (symbol_section.sh_type == SHT_NOBITS &&
					symbol_section.sh_flags == (SHF_ALLOC | SHF_WRITE))
				{
					symbol_type = 'B';
				}
				else if (symbol_section.sh_type == SHT_PROGBITS)
				{
					if (symbol_section.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
					{
						symbol_type = 'T';
					}
					else if (symbol_section.sh_flags == SHF_ALLOC)
					{
						symbol_type = 'R';
					}
					else if (symbol_section.sh_flags == (SHF_ALLOC | SHF_WRITE))
					{
						symbol_type = 'D';
					}
				}
				else if (symbol_section.sh_type == SHT_DYNAMIC)
				{
					symbol_type = 'D';
				}
				else
				{
					symbol_type = 't';
				}
			}
			/* Convertir en minuscule si le symbole est local */
			if (ELF64_ST_BIND(symbol.st_info) == STB_LOCAL)
			{
				symbol_type = tolower(symbol_type);
			}
			/* Ne pas afficher l'adresse du symbole si elle équivaut à U ou w */
			if (symbol_type != 'U' && symbol_type != 'w')
			{
				printf("%016lx %c %s\n", symbol.st_value, symbol_type, symbol_name);
			}
			else
			{
				printf("                 %c %s\n", symbol_type, symbol_name);
			}
		}
	}
}

/**
 * process_elf_file64 - program that processes a 64-bit ELF file
 * located at the given file path
 * this function opens the file, verifies its format and endianness,
 * and reads the section headers;
 * it locates the symbol table within the section headers and reads it;
 * finally, it reads the associated string table and calls
 * 'print_symbol_table64' to print the symbols
 * @file_path: a pointer to a string that contains the path to the ELF file
 *             to be processed
 * Return: nothing (void)
 */

void process_elf_file64(char *file_path)
{
	int symbol_table_index = -1;
	int i;
	int is_little_endian, is_big_endian;
	int string_table_index;

	FILE *file = fopen(file_path, "rb");

	if (file == NULL)
	{
		fprintf(stderr,"./hnm: %s: failed to open file\n", file_path);
		return;
	}

	Elf64_Ehdr elf_header;

	fread(&elf_header, sizeof(Elf64_Ehdr), 1, file);

	if (elf_header.e_ident[EI_CLASS] != ELFCLASS32 && elf_header.e_ident[EI_CLASS] != ELFCLASS64)
	{
		fprintf(stderr,"./hnm: %s: unsupported ELF file format\n", file_path);
		fclose(file);
		return;
	}

	is_little_endian = (elf_header.e_ident[EI_DATA] == ELFDATA2LSB);
	is_big_endian = (elf_header.e_ident[EI_DATA] == ELFDATA2MSB);

	if (!is_little_endian && !is_big_endian)
	{
		fprintf(stderr,"./hnm: %s: unsupported ELF file endianness\n", file_path);
		fclose(file);
		return;
	}

	/* Pour la table des sections */
	Elf64_Shdr *section_headers = malloc(elf_header.e_shentsize * elf_header.e_shnum);

	if (section_headers == NULL)
	{
		fprintf(stderr,"./hnm: %s: memory allocation error for section_headers\n", file_path);
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
		fprintf(stderr,"./hnm: %s: no symbols\n", file_path);
		fclose(file);
		free(section_headers);
		return;
	}

	// Lire la table des symboles */
	Elf64_Shdr symbol_table_header = section_headers[symbol_table_index];
	Elf64_Sym *symbol_table = malloc(symbol_table_header.sh_size);

	fseek(file, symbol_table_header.sh_offset, SEEK_SET);
	fread(symbol_table, symbol_table_header.sh_size, 1, file);

	string_table_index = symbol_table_header.sh_link;

	Elf64_Shdr string_table_header = section_headers[string_table_index];

	char *string_table = malloc(string_table_header.sh_size);

	fseek(file, string_table_header.sh_offset, SEEK_SET);
	fread(string_table, string_table_header.sh_size, 1, file);

	/* Afficher la table des symboles */
	print_symbol_table64(&symbol_table_header, symbol_table, string_table, section_headers);

	fclose(file);

	free(section_headers);
	free(symbol_table);
	free(string_table);
}