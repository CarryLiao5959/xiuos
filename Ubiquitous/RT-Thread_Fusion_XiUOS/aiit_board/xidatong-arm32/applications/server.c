#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>

#include "server.h"

#define SERVER_PORT 8881

