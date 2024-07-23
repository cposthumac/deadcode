#include <stdio.h>
#include <stdint.h>
#include <string.h>

void create_sample_elf(const char* filename) {
    uint8_t prologue[] = {0x55, 0x48, 0x89, 0xE5}; // push rbp, mov rbp, rsp
    uint8_t rest_of_section[60]; // Fill the rest with zeros
    memset(rest_of_section, 0x00, sizeof(rest_of_section));

    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error creating file");
        return;
    }

    fwrite(prologue, 1, sizeof(prologue), file);
    fwrite(rest_of_section, 1, sizeof(rest_of_section), file);

    fclose(file);
}

int main() {
    create_sample_elf("sample_elf.bin");
    return 0;
}
