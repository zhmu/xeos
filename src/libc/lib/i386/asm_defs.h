#define CNAME(csym)	csym

/* XXX should use .p2align 4,0x90 for -m486. */
#define _START_ENTRY    .text; .p2align 2,0x90

#define _ENTRY(x)       _START_ENTRY; \
                        .globl CNAME(x); .type CNAME(x),@function; CNAME(x):

#define ENTRY(x)	_ENTRY(x)
