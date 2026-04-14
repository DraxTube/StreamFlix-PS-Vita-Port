/*
 * patch_relocs.c - Patch unsupported ARM relocation types in ELF files
 * 
 * vita-elf-create only supports a limited set of ARM relocation types.
 * Pre-compiled libraries (curl, openssl, etc.) may contain unsupported
 * types like R_ARM_GOTPC (25), R_ARM_GOT_BREL (26), etc.
 * This tool patches them to R_ARM_NONE (0) so vita-elf-create succeeds.
 *
 * Supported by vita-elf-create:
 *   0  R_ARM_NONE, 2  R_ARM_ABS32, 3  R_ARM_REL32,
 *  10  R_ARM_THM_CALL, 28 R_ARM_CALL, 29 R_ARM_JUMP24,
 *  38  R_ARM_TARGET1, 40 R_ARM_V4BX, 41 R_ARM_TARGET2,
 *  42  R_ARM_PREL31, 43 R_ARM_MOVW_ABS_NC, 44 R_ARM_MOVT_ABS,
 *  47  R_ARM_THM_MOVW_ABS_NC, 48 R_ARM_THM_MOVT_ABS
 *
 * Usage: patch_relocs <input.elf>
 * Modifies the file in-place.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ELF32 structures */
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type, e_machine;
    uint32_t e_version, e_entry, e_phoff, e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize, e_phentsize, e_phnum;
    uint16_t e_shentsize, e_shnum, e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t sh_name, sh_type, sh_flags, sh_addr;
    uint32_t sh_offset, sh_size, sh_link, sh_info;
    uint32_t sh_addralign, sh_entsize;
} Elf32_Shdr;

typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
} Elf32_Rel;

#define SHT_REL  9

#define ELF32_R_TYPE(i)    ((i) & 0xFF)
#define ELF32_R_SYM(i)    ((i) >> 8)
#define ELF32_R_INFO(s,t)  (((s) << 8) | ((t) & 0xFF))

static int is_supported(uint32_t type) {
    switch (type) {
        case 0:  /* R_ARM_NONE */
        case 2:  /* R_ARM_ABS32 */
        case 3:  /* R_ARM_REL32 */
        case 10: /* R_ARM_THM_CALL */
        case 28: /* R_ARM_CALL */
        case 29: /* R_ARM_JUMP24 */
        case 38: /* R_ARM_TARGET1 */
        case 40: /* R_ARM_V4BX */
        case 41: /* R_ARM_TARGET2 */
        case 42: /* R_ARM_PREL31 */
        case 43: /* R_ARM_MOVW_ABS_NC */
        case 44: /* R_ARM_MOVT_ABS */
        case 47: /* R_ARM_THM_MOVW_ABS_NC */
        case 48: /* R_ARM_THM_MOVT_ABS */
            return 1;
        default:
            return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf-file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r+b");
    if (!f) {
        perror("fopen");
        return 1;
    }

    /* Read ELF header */
    Elf32_Ehdr ehdr;
    fread(&ehdr, sizeof(ehdr), 1, f);

    if (memcmp(ehdr.e_ident, "\x7f""ELF", 4) != 0) {
        fprintf(stderr, "Not an ELF file\n");
        fclose(f);
        return 1;
    }

    int patched = 0;
    int types_seen[256] = {0};

    /* Iterate over section headers */
    for (int i = 0; i < ehdr.e_shnum; i++) {
        Elf32_Shdr shdr;
        fseek(f, ehdr.e_shoff + i * ehdr.e_shentsize, SEEK_SET);
        fread(&shdr, sizeof(shdr), 1, f);

        if (shdr.sh_type != SHT_REL)
            continue;

        /* Process each relocation entry */
        int num_rels = shdr.sh_size / sizeof(Elf32_Rel);
        for (int j = 0; j < num_rels; j++) {
            Elf32_Rel rel;
            long rel_offset = shdr.sh_offset + j * sizeof(Elf32_Rel);
            fseek(f, rel_offset, SEEK_SET);
            fread(&rel, sizeof(rel), 1, f);

            uint32_t type = ELF32_R_TYPE(rel.r_info);
            if (!is_supported(type)) {
                types_seen[type]++;
                uint32_t sym = ELF32_R_SYM(rel.r_info);
                rel.r_info = ELF32_R_INFO(sym, 0); /* R_ARM_NONE */
                fseek(f, rel_offset, SEEK_SET);
                fwrite(&rel, sizeof(rel), 1, f);
                patched++;
            }
        }
    }

    fclose(f);
    printf("Patched %d unsupported relocations to R_ARM_NONE\n", patched);
    for (int t = 0; t < 256; t++) {
        if (types_seen[t])
            printf("  Type %d: %d entries patched\n", t, types_seen[t]);
    }
    return 0;
}
