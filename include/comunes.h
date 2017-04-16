#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include "../include/colors.h"
#define TAM 512
#define endMsg "/END"
#define disconnectMsg "/BYE"
#define start_UDP_Msg "/START"
#define ack_msg "/ACK"
#define end_UDP_Msg "/FINISH"
#define udp_ready "/UDP_READY"