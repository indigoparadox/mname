
#include "mname.h"

void mname_response( struct mname_msg* msg_in ) {
   msg_in->fields &= M_NAME_RESPONSE_FIELD;
}

