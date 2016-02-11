#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#define SWBUFMAX    128
#define SWREADMAX   64
#define SWWRITEMAX  64

#ifdef __cplusplus
extern "C"
{
	#endif

	typedef struct serial
	{
		char    *port;
		int     fd;
		int8_t  connected;
		int     baudrate;
		int     parity;

	  /* values */
		char    buffer[SWBUFMAX];
		char    readbuf[SWREADMAX];
		int8_t  readAvailable;
	} serial_t;

	/** Connect to a serial device.
	 *  @param connection
	 *    a pointer to the serial struct
	 *  @param port
	 *    a portname; if NULL, will open a random port
	 *  @param baudrate
	 *    the bits per second of information to transmit/receive
	 *  @return 0 on success, -1 on failure
	 */
	int serial_connect(serial_t *connection, char *port, int baudrate);

	/** Read a string from the serial communication link.
	 *  @param connection
	 *    the serial connection to read a message from
	 *  @return the readbuf if a message exists, else NULL
	 */
	char *serial_read(serial_t *connection);

	/** Write a message to the serial communication link.
	 *  @param connection
	 *    the serial communication link to write to
	 *  @param message
	 *    the message to send over to the other side
	 *  @note
	 *    be sure the message has a '\n' chararacter
	 */
	void serial_write(serial_t *connection, char *message);

	/** Disconnect from the USB Serial port.
	 *  @param connection
	 *    A pointer to the serial struct.
	 */
	void serial_disconnect(serial_t *connection);

	#ifdef __cplusplus
}
#endif
#endif
