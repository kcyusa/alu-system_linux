#include "hnm.h"

/**
 * main - the entry point
 * program made for the ELF file processing program
 * @argc: the count of command-line arguments
 * @argv: the array of command-line arguments,
 *        where argv[1] is expected to be the path to the ELF file
 * Return: 0 on successful execution,
 *         or 0 in case of an error such as missing file argument
 *         or unsupported ELF file type
 * Author: Frank Onyema Orji
 */

int main(int argc, char *argv[])
{
    char *file_path = argv[1];

    if (argc < 2)
    {
        printf("Il faut fournir un fichier ELF !\n");
        return (0);
    }

    FILE *file = fopen(file_path, "rb");

    if (file == NULL)
    {
        printf("Il y a une erreur pour de l'ouverture du fichier\n");
        return (0);
    }

    Elf64_Ehdr elf_header;
    fread(&elf_header, sizeof(Elf64_Ehdr), 1, file);

    if (elf_header.e_ident[EI_CLASS] == ELFCLASS32)
    {
        process_elf_file32(file_path);
    }
    else if (elf_header.e_ident[EI_CLASS] == ELFCLASS64)
    {
        process_elf_file64(file_path);
    }
    else
    {
        printf("Type de fichier ELF non pris en charge...\n");
    }

    fclose(file);

    return (0);
}