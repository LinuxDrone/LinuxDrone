#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>


#include "libwebsockets.h"

#include <bcon.h>
#include <bson.h>


enum demo_protocols {
    /* always first */
    PROTOCOL_HTTP = 0,

    PROTOCOL_DUMB_INCREMENT,
    //PROTOCOL_LWS_MIRROR,

    /* always last */
    DEMO_PROTOCOL_COUNT
};

static struct option options[] = {
    { "help",	no_argument,		NULL, 'h' },
    { "debug",	required_argument,	NULL, 'd' },
    { "port",	required_argument,	NULL, 'p' },
    { "ssl",	no_argument,		NULL, 's' },
    { "allow-non-ssl",	no_argument,		NULL, 'a' },
    { "interface",  required_argument,	NULL, 'i' },
    { "closetest",  no_argument,		NULL, 'c' },
    { "libev",  no_argument,		NULL, 'e' },
    #ifndef LWS_NO_DAEMONIZE
    { "daemonize", 	no_argument,		NULL, 'D' },
#endif
    { "resource_path", required_argument,		NULL, 'r' },
    { NULL, 0, 0, 0 }
};

#define LOCAL_RESOURCE_PATH "/root"

char *resource_path = LOCAL_RESOURCE_PATH;

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

struct per_session_data__http {
    int fd;
};

const char * get_mimetype(const char *file)
{
    int n = strlen(file);

    if (n < 5)
        return NULL;

    if (!strcmp(&file[n - 4], ".ico"))
        return "image/x-icon";

    if (!strcmp(&file[n - 4], ".png"))
        return "image/png";

    if (!strcmp(&file[n - 5], ".html"))
        return "text/html";

    if (!strcmp(&file[n - 3], ".js"))
        return "application/javascript";


    return NULL;
}


/* this protocol server (always the first one) just knows how to do HTTP */
static int callback_http(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason, void *user,
                               void *in, size_t len)
{
    char buf[256];
    char leaf_path[1024];
    char b64[64];
    struct timeval tv;
    int n, m;
    unsigned char *p;
    char *other_headers;
    static unsigned char buffer[4096];
    struct stat stat_buf;
    struct per_session_data__http *pss = (struct per_session_data__http *)user;
    const char *mimetype;

    switch (reason) {
    case LWS_CALLBACK_HTTP:

        //dump_handshake_info(wsi);

        if (len < 1) {
            libwebsockets_return_http_status(context, wsi,
                        HTTP_STATUS_BAD_REQUEST, NULL);
            return -1;
        }

        /* this server has no concept of directories */
        if (strchr((const char *)in + 1, '/')) {
            libwebsockets_return_http_status(context, wsi,
                        HTTP_STATUS_FORBIDDEN, NULL);
            return -1;
        }

        /* if a legal POST URL, let it continue and accept data */
        if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
            return 0;

        /* check for the "send a big file by hand" example case */

        if (!strcmp((const char *)in, "/leaf.jpg")) {
            if (strlen(resource_path) > sizeof(leaf_path) - 10)
                return -1;
            sprintf(leaf_path, "%s/leaf.jpg", resource_path);

            /* well, let's demonstrate how to send the hard way */

            p = buffer;


            pss->fd = open(leaf_path, O_RDONLY);

            if (pss->fd < 0)
                return -1;

            fstat(pss->fd, &stat_buf);

            /*
             * we will send a big jpeg file, but it could be
             * anything.  Set the Content-Type: appropriately
             * so the browser knows what to do with it.
             */

            p += sprintf((char *)p,
                "HTTP/1.0 200 OK\x0d\x0a"
                "Server: libwebsockets\x0d\x0a"
                "Content-Type: image/jpeg\x0d\x0a"
                    "Content-Length: %u\x0d\x0a\x0d\x0a",
                    (unsigned int)stat_buf.st_size);

            /*
             * send the http headers...
             * this won't block since it's the first payload sent
             * on the connection since it was established
             * (too small for partial)
             */

            n = libwebsocket_write(wsi, buffer,
                   p - buffer, LWS_WRITE_HTTP);

            if (n < 0) {
                close(pss->fd);
                return -1;
            }
            /*
             * book us a LWS_CALLBACK_HTTP_WRITEABLE callback
             */
            libwebsocket_callback_on_writable(context, wsi);
            break;
        }

        /* if not, send a file the easy way */
        strcpy(buf, resource_path);
        if (strcmp(in, "/")) {
            if (*((const char *)in) != '/')
                strcat(buf, "/");
            strncat(buf, in, sizeof(buf) - strlen(resource_path));
        } else /* default file to serve */
            strcat(buf, "/test.html");
        buf[sizeof(buf) - 1] = '\0';

        /* refuse to serve files we don't understand */
        mimetype = get_mimetype(buf);
        if (!mimetype) {
            lwsl_err("Unknown mimetype for %s\n", buf);
            libwebsockets_return_http_status(context, wsi,
                      HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
            return -1;
        }

        /* demostrates how to set a cookie on / */

        other_headers = NULL;
        if (!strcmp((const char *)in, "/") &&
               !lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_COOKIE)) {
            /* this isn't very unguessable but it'll do for us */
            gettimeofday(&tv, NULL);
            sprintf(b64, "LWS_%u_%u_COOKIE",
                (unsigned int)tv.tv_sec,
                (unsigned int)tv.tv_usec);

            sprintf(leaf_path,
                "Set-Cookie: test=LWS_%u_%u_COOKIE;Max-Age=360000\x0d\x0a",
                (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec);
            other_headers = leaf_path;
            lwsl_err(other_headers);
        }

        if (libwebsockets_serve_http_file(context, wsi, buf,
                        mimetype, other_headers))
            return -1; /* through completion or error, close the socket */

        /*
         * notice that the sending of the file completes asynchronously,
         * we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
         * it's done
         */

        break;

    case LWS_CALLBACK_HTTP_BODY:
        strncpy(buf, in, 20);
        buf[20] = '\0';
        if (len < 20)
            buf[len] = '\0';

        lwsl_notice("LWS_CALLBACK_HTTP_BODY: %s... len %d\n",
                (const char *)buf, (int)len);

        break;

    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
        lwsl_notice("LWS_CALLBACK_HTTP_BODY_COMPLETION\n");
        /* the whole of the sent body arried, close the connection */
        libwebsockets_return_http_status(context, wsi,
                        HTTP_STATUS_OK, NULL);

        return -1;

    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
//		lwsl_info("LWS_CALLBACK_HTTP_FILE_COMPLETION seen\n");
        /* kill the connection after we sent one file */
        return -1;

    case LWS_CALLBACK_HTTP_WRITEABLE:
        /*
         * we can send more of whatever it is we were sending
         */

        do {
            n = read(pss->fd, buffer, sizeof buffer);
            /* problem reading, close conn */
            if (n < 0)
                goto bail;
            /* sent it all, close conn */
            if (n == 0)
                goto flush_bail;
            /*
             * because it's HTTP and not websocket, don't need to take
             * care about pre and postamble
             */
            m = libwebsocket_write(wsi, buffer, n, LWS_WRITE_HTTP);
            if (m < 0)
                /* write failed, close conn */
                goto bail;
            if (m != n)
                /* partial write, adjust */
                lseek(pss->fd, m - n, SEEK_CUR);

            if (m) /* while still active, extend timeout */
                libwebsocket_set_timeout(wsi,
                    PENDING_TIMEOUT_HTTP_CONTENT, 5);

        } while (!lws_send_pipe_choked(wsi));
        libwebsocket_callback_on_writable(context, wsi);
        break;
flush_bail:
        /* true if still partial pending */
        if (lws_send_pipe_choked(wsi)) {
            libwebsocket_callback_on_writable(context, wsi);
            break;
        }

bail:
        close(pss->fd);
        return -1;

    /*
     * callback for confirming to continue with client IP appear in
     * protocol 0 callback since no websocket protocol has been agreed
     * yet.  You can just ignore this if you won't filter on client IP
     * since the default uhandled callback return is 0 meaning let the
     * connection continue.
     */

    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        /* if we returned non-zero from here, we kill the connection */
        break;

    case LWS_CALLBACK_GET_THREAD_ID:
        /*
         * if you will call "libwebsocket_callback_on_writable"
         * from a different thread, return the caller thread ID
         * here so lws can use this information to work out if it
         * should signal the poll() loop to exit and restart early
         */

        /* return pthread_getthreadid_np(); */

        break;

    default:
        break;
    }

    return 0;
}


struct per_session_data__dumb_increment {
    int number;
};


static int
callback_dumb_increment(struct libwebsocket_context *context,
            struct libwebsocket *wsi,
            enum libwebsocket_callback_reasons reason,
                           void *user, void *in, size_t len)
{
    int n, m;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
                          LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    struct per_session_data__dumb_increment *pss = (struct per_session_data__dumb_increment *)user;

    //bson_t bson;
    bson_oid_t oid;
    bson_t *doc;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        lwsl_info("callback_dumb_increment: " "LWS_CALLBACK_ESTABLISHED\n");
        pss->number = 0;
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:





        // insert a document
        bson_oid_init (&oid, NULL);
        doc = BCON_NEW ("_id", BCON_OID (&oid),
                        "mynumber", BCON_INT32 (pss->number++));


        m = libwebsocket_write(wsi, (unsigned char *)bson_get_data(doc), (size_t)doc->len, LWS_WRITE_BINARY);
        if (m < n) {
            lwsl_err("ERROR %d writing to di socket\n", n);
            return -1;
        }

        bson_destroy (doc);


        /*
        n = sprintf((char *)p, "%d", pss->number++);
        m = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
        if (m < n) {
            lwsl_err("ERROR %d writing to di socket\n", n);
            return -1;
        }
        if (close_testing && pss->number == 50) {
            lwsl_info("close tesing limit, closing\n");
            return -1;
        }
        */
        break;

    case LWS_CALLBACK_RECEIVE:
//		fprintf(stderr, "rx %d\n", (int)len);
        if (len < 6)
            break;
        if (strcmp((const char *)in, "reset\n") == 0)
            pss->number = 0;
        break;
    /*
     * this just demonstrates how to use the protocol filter. If you won't
     * study and reject connections based on header content, you don't need
     * to handle this callback
     */

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        //dump_handshake_info(wsi);
        /* you could return non-zero here and kill the connection */
        break;

    default:
        break;
    }

    return 0;
}



/* list of supported protocols and callbacks */
static struct libwebsocket_protocols protocols[] = {
    /* first protocol must always be HTTP handler */

    {
        "http-only",		/* name */
        callback_http,		/* callback */
        sizeof (struct per_session_data__http),	/* per_session_data_size */
        0,			/* max frame size / rx buffer */
    },
    {
        "dumb-increment-protocol",
        callback_dumb_increment,
        300,//sizeof(struct per_session_data__dumb_increment),
        100,
    },
    { NULL, NULL, 0, 0 } /* terminator */
};



int main(int argc, char **argv)
{
    char cert_path[1024];
    char key_path[1024];
    int n = 0;
    int use_ssl = 0;
    int opts = 0;
    char interface_name[128] = "";
    const char *iface = NULL;
    unsigned int oldus = 0;
    struct lws_context_creation_info info;

    int debug_level = 7;
#ifndef LWS_NO_DAEMONIZE
    int daemonize = 0;
#endif

    memset(&info, 0, sizeof info);
    info.port = 7681;

    while (n >= 0) {
        n = getopt_long(argc, argv, "eci:hsap:d:Dr:", options, NULL);
        if (n < 0)
            continue;
        switch (n) {
        case 'e':
            opts |= LWS_SERVER_OPTION_LIBEV;
            break;
#ifndef LWS_NO_DAEMONIZE
        case 'D':
            daemonize = 1;
            break;
#endif
        case 'd':
            debug_level = atoi(optarg);
            break;
        case 's':
            use_ssl = 1;
            break;
        case 'a':
            opts |= LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
            break;
        case 'p':
            info.port = atoi(optarg);
            break;
        case 'i':
            strncpy(interface_name, optarg, sizeof interface_name);
            interface_name[(sizeof interface_name) - 1] = '\0';
            iface = interface_name;
            break;
        case 'r':
            resource_path = optarg;
            printf("Setting resource path to \"%s\"\n", resource_path);
            break;
        case 'h':
            fprintf(stderr, "Usage: test-server "
                    "[--port=<p>] [--ssl] "
                    "[-d <log bitfield>] "
                    "[--resource_path <path>]\n");
            exit(1);
        }
    }

#if !defined(LWS_NO_DAEMONIZE) && !defined(WIN32)
    /*
     * normally lock path would be /var/lock/lwsts or similar, to
     * simplify getting started without having to take care about
     * permissions or running as root, set to /tmp/.lwsts-lock
     */
    if (daemonize && lws_daemonize("/tmp/.lwsts-lock")) {
        fprintf(stderr, "Failed to daemonize\n");
        return 1;
    }
#endif

    //signal(SIGINT, sighandler);

    /* tell the library what debug level to emit and to send it to syslog */
    lws_set_log_level(debug_level, lwsl_emit_syslog);

    lwsl_notice("libwebsockets test server - "
            "(C) Copyright 2010-2013 Andy Green <andy@warmcat.com> - "
                            "licensed under LGPL2.1\n");


    info.iface = iface;
    info.protocols = protocols;

    if (!use_ssl) {
        info.ssl_cert_filepath = NULL;
        info.ssl_private_key_filepath = NULL;
    } else {
        if (strlen(resource_path) > sizeof(cert_path) - 32) {
            lwsl_err("resource path too long\n");
            return -1;
        }
        sprintf(cert_path, "%s/libwebsockets-test-server.pem",
                                resource_path);
        if (strlen(resource_path) > sizeof(key_path) - 32) {
            lwsl_err("resource path too long\n");
            return -1;
        }
        sprintf(key_path, "%s/libwebsockets-test-server.key.pem",
                                resource_path);

        info.ssl_cert_filepath = cert_path;
        info.ssl_private_key_filepath = key_path;
    }
    info.gid = -1;
    info.uid = -1;
    info.options = opts;

    context = libwebsocket_create_context(&info);
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    n = 0;
    while (n >= 0 && !force_exit) {
        struct timeval tv;

        gettimeofday(&tv, NULL);

        /*
         * This provokes the LWS_CALLBACK_SERVER_WRITEABLE for every
         * live websocket connection using the DUMB_INCREMENT protocol,
         * as soon as it can take more packets (usually immediately)
         */

        if (((unsigned int)tv.tv_usec - oldus) > 50000) {
            libwebsocket_callback_on_writable_all_protocol(&protocols[PROTOCOL_DUMB_INCREMENT]);
            oldus = tv.tv_usec;
        }

        /*
         * If libwebsockets sockets are all we care about,
         * you can use this api which takes care of the poll()
         * and looping through finding who needed service.
         *
         * If no socket needs service, it'll return anyway after
         * the number of ms in the second argument.
         */

        n = libwebsocket_service(context, 50);
    }


    libwebsocket_context_destroy(context);

    lwsl_notice("libwebsockets-test-server exited cleanly\n");

    return 0;
}
