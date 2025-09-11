#ifndef MODBUS_HPP_INCLUDED
#define MODBUS_HPP_INCLUDED

extern "C" {
#include <mbinst.h>
}

enum { /* These values should probably be user configurable at runtime */
	MODBUS_SLAVE_ADDRESS=1u, /* Modbus slave address (1-247) */
	MODBUS_BAUD=19200u, /* Serial communication baud rate */
};

/**
 * @brief Initialize the Modbus slave
 *
 * Sets up the Modbus register maps and initializes RTU communication.
 * Call this once in setup().
 */
extern void modbus_init(void);

/**
 * @brief Get the Modbus instance
 *
 * @return Pointer to the Modbus instance structure
 */
extern struct mbinst_s *modbus_get(void);

#endif /* MODBUS_HPP_INCLUDED */
