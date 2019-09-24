
#pragma once

#include <stddef.h>
#include <stdint.h>

#define PROGMEM __attribute__((aligned(4))) __attribute__((section(".irom.text")))

/*
 * Define and use a flash string inline
 */
#define PSTR(_str)                                                                                                     \
	(__extension__({                                                                                                   \
		static const char __c[] PROGMEM = _str;                                                                        \
		&__c[0];                                                                                                       \
	}))

#ifdef ICACHE_FLASH
void *memcpy_P(void *dest, const void *src_P, size_t length);

// flash memory must be read using 32 bit aligned addresses else a processor exception will be triggered
// order within the 32 bit values are
// --------------
// b3, b2, b1, b0
//     w1,     w0

#define pgm_read_with_offset(addr, res)                                                                                \
	__asm__("extui    %0, %1, 0, 2\n" /* Extract offset within word (in bytes) */                                      \
			"sub      %1, %1, %0\n"   /* Subtract offset from addr, yielding an aligned address */                     \
			"l32i.n   %1, %1, 0x0\n"  /* Load word from aligned address */                                             \
			"slli     %0, %0, 3\n"	/* Mulitiply offset by 8, yielding an offset in bits */                          \
			"ssr      %0\n"			  /* Prepare to shift by offset (in bits) */                                       \
			"srl      %0, %1\n"		  /* Shift right; now the requested byte is the first one */                       \
			: "=r"(res), "=r"(addr)                                                                                    \
			: "1"(addr)                                                                                                \
			:);

static inline uint8_t pgm_read_byte_inlined(const void* addr)
{
	register uint32_t res;
	pgm_read_with_offset(addr, res);
	return (uint8_t)res; /* This masks the lower byte from the returned word */
}

/* Although this says "word", it's actually 16 bit, i.e. half word on Xtensa */
static inline uint16_t pgm_read_word_inlined(const void* addr)
{
	register uint32_t res;
	pgm_read_with_offset(addr, res);
	return (uint16_t)res; /* This masks the lower half-word from the returned word */
}

// Make sure, that libraries checking existence of this macro are not failing
#define pgm_read_byte(addr) pgm_read_byte_inlined(addr)
#define pgm_read_word(addr) pgm_read_word_inlined(addr)

// No translation necessary (provided address is aligned)
#define pgm_read_dword(addr) (*(const unsigned long*)(addr))
#define pgm_read_float(addr) (*(const float*)(addr))

#else

#define memcpy_P(dest, src, num) memcpy((dest), (src), (num))

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#define pgm_read_float(addr) (*(const float *)(addr))

#endif
