
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include "sd_raw.h"

#include <LUFA/Common/Common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SD_SIZE (1024 * 2)
static uint8_t A[SD_SIZE] = {0};

uint8_t sd_raw_init(void) { return 1; }

uint8_t sd_raw_available(void) { return 1; }
uint8_t sd_raw_locked(void) { return 0; }

uint8_t sd_raw_read(offset_t offset, uint8_t* buffer, uintptr_t length)
{
    //for(int i = 0; i < length; ++ i) 
    //{
    //    eeprom_busy_wait();
    //    buffer[i] = eeprom_read_byte((i + offset)%1024);
    //}
    if(offset >= 1024)
    {
        for(int i = 0; i < length; ++ i) buffer[i] = A[(offset + i)%SD_SIZE];
    }
    else
    {
        for(int i = 0; i < length; ++ i) 
        {
            eeprom_busy_wait();
            buffer[i] = eeprom_read_byte(i + offset);
        }
    }
    return 1;
}

uint8_t sd_raw_read_interval(offset_t offset, uint8_t* buffer, uintptr_t interval, uintptr_t length, sd_raw_read_interval_handler_t callback, void* p)
{
    if(!buffer || interval == 0 || length < interval || !callback)
        return 0;

    while(length >= interval)
    {
        /* as reading is now buffered, we directly
         * hand over the request to sd_raw_read()
         */
        if(!sd_raw_read(offset, buffer, interval))
            return 0;
        if(!callback(buffer, offset, p))
            break;
        offset += interval;
        length -= interval;
    }

    return 1;
}

uint8_t sd_raw_write(offset_t offset, const uint8_t* buffer, uintptr_t length)
{
    //for(int i = 0; i < length; ++ i) 
    //{
    //    eeprom_busy_wait();
    //    eeprom_update_byte((offset + i)%1024, buffer[i]);
    //}
    if(offset >= 1024)
    {
        for(int i = 0; i < length; ++ i) A[(offset + i)%SD_SIZE] = buffer[i];
    }
    else
    {
	    //printf("raw_write: %li %i\r\n", offset, length);
        for(int i = 0; i < length; ++ i) 
        {
            eeprom_busy_wait();
            eeprom_update_byte(offset + i, buffer[i]);
            //printf("%02x", buffer[i]);
        }
    }
    return 1;
}

uint8_t sd_raw_write_interval(offset_t offset, uint8_t* buffer, uintptr_t length, sd_raw_write_interval_handler_t callback, void* p)
{
    if(!buffer || !callback)
        return 0;

    uint8_t endless = (length == 0);
    while(endless || length > 0)
    {
        uint16_t bytes_to_write = callback(buffer, offset, p);
        if(!bytes_to_write)
            break;
        if(!endless && bytes_to_write > length)
            return 0;

        /* as writing is always buffered, we directly
         * hand over the request to sd_raw_write()
         */
        if(!sd_raw_write(offset, buffer, bytes_to_write))
            return 0;

        offset += bytes_to_write;
        length -= bytes_to_write;
    }

    return 1;
}

uint8_t sd_raw_sync(void) { return 1; }

uint8_t sd_raw_get_info(struct sd_raw_info* info) {
    memset(info, 0, sizeof(*info));
    info->capacity=1024*32; //Byte
    return 1; 
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


