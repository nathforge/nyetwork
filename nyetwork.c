/*
Example usage:
    NY_LOG=access.log LD_PRELOAD=`pwd`/nyetwork.so firefox

TODO:
    Store getaddrinfo results in lookup table (can probably get away with linked list)
*/

#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#define __USE_GNU
#include <dlfcn.h>

#define LOG_ENV "NY_LOG"

void _nyetwork_log_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res, int error);
void _nyetwork_log_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int result);
void _nyetwork_log(char *format, ...);

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    static int (*_getaddrinfo)(const char *, const char *, const struct addrinfo *, struct addrinfo **) = NULL;
    int error;

    if (!_getaddrinfo) _getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");

    error = _getaddrinfo(node, service, hints, res);
    _nyetwork_log_getaddrinfo(node, service, hints, res, error);
    return error;
}

int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    static int (*_connect)(int, const struct sockaddr *, socklen_t) = NULL;
    int error;

    if (!_connect) _connect = dlsym(RTLD_NEXT, "connect");

    error = _connect(sockfd, serv_addr, addrlen);
    _nyetwork_log_connect(sockfd, serv_addr, addrlen, error);
    return error;
}

void _nyetwork_log_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res, int error)
{
    struct addrinfo *addrinfo;
    char addrstr[INET_ADDRSTRLEN];

    if (error) {
        return;
    }

    for (addrinfo = *res; addrinfo != NULL; addrinfo = addrinfo->ai_next) {
        if (inet_ntop(addrinfo->ai_family, &((struct sockaddr_in *)addrinfo->ai_addr)->sin_addr, addrstr, INET_ADDRSTRLEN)) {
            _nyetwork_log("DNS: %s is %s", addrstr, node);
        }
    }
}

void _nyetwork_log_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int error)
{
    unsigned const char *c;
    int port;
    char addrstr[INET_ADDRSTRLEN];

    if (error) {
        return;
    }

    if (serv_addr->sa_family != AF_INET) {
        return;
    }

    // TODO: Nicen this up. Tried casting to a sockaddr_in, that didn't work -
    // give it another shot.
    c = serv_addr->sa_data;
    port = (256 * c[0]) + c[1];

    if (inet_ntop(serv_addr->sa_family, &((struct sockaddr_in *)serv_addr)->sin_addr, addrstr, INET_ADDRSTRLEN)) {
        _nyetwork_log("Conn: %s:%hu", addrstr, port);
    }
}

void _nyetwork_log(char *format, ...)
{
    va_list args;
    char *filename;
    FILE *fp;

    filename = getenv(LOG_ENV);

    if (!filename) {
        return;
    }

    fp = fopen(filename, "a");
    if (fp < 0) {
        fprintf(stderr, "Can't open " LOG_ENV " %s\n", filename);
        return;
    }

    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fputc('\n', fp);
    fclose(fp);
}
