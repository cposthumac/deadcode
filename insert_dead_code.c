#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t sh_offset;
    uint64_t sh_size;
} Elf_Shdr;

typedef struct {
    char* file_sections[10];
    Elf_Shdr s_hdr[10];
    int e_shnum;
} Elf_Manager;

Elf_Manager* load_elf_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    Elf_Manager* manager = malloc(sizeof(Elf_Manager));
    if (manager == NULL) {
        perror("Error allocating memory for Elf_Manager");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < 10; ++i) {
        manager->file_sections[i] = NULL;
    }

    manager->file_sections[0] = malloc(file_size);
    if (manager->file_sections[0] == NULL) {
        perror("Error allocating memory for file section");
        free(manager);
        fclose(file);
        return NULL;
    }

    fread(manager->file_sections[0], 1, file_size, file);
    manager->s_hdr[0].sh_offset = 0;
    manager->s_hdr[0].sh_size = file_size;
    manager->e_shnum = 1;

    fclose(file);
    return manager;
}

void insert_dead_code(Elf_Manager* manager, int section_index) {
    printf("Inserting dead code into section %d...\n", section_index);

    if (manager == NULL || manager->file_sections[section_index] == NULL) {
        fprintf(stderr, "Error: Null pointer encountered.\n");
        return;
    }

    uint64_t start = manager->s_hdr[section_index].sh_offset;
    uint64_t end = start + manager->s_hdr[section_index].sh_size;

    uint8_t prologue[] = {0x55, 0x48, 0x89, 0xE5};

    if (manager->s_hdr[section_index].sh_size < sizeof(prologue)) {
        fprintf(stderr, "Section size is too small for prologue.\n");
        return;
    }

    for (uint64_t j = start; j < end - sizeof(prologue); ++j) {
        if (memcmp(manager->file_sections[section_index] + j, prologue, sizeof(prologue)) == 0) {
            uint64_t insertion_point = j + sizeof(prologue);
            for (uint64_t k = 0; k < 16; ++k) {
                if (insertion_point + k < manager->s_hdr[section_index].sh_size) {
                    manager->file_sections[section_index][insertion_point + k] = 0x90;
                } else {
                    break;
                }
            }
            break;
        }
    }
}

void save_modified_elf(Elf_Manager* manager, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    fwrite(manager->file_sections[0], 1, manager->s_hdr[0].sh_size, file);
    fclose(file);
}

void free_manager(Elf_Manager* manager) {
    if (manager == NULL) return;

    for (int i = 0; i < 10; ++i) {
        free(manager->file_sections[i]);
    }
    free(manager);
}

int main() {
    Elf_Manager* manager = load_elf_file("sample_elf.bin");
    if (manager == NULL) {
        fprintf(stderr, "Error loading ELF file\n");
        return 1;
    }

    // Insert dead code
    insert_dead_code(manager, 0);

    // Save modified ELF file
    save_modified_elf(manager, "sample_elf_after_modification.bin");

    // Free allocated memory
    free_manager(manager);

    return 0;
}
