
# mname

This is a rough, experimental DNS implementation. It's written in simple C to
run on old or embedded systems with limited memory. For example, it creates the answer packet by modifying the question packet.

***It is not production-ready! There are doubtless bugs and vulnerabilities!***

`mquery.c` is the server implementation, which is intended as a demonstration of how to use mname.c. The server must be run as root or with sudo so that it can listen on port 53, which is a privileged port. It can then be tested by running e.g. `dig foo @127.0.0.1` to get the address for "foo" (the response to all queries as implemented is hard-coded to be `1.1.1.1`, or `0x01010101` hex).

The server will dump packet and activity information to the console.

