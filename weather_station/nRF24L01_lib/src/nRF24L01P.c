/**
 * @file nRF24L01P.cpp
 *
 * @author Owen Edwards
 * 
 * @section LICENSE
 *
 * Copyright (c) 2010 Owen Edwards
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * @section DESCRIPTION
 *
 * nRF24L01+ Single Chip 2.4GHz Transceiver from Nordic Semiconductor.
 *
 * Datasheet:
 *
 * http://www.nordicsemi.no/files/Product/data_sheet/nRF24L01P_Product_Specification_1_0.pdf
 */

/**
 * Includes
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "gpio_lib.h"
#include "nRF24L01P.h"

#define wait_ms(x)		usleep(x * 1000)
#define ARRAY_SIZE(arr) 	(sizeof(arr) / sizeof((arr)[0]))

/* */
static const char *device = "/dev/spidev32766.0";			// A default viene usata la spi1 CS0
static uint8_t spi_mode;
static uint8_t spi_bits;
static uint32_t spi_speed;
static int fd;

/**
 * Defines
 *
 * (Note that all defines here start with an underscore, e.g. '_NRF24L01P_MODE_UNKNOWN',
 *  and are local to this library.  The defines in the nRF24L01P.h file do not start
 *  with the underscore, and can be used by code to access this library.)
 */

typedef enum {
    _NRF24L01P_MODE_UNKNOWN,
    _NRF24L01P_MODE_POWER_DOWN,
    _NRF24L01P_MODE_STANDBY,
    _NRF24L01P_MODE_RX,
    _NRF24L01P_MODE_TX,
} nRF24L01P_Mode_Type;

#define error	printf
/*
 * The following FIFOs are present in nRF24L01+:
 *   TX three level, 32 byte FIFO
 *   RX three level, 32 byte FIFO
 */
#define _NRF24L01P_TX_FIFO_COUNT   3
#define _NRF24L01P_RX_FIFO_COUNT   3

#define _NRF24L01P_TX_FIFO_SIZE   32
#define _NRF24L01P_RX_FIFO_SIZE   32

#define _NRF24L01P_SPI_MAX_DATA_RATE     10000000

#define _NRF24L01P_SPI_CMD_RD_REG            0x00
#define _NRF24L01P_SPI_CMD_WR_REG            0x20
#define _NRF24L01P_SPI_CMD_RD_RX_PAYLOAD     0x61   
#define _NRF24L01P_SPI_CMD_WR_TX_PAYLOAD     0xa0
#define _NRF24L01P_SPI_CMD_FLUSH_TX          0xe1
#define _NRF24L01P_SPI_CMD_FLUSH_RX          0xe2
#define _NRF24L01P_SPI_CMD_REUSE_TX_PL       0xe3
#define _NRF24L01P_SPI_CMD_R_RX_PL_WID       0x60
#define _NRF24L01P_SPI_CMD_W_ACK_PAYLOAD     0xa8
#define _NRF24L01P_SPI_CMD_W_TX_PYLD_NO_ACK  0xb0
#define _NRF24L01P_SPI_CMD_NOP               0xff


#define _NRF24L01P_REG_CONFIG                0x00
#define _NRF24L01P_REG_EN_AA                 0x01
#define _NRF24L01P_REG_EN_RXADDR             0x02
#define _NRF24L01P_REG_SETUP_AW              0x03
#define _NRF24L01P_REG_SETUP_RETR            0x04
#define _NRF24L01P_REG_RF_CH                 0x05
#define _NRF24L01P_REG_RF_SETUP              0x06
#define _NRF24L01P_REG_STATUS                0x07
#define _NRF24L01P_REG_OBSERVE_TX            0x08
#define _NRF24L01P_REG_RPD                   0x09
#define _NRF24L01P_REG_RX_ADDR_P0            0x0a
#define _NRF24L01P_REG_RX_ADDR_P1            0x0b
#define _NRF24L01P_REG_RX_ADDR_P2            0x0c
#define _NRF24L01P_REG_RX_ADDR_P3            0x0d
#define _NRF24L01P_REG_RX_ADDR_P4            0x0e
#define _NRF24L01P_REG_RX_ADDR_P5            0x0f
#define _NRF24L01P_REG_TX_ADDR               0x10
#define _NRF24L01P_REG_RX_PW_P0              0x11
#define _NRF24L01P_REG_RX_PW_P1              0x12
#define _NRF24L01P_REG_RX_PW_P2              0x13
#define _NRF24L01P_REG_RX_PW_P3              0x14
#define _NRF24L01P_REG_RX_PW_P4              0x15
#define _NRF24L01P_REG_RX_PW_P5              0x16
#define _NRF24L01P_REG_FIFO_STATUS           0x17
#define _NRF24L01P_REG_DYNPD                 0x1c
#define _NRF24L01P_REG_FEATURE               0x1d

#define _NRF24L01P_REG_ADDRESS_MASK          0x1f

// CONFIG register:
#define _NRF24L01P_CONFIG_PRIM_RX        (1<<0)
#define _NRF24L01P_CONFIG_PWR_UP         (1<<1)
#define _NRF24L01P_CONFIG_CRC0           (1<<2)
#define _NRF24L01P_CONFIG_EN_CRC         (1<<3)
#define _NRF24L01P_CONFIG_MASK_MAX_RT    (1<<4)
#define _NRF24L01P_CONFIG_MASK_TX_DS     (1<<5)
#define _NRF24L01P_CONFIG_MASK_RX_DR     (1<<6)

#define _NRF24L01P_CONFIG_CRC_MASK       (_NRF24L01P_CONFIG_EN_CRC|_NRF24L01P_CONFIG_CRC0)
#define _NRF24L01P_CONFIG_CRC_NONE       (0)
#define _NRF24L01P_CONFIG_CRC_8BIT       (_NRF24L01P_CONFIG_EN_CRC)
#define _NRF24L01P_CONFIG_CRC_16BIT      (_NRF24L01P_CONFIG_EN_CRC|_NRF24L01P_CONFIG_CRC0)

// EN_AA register:
#define _NRF24L01P_EN_AA_NONE            0

// EN_RXADDR register:
#define _NRF24L01P_EN_RXADDR_NONE        0

// SETUP_AW register:
#define _NRF24L01P_SETUP_AW_AW_MASK      (0x3<<0)
#define _NRF24L01P_SETUP_AW_AW_3BYTE     (0x1<<0)
#define _NRF24L01P_SETUP_AW_AW_4BYTE     (0x2<<0)
#define _NRF24L01P_SETUP_AW_AW_5BYTE     (0x3<<0)

// SETUP_RETR register:
#define _NRF24L01P_SETUP_RETR_NONE       0

// RF_SETUP register:
#define _NRF24L01P_RF_SETUP_RF_PWR_MASK          (0x3<<1)
#define _NRF24L01P_RF_SETUP_RF_PWR_0DBM          (0x3<<1)
#define _NRF24L01P_RF_SETUP_RF_PWR_MINUS_6DBM    (0x2<<1)
#define _NRF24L01P_RF_SETUP_RF_PWR_MINUS_12DBM   (0x1<<1)
#define _NRF24L01P_RF_SETUP_RF_PWR_MINUS_18DBM   (0x0<<1)

#define _NRF24L01P_RF_SETUP_RF_DR_HIGH_BIT       (1 << 3)
#define _NRF24L01P_RF_SETUP_RF_DR_LOW_BIT        (1 << 5)
#define _NRF24L01P_RF_SETUP_RF_DR_MASK           (_NRF24L01P_RF_SETUP_RF_DR_LOW_BIT|_NRF24L01P_RF_SETUP_RF_DR_HIGH_BIT)
#define _NRF24L01P_RF_SETUP_RF_DR_250KBPS        (_NRF24L01P_RF_SETUP_RF_DR_LOW_BIT)
#define _NRF24L01P_RF_SETUP_RF_DR_1MBPS          (0)
#define _NRF24L01P_RF_SETUP_RF_DR_2MBPS          (_NRF24L01P_RF_SETUP_RF_DR_HIGH_BIT)

// STATUS register:
#define _NRF24L01P_STATUS_TX_FULL        (1<<0)
#define _NRF24L01P_STATUS_RX_P_NO        (0x7<<1)
#define _NRF24L01P_STATUS_MAX_RT         (1<<4)
#define _NRF24L01P_STATUS_TX_DS          (1<<5)
#define _NRF24L01P_STATUS_RX_DR          (1<<6)

// RX_PW_P0..RX_PW_P5 registers:
#define _NRF24L01P_RX_PW_Px_MASK         0x3F

#define _NRF24L01P_TIMING_Tundef2pd_us     100000   // 100mS
#define _NRF24L01P_TIMING_Tstby2a_us          130   // 130uS
#define _NRF24L01P_TIMING_Thce_us              10   //  10uS
#define _NRF24L01P_TIMING_Tpd2stby_us        4500   // 4.5mS worst case
#define _NRF24L01P_TIMING_Tpece2csn_us          4   //   4uS

int mode;

int nRF24L01P_SpiInit( void);

/**
 * Methods
 */

void nRF24L01P_nRF24L01P( void) 
{

    mode = _NRF24L01P_MODE_UNKNOWN;

    /* Set Port DIR in/out */
    NRF24L01_IRQ_IN;
    NRF24L01_CE_OUT;
    /* Set TX/RX to RX */
    NRF24L01_CE_LOW;

    nRF24L01P_disable();


    /* Configure the SPI0 interface */
    nRF24L01P_SpiInit();
    /* Wait for Power-on reset */
    usleep(_NRF24L01P_TIMING_Tundef2pd_us);    // Wait for Power-on reset
    /* Power Down */
    nRF24L01P_setRegister(_NRF24L01P_REG_CONFIG, 0); 
    /* Clear any pending interrupts */
    nRF24L01P_setRegister(_NRF24L01P_REG_STATUS, _NRF24L01P_STATUS_MAX_RT|_NRF24L01P_STATUS_TX_DS|_NRF24L01P_STATUS_RX_DR); 
    nRF24L01P_flush();

    /*
     * Setup default configuration
    */
    nRF24L01P_disableAllRxPipes();
    nRF24L01P_setRfFrequency( DEFAULT_NRF24L01P_RF_FREQUENCY);
    nRF24L01P_setRfOutputPower( DEFAULT_NRF24L01P_TX_PWR);
    nRF24L01P_setAirDataRate( DEFAULT_NRF24L01P_DATARATE);
    nRF24L01P_setCrcWidth( DEFAULT_NRF24L01P_CRC);
    nRF24L01P_setTxAddress(DEFAULT_NRF24L01P_ADDRESS, DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    nRF24L01P_setRxAddress( DEFAULT_NRF24L01P_ADDRESS, DEFAULT_NRF24L01P_ADDRESS_WIDTH, NRF24L01P_PIPE_P0);
    nRF24L01P_disableAutoAcknowledge();
    nRF24L01P_disableAutoRetransmit();
    nRF24L01P_setTransferSize(DEFAULT_NRF24L01P_TRANSFER_SIZE, NRF24L01P_PIPE_P0);

    mode = _NRF24L01P_MODE_POWER_DOWN;

}


void nRF24L01P_powerUp(void) {
    int config;
	
    config = nRF24L01P_getRegister(_NRF24L01P_REG_CONFIG);

    config |= _NRF24L01P_CONFIG_PWR_UP;

    nRF24L01P_setRegister(_NRF24L01P_REG_CONFIG, config);

    /* Wait until the nRF24L01+ powers up */
    usleep( _NRF24L01P_TIMING_Tpd2stby_us );

    mode = _NRF24L01P_MODE_STANDBY;

}


void nRF24L01P_powerDown(void) {
    int config;
	
    config = nRF24L01P_getRegister(_NRF24L01P_REG_CONFIG);

    config &= ~_NRF24L01P_CONFIG_PWR_UP;

    nRF24L01P_setRegister(_NRF24L01P_REG_CONFIG, config);

    /* Wait until the nRF24L01+ powers down */
    /*  This *may* not be necessary (no timing is shown in the Datasheet), but just to be safe */
    usleep( _NRF24L01P_TIMING_Tpd2stby_us );    

    mode = _NRF24L01P_MODE_POWER_DOWN;

}


void nRF24L01P_setReceiveMode(void) {
    int config;
	
    if ( _NRF24L01P_MODE_POWER_DOWN == mode ) nRF24L01P_powerUp();

    config = nRF24L01P_getRegister(_NRF24L01P_REG_CONFIG);

    config |= _NRF24L01P_CONFIG_PRIM_RX;

    nRF24L01P_setRegister(_NRF24L01P_REG_CONFIG, config);

    mode = _NRF24L01P_MODE_RX;

}


void nRF24L01P_setTransmitMode(void) {
    int config;
	
    if ( _NRF24L01P_MODE_POWER_DOWN == mode ) nRF24L01P_powerUp();

    config = nRF24L01P_getRegister(_NRF24L01P_REG_CONFIG);

    config &= ~_NRF24L01P_CONFIG_PRIM_RX;

    nRF24L01P_setRegister(_NRF24L01P_REG_CONFIG, config);

    mode = _NRF24L01P_MODE_TX;

}


void nRF24L01P_enable(void) {

    NRF24L01_CE_HIGH;
    usleep( _NRF24L01P_TIMING_Tpece2csn_us );

}


void nRF24L01P_disable(void) {

    NRF24L01_CE_LOW;

}

void nRF24L01P_setRfFrequency(int frequency) {
    int channel;
	
    if ( ( frequency < NRF24L01P_MIN_RF_FREQUENCY ) || ( frequency > NRF24L01P_MAX_RF_FREQUENCY ) ) {

        error( "nRF24L01P: Invalid RF Frequency setting %d\r\n", frequency );
        return;

    }

    channel = ( frequency - NRF24L01P_MIN_RF_FREQUENCY ) & 0x7F;

    nRF24L01P_setRegister(_NRF24L01P_REG_RF_CH, channel);

}


int nRF24L01P_getRfFrequency(void) {
    int channel;
	
    channel = nRF24L01P_getRegister(_NRF24L01P_REG_RF_CH) & 0x7F;

    return ( channel + NRF24L01P_MIN_RF_FREQUENCY );

}


void nRF24L01P_setRfOutputPower(int power) {
    int rfSetup;
	
    rfSetup = nRF24L01P_getRegister(_NRF24L01P_REG_RF_SETUP) & ~_NRF24L01P_RF_SETUP_RF_PWR_MASK;

    switch ( power ) {

        case NRF24L01P_TX_PWR_ZERO_DB:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_PWR_0DBM;
            break;

        case NRF24L01P_TX_PWR_MINUS_6_DB:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_PWR_MINUS_6DBM;
            break;

        case NRF24L01P_TX_PWR_MINUS_12_DB:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_PWR_MINUS_12DBM;
            break;

        case NRF24L01P_TX_PWR_MINUS_18_DB:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_PWR_MINUS_18DBM;
            break;

        default:
            error( "nRF24L01P: Invalid RF Output Power setting %d\r\n", power );
            return;

    }

    nRF24L01P_setRegister(_NRF24L01P_REG_RF_SETUP, rfSetup);

}


int nRF24L01P_getRfOutputPower(void) {
    int rfPwr;
	
    rfPwr = nRF24L01P_getRegister(_NRF24L01P_REG_RF_SETUP) & _NRF24L01P_RF_SETUP_RF_PWR_MASK;

    switch ( rfPwr ) {

        case _NRF24L01P_RF_SETUP_RF_PWR_0DBM:
            return NRF24L01P_TX_PWR_ZERO_DB;

        case _NRF24L01P_RF_SETUP_RF_PWR_MINUS_6DBM:
            return NRF24L01P_TX_PWR_MINUS_6_DB;

        case _NRF24L01P_RF_SETUP_RF_PWR_MINUS_12DBM:
            return NRF24L01P_TX_PWR_MINUS_12_DB;

        case _NRF24L01P_RF_SETUP_RF_PWR_MINUS_18DBM:
            return NRF24L01P_TX_PWR_MINUS_18_DB;

        default:
            error( "nRF24L01P: Unknown RF Output Power value %d\r\n", rfPwr );
            return 0;

    }
}


void nRF24L01P_setAirDataRate(int rate) {
    int rfSetup;
	
    rfSetup = nRF24L01P_getRegister(_NRF24L01P_REG_RF_SETUP) & ~_NRF24L01P_RF_SETUP_RF_DR_MASK;

    switch ( rate ) {

        case NRF24L01P_DATARATE_250_KBPS:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_DR_250KBPS;
            break;

        case NRF24L01P_DATARATE_1_MBPS:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_DR_1MBPS;
            break;

        case NRF24L01P_DATARATE_2_MBPS:
            rfSetup |= _NRF24L01P_RF_SETUP_RF_DR_2MBPS;
            break;

        default:
            error( "nRF24L01P: Invalid Air Data Rate setting %d\r\n", rate );
            return;

    }

    nRF24L01P_setRegister(_NRF24L01P_REG_RF_SETUP, rfSetup);

}


int nRF24L01P_getAirDataRate(void) {
    int rfDataRate;
	
    rfDataRate = nRF24L01P_getRegister(_NRF24L01P_REG_RF_SETUP) & _NRF24L01P_RF_SETUP_RF_DR_MASK;

    switch ( rfDataRate ) {

        case _NRF24L01P_RF_SETUP_RF_DR_250KBPS:
            return NRF24L01P_DATARATE_250_KBPS;

        case _NRF24L01P_RF_SETUP_RF_DR_1MBPS:
            return NRF24L01P_DATARATE_1_MBPS;

        case _NRF24L01P_RF_SETUP_RF_DR_2MBPS:
            return NRF24L01P_DATARATE_2_MBPS;

        default:
            error( "nRF24L01P: Unknown Air Data Rate value %d\r\n", rfDataRate );
            return 0;

    }
}


void nRF24L01P_setCrcWidth(int width) {
    int config;
	
    config = nRF24L01P_getRegister(_NRF24L01P_REG_CONFIG) & ~_NRF24L01P_CONFIG_CRC_MASK;

    switch ( width ) {

        case NRF24L01P_CRC_NONE:
            config |= _NRF24L01P_CONFIG_CRC_NONE;
            break;

        case NRF24L01P_CRC_8_BIT:
            config |= _NRF24L01P_CONFIG_CRC_8BIT;
            break;

        case NRF24L01P_CRC_16_BIT:
            config |= _NRF24L01P_CONFIG_CRC_16BIT;
            break;

        default:
            error( "nRF24L01P: Invalid CRC Width setting %d\r\n", width );
            return;

    }

    nRF24L01P_setRegister(_NRF24L01P_REG_CONFIG, config);

}


int nRF24L01P_getCrcWidth(void) {
    int crcWidth;
	
    crcWidth = nRF24L01P_getRegister(_NRF24L01P_REG_CONFIG) & _NRF24L01P_CONFIG_CRC_MASK;

    switch ( crcWidth ) {

        case _NRF24L01P_CONFIG_CRC_NONE:
            return NRF24L01P_CRC_NONE;

        case _NRF24L01P_CONFIG_CRC_8BIT:
            return NRF24L01P_CRC_8_BIT;

        case _NRF24L01P_CONFIG_CRC_16BIT:
            return NRF24L01P_CRC_16_BIT;

        default:
            error( "nRF24L01P: Unknown CRC Width value %d\r\n", crcWidth );
            return 0;

    }
}


void nRF24L01P_setTransferSize(int size, int pipe) {
    int rxPwPxRegister;
	
    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid Transfer Size pipe number %d\r\n", pipe );
        return;

    }

    if ( ( size < 0 ) || ( size > _NRF24L01P_RX_FIFO_SIZE ) ) {

        error( "nRF24L01P: Invalid Transfer Size setting %d\r\n", size );
        return;

    }

    rxPwPxRegister = _NRF24L01P_REG_RX_PW_P0 + ( pipe - NRF24L01P_PIPE_P0 );

    nRF24L01P_setRegister(rxPwPxRegister, ( size & _NRF24L01P_RX_PW_Px_MASK ) );

}


int nRF24L01P_getTransferSize(int pipe) {
    int rxPwPxRegister;
    int size;
	
    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid Transfer Size pipe number %d\r\n", pipe );
        return 0;

    }

    rxPwPxRegister = _NRF24L01P_REG_RX_PW_P0 + ( pipe - NRF24L01P_PIPE_P0 );

    size = nRF24L01P_getRegister(rxPwPxRegister);
    
    return ( size & _NRF24L01P_RX_PW_Px_MASK );

}


void nRF24L01P_disableAllRxPipes(void) {

	nRF24L01P_setRegister(_NRF24L01P_REG_EN_RXADDR, _NRF24L01P_EN_RXADDR_NONE);

}


void nRF24L01P_disableAutoAcknowledge(void) {

	nRF24L01P_setRegister(_NRF24L01P_REG_EN_AA, _NRF24L01P_EN_AA_NONE);

}


void nRF24L01P_enableAutoAcknowledge(int pipe) {
    int enAA;
	
    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid Enable AutoAcknowledge pipe number %d\r\n", pipe );
        return;

    }

    enAA = nRF24L01P_getRegister(_NRF24L01P_REG_EN_AA);

    enAA |= ( 1 << (pipe - NRF24L01P_PIPE_P0) );

    nRF24L01P_setRegister(_NRF24L01P_REG_EN_AA, enAA);

}


void nRF24L01P_disableAutoRetransmit(void) {

	nRF24L01P_setRegister(_NRF24L01P_REG_SETUP_RETR, _NRF24L01P_SETUP_RETR_NONE);

}

void nRF24L01P_setRxAddress(unsigned long long address, int width, int pipe) {
    int status;
    int cn;
    int rxAddrPxRegister;
    int setupAw;
    int ret;
    int tmp;
    unsigned char cmd[10];

    memset( cmd, '\0', 10);
	
    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid setRxAddress pipe number %d\r\n", pipe );
        return;

    }

    if ( ( pipe == NRF24L01P_PIPE_P0 ) || ( pipe == NRF24L01P_PIPE_P1 ) ) {

        setupAw = nRF24L01P_getRegister(_NRF24L01P_REG_SETUP_AW) & ~_NRF24L01P_SETUP_AW_AW_MASK;
    
        switch ( width ) {
    
            case 3:
                setupAw |= _NRF24L01P_SETUP_AW_AW_3BYTE;
                break;
    
            case 4:
                setupAw |= _NRF24L01P_SETUP_AW_AW_4BYTE;
                break;
    
            case 5:
                setupAw |= _NRF24L01P_SETUP_AW_AW_5BYTE;
                break;
    
            default:
                error( "nRF24L01P: Invalid setRxAddress width setting %d\r\n", width );
                return;
    
        }
    
        nRF24L01P_setRegister(_NRF24L01P_REG_SETUP_AW, setupAw);

    } else {
    
        width = 1;
    
    }

    rxAddrPxRegister = _NRF24L01P_REG_RX_ADDR_P0 + ( pipe - NRF24L01P_PIPE_P0 );

    cn = (_NRF24L01P_SPI_CMD_WR_REG | (rxAddrPxRegister & _NRF24L01P_REG_ADDRESS_MASK));

    cmd[0]=cn;
    tmp = 0;
	
    while( tmp < width) {
        //
        // LSByte first
        //
	cmd[ tmp+1] = (address & 0xFF);
        address >>= 8;
	tmp++;
    }
	
    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)NULL,
	.len = width+1,
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_setRxAddress Error %d\n", ret);
	return;
    }

    int enRxAddr = nRF24L01P_getRegister(_NRF24L01P_REG_EN_RXADDR);

    enRxAddr |= (1 << ( pipe - NRF24L01P_PIPE_P0 ) );

    nRF24L01P_setRegister(_NRF24L01P_REG_EN_RXADDR, enRxAddr);
}

/*
 * This version of setRxAddress is just a wrapper for the version that takes 'long long's,
 *  in case the main code doesn't want to deal with long long's.
 */
void nRF24L01P_setRxAddressNL(unsigned long msb_address, unsigned long lsb_address, int width, int pipe) {

    unsigned long long address = ( ( (unsigned long long) msb_address ) << 32 ) | ( ( (unsigned long long) lsb_address ) << 0 );

    nRF24L01P_setRxAddress(address, width, pipe);

}


/*
 * This version of setTxAddress is just a wrapper for the version that takes 'long long's,
 *  in case the main code doesn't want to deal with long long's.
 */
void nRF24L01P_setTxAddressNL(unsigned long msb_address, unsigned long lsb_address, int width) {

    unsigned long long address = ( ( (unsigned long long) msb_address ) << 32 ) | ( ( (unsigned long long) lsb_address ) << 0 );

    nRF24L01P_setTxAddress(address, width);

}


void nRF24L01P_setTxAddress(unsigned long long address, int width) {
    int status;
    int cn;
    int setupAw;
    int ret;
    int tmp;
    unsigned char cmd[10];
	
    memset( cmd, '\0', 10);

    setupAw = nRF24L01P_getRegister(_NRF24L01P_REG_SETUP_AW) & ~_NRF24L01P_SETUP_AW_AW_MASK;

    switch ( width ) {

        case 3:
            setupAw |= _NRF24L01P_SETUP_AW_AW_3BYTE;
            break;

        case 4:
            setupAw |= _NRF24L01P_SETUP_AW_AW_4BYTE;
            break;

        case 5:
            setupAw |= _NRF24L01P_SETUP_AW_AW_5BYTE;
            break;

        default:
            error( "nRF24L01P: Invalid setTxAddress width setting %d\r\n", width );
            return;

    }

    nRF24L01P_setRegister(_NRF24L01P_REG_SETUP_AW, setupAw);

    cn = (_NRF24L01P_SPI_CMD_WR_REG | (_NRF24L01P_REG_TX_ADDR & _NRF24L01P_REG_ADDRESS_MASK));

    cmd[0] = cn;
    tmp=0;
	
    while( tmp < width) {
        //
        // LSByte first
        //
	cmd[tmp+1] = (address & 0xFF);
        address >>= 8;
	tmp++;
    }

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)NULL,
	.len = width+1,
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_setTxAddress Error %d\n", ret);
	return;
    }

}


unsigned long long nRF24L01P_getRxAddress(int pipe) {
    int status;
    int cn;
    int width;
    int setupAw;
    int rxAddrPxRegister;
    int i;
    unsigned long long address;
    int ret;
    int tmp;
    unsigned char cmd[10];
    unsigned char data[10];
	
    memset( cmd, '\0', 10);
    memset( data, '\0', 10);

    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid setRxAddress pipe number %d\r\n", pipe );
        return 0;

    }

    if ( ( pipe == NRF24L01P_PIPE_P0 ) || ( pipe == NRF24L01P_PIPE_P1 ) ) {

        setupAw = nRF24L01P_getRegister(_NRF24L01P_REG_SETUP_AW) & _NRF24L01P_SETUP_AW_AW_MASK;

        switch ( setupAw ) {

            case _NRF24L01P_SETUP_AW_AW_3BYTE:
                width = 3;
                break;

            case _NRF24L01P_SETUP_AW_AW_4BYTE:
                width = 4;
                break;

            case _NRF24L01P_SETUP_AW_AW_5BYTE:
                width = 5;
                break;

            default:
                error( "nRF24L01P: Unknown getRxAddress width value %d\r\n", setupAw );
                return 0;

        }

    } else {

        width = 1;

    }

    rxAddrPxRegister = _NRF24L01P_REG_RX_ADDR_P0 + ( pipe - NRF24L01P_PIPE_P0 );

    cn = (_NRF24L01P_SPI_CMD_RD_REG | (rxAddrPxRegister & _NRF24L01P_REG_ADDRESS_MASK));

    address = 0;

    cmd[0]=cn;
	
    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)data,
	.len = (width+1),
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_getRxAddress Error %d\n", ret);
	return 0;
    }

    for ( i=0; i<width; i++ ) {
        //
        // LSByte first
        //
	address |= (unsigned long long)data[1+i]<<(i*8);
    }

    if ( !( ( pipe == NRF24L01P_PIPE_P0 ) || ( pipe == NRF24L01P_PIPE_P1 ) ) ) {

        address |= ( nRF24L01P_getRxAddress(NRF24L01P_PIPE_P1) & ~((unsigned long long) 0xFF) );

    }

    return address;

}

    
unsigned long long nRF24L01P_getTxAddress(void) {
    unsigned long long address;
    int setupAw;
    int width;
    int cn;
    int status;
    int i;
    int ret;
    int tmp;
    unsigned char cmd[10];
    unsigned char data[10];
	
    memset( cmd, '\0', 10);
    memset( data, '\0', 10);

    setupAw = nRF24L01P_getRegister(_NRF24L01P_REG_SETUP_AW) & _NRF24L01P_SETUP_AW_AW_MASK;

    switch ( setupAw ) {

        case _NRF24L01P_SETUP_AW_AW_3BYTE:
            width = 3;
            break;

        case _NRF24L01P_SETUP_AW_AW_4BYTE:
            width = 4;
            break;

        case _NRF24L01P_SETUP_AW_AW_5BYTE:
            width = 5;
            break;

        default:
            error( "nRF24L01P: Unknown getTxAddress width value %d\r\n", setupAw );
            return 0;

    }

    cn = (_NRF24L01P_SPI_CMD_RD_REG | (_NRF24L01P_REG_TX_ADDR & _NRF24L01P_REG_ADDRESS_MASK));

    address = 0;

    cmd[0]=cn;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)data,
	.len = (width+1),
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_getTxAddress Error %d\n", ret);
	return 0;
    }

    for ( i=0; i<width; i++ ) {

        //
        // LSByte first
        //
	address |= (unsigned long long)data[1+i]<<(i*8);
    }

    return address;
}


int nRF24L01P_readable(int pipe) {
    int status;
	
    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid readable pipe number %d\r\n", pipe );
        return 0;

    }

    status = nRF24L01P_getStatusRegister();

    return ( ( status & _NRF24L01P_STATUS_RX_DR ) && ( ( ( status & _NRF24L01P_STATUS_RX_P_NO ) >> 1 ) == ( pipe & 0x7 ) ) );

}


int nRF24L01P_write(int pipe, char *data, int count) {
    int status;
    int originalCe;
    int i;
    int originalMode;
    int ret;
    unsigned char cmd[_NRF24L01P_TX_FIFO_SIZE+1];
	
    // Note: the pipe number is ignored in a Transmit / write

    //
    // Save the CE state
    //
    originalCe = NRF24L01_CE_GET;
    nRF24L01P_disable();
    nRF24L01P_flush();

    if ( count <= 0 ) return 0;

    if ( count > _NRF24L01P_TX_FIFO_SIZE ) count = _NRF24L01P_TX_FIFO_SIZE;

    // Clear the Status bit
    nRF24L01P_setRegister(_NRF24L01P_REG_STATUS, _NRF24L01P_STATUS_TX_DS);
	
    cmd[0] = _NRF24L01P_SPI_CMD_WR_TX_PAYLOAD;
	
    for ( i = 0; i < count; i++ ) {

	cmd[1+i]=*data++;

    }
	
    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)NULL,
	.len = count+1,
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_write Error %d\n", ret);
	return 0;
    }

    originalMode = mode;
    nRF24L01P_setTransmitMode();

    nRF24L01P_enable();
    usleep(_NRF24L01P_TIMING_Thce_us*2);
    nRF24L01P_disable();

    i=0;
    while ( !( nRF24L01P_getStatusRegister() & _NRF24L01P_STATUS_TX_DS ) ) {

        // Wait for the transfer to complete
	usleep( 100);
	i++;
	if ( i>10)
		break;

    }

    // Clear the Status bit
    nRF24L01P_setRegister(_NRF24L01P_REG_STATUS, _NRF24L01P_STATUS_TX_DS);

    if ( originalMode == _NRF24L01P_MODE_RX ) {

    	nRF24L01P_setReceiveMode();

    }

    if ( originalCe)
    	NRF24L01_CE_HIGH;
    else
    	NRF24L01_CE_LOW;

    usleep( _NRF24L01P_TIMING_Tpece2csn_us );

    return count;

}


int nRF24L01P_read(int pipe, char *data, int count) {
    int status;
    int i;
    int rxPayloadWidth;
    int ret;
    unsigned char cmd[_NRF24L01P_TX_FIFO_SIZE+1];
    unsigned char spi_data[_NRF24L01P_TX_FIFO_SIZE+1];
	
    if ( ( pipe < NRF24L01P_PIPE_P0 ) || ( pipe > NRF24L01P_PIPE_P5 ) ) {

        error( "nRF24L01P: Invalid read pipe number %d\r\n", pipe );
        return -1;

    }

    if ( count <= 0 ) return 0;

    if ( count > _NRF24L01P_RX_FIFO_SIZE ) count = _NRF24L01P_RX_FIFO_SIZE;

    if ( nRF24L01P_readable(pipe) ) {

	cmd[0]=_NRF24L01P_SPI_CMD_R_RX_PL_WID;
        
	cmd[1]=_NRF24L01P_SPI_CMD_NOP;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)spi_data,
		.len = 2,
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1) {
		printf("nRF24L01P_read RX_PL_WID Error %d\n", ret);
		return -1;
	}

	rxPayloadWidth = spi_data[1];
		

        if ( ( rxPayloadWidth < 0 ) || ( rxPayloadWidth > _NRF24L01P_RX_FIFO_SIZE ) ) {
    
            // Received payload error: need to flush the FIFO

            cmd[0]=_NRF24L01P_SPI_CMD_FLUSH_RX;
			
            cmd[1]=_NRF24L01P_SPI_CMD_NOP;
            struct spi_ioc_transfer tr = {
                 .tx_buf = (unsigned long)cmd,
		 .rx_buf = (unsigned long)NULL,
		 .len = 2,
		 .delay_usecs = 0,
             };

             ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

             if (ret < 1) {
                 printf("nRF24L01P_read CMD FLUSH RX %d\n", ret);
                 return -1;
              }

            //
            // At this point, we should retry the reception,
            //  but for now we'll just fall through...
            //

        } else {

            if ( rxPayloadWidth < count ) count = rxPayloadWidth;

	    cmd[0]=_NRF24L01P_SPI_CMD_RD_RX_PAYLOAD;
	    struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)spi_data,
		.len = count+1,
		.delay_usecs = 0,
             };

             ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

             if (ret < 1) {
                 printf("nRF24L01P_read RX PAYLOAD %d\n", ret);
                 return -1;
             }

             for ( i = 0; i < count; i++ ) {
        
		*data++ = spi_data[1+i];
             }
            // Clear the Status bit
            nRF24L01P_setRegister(_NRF24L01P_REG_STATUS, _NRF24L01P_STATUS_RX_DR);

            return count;

        }

    } else {

        //
        // What should we do if there is no 'readable' data?
        //  We could wait for data to arrive, but for now, we'll
        //  just return with no data.
        //
        return 0;

    }

    //
    // We get here because an error condition occured;
    //  We could wait for data to arrive, but for now, we'll
    //  just return with no data.
    //
    return -1;

}

void nRF24L01P_setRegister(int regAddress, int regData) {
    int status;
    int cn;
    int originalCe;
    int ret;
    unsigned char cmd[2];

    //
    // Save the CE state
    //
    originalCe = NRF24L01_CE_GET;
    nRF24L01P_disable();

    cn = (_NRF24L01P_SPI_CMD_WR_REG | (regAddress & _NRF24L01P_REG_ADDRESS_MASK));

    cmd[0]=cn;
    cmd[1]=regData & 0xFF;	

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)NULL,
	.len = ARRAY_SIZE( cmd),
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_setRegister Error %d\n", ret);
	return;
    }
	
    if ( originalCe)
    	NRF24L01_CE_HIGH;
    else
    	NRF24L01_CE_LOW;

    usleep( _NRF24L01P_TIMING_Tpece2csn_us );

}


int nRF24L01P_getRegister(int regAddress) {
    int cn;
    int dn;
    int status;
    int ret;
    unsigned char cmd[2];
    unsigned char data[2];

	
    cn = (_NRF24L01P_SPI_CMD_RD_REG | (regAddress & _NRF24L01P_REG_ADDRESS_MASK));

    cmd[0]=cn;						
    cmd[1]=_NRF24L01P_SPI_CMD_NOP;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)data,
	.len = ARRAY_SIZE( cmd),
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_getRegister error %d\n", ret);
	return -1;
    }

    dn = data[1];
	
    return dn;

}

int nRF24L01P_getStatusRegister(void) {
    int status;
    int ret;
    unsigned char cmd[1];
    unsigned char data[1];

    cmd[0]=_NRF24L01P_SPI_CMD_NOP;	
	
    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)cmd,
	.rx_buf = (unsigned long)data,
	.len = ARRAY_SIZE( cmd),
	.delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1) {
	printf("nRF24L01P_getStatusRegister %d\n", ret);
	return 1;
    }
	
    status = data[0];
	
    return status;

}

int nRF24L01P_flush( void)
{
	int ret;
	unsigned char cmd[2];

	cmd[0]=_NRF24L01P_SPI_CMD_FLUSH_RX;
	cmd[1]=_NRF24L01P_SPI_CMD_NOP;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)NULL,
		.len = 2,
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1) {
		printf("nRF24L01P_read CMD FLUSH RX %d\n", ret);
		return 0;
	}
	
	usleep( 200);
	
	cmd[0]=_NRF24L01P_SPI_CMD_FLUSH_TX;
	cmd[1]=_NRF24L01P_SPI_CMD_NOP;

	tr.tx_buf = (unsigned long)cmd,
	tr.rx_buf = (unsigned long)NULL,
	tr.len = 2,
	tr.delay_usecs = 0,

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1) {
		printf("nRF24L01P_read CMD FLUSH TX %d\n", ret);
		return 0;
	}
	
	return 1;
}


int nRF24L01P_SpiInit( void)
{
	int ret;
	
	fd = open(device, O_FSYNC|O_RDWR);
	if (fd < 0)
		printf("can't open device\n");
	
	/* SPI param */
#if 0
	spi_mode |= SPI_CPHA;  				/* SPI_MODE_3 */
	spi_mode |= SPI_CPOL;
#else
	spi_mode = 0;
#endif
	spi_speed = 2*1000*1000;
	spi_bits = 8;

	/* SPI Mode */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &spi_mode);
	if (ret == -1)
		printf("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &spi_mode);
	if (ret == -1)
		printf("can't get spi mode");

	/* SPI speed */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
	if (ret == -1)
		printf("can't get max speed hz");

	/* SPI bit len */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits);
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bits);
	if (ret == -1)
		printf("can't get bits per word");

#if 0		
	printf("spi mode: %d\n", spi_mode);
	printf("bits per word: %d\n", spi_bits);
	printf("max speed: %d Hz (%d KHz)\n", spi_speed, spi_speed/1000);
#endif

	return ret;
}


