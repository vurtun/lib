/*
    Copyright (c) 2015, Andreas Fredriksson, Micha Mettke
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WBY_STATIC
#define WBY_IMPLEMENTATION
#define WBY_USE_FIXED_TYPES
#define WBY_USE_ASSERT
#include "../mm_wby.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#define MAX_WSCONN 8
struct server_state {
    int quit;
    unsigned frame_counter;
    struct wby_con *conn[MAX_WSCONN];
    int conn_count;
};

static void
sleep_for(long ms)
{
#if defined(__APPLE__)
    usleep(ms * 1000);
#elif defined(_WIN32)
    Sleep(ms);
#else
    time_t sec = (int)(ms / 1000);
    const long t = ms -(sec * 1000);
    struct timespec req;
    req.tv_sec = sec;
    req.tv_nsec = t * 1000000L;
    while(-1 == nanosleep(&req, &req));
#endif
}

static void
test_log(const char* text)
{
    printf("[debug] %s\n", text);
}

static wby_int
dispatch(struct wby_con *connection, void *userdata)
{
    struct server_state *state = (struct server_state*)userdata;
    if (!strcmp("/foo", connection->request.uri)) {
        wby_response_begin(connection, 200, 14, NULL, 0);
        wby_write(connection, "Hello, world!\n", 14);
        wby_response_end(connection);
        return 0;
    } else if (!strcmp("/bar", connection->request.uri)) {
        wby_response_begin(connection, 200, -1, NULL, 0);
        wby_write(connection, "Hello, world!\n", 14);
        wby_write(connection, "Hello, world?\n", 14);
        wby_response_end(connection);
        return 0;
    } else if (!strcmp("/quit", connection->request.uri)) {
        wby_response_begin(connection, 200, -1, NULL, 0);
        wby_write(connection, "Goodbye, cruel world\n", 22);
        wby_response_end(connection);
        state->quit = 1;
        return 0;
    } else return 1;
}

static int
websocket_connect(struct wby_con *connection, void *userdata)
{
    /* Allow websocket upgrades on /wstest */
    struct server_state *state = (struct server_state*)userdata;
    /* connection bound userdata */
    connection->user_data = NULL;
    if (0 == strcmp(connection->request.uri, "/wstest") && state->conn_count < MAX_WSCONN)
        return 0;
    else return 1;
}

static void
websocket_connected(struct wby_con *connection, void *userdata)
{
    struct server_state *state = (struct server_state*)userdata;
    printf("WebSocket connected\n");
    state->conn[state->conn_count++] = connection;
}

static int
websocket_frame(struct wby_con *connection, const struct wby_frame *frame, void *userdata)
{
    int i = 0;
    printf("WebSocket frame incoming\n");
    printf("  Frame OpCode: %d\n", frame->opcode);
    printf("  Final frame?: %s\n", (frame->flags & WBY_WSF_FIN) ? "yes" : "no");
    printf("  Masked?     : %s\n", (frame->flags & WBY_WSF_MASKED) ? "yes" : "no");
    printf("  Data Length : %d\n", (int) frame->payload_length);
    while (i < frame->payload_length) {
        unsigned char buffer[16];
        int remain = frame->payload_length - i;
        size_t read_size = remain > (int) sizeof buffer ? sizeof buffer : (size_t) remain;
        size_t k;

        printf("%08x ", (int) i);
        if (0 != wby_read(connection, buffer, read_size))
            break;
        for (k = 0; k < read_size; ++k)
            printf("%02x ", buffer[k]);
        for (k = read_size; k < 16; ++k)
            printf("   ");
        printf(" | ");
        for (k = 0; k < read_size; ++k)
            printf("%c", isprint(buffer[k]) ? buffer[k] : '?');
        printf("\n");
        i += (int)read_size;
    }
    return 0;
}

static void
websocket_closed(struct wby_con *connection, void *userdata)
{
    int i;
    struct server_state *state = (struct server_state*)userdata;
    printf("WebSocket closed\n");
    for (i = 0; i < state->conn_count; i++) {
        if (state->conn[i] == connection) {
            int remain = state->conn_count - i;
            memmove(state->conn + i, state->conn + i + 1, (size_t)remain * sizeof(struct wby_con*));
            --state->conn_count;
            break;
        }
    }
}

int
main(void)
{
    void *memory = NULL;
    wby_size needed_memory = 0;
    struct server_state state;
    struct wby_server server;

    struct wby_config config;
    memset(&config, 0, sizeof config);
    config.userdata = &state;
    config.address = "127.0.0.1";
    config.port = 8888;
    config.connection_max = 4;
    config.request_buffer_size = 2048;
    config.io_buffer_size = 8192;
    config.log = test_log;
    config.dispatch = dispatch;
    config.ws_connect = websocket_connect;
    config.ws_connected = websocket_connected;
    config.ws_frame = websocket_frame;
    config.ws_closed = websocket_closed;

#if defined(_WIN32)
    {WORD wsa_version = MAKEWORD(2,2);
    WSADATA wsa_data;
    if (WSAStartup(wsa_version, &wsa_data)) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }}
#endif

    wby_server_init(&server, &config, &needed_memory);
    memory = calloc(needed_memory, 1);
    wby_server_start(&server, memory);

    memset(&state, 0, sizeof state);
    while (!state.quit) {
        int i = 0;
        wby_server_update(&server);
        /* Push some test data over websockets */
        if (!(state.frame_counter & 0x7f)) {
            for (i = 0; i < state.conn_count; ++i) {
                wby_frame_begin(state.conn[i], WBY_WSOP_TEXT_FRAME);
                wby_write(state.conn[i], "Hello world over websockets!\n", 29);
                wby_frame_end(state.conn[i]);
            }
        }
        sleep_for(30);
        ++state.frame_counter;
    }
    wby_server_stop(&server);
    free(memory);
#if defined(_WIN32)
    WSACleanup();
#endif
    return 0;
}

