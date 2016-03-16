/*
 * SignalKCat - tool to test SignalK WS servers
 * Copyright (C) 2016 Pavel Kalian <pavel@kalian.cz>
 *
 * Based on libwebsockets-test-client - Copyright (C) 2011 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The person who associated a work with this deed has dedicated
 * the work to the public domain by waiving all of his or her rights
 * to the work worldwide under copyright law, including all related
 * and neighboring rights, to the extent allowed by law. You can copy,
 * modify, distribute and perform the work, even for commercial purposes,
 * all without asking permission.
 *
 * The app is intended to be adapted for use in your code, which
 * may be proprietary.  It is licensed Public Domain.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
#define random rand
#include "gettimeofday.h"
#else
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <libwebsockets.h>


static int deny_deflate, deny_mux, longlived;
static struct lws *wsi_dump;
static volatile int force_exit;
static unsigned int opts;

/*
 *
 *  sk-dump-protocol:  we connect to the server and dump the data
 *				we are given
 *
 */

enum demo_protocols {
    
    PROTOCOL_SK_DUMP,
    
    /* always last */
    DEMO_PROTOCOL_COUNT
};


/*
 * sk_dump protocol
 *
 * since this also happens to be protocols[0], some callbacks that are not
 * bound to a specific protocol also turn up here.
 */

bool stc_end = false;

static int
callback_sk_dump(struct lws *wsi, enum lws_callback_reasons reason,
                 void *user, void *in, size_t len)
{
    switch (reason) {
            
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_info("dump: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
            break;
            
        case LWS_CALLBACK_CLOSED:
            lwsl_notice("dump: LWS_CALLBACK_CLOSED\n");
            wsi_dump = NULL;
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
            ((char *)in)[len] = '\0';
            //lwsl_info("CR_rx %d '%s'\n", (int)len, (char *)in);
            if(!stc_end && ((char *)in)[len-1] == '}' )
                stc_end = true;
            else 
                if(stc_end && ((char *)in)[0] == '{')
                {
                    fprintf(stdout, "\n");
                    stc_end = false;
                }
            fprintf(stdout, (char *)in);
            break;
            
            /* because we are protocols[0] ... */
            
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            if (wsi == wsi_dump) {
                lwsl_err("dump: LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
                wsi_dump = NULL;
            }
            break;

        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
            if ((strcmp((const char*)in, "deflate-stream") == 0) && deny_deflate) {
                lwsl_notice("denied deflate-stream extension\n");
                return 1;
            }
            if ((strcmp((const char*)in, "deflate-frame") == 0) && deny_deflate) {
                lwsl_notice("denied deflate-frame extension\n");
                return 1;
            }
            if ((strcmp((const char*)in, "x-google-mux") == 0) && deny_mux) {
                lwsl_notice("denied x-google-mux extension\n");
                return 1;
            }
            break;

        default:
            break;
    }
    
    return 0;
}


/* list of supported protocols and callbacks */

static struct lws_protocols protocols[] = {
    {
        0,
        callback_sk_dump,
        0,
        50,
    },
    { NULL, NULL, 0, 0 } /* end */
};

void sighandler(int sig)
{
    force_exit = 1;
}

static struct option options[] = {
    { "help",	no_argument,		NULL, 'h' },
    { "debug",  required_argument,      NULL, 'd' },
    { "port",	required_argument,	NULL, 'p' },
    { "ssl",	no_argument,		NULL, 's' },
    { "version",	required_argument,	NULL, 'v' },
    { "undeflated",	no_argument,		NULL, 'u' },
    { "nomux",	no_argument,		NULL, 'n' },
    { "longlived",	no_argument,		NULL, 'l' },
    { NULL, 0, 0, 0 }
};

static int ratelimit_connects(unsigned int *last, int secs)
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    if (tv.tv_sec - (*last) < secs)
        return 0;
    
    *last = tv.tv_sec;
    
    return 1;
}

int main(int argc, char **argv)
{
    int n = 0, ret = 0, port = 3000, use_ssl = 0;
    int debug = 0;
    unsigned int rl_dump = 0;
    struct lws_context_creation_info info;
    struct lws_context *context;
    int ietf_version = -1; /* latest */
    const char *address;
    
    memset(&info, 0, sizeof info);
    
    if (argc < 2)
        goto usage;
    while (n >= 0) {
        n = getopt_long(argc, argv, "nuv:hsp:d:l", options, NULL);
        if (n < 0)
            continue;
        switch (n) {
            case 'd':
                debug = atoi(optarg);
                break;
            case 's':
                use_ssl = 2; /* 2 = allow selfsigned */
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'l':
                longlived = 1;
                break;
            case 'v':
                ietf_version = atoi(optarg);
                break;
            case 'u':
                deny_deflate = 1;
                break;
            case 'n':
                deny_mux = 1;
                break;
            case 'h':
                goto usage;
        }
    }

    lws_set_log_level(debug, NULL);


    if (optind >= argc)
        goto usage;
    
    signal(SIGINT, sighandler);
    
    address = argv[optind];
    
    /*
     * create the websockets context.  This tracks open connections and
     * knows how to route any traffic and which protocol version to use,
     * and if each connection is client or server side.
     *
     * For this tool, we tell it to not listen on any port.
     */
    
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
    info.extensions = lws_get_internal_extensions(); //extensions;//
#endif
    info.gid = -1;
    info.uid = -1;
    
    context = lws_create_context(&info);
    if (context == NULL) {
        fprintf(stderr, "Creating libwebsocket context failed\n");
        return 1;
    }
    
    /*
     * Sit there servicing the websocket context to handle incoming
     * packets.
     *
     * nothing happens until the client websocket connection is
     * asynchronously established... calling lws_client_connect() only
     * instantiates the connection logically, lws_service() processes it
     * asynchronously.
     */
    
    while (!force_exit) {
        
        if (!wsi_dump && ratelimit_connects(&rl_dump, 2)) {
            lwsl_notice("dump: connecting\n");
            char host[255];
            snprintf(host, 254, "%s:%d", address, port);
            wsi_dump = lws_client_connect(context, address, port,
                                          use_ssl, "/signalk/v1/stream", host, 0,
                                          protocols[PROTOCOL_SK_DUMP].name,
                                          ietf_version);
        }
        
        lws_service(context, 500);
    }
    
    lwsl_err("Exiting\n");
    lws_context_destroy(context);
    
    return ret;
usage:
    fprintf(stderr, "SignalK websockets test client\n"
            "(C) Copyright 2016 Pavel Kalian <pavel@kalian.cz> "
            "licensed under LGPL2.1\n\n");
    fprintf(stderr, "Usage: signalkcat "
            "<server host> [--port=<p>] "
            "[--ssl] [-n] [-l] [-u] [-v <ver>] "
            "[-d <log bitfield>]\n");
    return 1;
}
