/*
 * Put before every C snippet of the documentation by tools/docs/check_snippets.py:
 * the library and the board functions the snippets stand in for.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "x430.h"

/* Board and motor functions used by the snippets of docs/ */
void board_init(void);
int board_uart_write(const uint8_t* data, size_t length);
size_t board_uart_read(uint8_t* data, size_t size);
void board_uart_set_baud(uint8_t code);
uint32_t board_millis(void);
void board_delay_us(uint32_t us);
bool board_tick_1ms(void);
int flash_read(uint32_t key, uint16_t offset, uint8_t* data, uint16_t length);
int flash_write(uint32_t key, uint16_t offset, const uint8_t* data, uint16_t length);
void flash_erase(uint32_t key);
void mutex_lock(void* mutex);
void mutex_unlock(void* mutex);
extern int bus_mutex;
uint32_t port_millis(void* context);
void port_delay_us(void* context, uint32_t us);
bool motor_enable(bool on);
void motor_move_to(int32_t position);
void motor_run(int32_t velocity);
void motor_stop(void);
int32_t motor_position(void);
int32_t motor_velocity(void);
bool motor_moving(void);
