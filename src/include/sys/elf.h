/*
 * elf.h - XeOS ELF Reader
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This is an include file which describes ELF reading.
 *
 */
#include <sys/types.h>

#ifndef __ELF_H__
#define __ELF_H__

#define ELF_MAGIC 0x464c457f     /* elf magic */

#define ELF_MAX_PROGRAMS    16   /* maximum number of programs in ELF file */

#define ELF_OK                  0
#define ELF_ERROR_BADLEN        1
#define ELF_ERROR_NOTELF        2
#define ELF_ERROR_TOOMUCHPROGS  3	

struct ELF_HEADER {
	uint32_t	magic;                /*  0: elf magic */
	uint8_t		bitness;              /*  4: bitness */
	uint8_t		endian;               /*  5: endian */
	uint8_t		version;              /*  6: version */
	uint8_t		reserved1;            /*  7: reserved */
	uint32_t	reserved2;            /*  8: reserved */
	uint32_t	reserved3;            /* 0c: reserved */
	uint16_t	type;                 /* 10: file type */
	uint16_t	machine;              /* 12: machine */
	uint32_t	version2;             /* 14: more versions */
	uint32_t	entry;                /* 18: entry point */
	uint32_t	ph_offs;              /* 1c: Program Header table offset */
	uint32_t	sh_offs;              /* 20: Section Header table offset */
	uint32_t	flags;                /* 24: flags */
	uint16_t	size;                 /* 28: header size */
	uint16_t	ph_entry_size;        /* 2a: PH table entry size */
	uint16_t	ph_num_entries;       /* 2c: entries in PH table */
	uint16_t	sh_entry_size;        /* 2e: SH table entry size */
	uint16_t	sh_num_entries;       /* 30: entries in PH table */
	uint16_t	shstrtab_index;       /* 32: .shstrtab section index */
} __attribute__((packed));

#define ELF_STYPE_NULL		0
#define ELF_STYPE_BITS		1
#define ELF_STYPE_SYMTAB	2
#define ELF_STYPE_STRTAB	3
#define ELF_STYPE_RELA		4
#define ELF_STYPE_HASH		5
#define ELF_STYPE_DYNAMIC	6
#define ELF_STYPE_NOTE		7
#define ELF_STYPE_NOBITS	8
#define ELF_STYPE_REL     9
#define ELF_STYPE_SHLIB	  10
#define ELF_STYPE_DYNSYM	11

struct ELF_SECTION {
	uint32_t	name_offs;            /*  0: name, offset in .shstrtab */
	uint32_t	type;                 /*  4: section type */
	uint32_t	flags;                /*  8: flags */
	uint32_t	addr;                 /* 0c: load address */
	uint32_t	offset;               /* 10: offset of section in file */
	uint32_t	size;                 /* 14: section size */
	uint32_t	link;                 /* 18: link */
	uint32_t	info;                 /* 1c: info */
	uint32_t	align;                /* 20: required alignment */
	uint32_t	num_entries;          /* 24: number of entries */
} __attribute__((packed));

struct ELF_PROGRAM {
	uint32_t	type;                 /*  0: type */
	uint32_t	offset;               /*  4: file offset */
  uint32_t  virt_addr;            /*  8: virtual address */
  uint32_t  phys_addr;            /* 0c: physical address */
  uint32_t  file_size;            /* 10: size in file */
  uint32_t  mem_size;             /* 14: size in memory */
  uint32_t  flags;                /* 18: flags */
  uint32_t  align;                /* 1c: alignment */
} __attribute__((packed));

#ifdef __KERNEL
int elf_parse (uint8_t* data, size_t len, struct THREAD* t);
#endif /* __KERNEL */

#endif

/* vim:set ts=2: */
