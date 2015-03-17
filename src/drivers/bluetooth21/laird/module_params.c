#include <systemlib/param/param.h>

/* module_params.hpp is safe to include to C code. */
#include "../module_params.hpp"

/*
 * Mode and factory address index to connect to.
 *
 */
PARAM_DEFINE_INT32(A_BT_CONNECT_TO, -1);

/*
 * RFCOMM Frame Size, bytes in range 23..4096.
 */
PARAM_DEFINE_INT32(A_BT_S11_RFCOMM, BT_SREG_AS_IS);

/*
 * Link supervision timeout, seconds.
 */
PARAM_DEFINE_INT32(A_BT_S12_LINK, BT_SREG_AS_IS);

/*
 * UART latency time, microseconds.
 *
 * Packet 32 bytes 8n1 at 115200 takes 2500us.
 * Actual latency setting should be a little more.
 */
PARAM_DEFINE_INT32(A_BT_S80_LATENCY, BT_SREG_AS_IS);

/*
 * UART poll latency one of -- (worst) 0, 1, 2, 3 (best).
 */
PARAM_DEFINE_INT32(A_BT_S84_POLL, BT_SREG_AS_IS);
