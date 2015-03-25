#include <systemlib/param/param.h>

/*
 * Mode and factory address index to connect to.
 *
 * Default `-1` is the listen mode.
 *
 * A value in the range [0, n_factory_addresses) sets the connect mode
 * and the value is number of factory address to connect to.
 *
 * A value outside the range also sets the listen mode.
 *
 */
PARAM_DEFINE_INT32(A_BT_CONNECT_TO, -1);
