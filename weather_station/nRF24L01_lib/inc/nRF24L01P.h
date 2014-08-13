/**
 * @file nRF24L01P.h
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

#ifndef __NRF24L01P_H__
#define __NRF24L01P_H__

/**
 * Includes
 */

#if 0
/* PA4 S19, ID Kernel 3.1x = 4  (NRF24L01 CE pin to Rx/Tx) */
#define NRF24L01_CE_OUT		gpiosetdir( 4, "out")		// gpioSetDir( 0, 9, 1 )
/* Set port to input (NRF24L01 IRQ not used) */
#define NRF24L01_IRQ_IN		

#define NRF24L01_CE_HIGH	gpiosetbits( 4)			//gpioSetValue( 0, 9, 1 )
#define NRF24L01_CE_LOW		gpioclearbits( 4)		//gpioSetValue( 0, 9, 0 )
#define NRF24L01_CE_GET		gpiogetbits( 4)			//gpioGetPinValue( 0, 9)
#else
/* PA4 S16, ID Kernel 3.1x = 7  (NRF24L01 CE pin to Rx/Tx) */
#define NRF24L01_CE_OUT		gpiosetdir( 7, "out")		// gpioSetDir( 0, 9, 1 )
/* Set port to input (NRF24L01 IRQ not used) */
#define NRF24L01_IRQ_IN		

#define NRF24L01_CE_HIGH	gpiosetbits( 7)			//gpioSetValue( 0, 9, 1 )
#define NRF24L01_CE_LOW		gpioclearbits( 7)		//gpioSetValue( 0, 9, 0 )
#define NRF24L01_CE_GET		gpiogetbits( 7)			//gpioGetPinValue( 0, 9)
#endif

/**
 * Defines
 */
#define NRF24L01P_TX_PWR_ZERO_DB         0
#define NRF24L01P_TX_PWR_MINUS_6_DB     -6
#define NRF24L01P_TX_PWR_MINUS_12_DB   -12
#define NRF24L01P_TX_PWR_MINUS_18_DB   -18

#define NRF24L01P_DATARATE_250_KBPS    250
#define NRF24L01P_DATARATE_1_MBPS     1000
#define NRF24L01P_DATARATE_2_MBPS     2000

#define NRF24L01P_CRC_NONE               0
#define NRF24L01P_CRC_8_BIT              8
#define NRF24L01P_CRC_16_BIT            16

#define NRF24L01P_MIN_RF_FREQUENCY    2400
#define NRF24L01P_MAX_RF_FREQUENCY    2525

#define NRF24L01P_PIPE_P0                0
#define NRF24L01P_PIPE_P1                1
#define NRF24L01P_PIPE_P2                2
#define NRF24L01P_PIPE_P3                3
#define NRF24L01P_PIPE_P4                4
#define NRF24L01P_PIPE_P5                5

/**
* Default setup for the nRF24L01+, based on the Sparkfun "Nordic Serial Interface Board"
*  for evaluation (http://www.sparkfun.com/products/9019)
*/
#define DEFAULT_NRF24L01P_ADDRESS       ((unsigned long long) 0xE7E7E7E7E7 )
#define DEFAULT_NRF24L01P_ADDRESS_WIDTH  5
#define DEFAULT_NRF24L01P_CRC            NRF24L01P_CRC_8_BIT
#define DEFAULT_NRF24L01P_RF_FREQUENCY  (NRF24L01P_MIN_RF_FREQUENCY + 2)
#define DEFAULT_NRF24L01P_DATARATE       NRF24L01P_DATARATE_1_MBPS
#define DEFAULT_NRF24L01P_TX_PWR         NRF24L01P_TX_PWR_ZERO_DB
#define DEFAULT_NRF24L01P_TRANSFER_SIZE  4

/**
 * nRF24L01+ Single Chip 2.4GHz Transceiver from Nordic Semiconductor.
 */

    /**
     * Constructor.
     *
     * @param mosi mbed pin to use for MOSI line of SPI interface.
     * @param miso mbed pin to use for MISO line of SPI interface.
     * @param sck mbed pin to use for SCK line of SPI interface.
     * @param csn mbed pin to use for not chip select line of SPI interface.
     * @param ce mbed pin to use for the chip enable line.
     * @param irq mbed pin to use for the interrupt request line.
     */
    void nRF24L01P_nRF24L01P( void);

    /**
     * Set the RF frequency.
     *
     * @param frequency the frequency of RF transmission in MHz (2400..2525).
     */
    void nRF24L01P_setRfFrequency(int frequency);		// = DEFAULT_NRF24L01P_RF_FREQUENCY);

    /**
     * Get the RF frequency.
     *
     * @return the frequency of RF transmission in MHz (2400..2525).
     */
    int nRF24L01P_getRfFrequency(void);

    /**
     * Set the RF output power.
     *
     * @param power the RF output power in dBm (0, -6, -12 or -18).
     */
    void nRF24L01P_setRfOutputPower(int power);	// = DEFAULT_NRF24L01P_TX_PWR);

    /**
     * Get the RF output power.
     *
     * @return the RF output power in dBm (0, -6, -12 or -18).
     */
    int nRF24L01P_getRfOutputPower(void);

    /**
     * Set the Air data rate.
     *
     * @param rate the air data rate in kbps (250, 1M or 2M).
     */
    void nRF24L01P_setAirDataRate(int rate);		// = DEFAULT_NRF24L01P_DATARATE);

    /**
     * Get the Air data rate.
     *
     * @return the air data rate in kbps (250, 1M or 2M).
     */
    int nRF24L01P_getAirDataRate(void);

    /**
     * Set the CRC width.
     *
     * @param width the number of bits for the CRC (0, 8 or 16).
     */
    void nRF24L01P_setCrcWidth(int width);			// = DEFAULT_NRF24L01P_CRC);

    /**
     * Get the CRC width.
     *
     * @return the number of bits for the CRC (0, 8 or 16).
     */
    int nRF24L01P_getCrcWidth(void);

    /**
     * Set the Receive address.
     *
     * @param address address associated with the particular pipe
     * @param width width of the address in bytes (3..5)
     * @param pipe pipe to associate the address with (0..5, default 0)
     *
     * Note that Pipes 0 & 1 have 3, 4 or 5 byte addresses,
     *  while Pipes 2..5 only use the lowest byte (bits 7..0) of the
     *  address provided here, and use 2, 3 or 4 bytes from Pipe 1's address.
     *  The width parameter is ignored for Pipes 2..5.
     */
    //void setRxAddress(unsigned long long address = DEFAULT_NRF24L01P_ADDRESS, int width = DEFAULT_NRF24L01P_ADDRESS_WIDTH, int pipe = NRF24L01P_PIPE_P0);
	void nRF24L01P_setRxAddress(unsigned long long address, int width, int pipe);
    
	//void setRxAddressNL(unsigned long msb_address, unsigned long lsb_address, int width, int pipe = NRF24L01P_PIPE_P0);
	void nRF24L01P_setRxAddressNL(unsigned long msb_address, unsigned long lsb_address, int width, int pipe);	// = NRF24L01P_PIPE_P0);

    /**
     * Set the Transmit address.
     *
     * @param address address for transmission
     * @param width width of the address in bytes (3..5)
     *
     * Note that the address width is shared with the Receive pipes,
     *  so a change to that address width affect transmissions.
     */
    //void setTxAddress(unsigned long long address = DEFAULT_NRF24L01P_ADDRESS, int width = DEFAULT_NRF24L01P_ADDRESS_WIDTH);
	void nRF24L01P_setTxAddress(unsigned long long address, int width);

    //void setTxAddress(unsigned long msb_address, unsigned long lsb_address, int width);
	void nRF24L01P_setTxAddressNL(unsigned long msb_address, unsigned long lsb_address, int width);

    /**
     * Get the Receive address.
     *
     * @param pipe pipe to get the address from (0..5, default 0)
     * @return the address associated with the particular pipe
     */
    unsigned long long nRF24L01P_getRxAddress(int pipe);		// = NRF24L01P_PIPE_P0);

    /**
     * Get the Transmit address.
     *
     * @return address address for transmission
     */
    unsigned long long nRF24L01P_getTxAddress(void);

    /**
     * Set the transfer size.
     *
     * @param size the size of the transfer, in bytes (1..32)
     * @param pipe pipe for the transfer (0..5, default 0)
     */
    //void nRF24L01P_setTransferSize(int size = DEFAULT_NRF24L01P_TRANSFER_SIZE, int pipe = NRF24L01P_PIPE_P0);
	void nRF24L01P_setTransferSize(int size, int pipe);

    /**
     * Get the transfer size.
     *
     * @return the size of the transfer, in bytes (1..32).
     */
    int nRF24L01P_getTransferSize(int pipe);		// = NRF24L01P_PIPE_P0);


    /**
     * Get the RPD (Received Power Detector) state.
     *
     * @return true if the received power exceeded -64dBm
     */
    int nRF24L01P_getRPD(void);

    /**
     * Put the nRF24L01+ into Receive mode
     */
    void nRF24L01P_setReceiveMode(void);

    /**
     * Put the nRF24L01+ into Transmit mode
     */
    void nRF24L01P_setTransmitMode(void);

    /**
     * Power up the nRF24L01+ into Standby mode
     */
    void nRF24L01P_powerUp(void);

    /**
     * Power down the nRF24L01+ into Power Down mode
     */
    void nRF24L01P_powerDown(void);

    /**
     * Enable the nRF24L01+ to Receive or Transmit (using the CE pin)
     */
    void nRF24L01P_enable(void);

    /**
     * Disable the nRF24L01+ to Receive or Transmit (using the CE pin)
     */
    void nRF24L01P_disable(void);

    /**
     * Transmit data
     *
     * @param pipe is ignored (included for consistency with file write routine)
     * @param data pointer to an array of bytes to write
     * @param count the number of bytes to send (1..32)
     * @return the number of bytes actually written, or -1 for an error
     */
    int nRF24L01P_write(int pipe, char *data, int count);
    
    /**
     * Receive data
     *
     * @param pipe the receive pipe to get data from
     * @param data pointer to an array of bytes to store the received data
     * @param count the number of bytes to receive (1..32)
     * @return the number of bytes actually received, 0 if none are received, or -1 for an error
     */
    int nRF24L01P_read(int pipe, char *data, int count);

    /**
     * Determine if there is data available to read
     *
     * @param pipe the receive pipe to check for data
     * @return true if the is data waiting in the given pipe
     */
    int nRF24L01P_readable(int pipe);		// = NRF24L01P_PIPE_P0);

    /**
     * Disable all receive pipes
     *
     * Note: receive pipes are enabled when their address is set.
     */
    void nRF24L01P_disableAllRxPipes(void);
    
    /**
     * Disable AutoAcknowledge function
     */
    void nRF24L01P_disableAutoAcknowledge(void);
    
    /**
     * Enable AutoAcknowledge function
     *
     * @param pipe the receive pipe
     */
    void nRF24L01P_enableAutoAcknowledge(int pipe);		// = NRF24L01P_PIPE_P0);
    
    /**
     * Disable AutoRetransmit function
     */
    void nRF24L01P_disableAutoRetransmit(void);
    
    /**
     * Enable AutoRetransmit function
     *
     * @param delay the delay between restransmits, in uS (250uS..4000uS)
     * @param count number of retransmits before generating an error (1..15)
     */
    void nRF24L01P_enableAutoRetransmit(int delay, int count);

    int nRF24L01P_flush( void);

//private:

    /**
     * Get the contents of an addressable register.
     *
     * @param regAddress address of the register
     * @return the contents of the register
     */
    int nRF24L01P_getRegister(int regAddress);

    /**
     * Set the contents of an addressable register.
     *
     * @param regAddress address of the register
     * @param regData data to write to the register
     */
    void nRF24L01P_setRegister(int regAddress, int regData);

    /**
     * Get the contents of the status register.
     *
     * @return the contents of the status register
     */
    int nRF24L01P_getStatusRegister(void);

#endif /* __NRF24L01P_H__ */

