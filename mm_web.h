/*
    mm_web.h - BSD LICENSE - Andreas Fredriksson, Micha Mettke

ABOUT:
    This is a web server intended for debugging tools inside a
    program with a continously running main loop. It's intended to be used when
    all you need is something tiny and performance isn't a key concern.
    NOTE: this is a single header port of Andreas Fredriksson
    Webby(https://github.com/deplinenoise/webby).

    Features
    - Single header library to be easy to embed into your code.
    - No dynamic memory allocations -- server memory is completely fixed
    - No threading, all I/O and serving happens on the calling thread
    - Supports socket keep-alives
    - Supports the 100-Continue scheme for file uploading
    - Basic support for WebSockets is available.

    Because request/response I/O is synchronous on the calling thread, performance
    will suffer when you are serving data. For the use-cases wby is intended for,
    this is fine. You can still run wby in a background thread at your
    discretion if this is a problem.

DEFINES:
    MM_WBY_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    MM_WBY_STATIC
        The generated implementation will stay private inside implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    MM_WBY_ASSERT
    MM_WBY_USE_ASSERT
        If you define MM_WBY_USE_ASSERT without defining MM_ASSERT mm_web.h
        will use assert.h and asssert(). Otherwise it will use your assert
        method. If you do not define MM_WBY_USE_ASSERT no additional checks
        will be added. This is the only C standard library function used
        by mm_web.

    MM_WBY_UINT_PTR
        If your compiler is C99 you do not need to define this.
        Otherwise, mm_web will try default assignments for them
        and validate them at compile time. If they are incorrect, you will
        get compile errors and will need to define them yourself.

        You can define this to 'size_t' if you use the standard library,
        otherwise it needs to be able to hold the maximum addressable memory
        space. If you do not define this it will default to unsigned long.


LICENSE: (BSD)
    Copyright (c) 2016, Andreas Fredriksson, Micha Mettke
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

CONTRIBUTORS:
    Andreas Fredriksson (implementation)
    Micha Mettke (single header conversion)

USAGE:
    Request handling
        When you configure the server, you give it a function pointer to your
        dispatcher. The dispatcher is called by wby when a request has been fully
        read into memory and is ready for processing. The socket the request came in on
        has then been switched to blocking mode, and you're free to read any request
        data using `mm_wby_read()` (if present, check `content_length`) and then write
        your response.
        There are two ways to generate a response; explicit size or chunked.

    When you know the size of the data
        When you know in advance how big the response is going to be, you should pass
        that size in bytes to `mm_wby_response_begin()` (it will be sent as the
        Content-Length header). You then call `mm_wby_write()` to push that data out, and
        finally `mm_wby_response_end()` to finalize the response and prepare the socket
        for a new request.

    When the response size is dynamic
        Sometimes you want to generate an arbitrary amount of text in the response, and
        you don't know how much that will be. Rather than buffering everything in RAM,
        you can use chunked encoding. First call `mm_wby_response_begin()` as normal, but
        pass it -1 for the content length. This triggers sending the
        `Transfer-Encoding: chunked` header. You then call `mm_wby_write()` as desired
        until the response is complete. When you're done, call `mm_wby_response_end()` to finish up.

EXAMPLES:
    for a actual working example please look inside tests/mm_wby_test.c */
#if 0
/* request and websocket handling callback */
static int dispatch(struct mm_wby_con *connection, void *pArg);
static int websocket_connect(struct mm_wby_con *conn, void *pArg);
static void websocket_connected(struct mm_wby_con *con, void *pArg);
static int websocket_frame(struct mm_wby_con *conn, const struct mm_wby_frame *frame, void *pArg);
static void websocket_closed(struct mm_wby_con *connection, void *pArg);

int main(int argc, const char * argv[])
{
    /* setup config */
    struct mm_wby_config config;
    memset(config, 0, sizeof(config));
    config.address = "127.0.0.1";
    config.port = 8888;
    config.connection_max = 8;
    config.request_buffer_size = 2048;
    config.io_buffer_size = 8192;
    config.dispatch = dispatch;
    config.ws_connect = websocket_connect;
    config.ws_connected = websocket_connected;
    config.ws_frame = websocket_frame;
    config.ws_closed = websocket_closed;

    /* compute and allocate needed memory and start server */
    struct mm_wby_server server;
    size_t needed_memory;
    mm_wby_server_init(&server, &config, &needed_memory);
    void *memory = calloc(needed_memory, 1);
    mm_wby_server_start(&server, memory);
    while (1) {
        mm_wby_server_update(&server);

    }
    mm_wby_server_stop(&server);
    free(memory);
}
#endif
 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MM_WBY_H_
#define MM_WBY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MM_WBY_STATIC
#define MM_WBY_API static
#else
#define MM_WBY_API extern
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#include <stdint.h>
#ifndef MM_WBY_UINT_PTR
#define MM_WBY_UINT_PTR uintptr_t
#endif
#else
#ifndef MM_WBY_UINT_PTR
#define MM_WBY_UINT_PTR unsigned long
#endif
#endif
typedef unsigned char mm_wby_byte;
typedef MM_WBY_UINT_PTR mm_wby_size;
typedef MM_WBY_UINT_PTR mm_wby_ptr;

#define MM_WBY_OK (0)
#define MM_WBY_FLAG(x) (1 << (x))

#ifndef MM_WBY_MAX_HEADERS
#define MM_WBY_MAX_HEADERS 64
#endif

struct mm_wby_header {
    const char *name;
    const char *value;
};

/* A HTTP request. */
struct mm_wby_request {
    const char *method;
    /* The method of the request, e.g. "GET", "POST" and so on */
    const char *uri;
    /* The URI that was used. */
    const char *http_version;
    /* The used HTTP version */
    const char *query_params;
    /* The query parameters passed in the URL, or NULL if none were passed. */
    int content_length;
    /* The number of bytes of request body that are available via WebbyRead() */
    int header_count;
    /* The number of headers */
    struct mm_wby_header headers[MM_WBY_MAX_HEADERS];
    /* Request headers */
};

/* Connection state, as published to the serving callback. */
struct mm_wby_con {
    struct mm_wby_request request;
    /* The request being served. Read-only. */
    void *user_data;
    /* User data. Read-write. wby doesn't care about this. */
};

struct mm_wby_frame {
    mm_wby_byte flags;
    mm_wby_byte opcode;
    mm_wby_byte header_size;
    mm_wby_byte padding_;
    mm_wby_byte mask_key[4];
    int payload_length;
};

enum mm_wby_websock_flags {
    MM_WBY_WSF_FIN    = MM_WBY_FLAG(0),
    MM_WBY_WSF_MASKED = MM_WBY_FLAG(1)
};

enum mm_wby_websock_operation {
    MM_WBY_WSOP_CONTINUATION    = 0,
    MM_WBY_WSOP_TEXT_FRAME      = 1,
    MM_WBY_WSOP_BINARY_FRAME    = 2,
    MM_WBY_WSOP_CLOSE           = 8,
    MM_WBY_WSOP_PING            = 9,
    MM_WBY_WSOP_PONG            = 10
};

/* Configuration data required for starting a server. */
typedef void(*mm_wby_log_f)(const char *msg);
struct mm_wby_config {
    void *userdata;
    /* userdata which will be passed */
    const char *address;
    /* The bind address. Must be a textual IP address. */
    unsigned short port;
    /* The port to listen to. */
    unsigned int connection_max;
    /* Maximum number of simultaneous connections. */
    mm_wby_size request_buffer_size;
    /* The size of the request buffer. This must be big enough to contain all
    * headers and the request line sent by the client. 2-4k is a good size for
    * this buffer. */
    mm_wby_size io_buffer_size;
    /* The size of the I/O buffer, used when writing the reponse. 4k is a good
    * choice for this buffer.*/
    mm_wby_log_f log;
    /* Optional callback function that receives debug log text (without newlines). */
    int(*dispatch)(struct mm_wby_con *con, void *userdata);
    /* Request dispatcher function. This function is called when the request
    * structure is ready.
    * If you decide to handle the request, call mm_wby_response_begin(),
    * mm_wby_write() and mm_wby_response_end() and then return 0. Otherwise, return a
    * non-zero value to have Webby send back a 404 response. */
    int(*ws_connect)(struct mm_wby_con*, void *userdata);
    /*WebSocket connection dispatcher. Called when an incoming request wants to
    * update to a WebSocket connection.
    * Return 0 to allow the connection.
    * Return 1 to ignore the connection.*/
    void (*ws_connected)(struct mm_wby_con*, void *userdata);
    /* Called when a WebSocket connection has been established.*/
    void (*ws_closed)(struct mm_wby_con*, void *userdata);
    /*Called when a WebSocket connection has been closed.*/
    int (*ws_frame)(struct mm_wby_con*, const struct mm_wby_frame *frame, void *userdata);
    /*Called when a WebSocket data frame is incoming.
    * Call mm_wby_read() to read the payload data.
    * Return non-zero to close the connection.*/
};

struct mm_wby_connection;
struct mm_wby_server {
    struct mm_wby_config config;
    /* server configuration */
    mm_wby_size memory_size;
    /* minimum required memory */
    mm_wby_ptr socket;
    /* server socket */
    mm_wby_size con_count;
    /* number of active connection */
    struct mm_wby_connection *con;
    /* connections */
};

MM_WBY_API void mm_wby_init(struct mm_wby_server*, const struct mm_wby_config*,
                            mm_wby_size *needed_memory);
/*  this function clears the server and calculates the needed memory to run
    Input:
    -   filled server configuration data to calculate the needed memory
    Output:
    -   needed memory for the server to run
*/
MM_WBY_API int mm_wby_start(struct mm_wby_server*, void *memory);
/*  this function starts running the server in the specificed memory space. Size
 *  must be at least big enough as determined in the mm_wby_server_init().
    Input:
    -   allocated memory space to create the server into
*/
MM_WBY_API void mm_wby_update(struct mm_wby_server*);
/* updates the server by being called frequenctly (at least once a frame) */
MM_WBY_API void mm_wby_stop(struct mm_wby_server*);
/* stops and shutdown the server */
MM_WBY_API int mm_wby_response_begin(struct mm_wby_con*, int status_code, int content_length,
                                    const struct mm_wby_header headers[], int header_count);
/*  this function begins a response
    Input:
    -   HTTP status code to send. (Normally 200).
    -   size in bytes you intend to write, or -1 for chunked encoding
    -   array of HTTP headers to transmit (can be NULL of header_count == 0)
    -   number of headers in the array
    Output:
    -   returns 0 on success, non-zero on error.
*/
MM_WBY_API void mm_wby_response_end(struct mm_wby_con*);
/*  this function finishes a response. When you're done wirting the response
 *  body, call this function. this makes sure chunked encoding is terminated
 *  correctly and that the connection is setup for reuse. */
MM_WBY_API int mm_wby_read(struct mm_wby_con*, void *ptr, mm_wby_size len);
/*  this function reads data from the request body. Only read what the client
 *  has priovided (via content_length) parameter, or you will end up blocking
 *  forever.
    Input:
    - pointer to a memory block that will be filled
    - size of the memory block to fill
*/
MM_WBY_API int mm_wby_write(struct mm_wby_con*, const void *ptr, mm_wby_size len);
/*  this function writes a response data to the connection. If you're not using
 *  chunked encoding, be careful not to send more than the specified content
 *  length. You can call this function multiple times as long as the total
 *  number of bytes matches up with the content length.
    Input:
    - pointer to a memory block that will be send
    - size of the memory block to send
*/
MM_WBY_API int mm_wby_frame_begin(struct mm_wby_con*, int opcode);
/*  this function begins an outgoing websocket frame */
MM_WBY_API int mm_wby_frame_end(struct mm_wby_con*);
/*  this function finishes an outgoing websocket frame */
MM_WBY_API int mm_wby_find_query_var(const char *buf, const char *name, char *dst, mm_wby_size dst_len);
/*  this function is a helper function to lookup a query parameter given a URL
 *  encoded string. Returns the size of the returned data, or -1 if the query
 *  var wasn't found. */
MM_WBY_API const char* mm_wby_find_header(struct mm_wby_con*, const char *name);
/*  this convenience function to find a header in a request. Returns the value
 *  of the specified header, or NULL if its was not present. */

#ifdef __cplusplus
}
#endif
#endif /* MM_WBY_H_ */
/* ===============================================================
 *
 *                      IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MM_WBY_IMPLEMENTATION

typedef int mm_wby__check_ptr_size[(sizeof(void*) == sizeof(MM_WBY_UINT_PTR)) ? 1 : -1];
#define MM_WBY_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define MM_WBY_UNUSED(a) ((void)(a))

#ifdef MM_WBY_USE_ASSERT
#ifndef MM_WBY_ASSERT
#include <assert.h>
#define MM_WBY_ASSERT(expr) assert(expr)
#endif
#else
#define MM_WBY_ASSERT(expr)
#endif

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define MM_WBY_SOCK(s) ((mm_wby_socket)(s))
#define MM_WBY_INTERN static
#define MM_WBY_GLOBAL static
#define MM_WBY_STORAGE static

/* ===============================================================
 *                          UTIL
 * ===============================================================*/
struct mm_wby_buffer {
    mm_wby_size used;
    /* current buffer size */
    mm_wby_size max;
    /* buffer capacity */
    mm_wby_byte *data;
    /* pointer inside a global buffer */
};

MM_WBY_INTERN void
mm_wby_dbg(mm_wby_log_f log, const char *fmt, ...)
{
    char buffer[1024];
    va_list args;
    if (!log) return;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof buffer, fmt, args);
    va_end(args);
    buffer[(sizeof buffer)-1] = '\0';
    log(buffer);
}

MM_WBY_INTERN int
wb_peek_request_size(const mm_wby_byte *buf, int len)
{
    int i;
    int max = len - 3;
    for (i = 0; i < max; ++i) {
        if ('\r' != buf[i + 0]) continue;
        if ('\n' != buf[i + 1]) continue;
        if ('\r' != buf[i + 2]) continue;
        if ('\n' != buf[i + 3]) continue;
        /* OK; we have CRLFCRLF which indicates the end of the header section */
        return i + 4;
    }
    return -1;
}

MM_WBY_INTERN char*
mm_wby_skipws(char *p)
{
    for (;;) {
        char ch = *p;
        if (' ' == ch || '\t' == ch) ++p;
        else break;
    }
    return p;
}

#define MM_WBY_TOK_SKIPWS MM_WBY_FLAG(0)
MM_WBY_INTERN int
mm_wby_tok_inplace(char *buf, const char* separator, char *tokens[], int max, int flags)
{
    char *b = buf;
    char *e = buf;
    int token_count = 0;
    int separator_len = (int)strlen(separator);
    while (token_count < max) {
        if (flags & MM_WBY_TOK_SKIPWS)
            b = mm_wby_skipws(b);
        if (NULL != (e = strstr(b, separator))) {
            int len = (int) (e - b);
            if (len > 0)
                tokens[token_count++] = b;
            *e = '\0';
            b = e + separator_len;
        } else {
            tokens[token_count++] = b;
            break;
        }
    }
    return token_count;
}

MM_WBY_INTERN mm_wby_size
mm_wby_make_websocket_header(mm_wby_byte buffer[10], mm_wby_byte opcode,
    int payload_len, int fin)
{
    buffer[0] = (mm_wby_byte)((fin ? 0x80 : 0x00) | opcode);
    if (payload_len < 126) {
        buffer[1] = (mm_wby_byte)(payload_len & 0x7f);
        return 2;
    } else if (payload_len < 65536) {
        buffer[1] = 126;
        buffer[2] = (mm_wby_byte)(payload_len >> 8);
        buffer[3] = (mm_wby_byte)payload_len;
        return 4;
    } else {
        buffer[1] = 127;
        /* Ignore high 32-bits. I didn't want to require 64-bit types and typdef hell in the API. */
        buffer[2] = buffer[3] = buffer[4] = buffer[5] = 0;
        buffer[6] = (mm_wby_byte)(payload_len >> 24);
        buffer[7] = (mm_wby_byte)(payload_len >> 16);
        buffer[8] = (mm_wby_byte)(payload_len >> 8);
        buffer[9] = (mm_wby_byte)payload_len;
        return 10;
    }
}

MM_WBY_INTERN int
mm_wby_read_buffered_data(int *data_left, struct mm_wby_buffer* buffer,
    char **dest_ptr, mm_wby_size *dest_len)
{
    int offset, read_size;
    int left = *data_left;
    int len;
    if (left == 0)
        return 0;

    len = (int) *dest_len;
    offset = (int)buffer->used - left;
    read_size = (len > left) ? left : len;
    memcpy(*dest_ptr, buffer->data + offset, (mm_wby_size)read_size);

    (*dest_ptr) += read_size;
    (*dest_len) -= (mm_wby_size) read_size;
    (*data_left) -= read_size;
    return read_size;
}

/* ---------------------------------------------------------------
 *                          SOCKET
 * ---------------------------------------------------------------*/
#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET mm_wby_socket;
typedef int mm_wby_socklen;

#if defined(__GNUC__)
#define MM_WBY_ALIGN(x) __attribute__((aligned(x)))
#else
#define MM_WBY_ALIGN(x) __declspec(align(x))
#endif

#define MM_WBY_INVALID_SOCKET INVALID_SOCKET
#define snprintf _snprintf

MM_WBY_INTERN int
mm_wby_socket_error(void)
{
    return WSAGetLastError();
}

#if !defined(__GNUC__)
MM_WBY_INTERN int
strcasecmp(const char *a, const char *b)
{
    return _stricmp(a, b);
}

MM_WBY_INTERN int
strncasecmp(const char *a, const char *b, mm_wby_size len)
{
    return _strnicmp(a, b, len);
}
#endif

MM_WBY_INTERN int
mm_wby_socket_set_blocking(mm_wby_socket socket, int blocking)
{
    u_long val = !blocking;
    return ioctlsocket(socket, FIONBIO, &val);
}

MM_WBY_INTERN int
mm_wby_socket_is_valid(mm_wby_socket socket)
{
    return (INVALID_SOCKET != socket);
}

MM_WBY_INTERN void
mm_wby_socket_close(mm_wby_socket socket)
{
    closesocket(socket);
}

MM_WBY_INTERN int
mm_wby_socket_is_blocking_error(int error)
{
    return WSAEWOULDBLOCK == error;
}

#else /* UNIX */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

typedef int mm_wby_socket;
typedef socklen_t mm_wby_socklen;

#define MM_WBY_ALIGN(x) __attribute__((aligned(x)))
#define MM_WBY_INVALID_SOCKET (-1)

MM_WBY_INTERN int
mm_wby_socket_error(void)
{
    return errno;
}

MM_WBY_INTERN int
mm_wby_socket_is_valid(mm_wby_socket socket)
{
    return (socket > 0);
}

MM_WBY_INTERN void
mm_wby_socket_close(mm_wby_socket socket)
{
    close(socket);
}

MM_WBY_INTERN int
mm_wby_socket_is_blocking_error(int error)
{
    return (EAGAIN == error);
}

MM_WBY_INTERN int
mm_wby_socket_set_blocking(mm_wby_socket socket, int blocking)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0) return flags;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(socket, F_SETFL, flags);
}
#endif

MM_WBY_INTERN int
mm_wby_socket_config_incoming(mm_wby_socket socket)
{
    int off = 0;
    int err;
    if ((err = mm_wby_socket_set_blocking(socket, 0)) != MM_WBY_OK) return err;
    setsockopt(socket, SOL_SOCKET, SO_LINGER, (const char*) &off, sizeof(int));
    return 0;
}

MM_WBY_INTERN int
mm_wby_socket_send(mm_wby_socket socket, const mm_wby_byte *buffer, int size)
{
    while (size > 0) {
        long err = send(socket, (const char*)buffer, (mm_wby_size)size, 0);
        if (err <= 0) return 1;
        buffer += err;
        size -= (int)err;
    }
    return 0;
}

/* Read as much as possible without blocking while there is buffer space. */
enum {MM_WBY_FILL_OK, MM_WBY_FILL_ERROR, MM_WBY_FILL_FULL};
MM_WBY_INTERN int
mm_wby_socket_recv(mm_wby_socket socket, struct mm_wby_buffer *buf, mm_wby_log_f log)
{
    long err;
    int buf_left;
    for (;;) {
        buf_left = (int)buf->max - (int)buf->used;
        mm_wby_dbg(log, "buffer space left = %d", buf_left);
        if (buf_left == 0)
            return MM_WBY_FILL_FULL;

        /* Read what we can into the current buffer space. */
        err = recv(socket, (char*)buf->data + buf->used, (mm_wby_size)buf_left, 0);
        if (err < 0) {
            int sock_err = mm_wby_socket_error();
            if (mm_wby_socket_is_blocking_error(sock_err)) {
                return MM_WBY_FILL_OK;
            } else {
                /* Read error. Give up. */
                mm_wby_dbg(log, "read error %d - connection dead", sock_err);
                return MM_WBY_FILL_ERROR;
            }
        } else if (err == 0) {
          /* The peer has closed the connection. */
          mm_wby_dbg(log, "peer has closed the connection");
          return MM_WBY_FILL_ERROR;
        } else buf->used += (mm_wby_size)err;
    }
}

MM_WBY_INTERN int
mm_wby_socket_flush(mm_wby_socket socket, struct mm_wby_buffer *buf)
{
    if (buf->used > 0){
        if (mm_wby_socket_send(socket, buf->data, (int)buf->used) != MM_WBY_OK)
            return 1;
    }
    buf->used = 0;
    return 0;
}

/* ---------------------------------------------------------------
 *                          URL
 * ---------------------------------------------------------------*/
/* URL-decode input buffer into destination buffer.
 * 0-terminate the destination buffer. Return the length of decoded data.
 * form-url-encoded data differs from URI encoding in a way that it
 * uses '+' as character for space, see RFC 1866 section 8.2.1
 * http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
 *
 * This bit of code was taken from mongoose.
 */
MM_WBY_INTERN mm_wby_size
mm_wby_url_decode(const char *src, mm_wby_size src_len, char *dst, mm_wby_size dst_len,
    int is_form_url_encoded)
{
    int a, b;
    mm_wby_size i, j;
    #define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')
    for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++) {
        if (src[i] == '%' &&
            isxdigit(*(const mm_wby_byte*)(src + i + 1)) &&
            isxdigit(*(const mm_wby_byte*)(src + i + 2)))
        {
            a = tolower(*(const mm_wby_byte*)(src + i + 1));
            b = tolower(*(const mm_wby_byte*)(src + i + 2));
            dst[j] = (char)((HEXTOI(a) << 4) | HEXTOI(b));
            i += 2;
        } else if (is_form_url_encoded && src[i] == '+') {
            dst[j] = ' ';
        } else dst[j] = src[i];
    }
    #undef HEXTOI
    dst[j] = '\0'; /* Null-terminate the destination */
    return j;
}

/* Pulled from mongoose */
MM_WBY_API int
mm_wby_find_query_var(const char *buf, const char *name, char *dst, mm_wby_size dst_len)
{
    const char *p, *e, *s;
    mm_wby_size name_len;
    int len;
    mm_wby_size buf_len = strlen(buf);

    name_len = strlen(name);
    e = buf + buf_len;
    len = -1;
    dst[0] = '\0';

    /* buf is "var1=val1&var2=val2...". Find variable first */
    for (p = buf; p != NULL && p + name_len < e; p++) {
        if ((p == buf || p[-1] == '&') && p[name_len] == '=' &&
            strncasecmp(name, p, name_len) == 0)
        {
            /* Point p to variable value */
            p += name_len + 1;
            /* Point s to the end of the value */
            s = (const char *) memchr(p, '&', (mm_wby_size)(e - p));
            if (s == NULL) s = e;
            MM_WBY_ASSERT(s >= p);
            /* Decode variable into destination buffer */
            if ((mm_wby_size) (s - p) < dst_len)
                len = (int)mm_wby_url_decode(p, (mm_wby_size)(s - p), dst, dst_len, 1);
            break;
        }
    }
    return len;
}

/* ---------------------------------------------------------------
 *                          BASE64
 * ---------------------------------------------------------------*/
#define MM_WBY_BASE64_QUADS_BEFORE_LINEBREAK 19

MM_WBY_INTERN mm_wby_size
mm_wby_base64_bufsize(mm_wby_size input_size)
{
    mm_wby_size triplets = (input_size + 2) / 3;
    mm_wby_size base_size = 4 * triplets;
    mm_wby_size line_breaks = 2 * (triplets / MM_WBY_BASE64_QUADS_BEFORE_LINEBREAK);
    mm_wby_size null_termination = 1;
    return base_size + line_breaks + null_termination;
}

MM_WBY_INTERN int
mm_wby_base64_encode(char* output, mm_wby_size output_size,
    const mm_wby_byte *input, mm_wby_size input_size)
{
    mm_wby_size i = 0;
    int line_out = 0;
    MM_WBY_STORAGE const char enc[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/=";
    if (output_size < mm_wby_base64_bufsize(input_size))
        return 1;

    while (i < input_size) {
        unsigned int idx_0, idx_1, idx_2, idx_3;
        unsigned int i0;

        i0 = (unsigned int)(input[i]) << 16; i++;
        i0 |= (unsigned int)(i < input_size ? input[i] : 0) << 8; i++;
        i0 |= (i < input_size ? input[i] : 0); i++;

        idx_0 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_1 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_2 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_3 = (i0 & 0xfc0000) >> 18;

        if (i - 1 > input_size) idx_2 = 64;
        if (i > input_size) idx_3 = 64;

        *output++ = enc[idx_0];
        *output++ = enc[idx_1];
        *output++ = enc[idx_2];
        *output++ = enc[idx_3];

        if (++line_out == MM_WBY_BASE64_QUADS_BEFORE_LINEBREAK) {
          *output++ = '\r';
          *output++ = '\n';
        }
    }
    *output = '\0';
    return 0;
}

/* ---------------------------------------------------------------
 *                          SHA1
 * ---------------------------------------------------------------*/
struct mm_wby_sha1 {
    unsigned int state[5];
    unsigned int msg_size[2];
    mm_wby_size buf_used;
    mm_wby_byte buffer[64];
};

MM_WBY_INTERN unsigned int
mm_wby_sha1_rol(unsigned int value, unsigned int bits)
{
    return ((value) << bits) | (value >> (32 - bits));
}

MM_WBY_INTERN void
mm_wby_sha1_hash_block(unsigned int state[5], const mm_wby_byte *block)
{
    int i;
    unsigned int a, b, c, d, e;
    unsigned int w[80];

    /* Prepare message schedule */
    for (i = 0; i < 16; ++i) {
        w[i] =  (((unsigned int)block[(i*4)+0]) << 24) |
                (((unsigned int)block[(i*4)+1]) << 16) |
                (((unsigned int)block[(i*4)+2]) <<  8) |
                (((unsigned int)block[(i*4)+3]) <<  0);
    }

    for (i = 16; i < 80; ++i)
        w[i] = mm_wby_sha1_rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    /* Initialize working variables */
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];

    /* This is the core loop for each 20-word span. */
    #define SHA1_LOOP(start, end, func, constant) \
        for (i = (start); i < (end); ++i) { \
            unsigned int t = mm_wby_sha1_rol(a, 5) + (func) + e + (constant) + w[i]; \
            e = d; d = c; c = mm_wby_sha1_rol(b, 30); b = a; a = t;}

    SHA1_LOOP( 0, 20, ((b & c) ^ (~b & d)),           0x5a827999)
    SHA1_LOOP(20, 40, (b ^ c ^ d),                    0x6ed9eba1)
    SHA1_LOOP(40, 60, ((b & c) ^ (b & d) ^ (c & d)),  0x8f1bbcdc)
    SHA1_LOOP(60, 80, (b ^ c ^ d),                    0xca62c1d6)
    #undef SHA1_LOOP

    /* Update state */
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

MM_WBY_INTERN void
mm_wby_sha1_init(struct mm_wby_sha1 *s)
{
    s->state[0] = 0x67452301;
    s->state[1] = 0xefcdab89;
    s->state[2] = 0x98badcfe;
    s->state[3] = 0x10325476;
    s->state[4] = 0xc3d2e1f0;
    s->msg_size[0] = 0;
    s->msg_size[1] = 0;
    s->buf_used = 0;
}

MM_WBY_INTERN void
mm_wby_sha1_update(struct mm_wby_sha1 *s, const void *data_, mm_wby_size size)
{
    const char *data = (const char*)data_;
    unsigned int size_lo;
    unsigned int size_lo_orig;
    mm_wby_size remain = size;

    while (remain > 0) {
        mm_wby_size buf_space = sizeof(s->buffer) - s->buf_used;
        mm_wby_size copy_size = (remain < buf_space) ? remain : buf_space;
        memcpy(s->buffer + s->buf_used, data, copy_size);
        s->buf_used += copy_size;
        data += copy_size;
        remain -= copy_size;

        if (s->buf_used == sizeof(s->buffer)) {
            mm_wby_sha1_hash_block(s->state, s->buffer);
            s->buf_used = 0;
        }
    }

    size_lo = size_lo_orig = s->msg_size[1];
    size_lo += (unsigned int)(size * 8);
    if (size_lo < size_lo_orig)
        s->msg_size[0] += 1;
    s->msg_size[1] = size_lo;
}

MM_WBY_INTERN void
mm_wby_sha1_final(mm_wby_byte digest[20], struct mm_wby_sha1 *s)
{
    mm_wby_byte zero = 0x00;
    mm_wby_byte one_bit = 0x80;
    mm_wby_byte count_data[8];
    int i;

    /* Generate size data in bit endian format */
    for (i = 0; i < 8; ++i) {
        unsigned int word = s->msg_size[i >> 2];
        count_data[i] = (mm_wby_byte)(word >> ((3 - (i & 3)) * 8));
    }

    /* Set trailing one-bit */
    mm_wby_sha1_update(s, &one_bit, 1);
    /* Emit null padding to to make room for 64 bits of size info in the last 512 bit block */
    while (s->buf_used != 56)
        mm_wby_sha1_update(s, &zero, 1);

    /* Write size in bits as last 64-bits */
    mm_wby_sha1_update(s, count_data, 8);
    /* Make sure we actually finalized our last block */
    MM_WBY_ASSERT(s->buf_used == 0);

    /* Generate digest */
    for (i = 0; i < 20; ++i) {
        unsigned int word = s->state[i >> 2];
        mm_wby_byte byte = (mm_wby_byte) ((word >> ((3 - (i & 3)) * 8)) & 0xff);
        digest[i] = byte;
    }
}

/* ---------------------------------------------------------------
 *                          CONNECTION
 * ---------------------------------------------------------------*/
#define MM_WBY_WEBSOCKET_VERSION "13"
MM_WBY_GLOBAL const char mm_wby_continue_header[] = "HTTP/1.1 100 Continue\r\n\r\n";
MM_WBY_GLOBAL const mm_wby_size mm_wby_continue_header_len = sizeof(mm_wby_continue_header) - 1;
MM_WBY_GLOBAL const char mm_wby_websocket_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
MM_WBY_GLOBAL const mm_wby_size mm_wby_websocket_guid_len = sizeof(mm_wby_websocket_guid) - 1;
MM_WBY_GLOBAL const mm_wby_byte mm_wby_websocket_pong[] = { 0x80, MM_WBY_WSOP_PONG, 0x00 };
MM_WBY_GLOBAL const struct mm_wby_header mm_wby_plain_text_headers[]={{"Content-Type","text/plain"}};

enum mm_wby_connection_flags {
    MM_WBY_CON_FLAG_ALIVE                  = MM_WBY_FLAG(0),
    MM_WBY_CON_FLAG_FRESH_CONNECTION       = MM_WBY_FLAG(1),
    MM_WBY_CON_FLAG_CLOSE_AFTER_RESPONSE   = MM_WBY_FLAG(2),
    MM_WBY_CON_FLAG_CHUNKED_RESPONSE       = MM_WBY_FLAG(3),
    MM_WBY_CON_FLAG_WEBSOCKET              = MM_WBY_FLAG(4)
};

enum mm_wby_connection_state {
    MM_WBY_CON_STATE_REQUEST,
    MM_WBY_CON_STATE_SEND_CONTINUE,
    MM_WBY_CON_STATE_SERVE,
    MM_WBY_CON_STATE_WEBSOCKET
};

struct mm_wby_connection {
    struct mm_wby_con public_data;
    unsigned short flags;
    unsigned short state;
    mm_wby_ptr socket;
    mm_wby_log_f log;
    mm_wby_size request_buffer_size;
    mm_wby_size io_buffer_size;
    struct mm_wby_buffer header_buf;
    struct mm_wby_buffer io_buf;
    int header_body_left;
    int io_data_left;
    int continue_data_left;
    int body_bytes_read;
    struct mm_wby_frame ws_frame;
    mm_wby_byte ws_opcode;
    mm_wby_size blocking_count;
};

MM_WBY_INTERN int
mm_wby_connection_set_blocking(struct mm_wby_connection *conn)
{
    if (conn->blocking_count == 0) {
        if (mm_wby_socket_set_blocking(MM_WBY_SOCK(conn->socket), 1) != MM_WBY_OK) {
            mm_wby_dbg(conn->log, "failed to switch connection to blocking");
            conn->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
            return -1;
        }
    }
    ++conn->blocking_count;
    return 0;
}

MM_WBY_INTERN int
mm_wby_connection_set_nonblocking(struct mm_wby_connection *conn)
{
    mm_wby_size count = conn->blocking_count;
    if (count == 1) {
        if (mm_wby_socket_set_blocking(MM_WBY_SOCK(conn->socket), 0) != MM_WBY_OK) {
            mm_wby_dbg(conn->log, "failed to switch connection to non-blocking");
            conn->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
            return -1;
        }
    }
    conn->blocking_count = count - 1;
    return 0;
}

MM_WBY_INTERN void
mm_wby_connection_reset(struct mm_wby_connection *conn, mm_wby_size request_buffer_size,
    mm_wby_size io_buffer_size)
{
    conn->header_buf.used = 0;
    conn->header_buf.max = request_buffer_size;
    conn->io_buf.used = 0;
    conn->io_buf.max = io_buffer_size;
    conn->header_body_left = 0;
    conn->io_data_left = 0;
    conn->continue_data_left = 0;
    conn->body_bytes_read = 0;
    conn->state = MM_WBY_CON_STATE_REQUEST;
    conn->public_data.user_data = NULL;
    conn->blocking_count = 0;
}

MM_WBY_INTERN void
mm_wby_connection_close(struct mm_wby_connection* connection)
{
    if (MM_WBY_SOCK(connection->socket) != MM_WBY_INVALID_SOCKET) {
        mm_wby_socket_close(MM_WBY_SOCK(connection->socket));
        connection->socket = (mm_wby_ptr)MM_WBY_INVALID_SOCKET;
    }
    connection->flags = 0;
}

MM_WBY_INTERN int
mm_wby_connection_setup_request(struct mm_wby_connection *connection, int request_size)
{
    char* lines[MM_WBY_MAX_HEADERS + 2];
    int line_count;
    char* tok[16];
    char* query_params;
    int tok_count;

    int i;
    int header_count;
    char *buf = (char*) connection->header_buf.data;
    struct mm_wby_request *req = &connection->public_data.request;

    /* Null-terminate the request envelope by overwriting the last CRLF with 00LF */
    buf[request_size - 2] = '\0';
    /* Split header into lines */
    line_count = mm_wby_tok_inplace(buf, "\r\n", lines, MM_WBY_LEN(lines), 0);
    header_count = line_count - 2;
    if (line_count < 1 || header_count > (int) MM_WBY_LEN(req->headers))
        return 1;

    /* Parse request line */
    tok_count = mm_wby_tok_inplace(lines[0], " ", tok, MM_WBY_LEN(tok), 0);
    if (3 != tok_count) return 1;

    req->method = tok[0];
    req->uri = tok[1];
    req->http_version = tok[2];
    req->content_length = 0;

    /* See if there are any query parameters */
    if ((query_params = (char*) strchr(req->uri, '?')) != NULL) {
        req->query_params = query_params + 1;
        *query_params = '\0';
    } else req->query_params = NULL;

    {
        /* Decode the URI in place */
        mm_wby_size uri_len = strlen(req->uri);
        mm_wby_url_decode(req->uri, uri_len, (char*)req->uri, uri_len + 1, 1);
    }

    /* Parse headers */
    for (i = 0; i < header_count; ++i) {
        tok_count = mm_wby_tok_inplace(lines[i + 1], ":", tok, 2, MM_WBY_TOK_SKIPWS);
        if (tok_count != 2) return 1;
        req->headers[i].name = tok[0];
        req->headers[i].value = tok[1];

        if (!strcasecmp("content-length", tok[0])) {
            req->content_length = (int)strtoul(tok[1], NULL, 10);
            mm_wby_dbg(connection->log, "request has body; content length is %d", req->content_length);
        } else if (!strcasecmp("transfer-encoding", tok[0])) {
            mm_wby_dbg(connection->log, "cowardly refusing to handle Transfer-Encoding: %s", tok[1]);
            return 1;
        }
    }
    req->header_count = header_count;
    return 0;
}

MM_WBY_INTERN int
mm_wby_connection_send_websocket_upgrade(struct mm_wby_connection* connection)
{
    const char *hdr;
    struct mm_wby_sha1 sha;
    mm_wby_byte digest[20];
    char output_digest[64];
    struct mm_wby_header headers[3];
    struct mm_wby_con *conn = &connection->public_data;
    if ((hdr = mm_wby_find_header(conn, "Sec-WebSocket-Version")) == NULL) {
        mm_wby_dbg(connection->log, "Sec-WebSocket-Version header not present");
        return 1;
    }
    if (strcmp(hdr, MM_WBY_WEBSOCKET_VERSION)) {
        mm_wby_dbg(connection->log,"WebSocket version %s not supported (we only do %s)",
                hdr, MM_WBY_WEBSOCKET_VERSION);
        return 1;
    }
    if ((hdr = mm_wby_find_header(conn, "Sec-WebSocket-Key")) == NULL) {
        mm_wby_dbg(connection->log, "Sec-WebSocket-Key header not present");
        return 1;
    }
    /* Compute SHA1 hash of Sec-Websocket-Key + the websocket guid as required by
    * the RFC.
    *
    * This handshake is bullshit. It adds zero security. Just forces me to drag
    * in SHA1 and create a base64 encoder.
    */
    mm_wby_sha1_init(&sha);
    mm_wby_sha1_update(&sha, hdr, strlen(hdr));
    mm_wby_sha1_update(&sha, mm_wby_websocket_guid, mm_wby_websocket_guid_len);
    mm_wby_sha1_final(&digest[0], &sha);
    if (mm_wby_base64_encode(output_digest, sizeof output_digest, &digest[0], sizeof(digest)) != MM_WBY_OK)
        return 1;

    headers[0].name  = "Upgrade";
    headers[0].value = "websocket";
    headers[1].name  = "Connection";
    headers[1].value = "Upgrade";
    headers[2].name  = "Sec-WebSocket-Accept";
    headers[2].value = output_digest;
    mm_wby_response_begin(&connection->public_data, 101, 0, headers, MM_WBY_LEN(headers));
    mm_wby_response_end(&connection->public_data);
    return 0;
}

MM_WBY_INTERN int
mm_wby_connection_push(struct mm_wby_connection *conn, const void *data_, int len)
{
    struct mm_wby_buffer *buf = &conn->io_buf;
    const mm_wby_byte* data = (const mm_wby_byte*)data_;
    if (conn->state != MM_WBY_CON_STATE_SERVE) {
        mm_wby_dbg(conn->log, "attempt to write in non-serve state");
        return 1;
    }
    if (len == 0)
        return mm_wby_socket_flush(MM_WBY_SOCK(conn->socket), buf);

    while (len > 0) {
        int buf_space = (int)buf->max - (int)buf->used;
        int copy_size = len < buf_space ? len : buf_space;
        memcpy(buf->data + buf->used, data, (mm_wby_size)copy_size);
        buf->used += (mm_wby_size)copy_size;

        data += copy_size;
        len -= copy_size;
        if (buf->used == buf->max) {
            if (mm_wby_socket_flush(MM_WBY_SOCK(conn->socket), buf) != MM_WBY_OK)
                return 1;
            if ((mm_wby_size)len >= buf->max)
                return mm_wby_socket_send(MM_WBY_SOCK(conn->socket), data, len);
        }
    }
    return 0;
}

/* ---------------------------------------------------------------
 *                          CON/REQUEST
 * ---------------------------------------------------------------*/
MM_WBY_INTERN int
mm_wby_con_discard_incoming_data(struct mm_wby_con* conn, int count)
{
    while (count > 0) {
        char buffer[1024];
        int read_size = (int)(((mm_wby_size)count > sizeof(buffer)) ?
            sizeof(buffer) : (mm_wby_size)count);
        if (mm_wby_read(conn, buffer, (mm_wby_size)read_size) != MM_WBY_OK)
            return -1;
        count -= read_size;
    }
    return 0;
}

MM_WBY_API const char*
mm_wby_find_header(struct mm_wby_con *conn, const char *name)
{
    int i, count;
    for (i = 0, count = conn->request.header_count; i < count; ++i) {
        if (!strcasecmp(conn->request.headers[i].name, name))
            return conn->request.headers[i].value;
    }
    return NULL;
}

MM_WBY_INTERN int
mm_wby_con_is_websocket_request(struct mm_wby_con* conn)
{
    const char *hdr;
    if ((hdr = mm_wby_find_header(conn, "Connection")) == NULL) return 0;
    if (strcasecmp(hdr, "Upgrade")) return 0;
    if ((hdr = mm_wby_find_header(conn, "Upgrade")) == NULL) return 0;
    if (strcasecmp(hdr, "websocket")) return 0;
    return 1;
}

MM_WBY_INTERN int
mm_wby_scan_websocket_frame(struct mm_wby_frame *frame, const struct mm_wby_buffer *buf)
{
    mm_wby_byte flags = 0;
    unsigned int len = 0;
    unsigned int opcode = 0;
    mm_wby_byte* data = buf->data;
    mm_wby_byte* data_max = data + buf->used;

    int i;
    int len_bytes = 0;
    int mask_bytes = 0;
    mm_wby_byte header0, header1;
    if (buf->used < 2)
        return -1;

    header0 = *data++;
    header1 = *data++;
    if (header0 & 0x80)
        flags |= MM_WBY_WSF_FIN;
    if (header1 & 0x80) {
        flags |= MM_WBY_WSF_MASKED;
        mask_bytes = 4;
    }

    opcode = header0 & 0xf;
    len = header1 & 0x7f;
    if (len == 126)
        len_bytes = 2;
    else if (len == 127)
        len_bytes = 8;
    if (data + len_bytes + mask_bytes > data_max)
        return -1;

    /* Read big endian length from length bytes (if greater than 125) */
    len = len_bytes == 0 ? len : 0;
    for (i = 0; i < len_bytes; ++i) {
        /* This will totally overflow for 64-bit values. I don't care.
         * If you're transmitting more than 4 GB of data using Webby,
         * seek help. */
        len <<= 8;
        len |= *data++;
    }

    /* Read mask word if present */
    for (i = 0; i < mask_bytes; ++i)
        frame->mask_key[i] = *data++;
    frame->header_size = (mm_wby_byte) (data - buf->data);
    frame->flags = flags;
    frame->opcode = (mm_wby_byte) opcode;
    frame->payload_length = (int)len;
    return 0;
}

MM_WBY_API int
mm_wby_frame_begin(struct mm_wby_con *conn_pub, int opcode)
{
    struct mm_wby_connection *conn = (struct mm_wby_connection*)conn_pub;
    conn->ws_opcode = (mm_wby_byte) opcode;
    /* Switch socket to blocking mode */
    return mm_wby_connection_set_blocking(conn);
}

MM_WBY_API int
mm_wby_frame_end(struct mm_wby_con *conn_pub)
{
    mm_wby_byte header[10];
    mm_wby_size header_size;
    struct mm_wby_connection *conn = (struct mm_wby_connection*) conn_pub;
    header_size = mm_wby_make_websocket_header(header, conn->ws_opcode, 0, 1);
    if (mm_wby_socket_send(MM_WBY_SOCK(conn->socket), header, (int) header_size) != MM_WBY_OK)
        conn->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
    /* Switch socket to non-blocking mode */
    return mm_wby_connection_set_nonblocking(conn);
}

MM_WBY_API int
mm_wby_read(struct mm_wby_con *conn, void *ptr_, mm_wby_size len)
{
    struct mm_wby_connection* conn_prv = (struct mm_wby_connection*)conn;
    char *ptr = (char*) ptr_;
    int count;

    int start_pos = conn_prv->body_bytes_read;
    if (conn_prv->header_body_left > 0) {
        count = mm_wby_read_buffered_data(&conn_prv->header_body_left, &conn_prv->header_buf, &ptr, &len);
        conn_prv->body_bytes_read += count;
    }

    /* Read buffered websocket data */
    if (conn_prv->io_data_left > 0) {
        count = mm_wby_read_buffered_data(&conn_prv->io_data_left, &conn_prv->io_buf, &ptr, &len);
        conn_prv->body_bytes_read += count;
    }

    while (len > 0) {
        long err = recv(MM_WBY_SOCK(conn_prv->socket), ptr, (mm_wby_size)len, 0);
        if (err < 0) {
            conn_prv->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
            return (int)err;
        }
        len -= (mm_wby_size)err;
        ptr += (mm_wby_size)err;
        conn_prv->body_bytes_read += (int)err;
    }

    if ((conn_prv->flags & MM_WBY_CON_FLAG_WEBSOCKET) && (conn_prv->ws_frame.flags & MM_WBY_WSF_MASKED)) {
        /* XOR outgoing data with websocket ofuscation key */
        int i, end_pos = conn_prv->body_bytes_read;
        const mm_wby_byte *mask = conn_prv->ws_frame.mask_key;
        ptr = (char*) ptr_; /* start over */
        for (i = start_pos; i < end_pos; ++i) {
            mm_wby_byte byte = (mm_wby_byte)*ptr;
            *ptr++ = (char)(byte ^ mask[i & 3]);
        }
    }
    return 0;
}

MM_WBY_API int
mm_wby_write(struct mm_wby_con *conn, const void *ptr, mm_wby_size len)
{
    struct mm_wby_connection *conn_priv = (struct mm_wby_connection*) conn;
    if (conn_priv->flags & MM_WBY_CON_FLAG_WEBSOCKET) {
        mm_wby_byte header[10];
        mm_wby_size header_size;
        header_size = mm_wby_make_websocket_header(header, conn_priv->ws_opcode, (int)len, 0);

        /* Overwrite opcode to be continuation packages from here on out */
        conn_priv->ws_opcode = MM_WBY_WSOP_CONTINUATION;
        if (mm_wby_socket_send(MM_WBY_SOCK(conn_priv->socket), header, (int)header_size) != MM_WBY_OK) {
            conn_priv->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
            return -1;
        }
        if (mm_wby_socket_send(MM_WBY_SOCK(conn_priv->socket),(const mm_wby_byte*)ptr, (int)len) != MM_WBY_OK) {
            conn_priv->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
            return -1;
        }
        return 0;
    } else if (conn_priv->flags & MM_WBY_CON_FLAG_CHUNKED_RESPONSE) {
        char chunk_header[128];
        int header_len = snprintf(chunk_header, sizeof chunk_header, "%x\r\n", (int) len);
        mm_wby_connection_push(conn_priv, chunk_header, header_len);
        mm_wby_connection_push(conn_priv, ptr, (int)len);
        return mm_wby_connection_push(conn_priv, "\r\n", 2);
    } else return mm_wby_connection_push(conn_priv, ptr, (int) len);
}

MM_WBY_INTERN int
mm_wby_printf(struct mm_wby_con* conn, const char* fmt, ...)
{
    int len;
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof buffer, fmt, args);
    va_end(args);
    return mm_wby_write(conn, buffer, (mm_wby_size)len);
}

/* ---------------------------------------------------------------
 *                          RESPONSE
 * ---------------------------------------------------------------*/
#define MM_WBY_STATUS_MAP(STATUS)\
    STATUS(100, "Continue")\
    STATUS(101, "Switching Protocols")\
    STATUS(200, "OK")\
    STATUS(201, "Created")\
    STATUS(202, "Accepted")\
    STATUS(203, "Non-Authoritative Information")\
    STATUS(204, "No Content")\
    STATUS(205, "Reset Content")\
    STATUS(206, "Partial Content")\
    STATUS(300, "Multiple Choices")\
    STATUS(301, "Moved Permanently")\
    STATUS(302, "Found")\
    STATUS(303, "See Other")\
    STATUS(304, "Not Modified")\
    STATUS(305, "Use Proxy")\
    STATUS(307, "Temporary Redirect")\
    STATUS(400, "Bad Request")\
    STATUS(401, "Unauthorized")\
    STATUS(402, "Payment Required")\
    STATUS(403, "Forbidden")\
    STATUS(404, "Not Found")\
    STATUS(405, "Method Not Allowed")\
    STATUS(406, "Not Acceptable")\
    STATUS(407, "Proxy Authentication Required")\
    STATUS(408, "Request Time-out")\
    STATUS(409, "Conflict")\
    STATUS(410, "Gone")\
    STATUS(411, "Length Required")\
    STATUS(412, "Precondition Failed")\
    STATUS(413, "Request Entity Too Large")\
    STATUS(414, "Request-URI Too Large")\
    STATUS(415, "Unsupported Media Type")\
    STATUS(416, "Requested range not satisfiable")\
    STATUS(417, "Expectation Failed")\
    STATUS(500, "Internal Server Error")\
    STATUS(501, "Not Implemented")\
    STATUS(502, "Bad Gateway")\
    STATUS(503, "Service Unavailable")\
    STATUS(504, "Gateway Time-out")\
    STATUS(505, "HTTP Version not supported")

MM_WBY_GLOBAL const short mm_wby_status_nums[] = {
#define MM_WBY_STATUS(id, name) id,
    MM_WBY_STATUS_MAP(MM_WBY_STATUS)
#undef MM_WBY_STATUS
};
MM_WBY_GLOBAL const char* mm_wby_status_text[] = {
#define MM_WBY_STATUS(id, name) name,
    MM_WBY_STATUS_MAP(MM_WBY_STATUS)
#undef MM_WBY_STATUS
};

MM_WBY_INTERN const char*
mm_wby_response_status_text(int status_code)
{
    int i;
    for (i = 0; i < (int) MM_WBY_LEN(mm_wby_status_nums); ++i) {
        if (mm_wby_status_nums[i] == status_code)
            return mm_wby_status_text[i];
    }
    return "Unknown";
}

MM_WBY_API int
mm_wby_response_begin(struct mm_wby_con *conn_pub, int status_code, int content_length,
    const struct mm_wby_header *headers, int header_count)
{
    int i = 0;
    struct mm_wby_connection *conn = (struct mm_wby_connection *)conn_pub;
    if (conn->body_bytes_read < (int)conn->public_data.request.content_length) {
        int body_left = conn->public_data.request.content_length - (int)conn->body_bytes_read;
        if (mm_wby_con_discard_incoming_data(conn_pub, body_left) != MM_WBY_OK) {
            conn->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
            return -1;
        }
    }

    mm_wby_printf(conn_pub, "HTTP/1.1 %d %s\r\n", status_code, mm_wby_response_status_text(status_code));
    if (content_length >= 0)
        mm_wby_printf(conn_pub, "Content-Length: %d\r\n", content_length);
    else mm_wby_printf(conn_pub, "Transfer-Encoding: chunked\r\n");
    mm_wby_printf(conn_pub, "Server: wby\r\n");

    for (i = 0; i < header_count; ++i) {
        if (!strcasecmp(headers[i].name, "Connection")) {
            if (!strcasecmp(headers[i].value, "close"))
            conn->flags |= MM_WBY_CON_FLAG_CLOSE_AFTER_RESPONSE;
        }
        mm_wby_printf(conn_pub, "%s: %s\r\n", headers[i].name, headers[i].value);
    }

    if (!(conn->flags & MM_WBY_CON_FLAG_CLOSE_AFTER_RESPONSE)) {
        /* See if the client wants us to close the connection. */
        const char* connection_header = mm_wby_find_header(conn_pub, "Connection");
        if (connection_header && !strcasecmp("close", connection_header)) {
            conn->flags |= MM_WBY_CON_FLAG_CLOSE_AFTER_RESPONSE;
            mm_wby_printf(conn_pub, "Connection: close\r\n");
        }
    }
    mm_wby_printf(conn_pub, "\r\n");
    if (content_length < 0)
        conn->flags |= MM_WBY_CON_FLAG_CHUNKED_RESPONSE;
    return 0;
}

MM_WBY_API void
mm_wby_response_end(struct mm_wby_con *conn)
{
    struct mm_wby_connection *conn_priv = (struct mm_wby_connection*) conn;
    if (conn_priv->flags & MM_WBY_CON_FLAG_CHUNKED_RESPONSE) {
        /* Write final chunk */
        mm_wby_connection_push(conn_priv, "0\r\n\r\n", 5);
        conn_priv->flags &= (unsigned short)~MM_WBY_CON_FLAG_CHUNKED_RESPONSE;
    }
    /* Flush buffers */
    mm_wby_connection_push(conn_priv, "", 0);
}

/* ---------------------------------------------------------------
 *                          SERVER
 * ---------------------------------------------------------------*/
/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MM_WBY_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MM_WBY_PTR_TO_UINT(x) ((mm_wby_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MM_WBY_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MM_WBY_PTR_TO_UINT(x) ((mm_wby_size)(((char*)x)-(char*)0))
#elif defined(MM_WBY_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MM_WBY_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MM_WBY_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MM_WBY_UINT_TO_PTR(x) ((void*)(x))
# define MM_WBY_PTR_TO_UINT(x) ((mm_wby_size)(x))
#endif

/* simple pointer math */
#define MM_WBY_PTR_ADD(t, p, i) ((t*)((void*)((mm_wby_byte*)(p) + (i))))
#define MM_WBY_ALIGN_PTR(x, mask)\
    (MM_WBY_UINT_TO_PTR((MM_WBY_PTR_TO_UINT((mm_wby_byte*)(x) + (mask-1)) & ~(mask-1))))

/* pointer alignment  */
#ifdef __cplusplus
template<typename T> struct mm_wby_alignof;
template<typename T, int size_diff> struct mm_wby_helper{enum {value = size_diff};};
template<typename T> struct mm_wby_helper<T,0>{enum {value = mm_wby_alignof<T>::value};};
template<typename T> struct mm_wby_alignof{struct Big {T x; char c;}; enum {
    diff = sizeof(Big) - sizeof(T), value = mm_wby_helper<Big, diff>::value};};
#define MM_WBY_ALIGNOF(t) (mm_wby_alignof<t>::value);
#else
#define MM_WBY_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
#endif

MM_WBY_API void
mm_wby_init(struct mm_wby_server *srv, const struct mm_wby_config *cfg, mm_wby_size *needed_memory)
{
    MM_WBY_STORAGE const mm_wby_size mm_wby_conn_align = MM_WBY_ALIGNOF(struct mm_wby_connection);
    MM_WBY_ASSERT(srv);
    MM_WBY_ASSERT(cfg);
    MM_WBY_ASSERT(needed_memory);
    memset(srv, 0, sizeof(*srv));
    srv->config = *cfg;
    MM_WBY_ASSERT(cfg->dispatch);

    *needed_memory = 0;
    *needed_memory += cfg->connection_max * sizeof(struct mm_wby_connection);
    *needed_memory += cfg->connection_max * cfg->request_buffer_size;
    *needed_memory += cfg->connection_max * cfg->io_buffer_size;
    *needed_memory += mm_wby_conn_align;
    srv->memory_size = *needed_memory;
}

MM_WBY_API int
mm_wby_start(struct mm_wby_server *server, void *memory)
{
    mm_wby_size i;
    mm_wby_socket sock;
    int on = 1;
    mm_wby_byte *buffer = (mm_wby_byte*)memory;
    struct sockaddr_in bind_addr;
    MM_WBY_STORAGE const mm_wby_size mm_wby_conn_align = MM_WBY_ALIGNOF(struct mm_wby_connection);

    MM_WBY_ASSERT(server);
    MM_WBY_ASSERT(memory);
    memset(buffer, 0, server->memory_size);

    /* setup sever memory */
    server->socket = (mm_wby_ptr)MM_WBY_INVALID_SOCKET;
    server->con = (struct mm_wby_connection*)MM_WBY_ALIGN_PTR(buffer, mm_wby_conn_align);
    buffer += ((mm_wby_byte*)server->con - buffer);
    buffer += server->config.connection_max * sizeof(struct mm_wby_connection);

    for (i = 0; i < server->config.connection_max; ++i) {
        server->con[i].log = server->config.log;
        server->con[i].header_buf.data = buffer;
        buffer += server->config.request_buffer_size;
        server->con[i].io_buf.data = buffer;
        server->con[i].request_buffer_size = server->config.request_buffer_size;
        server->con[i].io_buffer_size = server->config.io_buffer_size;
        buffer += server->config.io_buffer_size;
    }
    MM_WBY_ASSERT((mm_wby_size)(buffer - (mm_wby_byte*)memory) <= server->memory_size);

    /* server socket setup */
    sock = (mm_wby_ptr)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    mm_wby_dbg(server->config.log, "Server socket = %d", (int)sock);
    if (!mm_wby_socket_is_valid(sock)) {
        mm_wby_dbg(server->config.log, "failed to initialized server socket: %d", mm_wby_socket_error());
        goto error;
    }
    setsockopt(MM_WBY_SOCK(sock), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    #ifdef __APPLE__ /* Don't generate SIGPIPE when writing to dead socket, we check all writes. */
    signal(SIGPIPE, SIG_IGN);
    #endif
    if (mm_wby_socket_set_blocking(sock, 0) != MM_WBY_OK) goto error;

    /* bind server socket */
    mm_wby_dbg(server->config.log, "binding to %s:%d", server->config.address, server->config.port);
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons((unsigned short)server->config.port);
    bind_addr.sin_addr.s_addr = inet_addr(server->config.address);
    if (bind(sock, (struct sockaddr*) &bind_addr, sizeof(bind_addr)) != MM_WBY_OK) {
        mm_wby_dbg(server->config.log, "bind() failed: %d", mm_wby_socket_error());
        mm_wby_dbg(server->config.log, "bind() failed: %s", strerror(mm_wby_socket_error()));
        goto error;
    }

    /* set server socket to listening */
    if (listen(sock, SOMAXCONN) != MM_WBY_OK) {
        mm_wby_dbg(server->config.log, "listen() failed: %d", mm_wby_socket_error());
        mm_wby_socket_close(MM_WBY_SOCK(sock));
        goto error;
    }
    server->socket = (mm_wby_ptr)sock;
    mm_wby_dbg(server->config.log, "server initialized: %s", strerror(errno));
    return 0;

error:
    if (mm_wby_socket_is_valid(MM_WBY_SOCK(sock)))
        mm_wby_socket_close(MM_WBY_SOCK(sock));
    return -1;
}

MM_WBY_API void
mm_wby_stop(struct mm_wby_server *srv)
{
    mm_wby_size i;
    mm_wby_socket_close(MM_WBY_SOCK(srv->socket));
    for (i = 0; i < srv->con_count; ++i)
        mm_wby_socket_close(MM_WBY_SOCK(srv->con[i].socket));
}

MM_WBY_INTERN int
mm_wby_on_incoming(struct mm_wby_server *srv)
{
    mm_wby_size connection_index;
    char MM_WBY_ALIGN(8) client_addr[64];
    struct mm_wby_connection* connection;
    mm_wby_socklen client_addr_len = sizeof(client_addr);
    mm_wby_socket fd;

    /* Make sure we have space for a new connection */
    connection_index = srv->con_count;
    if (connection_index == srv->config.connection_max) {
        mm_wby_dbg(srv->config.log, "out of connection slots");
        return 1;
    }

    /* Accept the incoming connection. */
    fd = accept(MM_WBY_SOCK(srv->socket), (struct sockaddr*)&client_addr[0], &client_addr_len);
    if (!mm_wby_socket_is_valid(fd)) {
        int err = mm_wby_socket_error();
        if (!mm_wby_socket_is_blocking_error(err))
            mm_wby_dbg(srv->config.log, "accept() failed: %d", err);
        return 1;
    }

    connection = &srv->con[connection_index];
    mm_wby_connection_reset(connection, srv->config.request_buffer_size, srv->config.io_buffer_size);
    connection->flags = MM_WBY_CON_FLAG_FRESH_CONNECTION;
    srv->con_count = connection_index + 1;

    /* Configure socket */
    if (mm_wby_socket_config_incoming(fd) != MM_WBY_OK) {
        mm_wby_socket_close(fd);
        return 1;
    }

    /* OK, keep this connection */
    mm_wby_dbg(srv->config.log, "tagging connection %d as alive", connection_index);
    connection->flags |= MM_WBY_CON_FLAG_ALIVE;
    connection->socket = (mm_wby_ptr)fd;
    return 0;
}

MM_WBY_INTERN void
mm_wby_update_connection(struct mm_wby_server *srv, struct mm_wby_connection* connection)
{
    /* This is no longer a fresh connection. Only read from it when select() says
    * so in the future. */
    connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_FRESH_CONNECTION;
    for (;;)
    {
        switch (connection->state) {
        default: break;
        case MM_WBY_CON_STATE_REQUEST: {
            const char *expect_header;
            int request_size;
            int result = mm_wby_socket_recv(MM_WBY_SOCK(connection->socket),
                &connection->header_buf, srv->config.log);
            if (MM_WBY_FILL_ERROR == result) {
                connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                return;
            }

            /* Scan to see if the buffer has a complete HTTP request header package. */
            request_size = wb_peek_request_size(connection->header_buf.data,
                (int)connection->header_buf.used);
            if (request_size < 0) {
                /* Nothing yet. */
                if (connection->header_buf.max == connection->header_buf.used) {
                    mm_wby_dbg(srv->config.log, "giving up as buffer is full");
                    /* Give up, we can't fit the request in our buffer. */
                    connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                }
                return;
            }
            mm_wby_dbg(srv->config.log, "peek request size: %d", request_size);


            /* Set up request data. */
            if (mm_wby_connection_setup_request(connection, request_size) != MM_WBY_OK) {
                mm_wby_dbg(srv->config.log, "failed to set up request");
                connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                return;
            }

            /* Remember how much of the remaining buffer is body data. */
            connection->header_body_left = (int)connection->header_buf.used - request_size;
            /* If the client expects a 100 Continue, send one now. */
            if (NULL != (expect_header = mm_wby_find_header(&connection->public_data, "Expect"))) {
                if (!strcasecmp(expect_header, "100-continue")) {
                    mm_wby_dbg(srv->config.log, "connection expects a 100 Continue header.. making him happy");
                    connection->continue_data_left = (int)mm_wby_continue_header_len;
                    connection->state = MM_WBY_CON_STATE_SEND_CONTINUE;
                } else {
                    mm_wby_dbg(srv->config.log, "unrecognized Expected header %s", expect_header);
                    connection->state = MM_WBY_CON_STATE_SERVE;
                }
            } else connection->state = MM_WBY_CON_STATE_SERVE;
        } break; /* MM_WBY_REQUEST */

        case MM_WBY_CON_STATE_SEND_CONTINUE: {
            int left = connection->continue_data_left;
            int offset = (int)mm_wby_continue_header_len - left;
            long written = 0;

            written = send(MM_WBY_SOCK(connection->socket), mm_wby_continue_header + offset, (mm_wby_size)left, 0);
            mm_wby_dbg(srv->config.log, "continue write: %d bytes", written);
            if (written < 0) {
                mm_wby_dbg(srv->config.log, "failed to write 100-continue header");
                connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                return;
            }
            left -= (int)written;
            connection->continue_data_left = left;
            if (left == 0)
                connection->state = MM_WBY_CON_STATE_SERVE;
        } break; /* MM_WBY_SEND_cONTINUE */

        case MM_WBY_CON_STATE_SERVE: {
            /* Clear I/O buffer for output */
            connection->io_buf.used = 0;
            /* Switch socket to blocking mode. */
            if (mm_wby_connection_set_blocking(connection) != MM_WBY_OK)
                return;

            /* Figure out if this is a request to upgrade to WebSockets */
            if (mm_wby_con_is_websocket_request(&connection->public_data)) {
                mm_wby_dbg(srv->config.log, "received a websocket upgrade request");
                if (!srv->config.ws_connect ||
                    srv->config.ws_connect(&connection->public_data, srv->config.userdata) != MM_WBY_OK)
                {
                    mm_wby_dbg(srv->config.log, "user callback failed connection attempt");
                    mm_wby_response_begin(&connection->public_data, 400, -1,
                        mm_wby_plain_text_headers, MM_WBY_LEN(mm_wby_plain_text_headers));
                    mm_wby_printf(&connection->public_data, "WebSockets not supported at %s\r\n",
                        connection->public_data.request.uri);
                    mm_wby_response_end(&connection->public_data);
                } else {
                    /* OK, let's try to upgrade the connection to WebSockets */
                    if (mm_wby_connection_send_websocket_upgrade(connection) != MM_WBY_OK) {
                        mm_wby_dbg(srv->config.log, "websocket upgrade failed");
                        mm_wby_response_begin(&connection->public_data, 400, -1,
                            mm_wby_plain_text_headers, MM_WBY_LEN(mm_wby_plain_text_headers));
                        mm_wby_printf(&connection->public_data, "WebSockets couldn't not be enabled\r\n");
                        mm_wby_response_end(&connection->public_data);
                    } else {
                        /* OK, we're now a websocket */
                        connection->flags |= MM_WBY_CON_FLAG_WEBSOCKET;
                        mm_wby_dbg(srv->config.log, "connection %d upgraded to websocket",
                            (int)(connection - srv->con));
                        srv->config.ws_connected(&connection->public_data, srv->config.userdata);
                    }
                }
            } else if (srv->config.dispatch(&connection->public_data, srv->config.userdata) != 0) {
                static const struct mm_wby_header headers[] = {{ "Content-Type", "text/plain" }};
                mm_wby_response_begin(&connection->public_data, 404, -1, headers, MM_WBY_LEN(headers));
                mm_wby_printf(&connection->public_data, "No handler for %s\r\n",
                    connection->public_data.request.uri);
                mm_wby_response_end(&connection->public_data);
            }

            /* Back to non-blocking mode, can make the socket die. */
            mm_wby_connection_set_nonblocking(connection);
            /* Ready for another request, unless we should close the connection. */
            if (connection->flags & MM_WBY_CON_FLAG_ALIVE) {
                if (connection->flags & MM_WBY_CON_FLAG_CLOSE_AFTER_RESPONSE) {
                    connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                    return;
                } else {
                    /* Reset connection for next request. */
                    mm_wby_connection_reset(connection, srv->config.request_buffer_size,
                        srv->config.io_buffer_size);
                    if (!(connection->flags & MM_WBY_CON_FLAG_WEBSOCKET)) {
                        /* Loop back to request state */
                        connection->state = MM_WBY_CON_STATE_REQUEST;
                    } else {
                        /* Clear I/O buffer for input */
                        connection->io_buf.used = 0;
                        /* Go to the web socket serving state */
                        connection->state = MM_WBY_CON_STATE_WEBSOCKET;
                    }
                }
            }
        } break; /* MM_WBY_SERVE */

        case MM_WBY_CON_STATE_WEBSOCKET: {
            /* In this state, we're trying to read a websocket frame into the I/O
            * buffer. Once we have enough data, we call the websocket frame
            * callback and let the client read the data through WebbyRead. */
            if (MM_WBY_FILL_ERROR == mm_wby_socket_recv(MM_WBY_SOCK(connection->socket),
                &connection->io_buf, srv->config.log)) {
                /* Give up on this connection */
                connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                return;
            }

            if (mm_wby_scan_websocket_frame(&connection->ws_frame, &connection->io_buf) != MM_WBY_OK)
                return;

            connection->body_bytes_read = 0;
            connection->io_data_left = (int)connection->io_buf.used - connection->ws_frame.header_size;
            mm_wby_dbg(srv->config.log, "%d bytes of incoming websocket data buffered",
                (int)connection->io_data_left);

            /* Switch socket to blocking mode */
            if (mm_wby_connection_set_blocking(connection) != MM_WBY_OK)
                return;

            switch (connection->ws_frame.opcode)
            {
            case MM_WBY_WSOP_CLOSE:
                mm_wby_dbg(srv->config.log, "received websocket close request");
                connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                return;

              case MM_WBY_WSOP_PING:
                mm_wby_dbg(srv->config.log, "received websocket ping request");
                if (mm_wby_socket_send(MM_WBY_SOCK(connection->socket), mm_wby_websocket_pong,
                    sizeof mm_wby_websocket_pong)){
                    connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                    return;
                }
                break;

              default:
                /* Dispatch frame to user handler. */
                if (srv->config.ws_frame(&connection->public_data,
                    &connection->ws_frame, srv->config.userdata) != MM_WBY_OK) {
                  connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                  return;
                }
            }

            /* Discard any data the client didn't read to retain the socket state. */
            if (connection->body_bytes_read < connection->ws_frame.payload_length) {
                int size = connection->ws_frame.payload_length - connection->body_bytes_read;
                if (mm_wby_con_discard_incoming_data(&connection->public_data, size) != MM_WBY_OK) {
                    connection->flags &= (unsigned short)~MM_WBY_CON_FLAG_ALIVE;
                    return;
                }
            }

            /* Back to non-blocking mode */
            if (mm_wby_connection_set_nonblocking(connection) != MM_WBY_OK)
                return;

            mm_wby_connection_reset(connection, srv->config.request_buffer_size, srv->config.io_buffer_size);
            connection->state = MM_WBY_CON_STATE_WEBSOCKET;
        } break; /* MM_WBY_WEBSOCKET */
        } /* switch */
    } /* for */
}

MM_WBY_API void
mm_wby_update(struct mm_wby_server *srv)
{
    int err;
    mm_wby_size i, count;
    mm_wby_socket max_socket;
    fd_set read_fds, write_fds, except_fds;
    struct timeval timeout;

    /* Build set of sockets to check for events */
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    max_socket = 0;

    /* Only accept incoming connections if we have space */
    if (srv->con_count < srv->config.connection_max) {
        FD_SET(srv->socket, &read_fds);
        FD_SET(srv->socket, &except_fds);
        max_socket = MM_WBY_SOCK(srv->socket);
    }

    for (i = 0, count = srv->con_count; i < count; ++i) {
        mm_wby_socket socket = MM_WBY_SOCK(srv->con[i].socket);
        FD_SET(socket, &read_fds);
        FD_SET(socket, &except_fds);
        if (srv->con[i].state == MM_WBY_CON_STATE_SEND_CONTINUE)
            FD_SET(socket, &write_fds);
        if (socket > max_socket)
            max_socket = socket;
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 5;
    err = select((int)(max_socket + 1), &read_fds, &write_fds, &except_fds, &timeout);
    if (err < 0) {
        mm_wby_dbg(srv->config.log, "failed to select");
        return;
    }

    /* Handle incoming connections */
    if (FD_ISSET(MM_WBY_SOCK(srv->socket), &read_fds)) {
        do {
            mm_wby_dbg(srv->config.log, "awake on incoming");
            err = mm_wby_on_incoming(srv);
        } while (err == 0);
    }

    /* Handle incoming connection data */
    for (i = 0, count = srv->con_count; i < count; ++i) {
        struct mm_wby_connection *conn = &srv->con[i];
        if (FD_ISSET(MM_WBY_SOCK(conn->socket), &read_fds) ||
            FD_ISSET(MM_WBY_SOCK(conn->socket), &write_fds) ||
            conn->flags & MM_WBY_CON_FLAG_FRESH_CONNECTION)
        {
            mm_wby_dbg(srv->config.log, "reading from connection %d", i);
            mm_wby_update_connection(srv, conn);
        }
    }

    /* Close stale connections & compact connection array. */
    for (i = 0; i < srv->con_count; ) {
        struct mm_wby_connection *connection = &srv->con[i];
        if (!(connection->flags & MM_WBY_CON_FLAG_ALIVE)) {
            mm_wby_size remain;
            mm_wby_dbg(srv->config.log, "closing connection %d (%08x)", i, connection->flags);
            if (connection->flags & MM_WBY_CON_FLAG_WEBSOCKET)
                srv->config.ws_closed(&connection->public_data, srv->config.userdata);
            remain = srv->con_count - (mm_wby_size)i - 1;
            mm_wby_connection_close(connection);
            memmove(&srv->con[i], &srv->con[i + 1], remain*sizeof(srv->con[i]));
            --srv->con_count;
        } else ++i;
    }
}

#endif /* MM_WBY_IMPLEMENTATION */
