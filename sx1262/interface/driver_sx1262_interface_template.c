/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_sx1262_interface_template.c
 * @brief     driver sx1262 interface source file (STM32 HAL)
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2024-10-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2024/10/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_sx1262_interface.h"
#include "spi.h"
#include "main.h"
#include <stdarg.h>

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t sx1262_interface_spi_init(void)
{
    /* SPI2 has been initialized by MX_SPI2_Init, do nothing here. */
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t sx1262_interface_spi_deinit(void)
{
    return 0;
}

/**
 * @brief      interface spi bus write read
 * @param[in]  *in_buf pointer to a input buffer
 * @param[in]  in_len input length
 * @param[out] *out_buf pointer to a output buffer
 * @param[in]  out_len output length
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t sx1262_interface_spi_write_read(uint8_t *in_buf, uint32_t in_len,
                                        uint8_t *out_buf, uint32_t out_len)
{
    HAL_StatusTypeDef st = HAL_OK;

    HAL_GPIO_WritePin(SX_CS_GPIO_Port, SX_CS_Pin, GPIO_PIN_RESET);
    if (out_buf == NULL || out_len == 0)
    {
        /* pure write: send command + data */
        st = HAL_SPI_Transmit(&hspi2, in_buf, in_len, HAL_MAX_DELAY);
    }
    else
    {
        /* command + read back: send command, then clock out data on the
         * same CS low period (SX126x half-duplex protocol). */
        st = HAL_SPI_Transmit(&hspi2, in_buf, in_len, HAL_MAX_DELAY);
        if (st == HAL_OK)
        {
            st = HAL_SPI_Receive(&hspi2, out_buf, out_len, HAL_MAX_DELAY);
        }
    }
    HAL_GPIO_WritePin(SX_CS_GPIO_Port, SX_CS_Pin, GPIO_PIN_SET);

    return (st == HAL_OK) ? 0 : 1;
}

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t sx1262_interface_reset_gpio_init(void)
{
    return 0;
}

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t sx1262_interface_reset_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief     interface reset gpio write
 * @param[in] data written data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t sx1262_interface_reset_gpio_write(uint8_t data)
{
    HAL_GPIO_WritePin(SX_REST_GPIO_Port, SX_REST_Pin,
                      data ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return 0;
}

/**
 * @brief  interface busy gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t sx1262_interface_busy_gpio_init(void)
{
    return 0;
}

/**
 * @brief  interface busy gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t sx1262_interface_busy_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief      interface busy gpio read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t sx1262_interface_busy_gpio_read(uint8_t *value)
{
    if (value == NULL)
    {
        return 1;
    }
    *value = (HAL_GPIO_ReadPin(SX_BUSY_GPIO_Port, SX_BUSY_Pin) == GPIO_PIN_SET) ? 1 : 0;
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void sx1262_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void sx1262_interface_debug_print(const char *const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/**
 * @brief     interface receive callback
 * @param[in] type receive callback type
 * @param[in] *buf pointer to a buffer address
 * @param[in] len buffer length
 * @note      none
 */
void sx1262_interface_receive_callback(uint16_t type, uint8_t *buf, uint16_t len)
{
    (void)type;
    (void)buf;
    (void)len;
}