/**
 * @file debug.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Defines some debug stuff if set in the build config
 * @version 0.1
 * @date 2025-02-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define SCAN_DEBUG

#define CLOCK_DEBUG

#define UART_DEBUG
#endif

#endif