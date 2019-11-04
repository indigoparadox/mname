
#ifndef MPKTDMP_H
#define MPKTDMP_H

#include "mname.h"

#define NAME_BUF_SZ 512
#define PKT_BUF_SZ 512
#define RDATA_BUF_SZ 512

struct pkt_dump_highlight {
   uint16_t offset;
   char ansi[6];
};

void pkt_dump_display(
   const struct mname_msg* dns_msg, size_t sz );
void pkt_dump_file( const char* name, const uint8_t* buffer, size_t sz );
void pkt_summarize( const uint8_t* pkt_buf, size_t count );

#endif /* MPKTDMP_H */

