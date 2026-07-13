#ifndef AUD_EURO_EXAMPLE_STM32H7_MEMORY_H_
#define AUD_EURO_EXAMPLE_STM32H7_MEMORY_H_

/* Map these sections in the Keil scatter file or GNU linker script. */
#if defined(__GNUC__)
#define AUD_EXAMPLE_FAST_RAM __attribute__((section(".aud_fast"), aligned(32)))
#define AUD_EXAMPLE_LARGE_RAM __attribute__((section(".aud_large"), aligned(32)))
#else
#define AUD_EXAMPLE_FAST_RAM
#define AUD_EXAMPLE_LARGE_RAM
#endif

#endif  /* AUD_EURO_EXAMPLE_STM32H7_MEMORY_H_ */
