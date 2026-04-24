/**
 * @file    delay.c
 * @brief   Software delay routines for LPC2148 (12 MHz oscillator assumed).
 *
 * These are blocking, busy-wait delay functions calibrated for the
 * LPC2148 running at FOSC = 12 MHz (CCLK = 60 MHz, PCLK = 15 MHz).
 * The multiplier constants (12, 12000, 12000000) are tuned so that
 * each loop iteration consumes approximately 1 microsecond of CPU time.
 *
 * @note    Accuracy depends on compiler optimisation level and CPU speed.
 *          These delays are NOT interrupt-driven; the CPU is fully blocked.
 */

/**
 * @brief  Busy-wait delay for the specified number of microseconds.
 * @param  delay  Number of microseconds to wait.
 */
void delay_us(unsigned int delay)
{
    delay *= 12;          /* Scale to approx. loop count for 1 µs per iteration */
    while (delay--);      /* Decrement loop — CPU busy waits here */
}

/**
 * @brief  Busy-wait delay for the specified number of milliseconds.
 * @param  delay  Number of milliseconds to wait.
 */
void delay_ms(unsigned int delay)
{
    delay *= 12000;       /* 12000 iterations ≈ 1 ms at 12 MHz */
    while (delay--);
}

/**
 * @brief  Busy-wait delay for the specified number of seconds.
 * @param  delay  Number of seconds to wait.
 *
 * @warning  The multiplier 12000000 may overflow a 16-bit unsigned int.
 *           Ensure the argument is small (e.g., 1–3) to avoid wrap-around.
 */
void delay_s(unsigned int delay)
{
    delay *= 12000000;    /* 12,000,000 iterations ≈ 1 s at 12 MHz */
    while (delay--);
}
