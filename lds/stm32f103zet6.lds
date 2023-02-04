/* SPDX-License-Identifier: MIT */
/* memopry description script for stm32f103zet6 */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}