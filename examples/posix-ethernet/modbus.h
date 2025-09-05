#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED

#include <mbinst.h>

extern void modbus_init(void);
extern struct mbinst_s *modbus_get(void);

#endif /* MODBUS_H_INCLUDED */
