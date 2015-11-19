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
    MMW_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    MMW_STATIC
        The generated implementation will stay private inside implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    MMW_ASSERT
    MMW_USE_ASSERT
        If you define MMW_USE_ASSERT without defining MM_ASSERT mm_web.h
        will use assert.h and asssert(). Otherwise it will use your assert
        method. If you do not define MMW_USE_ASSERT no additional checks
        will be added. This is the only C standard library function used
        by mm_web.

    MMW_UINT_PTR
        If your compiler is C99 you do not need to define this.
        Otherwise, mm_web will try default assignments for them
        and validate them at compile time. If they are incorrect, you will
        get compile errors and will need to define them yourself.

        You can define this to 'size_t' if you use the standard library,
        otherwise it needs to be able to hold the maximum addressable memory
        space. If you do not define this it will default to unsigned long.


LICENSE: (BSD)
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

CONTRIBUTORS:
    Andreas Fredriksson (implementation)
    Micha Mettke (single header conversion)

USAGE:
    Request handling
        When you configure the server, you give it a function pointer to your
        dispatcher. The dispatcher is called by wby when a request has been fully
        read into memory and is ready for processing. The socket the request came in on
        has then been switched to blocking mode, and you're free to read any request
        data using `mmw_read()` (if present, check `content_length`) and then write
        your response.
        There are two ways to generate a response; explicit size or chunked.

    When you know the size of the data
        When you know in advance how big the response is going to be, you should pass
        that size in bytes to `mmw_response_begin()` (it will be sent as the
        Content-Length header). You then call `mmw_write()` to push that data out, and
        finally `mmw_response_end()` to finalize the response and prepare the socket
        for a new request.

    When the response size is dynamic
        Sometimes you want to generate an arbitrary amount of text in the response, and
        you don't know how much that will be. Rather than buffering everything in RAM,
        you can use chunked encoding. First call `mmw_response_begin()` as normal, but
        pass it -1 for the content length. This triggers sending the
        `Transfer-Encoding: chunked` header. You then call `mmw_write()` as desired
        until the response is complete. When you're done, call `mmw_response_end()` to finish up.

EXAMPLES:
    for a actual working example please look inside tests/mmw_test.c */
#if 0
/* request and websocket handling callback */
static int dispatch(struct mmw_con *connection, void *pArg);
static int websocket_connect(struct mmw_con *conn, void *pArg);
static void websocket_connected(struct mmw_con *con, void *pArg);
static int websocket_frame(struct mmw_con *conn, const struct mmw_frame *frame, void *pArg);
static void websocket_closed(struct mmw_con *connection, void *pArg);

int main(int argc, const char * argv[])
{
    /* setup config */
    struct mmw_config config;
    memset(config, 0, sizeof(config));
    config.address = "127.0.0.1";
    config.port = 8888;
    config.dispatch = dispatch;
    config.connection_max = 8;
    config.request_buffer_size = 2048;
    config.io_buffer_size = 8192;
    config.dispatch = dispatch;
    config.ws_connect = websocket_connect;
    config.ws_connected = websocket_connected;
    config.ws_frame = websocket_frame;
    config.ws_closed = websocket_closed;

    /* compute and allocate needed memory and start server */
    struct mmw_server server;
    size_t needed_memory;
    mmw_server_init(&server, &config, &needed_memory);
    void *memory = calloc(needed_memory, 1);
    mmw_server_start(&server, memory);
    while (1) {
        mmw_server_update(&server);

    }
    mmw_server_stop(&server);
    free(memory);
}
#endif
 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MMW_H_
#define MMW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MMW_STATIC
#define MMW_API static
#else
#define MMW_API extern
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#include <stdint.h>
#ifndef MMW_UINT_PTR
#define MMW_UINT_PTR uintptr_t
#endif
#else
#ifndef MMW_UINT_PTR
#define MMW_UINT_PTR unsigned long
#endif
#endif
typedef unsigned char mmw_byte;
typedef MMW_UINT_PTR mmw_size;
typedef MMW_UINT_PTR mmw_ptr;

#define MMW_OK (0)
#define MMW_FLAG(x) (1 << (x))

#ifndef MMW_MAX_HEADERS
#define MMW_MAX_HEADERS 64
#endif

struct mmw_header {
    const char *name;
    const char *value;
};

/* A HTTP request. */
struct mmw_request {
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
    struct mmw_header headers[MMW_MAX_HEADERS];
    /* Request headers */
};

/* Connection state, as published to the serving callback. */
struct mmw_con {
    struct mmw_request request;
    /* The request being served. Read-only. */
    void *user_data;
    /* User data. Read-write. wby doesn't care about this. */
};

struct mmw_frame {
    mmw_byte flags;
    mmw_byte opcode;
    mmw_byte header_size;
    mmw_byte padding_;
    mmw_byte mask_key[4];
    int payload_length;
};

enum mmw_websock_flags {
    MMW_WSF_FIN    = MMW_FLAG(0),
    MMW_WSF_MASKED = MMW_FLAG(1)
};

enum mmw_websock_operation {
    MMW_WSOP_CONTINUATION    = 0,
    MMW_WSOP_TEXT_FRAME      = 1,
    MMW_WSOP_BINARY_FRAME    = 2,
    MMW_WSOP_CLOSE           = 8,
    MMW_WSOP_PING            = 9,
    MMW_WSOP_PONG            = 10
};

/* Configuration data required for starting a server. */
typedef void(*mmw_log_f)(const char *msg);
struct mmw_config {
    void *userdata;
    /* userdata which will be passed */
    const char *address;
    /* The bind address. Must be a textual IP address. */
    unsigned short port;
    /* The port to listen to. */
    unsigned int connection_max;
    /* Maximum number of simultaneous connections. */
    mmw_size request_buffer_size;
    /* The size of the request buffer. This must be big enough to contain all
    * headers and the request line sent by the client. 2-4k is a good size for
    * this buffer. */
    mmw_size io_buffer_size;
    /* The size of the I/O buffer, used when writing the reponse. 4k is a good
    * choice for this buffer.*/
    mmw_log_f log;
    /* Optional callback function that receives debug log text (without newlines). */
    int(*dispatch)(struct mmw_con *con, void *userdata);
    /* Request dispatcher function. This function is called when the request
    * structure is ready.
    * If you decide to handle the request, call mmw_response_begin(),
    * mmw_write() and mmw_response_end() and then return 0. Otherwise, return a
    * non-zero value to have Webby send back a 404 response. */
    int(*ws_connect)(struct mmw_con*, void *userdata);
    /*WebSocket connection dispatcher. Called when an incoming request wants to
    * update to a WebSocket connection.
    * Return 0 to allow the connection.
    * Return 1 to ignore the connection.*/
    void (*ws_connected)(struct mmw_con*, void *userdata);
    /* Called when a WebSocket connection has been established.*/
    void (*ws_closed)(struct mmw_con*, void *userdata);
    /*Called when a WebSocket connection has been closed.*/
    int (*ws_frame)(struct mmw_con*, const struct mmw_frame *frame, void *userdata);
    /*Called when a WebSocket data frame is incoming.
    * Call mmw_read() to read the payload data.
    * Return non-zero to close the connection.*/
};

struct mmw_connection;
struct mmw_server {
    struct mmw_config config;
    /* server configuration */
    mmw_size memory_size;
    /* minimum required memory */
    mmw_ptr socket;
    /* server socket */
    mmw_size con_count;
    /* number of active connection */
    struct mmw_connection *con;
    /* connections */
};

MMW_API void mmw_server_init(struct mmw_server*, const struct mmw_config*,
                            mmw_size *needed_memory);
/*  this function clears the server and calculates the needed memory to run
    Input:
    -   filled server configuration data to calculate the needed memory
    Output:
    -   needed memory for the server to run
*/
MMW_API int mmw_server_start(struct mmw_server*, void *memory);
/*  this function starts running the server in the specificed memory space. Size
 *  must be at least big enough as determined in the mmw_server_init().
    Input:
    -   allocated memory space to create the server into
*/
MMW_API void mmw_server_update(struct mmw_server*);
/* updates the server by being called frequenctly (at least once a frame) */
MMW_API void mmw_server_stop(struct mmw_server*);
/* stops and shutdown the server */
MMW_API int mmw_response_begin(struct mmw_con*, int status_code, int content_length,
                                    const struct mmw_header headers[], int header_count);
/*  this function begins a response
    Input:
    -   HTTP status code to send. (Normally 200).
    -   size in bytes you intend to write, or -1 for chunked encoding
    -   array of HTTP headers to transmit (can be NULL of header_count == 0)
    -   number of headers in the array
    Output:
    -   returns 0 on success, non-zero on error.
*/
MMW_API void mmw_response_end(struct mmw_con*);
/*  this function finishes a response. When you're done wirting the response
 *  body, call this function. this makes sure chunked encoding is terminated
 *  correctly and that the connection is setup for reuse. */
MMW_API int mmw_read(struct mmw_con*, void *ptr, mmw_size len);
/*  this function reads data from the request body. Only read what the client
 *  has priovided (via content_length) parameter, or you will end up blocking
 *  forever.
    Input:
    - pointer to a memory block that will be filled
    - size of the memory block to fill
*/
MMW_API int mmw_write(struct mmw_con*, const void *ptr, mmw_size len);
/*  this function writes a response data to the connection. If you're not using
 *  chunked encoding, be careful not to send more than the specified content
 *  length. You can call this function multiple times as long as the total
 *  number of bytes matches up with the content length.
    Input:
    - pointer to a memory block that will be send
    - size of the memory block to send
*/
MMW_API int mmw_frame_begin(struct mmw_con*, int opcode);
/*  this function begins an outgoing websocket frame */
MMW_API int mmw_frame_end(struct mmw_con*);
/*  this function finishes an outgoing websocket frame */
MMW_API int mmw_find_query_var(const char *buf, const char *name, char *dst, mmw_size dst_len);
/*  this function is a helper function to lookup a query parameter given a URL
 *  encoded string. Returns the size of the returned data, or -1 if the query
 *  var wasn't found. */
MMW_API const char* mmw_find_header(struct mmw_con*, const char *name);
/*  this convenience function to find a header in a request. Returns the value
 *  of the specified header, or NULL if its was not present. */

#ifdef __cplusplus
}
#endif
#endif /* MMW_H_ */
/* ===============================================================
 *
 *                      IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MMW_IMPLEMENTATION

typedef int mmw__check_ptr_size[(sizeof(void*) == sizeof(MMW_UINT_PTR)) ? 1 : -1];
#define MMW_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define MMW_UNUSED(a) ((void)(a))

#ifdef MMW_USE_ASSERT
#ifndef MMW_ASSERT
#include <assert.h>
#define MMW_ASSERT(expr) assert(expr)
#endif
#else
#define MMW_ASSERT(expr)
#endif

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define MMW_SOCK(s) ((mmw_socket)(s))
#define MMW_INTERN static
#define MMW_GLOBAL static
#define MMW_STORAGE static

/* ===============================================================
 *                          UTIL
 * ===============================================================*/
struct mmw_buffer {
    mmw_size used;
    /* current buffer size */
    mmw_size max;
    /* buffer capacity */
    mmw_byte *data;
    /* pointer inside a global buffer */
};

MMW_INTERN void
mmw_dbg(mmw_log_f log, const char *fmt, ...)
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

MMW_INTERN int
wb_peek_request_size(const mmw_byte *buf, int len)
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

MMW_INTERN char*
mmw_skipws(char *p)
{
    for (;;) {
        char ch = *p;
        if (' ' == ch || '\t' == ch) ++p;
        else break;
    }
    return p;
}

#define MMW_TOK_SKIPWS MMW_FLAG(0)
MMW_INTERN int
mmw_tok_inplace(char *buf, const char* separator, char *tokens[], int max, int flags)
{
    char *b = buf;
    char *e = buf;
    int token_count = 0;
    int separator_len = (int)strlen(separator);
    while (token_count < max) {
        if (flags & MMW_TOK_SKIPWS)
            b = mmw_skipws(b);
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

MMW_INTERN mmw_size
mmw_make_websocket_header(mmw_byte buffer[10], mmw_byte opcode,
    int payload_len, int fin)
{
    buffer[0] = (mmw_byte)((fin ? 0x80 : 0x00) | opcode);
    if (payload_len < 126) {
        buffer[1] = (mmw_byte)(payload_len & 0x7f);
        return 2;
    } else if (payload_len < 65536) {
        buffer[1] = 126;
        buffer[2] = (mmw_byte)(payload_len >> 8);
        buffer[3] = (mmw_byte)payload_len;
        return 4;
    } else {
        buffer[1] = 127;
        /* Ignore high 32-bits. I didn't want to require 64-bit types and typdef hell in the API. */
        buffer[2] = buffer[3] = buffer[4] = buffer[5] = 0;
        buffer[6] = (mmw_byte)(payload_len >> 24);
        buffer[7] = (mmw_byte)(payload_len >> 16);
        buffer[8] = (mmw_byte)(payload_len >> 8);
        buffer[9] = (mmw_byte)payload_len;
        return 10;
    }
}

MMW_INTERN int
mmw_read_buffered_data(int *data_left, struct mmw_buffer* buffer,
    char **dest_ptr, mmw_size *dest_len)
{
    int offset, read_size;
    int left = *data_left;
    int len;
    if (left == 0)
        return 0;

    len = (int) *dest_len;
    offset = (int)buffer->used - left;
    read_size = (len > left) ? left : len;
    memcpy(*dest_ptr, buffer->data + offset, (mmw_size)read_size);

    (*dest_ptr) += read_size;
    (*dest_len) -= (mmw_size) read_size;
    (*data_left) -= read_size;
    return read_size;
}

/* ---------------------------------------------------------------
 *                          SOCKET
 * ---------------------------------------------------------------*/
#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET mmw_socket;
typedef int mmw_socklen;

#if defined(__GNUC__)
#define MMW_ALIGN(x) __attribute__((aligned(x)))
#else
#define MMW_ALIGN(x) __declspec(align(x))
#endif

#define MMW_INVALID_SOCKET INVALID_SOCKET
#define snprintf _snprintf

MMW_INTERN int
mmw_socket_error(void)
{
    return WSAGetLastError();
}

#if !defined(__GNUC__)
MMW_INTERN int
strcasecmp(const char *a, const char *b)
{
    return _stricmp(a, b);
}

MMW_INTERN int
strncasecmp(const char *a, const char *b, mmw_size len)
{
    return _strnicmp(a, b, len);
}
#endif

MMW_INTERN int
mmw_socket_set_blocking(mmw_socket socket, int blocking)
{
    u_long val = !blocking;
    return ioctlsocket(socket, FIONBIO, &val);
}

MMW_INTERN int
mmw_socket_is_valid(mmw_socket socket)
{
    return (INVALID_SOCKET != socket);
}

MMW_INTERN void
mmw_socket_close(mmw_socket socket)
{
    closesocket(socket);
}

MMW_INTERN int
mmw_socket_is_blocking_error(int error)
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

typedef int mmw_socket;
typedef socklen_t mmw_socklen;

#define MMW_ALIGN(x) __attribute__((aligned(x)))
#define MMW_INVALID_SOCKET (-1)

MMW_INTERN int
mmw_socket_error(void)
{
    return errno;
}

MMW_INTERN int
mmw_socket_is_valid(mmw_socket socket)
{
    return (socket > 0);
}

MMW_INTERN void
mmw_socket_close(mmw_socket socket)
{
    close(socket);
}

MMW_INTERN int
mmw_socket_is_blocking_error(int error)
{
    return (EAGAIN == error);
}

MMW_INTERN int
mmw_socket_set_blocking(mmw_socket socket, int blocking)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0) return flags;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(socket, F_SETFL, flags);
}
#endif

MMW_INTERN int
mmw_socket_config_incoming(mmw_socket socket)
{
    int off = 0;
    int err;
    if ((err = mmw_socket_set_blocking(socket, 0)) != MMW_OK) return err;
    setsockopt(socket, SOL_SOCKET, SO_LINGER, (const char*) &off, sizeof(int));
    return 0;
}

MMW_INTERN int
mmw_socket_send(mmw_socket socket, const mmw_byte *buffer, int size)
{
    while (size > 0) {
        long err = send(socket, (const char*)buffer, (mmw_size)size, 0);
        if (err <= 0) return 1;
        buffer += err;
        size -= (int)err;
    }
    return 0;
}

/* Read as much as possible without blocking while there is buffer space. */
enum {MMW_FILL_OK, MMW_FILL_ERROR, MMW_FILL_FULL};
MMW_INTERN int
mmw_socket_recv(mmw_socket socket, struct mmw_buffer *buf, mmw_log_f log)
{
    long err;
    int buf_left;
    for (;;) {
        buf_left = (int)buf->max - (int)buf->used;
        mmw_dbg(log, "buffer space left = %d", buf_left);
        if (buf_left == 0)
            return MMW_FILL_FULL;

        /* Read what we can into the current buffer space. */
        err = recv(socket, (char*)buf->data + buf->used, (mmw_size)buf_left, 0);
        if (err < 0) {
            int sock_err = mmw_socket_error();
            if (mmw_socket_is_blocking_error(sock_err)) {
                return MMW_FILL_OK;
            } else {
                /* Read error. Give up. */
                mmw_dbg(log, "read error %d - connection dead", sock_err);
                return MMW_FILL_ERROR;
            }
        } else if (err == 0) {
          /* The peer has closed the connection. */
          mmw_dbg(log, "peer has closed the connection");
          return MMW_FILL_ERROR;
        } else buf->used += (mmw_size)err;
    }
}

MMW_INTERN int
mmw_socket_flush(mmw_socket socket, struct mmw_buffer *buf)
{
    if (buf->used > 0){
        if (mmw_socket_send(socket, buf->data, (int)buf->used) != MMW_OK)
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
MMW_INTERN mmw_size
mmw_url_decode(const char *src, mmw_size src_len, char *dst, mmw_size dst_len,
    int is_form_url_encoded)
{
    int a, b;
    mmw_size i, j;
    #define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')
    for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++) {
        if (src[i] == '%' &&
            isxdigit(*(const mmw_byte*)(src + i + 1)) &&
            isxdigit(*(const mmw_byte*)(src + i + 2)))
        {
            a = tolower(*(const mmw_byte*)(src + i + 1));
            b = tolower(*(const mmw_byte*)(src + i + 2));
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
MMW_API int
mmw_find_query_var(const char *buf, const char *name, char *dst, mmw_size dst_len)
{
    const char *p, *e, *s;
    mmw_size name_len;
    int len;
    mmw_size buf_len = strlen(buf);

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
            s = (const char *) memchr(p, '&', (mmw_size)(e - p));
            if (s == NULL) s = e;
            MMW_ASSERT(s >= p);
            /* Decode variable into destination buffer */
            if ((mmw_size) (s - p) < dst_len)
                len = (int)mmw_url_decode(p, (mmw_size)(s - p), dst, dst_len, 1);
            break;
        }
    }
    return len;
}

/* ---------------------------------------------------------------
 *                          BASE64
 * ---------------------------------------------------------------*/
#define MMW_BASE64_QUADS_BEFORE_LINEBREAK 19

MMW_INTERN mmw_size
mmw_base64_bufsize(mmw_size input_size)
{
    mmw_size triplets = (input_size + 2) / 3;
    mmw_size base_size = 4 * triplets;
    mmw_size line_breaks = 2 * (triplets / MMW_BASE64_QUADS_BEFORE_LINEBREAK);
    mmw_size null_termination = 1;
    return base_size + line_breaks + null_termination;
}

MMW_INTERN int
mmw_base64_encode(char* output, mmw_size output_size,
    const mmw_byte *input, mmw_size input_size)
{
    mmw_size i = 0;
    int line_out = 0;
    MMW_STORAGE const char enc[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/=";
    if (output_size < mmw_base64_bufsize(input_size))
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

        if (++line_out == MMW_BASE64_QUADS_BEFORE_LINEBREAK) {
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
struct mmw_sha1 {
    unsigned int state[5];
    unsigned int msg_size[2];
    mmw_size buf_used;
    mmw_byte buffer[64];
};

MMW_INTERN unsigned int
mmw_sha1_rol(unsigned int value, unsigned int bits)
{
    return ((value) << bits) | (value >> (32 - bits));
}

MMW_INTERN void
mmw_sha1_hash_block(unsigned int state[5], const mmw_byte *block)
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
        w[i] = mmw_sha1_rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    /* Initialize working variables */
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];

    /* This is the core loop for each 20-word span. */
    #define SHA1_LOOP(start, end, func, constant) \
        for (i = (start); i < (end); ++i) { \
            unsigned int t = mmw_sha1_rol(a, 5) + (func) + e + (constant) + w[i]; \
            e = d; d = c; c = mmw_sha1_rol(b, 30); b = a; a = t;}

    SHA1_LOOP( 0, 20, ((b & c) ^ (~b & d)),           0x5a827999)
    SHA1_LOOP(20, 40, (b ^ c ^ d),                    0x6ed9eba1)
    SHA1_LOOP(40, 60, ((b & c) ^ (b & d) ^ (c & d)),  0x8f1bbcdc)
    SHA1_LOOP(60, 80, (b ^ c ^ d),                    0xca62c1d6)
    #undef SHA1_LOOP

    /* Update state */
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

MMW_INTERN void
mmw_sha1_init(struct mmw_sha1 *s)
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

MMW_INTERN void
mmw_sha1_update(struct mmw_sha1 *s, const void *data_, mmw_size size)
{
    const char *data = (const char*)data_;
    unsigned int size_lo;
    unsigned int size_lo_orig;
    mmw_size remain = size;

    while (remain > 0) {
        mmw_size buf_space = sizeof(s->buffer) - s->buf_used;
        mmw_size copy_size = (remain < buf_space) ? remain : buf_space;
        memcpy(s->buffer + s->buf_used, data, copy_size);
        s->buf_used += copy_size;
        data += copy_size;
        remain -= copy_size;

        if (s->buf_used == sizeof(s->buffer)) {
            mmw_sha1_hash_block(s->state, s->buffer);
            s->buf_used = 0;
        }
    }

    size_lo = size_lo_orig = s->msg_size[1];
    size_lo += (unsigned int)(size * 8);
    if (size_lo < size_lo_orig)
        s->msg_size[0] += 1;
    s->msg_size[1] = size_lo;
}

MMW_INTERN void
mmw_sha1_final(mmw_byte digest[20], struct mmw_sha1 *s)
{
    mmw_byte zero = 0x00;
    mmw_byte one_bit = 0x80;
    mmw_byte count_data[8];
    int i;

    /* Generate size data in bit endian format */
    for (i = 0; i < 8; ++i) {
        unsigned int word = s->msg_size[i >> 2];
        count_data[i] = (mmw_byte)(word >> ((3 - (i & 3)) * 8));
    }

    /* Set trailing one-bit */
    mmw_sha1_update(s, &one_bit, 1);
    /* Emit null padding to to make room for 64 bits of size info in the last 512 bit block */
    while (s->buf_used != 56)
        mmw_sha1_update(s, &zero, 1);

    /* Write size in bits as last 64-bits */
    mmw_sha1_update(s, count_data, 8);
    /* Make sure we actually finalized our last block */
    MMW_ASSERT(s->buf_used == 0);

    /* Generate digest */
    for (i = 0; i < 20; ++i) {
        unsigned int word = s->state[i >> 2];
        mmw_byte byte = (mmw_byte) ((word >> ((3 - (i & 3)) * 8)) & 0xff);
        digest[i] = byte;
    }
}

/* ---------------------------------------------------------------
 *                          CONNECTION
 * ---------------------------------------------------------------*/
#define MMW_WEBSOCKET_VERSION "13"
MMW_GLOBAL const char mmw_continue_header[] = "HTTP/1.1 100 Continue\r\n\r\n";
MMW_GLOBAL const mmw_size mmw_continue_header_len = sizeof(mmw_continue_header) - 1;
MMW_GLOBAL const char mmw_websocket_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
MMW_GLOBAL const mmw_size mmw_websocket_guid_len = sizeof(mmw_websocket_guid) - 1;
MMW_GLOBAL const mmw_byte mmw_websocket_pong[] = { 0x80, MMW_WSOP_PONG, 0x00 };
MMW_GLOBAL const struct mmw_header mmw_plain_text_headers[]={{"Content-Type","text/plain"}};

enum mmw_connection_flags {
    MMW_CON_FLAG_ALIVE                  = MMW_FLAG(0),
    MMW_CON_FLAG_FRESH_CONNECTION       = MMW_FLAG(1),
    MMW_CON_FLAG_CLOSE_AFTER_RESPONSE   = MMW_FLAG(2),
    MMW_CON_FLAG_CHUNKED_RESPONSE       = MMW_FLAG(3),
    MMW_CON_FLAG_WEBSOCKET              = MMW_FLAG(4)
};

enum mmw_connection_state {
    MMW_CON_STATE_REQUEST,
    MMW_CON_STATE_SEND_CONTINUE,
    MMW_CON_STATE_SERVE,
    MMW_CON_STATE_WEBSOCKET
};

struct mmw_connection {
    struct mmw_con public_data;
    unsigned short flags;
    unsigned short state;
    mmw_ptr socket;
    mmw_log_f log;
    mmw_size request_buffer_size;
    mmw_size io_buffer_size;
    struct mmw_buffer header_buf;
    struct mmw_buffer io_buf;
    int header_body_left;
    int io_data_left;
    int continue_data_left;
    int body_bytes_read;
    struct mmw_frame ws_frame;
    mmw_byte ws_opcode;
    mmw_size blocking_count;
};

MMW_INTERN int
mmw_connection_set_blocking(struct mmw_connection *conn)
{
    if (conn->blocking_count == 0) {
        if (mmw_socket_set_blocking(MMW_SOCK(conn->socket), 1) != MMW_OK) {
            mmw_dbg(conn->log, "failed to switch connection to blocking");
            conn->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
            return -1;
        }
    }
    ++conn->blocking_count;
    return 0;
}

MMW_INTERN int
mmw_connection_set_nonblocking(struct mmw_connection *conn)
{
    mmw_size count = conn->blocking_count;
    if (count == 1) {
        if (mmw_socket_set_blocking(MMW_SOCK(conn->socket), 0) != MMW_OK) {
            mmw_dbg(conn->log, "failed to switch connection to non-blocking");
            conn->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
            return -1;
        }
    }
    conn->blocking_count = count - 1;
    return 0;
}

MMW_INTERN void
mmw_connection_reset(struct mmw_connection *conn, mmw_size request_buffer_size,
    mmw_size io_buffer_size)
{
    conn->header_buf.used = 0;
    conn->header_buf.max = request_buffer_size;
    conn->io_buf.used = 0;
    conn->io_buf.max = io_buffer_size;
    conn->header_body_left = 0;
    conn->io_data_left = 0;
    conn->continue_data_left = 0;
    conn->body_bytes_read = 0;
    conn->state = MMW_CON_STATE_REQUEST;
    conn->public_data.user_data = NULL;
    conn->blocking_count = 0;
}

MMW_INTERN void
mmw_connection_close(struct mmw_connection* connection)
{
    if (MMW_SOCK(connection->socket) != MMW_INVALID_SOCKET) {
        mmw_socket_close(MMW_SOCK(connection->socket));
        connection->socket = (mmw_ptr)MMW_INVALID_SOCKET;
    }
    connection->flags = 0;
}

MMW_INTERN int
mmw_connection_setup_request(struct mmw_connection *connection, int request_size)
{
    char* lines[MMW_MAX_HEADERS + 2];
    int line_count;
    char* tok[16];
    char* query_params;
    int tok_count;

    int i;
    int header_count;
    char *buf = (char*) connection->header_buf.data;
    struct mmw_request *req = &connection->public_data.request;

    /* Null-terminate the request envelope by overwriting the last CRLF with 00LF */
    buf[request_size - 2] = '\0';
    /* Split header into lines */
    line_count = mmw_tok_inplace(buf, "\r\n", lines, MMW_LEN(lines), 0);
    header_count = line_count - 2;
    if (line_count < 1 || header_count > (int) MMW_LEN(req->headers))
        return 1;

    /* Parse request line */
    tok_count = mmw_tok_inplace(lines[0], " ", tok, MMW_LEN(tok), 0);
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
        mmw_size uri_len = strlen(req->uri);
        mmw_url_decode(req->uri, uri_len, (char*)req->uri, uri_len + 1, 1);
    }

    /* Parse headers */
    for (i = 0; i < header_count; ++i) {
        tok_count = mmw_tok_inplace(lines[i + 1], ":", tok, 2, MMW_TOK_SKIPWS);
        if (tok_count != 2) return 1;
        req->headers[i].name = tok[0];
        req->headers[i].value = tok[1];

        if (!strcasecmp("content-length", tok[0])) {
            req->content_length = (int)strtoul(tok[1], NULL, 10);
            mmw_dbg(connection->log, "request has body; content length is %d", req->content_length);
        } else if (!strcasecmp("transfer-encoding", tok[0])) {
            mmw_dbg(connection->log, "cowardly refusing to handle Transfer-Encoding: %s", tok[1]);
            return 1;
        }
    }
    req->header_count = header_count;
    return 0;
}

MMW_INTERN int
mmw_connection_send_websocket_upgrade(struct mmw_connection* connection)
{
    const char *hdr;
    struct mmw_sha1 sha;
    mmw_byte digest[20];
    char output_digest[64];
    struct mmw_header headers[3];
    struct mmw_con *conn = &connection->public_data;
    if ((hdr = mmw_find_header(conn, "Sec-WebSocket-Version")) == NULL) {
        mmw_dbg(connection->log, "Sec-WebSocket-Version header not present");
        return 1;
    }
    if (strcmp(hdr, MMW_WEBSOCKET_VERSION)) {
        mmw_dbg(connection->log,"WebSocket version %s not supported (we only do %s)",
                hdr, MMW_WEBSOCKET_VERSION);
        return 1;
    }
    if ((hdr = mmw_find_header(conn, "Sec-WebSocket-Key")) == NULL) {
        mmw_dbg(connection->log, "Sec-WebSocket-Key header not present");
        return 1;
    }
    /* Compute SHA1 hash of Sec-Websocket-Key + the websocket guid as required by
    * the RFC.
    *
    * This handshake is bullshit. It adds zero security. Just forces me to drag
    * in SHA1 and create a base64 encoder.
    */
    mmw_sha1_init(&sha);
    mmw_sha1_update(&sha, hdr, strlen(hdr));
    mmw_sha1_update(&sha, mmw_websocket_guid, mmw_websocket_guid_len);
    mmw_sha1_final(&digest[0], &sha);
    if (mmw_base64_encode(output_digest, sizeof output_digest, &digest[0], sizeof(digest)) != MMW_OK)
        return 1;

    headers[0].name  = "Upgrade";
    headers[0].value = "websocket";
    headers[1].name  = "Connection";
    headers[1].value = "Upgrade";
    headers[2].name  = "Sec-WebSocket-Accept";
    headers[2].value = output_digest;
    mmw_response_begin(&connection->public_data, 101, 0, headers, MMW_LEN(headers));
    mmw_response_end(&connection->public_data);
    return 0;
}

MMW_INTERN int
mmw_connection_push(struct mmw_connection *conn, const void *data_, int len)
{
    struct mmw_buffer *buf = &conn->io_buf;
    const mmw_byte* data = (const mmw_byte*)data_;
    if (conn->state != MMW_CON_STATE_SERVE) {
        mmw_dbg(conn->log, "attempt to write in non-serve state");
        return 1;
    }
    if (len == 0)
        return mmw_socket_flush(MMW_SOCK(conn->socket), buf);

    while (len > 0) {
        int buf_space = (int)buf->max - (int)buf->used;
        int copy_size = len < buf_space ? len : buf_space;
        memcpy(buf->data + buf->used, data, (mmw_size)copy_size);
        buf->used += (mmw_size)copy_size;

        data += copy_size;
        len -= copy_size;
        if (buf->used == buf->max) {
            if (mmw_socket_flush(MMW_SOCK(conn->socket), buf) != MMW_OK)
                return 1;
            if ((mmw_size)len >= buf->max)
                return mmw_socket_send(MMW_SOCK(conn->socket), data, len);
        }
    }
    return 0;
}

/* ---------------------------------------------------------------
 *                          CON/REQUEST
 * ---------------------------------------------------------------*/
MMW_INTERN int
mmw_con_discard_incoming_data(struct mmw_con* conn, int count)
{
    while (count > 0) {
        char buffer[1024];
        int read_size = (int)(((mmw_size)count > sizeof(buffer)) ?
            sizeof(buffer) : (mmw_size)count);
        if (mmw_read(conn, buffer, (mmw_size)read_size) != MMW_OK)
            return -1;
        count -= read_size;
    }
    return 0;
}

MMW_API const char*
mmw_find_header(struct mmw_con *conn, const char *name)
{
    int i, count;
    for (i = 0, count = conn->request.header_count; i < count; ++i) {
        if (!strcasecmp(conn->request.headers[i].name, name))
            return conn->request.headers[i].value;
    }
    return NULL;
}

MMW_INTERN int
mmw_con_is_websocket_request(struct mmw_con* conn)
{
    const char *hdr;
    if ((hdr = mmw_find_header(conn, "Connection")) == NULL) return 0;
    if (strcasecmp(hdr, "Upgrade")) return 0;
    if ((hdr = mmw_find_header(conn, "Upgrade")) == NULL) return 0;
    if (strcasecmp(hdr, "websocket")) return 0;
    return 1;
}

MMW_INTERN int
mmw_scan_websocket_frame(struct mmw_frame *frame, const struct mmw_buffer *buf)
{
    mmw_byte flags = 0;
    unsigned int len = 0;
    unsigned int opcode = 0;
    mmw_byte* data = buf->data;
    mmw_byte* data_max = data + buf->used;

    int i;
    int len_bytes = 0;
    int mask_bytes = 0;
    mmw_byte header0, header1;
    if (buf->used < 2)
        return -1;

    header0 = *data++;
    header1 = *data++;
    if (header0 & 0x80)
        flags |= MMW_WSF_FIN;
    if (header1 & 0x80) {
        flags |= MMW_WSF_MASKED;
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
    frame->header_size = (mmw_byte) (data - buf->data);
    frame->flags = flags;
    frame->opcode = (mmw_byte) opcode;
    frame->payload_length = (int)len;
    return 0;
}

MMW_API int
mmw_frame_begin(struct mmw_con *conn_pub, int opcode)
{
    struct mmw_connection *conn = (struct mmw_connection*)conn_pub;
    conn->ws_opcode = (mmw_byte) opcode;
    /* Switch socket to blocking mode */
    return mmw_connection_set_blocking(conn);
}

MMW_API int
mmw_frame_end(struct mmw_con *conn_pub)
{
    mmw_byte header[10];
    mmw_size header_size;
    struct mmw_connection *conn = (struct mmw_connection*) conn_pub;
    header_size = mmw_make_websocket_header(header, conn->ws_opcode, 0, 1);
    if (mmw_socket_send(MMW_SOCK(conn->socket), header, (int) header_size) != MMW_OK)
        conn->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
    /* Switch socket to non-blocking mode */
    return mmw_connection_set_nonblocking(conn);
}

MMW_API int
mmw_read(struct mmw_con *conn, void *ptr_, mmw_size len)
{
    struct mmw_connection* conn_prv = (struct mmw_connection*)conn;
    char *ptr = (char*) ptr_;
    int count;

    int start_pos = conn_prv->body_bytes_read;
    if (conn_prv->header_body_left > 0) {
        count = mmw_read_buffered_data(&conn_prv->header_body_left, &conn_prv->header_buf, &ptr, &len);
        conn_prv->body_bytes_read += count;
    }

    /* Read buffered websocket data */
    if (conn_prv->io_data_left > 0) {
        count = mmw_read_buffered_data(&conn_prv->io_data_left, &conn_prv->io_buf, &ptr, &len);
        conn_prv->body_bytes_read += count;
    }

    while (len > 0) {
        long err = recv(MMW_SOCK(conn_prv->socket), ptr, (mmw_size)len, 0);
        if (err < 0) {
            conn_prv->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
            return (int)err;
        }
        len -= (mmw_size)err;
        ptr += (mmw_size)err;
        conn_prv->body_bytes_read += (int)err;
    }

    if ((conn_prv->flags & MMW_CON_FLAG_WEBSOCKET) && (conn_prv->ws_frame.flags & MMW_WSF_MASKED)) {
        /* XOR outgoing data with websocket ofuscation key */
        int i, end_pos = conn_prv->body_bytes_read;
        const mmw_byte *mask = conn_prv->ws_frame.mask_key;
        ptr = (char*) ptr_; /* start over */
        for (i = start_pos; i < end_pos; ++i) {
            mmw_byte byte = (mmw_byte)*ptr;
            *ptr++ = (char)(byte ^ mask[i & 3]);
        }
    }
    return 0;
}

MMW_API int
mmw_write(struct mmw_con *conn, const void *ptr, mmw_size len)
{
    struct mmw_connection *conn_priv = (struct mmw_connection*) conn;
    if (conn_priv->flags & MMW_CON_FLAG_WEBSOCKET) {
        mmw_byte header[10];
        mmw_size header_size;
        header_size = mmw_make_websocket_header(header, conn_priv->ws_opcode, (int)len, 0);

        /* Overwrite opcode to be continuation packages from here on out */
        conn_priv->ws_opcode = MMW_WSOP_CONTINUATION;
        if (mmw_socket_send(MMW_SOCK(conn_priv->socket), header, (int)header_size) != MMW_OK) {
            conn_priv->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
            return -1;
        }
        if (mmw_socket_send(MMW_SOCK(conn_priv->socket),(const mmw_byte*)ptr, (int)len) != MMW_OK) {
            conn_priv->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
            return -1;
        }
        return 0;
    } else if (conn_priv->flags & MMW_CON_FLAG_CHUNKED_RESPONSE) {
        char chunk_header[128];
        int header_len = snprintf(chunk_header, sizeof chunk_header, "%x\r\n", (int) len);
        mmw_connection_push(conn_priv, chunk_header, header_len);
        mmw_connection_push(conn_priv, ptr, (int)len);
        return mmw_connection_push(conn_priv, "\r\n", 2);
    } else return mmw_connection_push(conn_priv, ptr, (int) len);
}

MMW_INTERN int
mmw_printf(struct mmw_con* conn, const char* fmt, ...)
{
    int len;
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof buffer, fmt, args);
    va_end(args);
    return mmw_write(conn, buffer, (mmw_size)len);
}

/* ---------------------------------------------------------------
 *                          RESPONSE
 * ---------------------------------------------------------------*/
#define MMW_STATUS_MAP(STATUS)\
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

MMW_GLOBAL const short mmw_status_nums[] = {
#define MMW_STATUS(id, name) id,
    MMW_STATUS_MAP(MMW_STATUS)
#undef MMW_STATUS
};
MMW_GLOBAL const char* mmw_status_text[] = {
#define MMW_STATUS(id, name) name,
    MMW_STATUS_MAP(MMW_STATUS)
#undef MMW_STATUS
};

MMW_INTERN const char*
mmw_response_status_text(int status_code)
{
    int i;
    for (i = 0; i < (int) MMW_LEN(mmw_status_nums); ++i) {
        if (mmw_status_nums[i] == status_code)
            return mmw_status_text[i];
    }
    return "Unknown";
}

MMW_API int
mmw_response_begin(struct mmw_con *conn_pub, int status_code, int content_length,
    const struct mmw_header *headers, int header_count)
{
    int i = 0;
    struct mmw_connection *conn = (struct mmw_connection *)conn_pub;
    if (conn->body_bytes_read < (int)conn->public_data.request.content_length) {
        int body_left = conn->public_data.request.content_length - (int)conn->body_bytes_read;
        if (mmw_con_discard_incoming_data(conn_pub, body_left) != MMW_OK) {
            conn->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
            return -1;
        }
    }

    mmw_printf(conn_pub, "HTTP/1.1 %d %s\r\n", status_code, mmw_response_status_text(status_code));
    if (content_length >= 0)
        mmw_printf(conn_pub, "Content-Length: %d\r\n", content_length);
    else mmw_printf(conn_pub, "Transfer-Encoding: chunked\r\n");
    mmw_printf(conn_pub, "Server: Webby\r\n");

    for (i = 0; i < header_count; ++i) {
        if (!strcasecmp(headers[i].name, "Connection")) {
            if (!strcasecmp(headers[i].value, "close"))
            conn->flags |= MMW_CON_FLAG_CLOSE_AFTER_RESPONSE;
        }
        mmw_printf(conn_pub, "%s: %s\r\n", headers[i].name, headers[i].value);
    }

    if (!(conn->flags & MMW_CON_FLAG_CLOSE_AFTER_RESPONSE)) {
        /* See if the client wants us to close the connection. */
        const char* connection_header = mmw_find_header(conn_pub, "Connection");
        if (connection_header && !strcasecmp("close", connection_header)) {
            conn->flags |= MMW_CON_FLAG_CLOSE_AFTER_RESPONSE;
            mmw_printf(conn_pub, "Connection: close\r\n");
        }
    }
    mmw_printf(conn_pub, "\r\n");
    if (content_length < 0)
        conn->flags |= MMW_CON_FLAG_CHUNKED_RESPONSE;
    return 0;
}

MMW_API void
mmw_response_end(struct mmw_con *conn)
{
    struct mmw_connection *conn_priv = (struct mmw_connection*) conn;
    if (conn_priv->flags & MMW_CON_FLAG_CHUNKED_RESPONSE) {
        /* Write final chunk */
        mmw_connection_push(conn_priv, "0\r\n\r\n", 5);
        conn_priv->flags &= (unsigned short)~MMW_CON_FLAG_CHUNKED_RESPONSE;
    }
    /* Flush buffers */
    mmw_connection_push(conn_priv, "", 0);
}

/* ---------------------------------------------------------------
 *                          SERVER
 * ---------------------------------------------------------------*/
/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MMW_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MMW_PTR_TO_UINT(x) ((mmw_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MMW_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MMW_PTR_TO_UINT(x) ((mmw_size)(((char*)x)-(char*)0))
#elif defined(MMW_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MMW_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MMW_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MMW_UINT_TO_PTR(x) ((void*)(x))
# define MMW_PTR_TO_UINT(x) ((mmw_size)(x))
#endif

/* simple pointer math */
#define MMW_PTR_ADD(t, p, i) ((t*)((void*)((mmw_byte*)(p) + (i))))
#define MMW_ALIGN_PTR(x, mask)\
    (MMW_UINT_TO_PTR((MMW_PTR_TO_UINT((mmw_byte*)(x) + (mask-1)) & ~(mask-1))))

/* pointer alignment  */
#ifdef __cplusplus
template<typename T> struct mmw_alignof;
template<typename T, int size_diff> struct mmw_helper{enum {value = size_diff};};
template<typename T> struct mmw_helper<T,0>{enum {value = mmw_alignof<T>::value};};
template<typename T> struct mmw_alignof{struct Big {T x; char c;}; enum {
    diff = sizeof(Big) - sizeof(T), value = mmw_helper<Big, diff>::value};};
#define MMW_ALIGNOF(t) (mmw_alignof<t>::value);
#else
#define MMW_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
#endif

MMW_API void
mmw_server_init(struct mmw_server *srv, const struct mmw_config *cfg, mmw_size *needed_memory)
{
    MMW_STORAGE const mmw_size mmw_conn_align = MMW_ALIGNOF(struct mmw_connection);
    MMW_ASSERT(srv);
    MMW_ASSERT(cfg);
    MMW_ASSERT(needed_memory);
    memset(srv, 0, sizeof(*srv));
    srv->config = *cfg;
    MMW_ASSERT(cfg->dispatch);

    *needed_memory = 0;
    *needed_memory += cfg->connection_max * sizeof(struct mmw_connection);
    *needed_memory += cfg->connection_max * cfg->request_buffer_size;
    *needed_memory += cfg->connection_max * cfg->io_buffer_size;
    *needed_memory += mmw_conn_align;
    srv->memory_size = *needed_memory;
}

MMW_API int
mmw_server_start(struct mmw_server *server, void *memory)
{
    mmw_size i;
    mmw_socket sock;
    int on = 1;
    mmw_byte *buffer = (mmw_byte*)memory;
    struct sockaddr_in bind_addr;
    MMW_STORAGE const mmw_size mmw_conn_align = MMW_ALIGNOF(struct mmw_connection);

    MMW_ASSERT(server);
    MMW_ASSERT(memory);
    memset(buffer, 0, server->memory_size);

    /* setup sever memory */
    server->socket = (mmw_ptr)MMW_INVALID_SOCKET;
    server->con = (struct mmw_connection*)MMW_ALIGN_PTR(buffer, mmw_conn_align);
    buffer += ((mmw_byte*)server->con - buffer);
    buffer += server->config.connection_max * sizeof(struct mmw_connection);

    for (i = 0; i < server->config.connection_max; ++i) {
        server->con[i].log = server->config.log;
        server->con[i].header_buf.data = buffer;
        buffer += server->config.request_buffer_size;
        server->con[i].io_buf.data = buffer;
        server->con[i].request_buffer_size = server->config.request_buffer_size;
        server->con[i].io_buffer_size = server->config.io_buffer_size;
        buffer += server->config.io_buffer_size;
    }
    MMW_ASSERT((mmw_size)(buffer - (mmw_byte*)memory) <= server->memory_size);

    /* server socket setup */
    sock = (mmw_ptr)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    mmw_dbg(server->config.log, "Server socket = %d", (int)sock);
    if (!mmw_socket_is_valid(sock)) {
        mmw_dbg(server->config.log, "failed to initialized server socket: %d", mmw_socket_error());
        goto error;
    }
    setsockopt(MMW_SOCK(sock), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    #ifdef __APPLE__ /* Don't generate SIGPIPE when writing to dead socket, we check all writes. */
    signal(SIGPIPE, SIG_IGN);
    #endif
    if (mmw_socket_set_blocking(sock, 0) != MMW_OK) goto error;

    /* bind server socket */
    mmw_dbg(server->config.log, "binding to %s:%d", server->config.address, server->config.port);
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons((unsigned short)server->config.port);
    bind_addr.sin_addr.s_addr = inet_addr(server->config.address);
    if (bind(sock, (struct sockaddr*) &bind_addr, sizeof(bind_addr)) != MMW_OK) {
        mmw_dbg(server->config.log, "bind() failed: %d", mmw_socket_error());
        mmw_dbg(server->config.log, "bind() failed: %s", strerror(mmw_socket_error()));
        goto error;
    }

    /* set server socket to listening */
    if (listen(sock, SOMAXCONN) != MMW_OK) {
        mmw_dbg(server->config.log, "listen() failed: %d", mmw_socket_error());
        mmw_socket_close(MMW_SOCK(sock));
        goto error;
    }
    server->socket = (mmw_ptr)sock;
    mmw_dbg(server->config.log, "server initialized: %s", strerror(errno));
    return 0;

error:
    if (mmw_socket_is_valid(MMW_SOCK(sock)))
        mmw_socket_close(MMW_SOCK(sock));
    return -1;
}

MMW_API void
mmw_server_stop(struct mmw_server *srv)
{
    mmw_size i;
    mmw_socket_close(MMW_SOCK(srv->socket));
    for (i = 0; i < srv->con_count; ++i)
        mmw_socket_close(MMW_SOCK(srv->con[i].socket));
}

MMW_INTERN int
mmw_server_on_incoming(struct mmw_server *srv)
{
    mmw_size connection_index;
    char MMW_ALIGN(8) client_addr[64];
    struct mmw_connection* connection;
    mmw_socklen client_addr_len = sizeof(client_addr);
    mmw_socket fd;

    /* Make sure we have space for a new connection */
    connection_index = srv->con_count;
    if (connection_index == srv->config.connection_max) {
        mmw_dbg(srv->config.log, "out of connection slots");
        return 1;
    }

    /* Accept the incoming connection. */
    fd = accept(MMW_SOCK(srv->socket), (struct sockaddr*)&client_addr[0], &client_addr_len);
    if (!mmw_socket_is_valid(fd)) {
        int err = mmw_socket_error();
        if (!mmw_socket_is_blocking_error(err))
            mmw_dbg(srv->config.log, "accept() failed: %d", err);
        return 1;
    }

    connection = &srv->con[connection_index];
    mmw_connection_reset(connection, srv->config.request_buffer_size, srv->config.io_buffer_size);
    connection->flags = MMW_CON_FLAG_FRESH_CONNECTION;
    srv->con_count = connection_index + 1;

    /* Configure socket */
    if (mmw_socket_config_incoming(fd) != MMW_OK) {
        mmw_socket_close(fd);
        return 1;
    }

    /* OK, keep this connection */
    mmw_dbg(srv->config.log, "tagging connection %d as alive", connection_index);
    connection->flags |= MMW_CON_FLAG_ALIVE;
    connection->socket = (mmw_ptr)fd;
    return 0;
}

MMW_INTERN void
mmw_server_update_connection(struct mmw_server *srv, struct mmw_connection* connection)
{
    /* This is no longer a fresh connection. Only read from it when select() says
    * so in the future. */
    connection->flags &= (unsigned short)~MMW_CON_FLAG_FRESH_CONNECTION;
    for (;;)
    {
        switch (connection->state) {
        default: break;
        case MMW_CON_STATE_REQUEST: {
            const char *expect_header;
            int request_size;
            int result = mmw_socket_recv(MMW_SOCK(connection->socket),
                &connection->header_buf, srv->config.log);
            if (MMW_FILL_ERROR == result) {
                connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                return;
            }

            /* Scan to see if the buffer has a complete HTTP request header package. */
            request_size = wb_peek_request_size(connection->header_buf.data,
                (int)connection->header_buf.used);
            if (request_size < 0) {
                /* Nothing yet. */
                if (connection->header_buf.max == connection->header_buf.used) {
                    mmw_dbg(srv->config.log, "giving up as buffer is full");
                    /* Give up, we can't fit the request in our buffer. */
                    connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                }
                return;
            }
            mmw_dbg(srv->config.log, "peek request size: %d", request_size);


            /* Set up request data. */
            if (mmw_connection_setup_request(connection, request_size) != MMW_OK) {
                mmw_dbg(srv->config.log, "failed to set up request");
                connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                return;
            }

            /* Remember how much of the remaining buffer is body data. */
            connection->header_body_left = (int)connection->header_buf.used - request_size;
            /* If the client expects a 100 Continue, send one now. */
            if (NULL != (expect_header = mmw_find_header(&connection->public_data, "Expect"))) {
                if (!strcasecmp(expect_header, "100-continue")) {
                    mmw_dbg(srv->config.log, "connection expects a 100 Continue header.. making him happy");
                    connection->continue_data_left = (int)mmw_continue_header_len;
                    connection->state = MMW_CON_STATE_SEND_CONTINUE;
                } else {
                    mmw_dbg(srv->config.log, "unrecognized Expected header %s", expect_header);
                    connection->state = MMW_CON_STATE_SERVE;
                }
            } else connection->state = MMW_CON_STATE_SERVE;
        } break; /* MMW_REQUEST */

        case MMW_CON_STATE_SEND_CONTINUE: {
            int left = connection->continue_data_left;
            int offset = (int)mmw_continue_header_len - left;
            long written = 0;

            written = send(MMW_SOCK(connection->socket), mmw_continue_header + offset, (mmw_size)left, 0);
            mmw_dbg(srv->config.log, "continue write: %d bytes", written);
            if (written < 0) {
                mmw_dbg(srv->config.log, "failed to write 100-continue header");
                connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                return;
            }
            left -= (int)written;
            connection->continue_data_left = left;
            if (left == 0)
                connection->state = MMW_CON_STATE_SERVE;
        } break; /* MMW_SEND_cONTINUE */

        case MMW_CON_STATE_SERVE: {
            /* Clear I/O buffer for output */
            connection->io_buf.used = 0;
            /* Switch socket to blocking mode. */
            if (mmw_connection_set_blocking(connection) != MMW_OK)
                return;

            /* Figure out if this is a request to upgrade to WebSockets */
            if (mmw_con_is_websocket_request(&connection->public_data)) {
                mmw_dbg(srv->config.log, "received a websocket upgrade request");
                if (!srv->config.ws_connect ||
                    srv->config.ws_connect(&connection->public_data, srv->config.userdata) != MMW_OK)
                {
                    mmw_dbg(srv->config.log, "user callback failed connection attempt");
                    mmw_response_begin(&connection->public_data, 400, -1,
                        mmw_plain_text_headers, MMW_LEN(mmw_plain_text_headers));
                    mmw_printf(&connection->public_data, "WebSockets not supported at %s\r\n",
                        connection->public_data.request.uri);
                    mmw_response_end(&connection->public_data);
                } else {
                    /* OK, let's try to upgrade the connection to WebSockets */
                    if (mmw_connection_send_websocket_upgrade(connection) != MMW_OK) {
                        mmw_dbg(srv->config.log, "websocket upgrade failed");
                        mmw_response_begin(&connection->public_data, 400, -1,
                            mmw_plain_text_headers, MMW_LEN(mmw_plain_text_headers));
                        mmw_printf(&connection->public_data, "WebSockets couldn't not be enabled\r\n");
                        mmw_response_end(&connection->public_data);
                    } else {
                        /* OK, we're now a websocket */
                        connection->flags |= MMW_CON_FLAG_WEBSOCKET;
                        mmw_dbg(srv->config.log, "connection %d upgraded to websocket",
                            (int)(connection - srv->con));
                        srv->config.ws_connected(&connection->public_data, srv->config.userdata);
                    }
                }
            } else if (srv->config.dispatch(&connection->public_data, srv->config.userdata) != 0) {
                static const struct mmw_header headers[] = {{ "Content-Type", "text/plain" }};
                mmw_response_begin(&connection->public_data, 404, -1, headers, MMW_LEN(headers));
                mmw_printf(&connection->public_data, "No handler for %s\r\n",
                    connection->public_data.request.uri);
                mmw_response_end(&connection->public_data);
            }

            /* Back to non-blocking mode, can make the socket die. */
            mmw_connection_set_nonblocking(connection);
            /* Ready for another request, unless we should close the connection. */
            if (connection->flags & MMW_CON_FLAG_ALIVE) {
                if (connection->flags & MMW_CON_FLAG_CLOSE_AFTER_RESPONSE) {
                    connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                    return;
                } else {
                    /* Reset connection for next request. */
                    mmw_connection_reset(connection, srv->config.request_buffer_size,
                        srv->config.io_buffer_size);
                    if (!(connection->flags & MMW_CON_FLAG_WEBSOCKET)) {
                        /* Loop back to request state */
                        connection->state = MMW_CON_STATE_REQUEST;
                    } else {
                        /* Clear I/O buffer for input */
                        connection->io_buf.used = 0;
                        /* Go to the web socket serving state */
                        connection->state = MMW_CON_STATE_WEBSOCKET;
                    }
                }
            }
        } break; /* MMW_SERVE */

        case MMW_CON_STATE_WEBSOCKET: {
            /* In this state, we're trying to read a websocket frame into the I/O
            * buffer. Once we have enough data, we call the websocket frame
            * callback and let the client read the data through WebbyRead. */
            if (MMW_FILL_ERROR == mmw_socket_recv(MMW_SOCK(connection->socket),
                &connection->io_buf, srv->config.log)) {
                /* Give up on this connection */
                connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                return;
            }

            if (mmw_scan_websocket_frame(&connection->ws_frame, &connection->io_buf) != MMW_OK)
                return;

            connection->body_bytes_read = 0;
            connection->io_data_left = (int)connection->io_buf.used - connection->ws_frame.header_size;
            mmw_dbg(srv->config.log, "%d bytes of incoming websocket data buffered",
                (int)connection->io_data_left);

            /* Switch socket to blocking mode */
            if (mmw_connection_set_blocking(connection) != MMW_OK)
                return;

            switch (connection->ws_frame.opcode)
            {
            case MMW_WSOP_CLOSE:
                mmw_dbg(srv->config.log, "received websocket close request");
                connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                return;

              case MMW_WSOP_PING:
                mmw_dbg(srv->config.log, "received websocket ping request");
                if (mmw_socket_send(MMW_SOCK(connection->socket), mmw_websocket_pong,
                    sizeof mmw_websocket_pong)){
                    connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                    return;
                }
                break;

              default:
                /* Dispatch frame to user handler. */
                if (srv->config.ws_frame(&connection->public_data,
                    &connection->ws_frame, srv->config.userdata) != MMW_OK) {
                  connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                  return;
                }
            }

            /* Discard any data the client didn't read to retain the socket state. */
            if (connection->body_bytes_read < connection->ws_frame.payload_length) {
                int size = connection->ws_frame.payload_length - connection->body_bytes_read;
                if (mmw_con_discard_incoming_data(&connection->public_data, size) != MMW_OK) {
                    connection->flags &= (unsigned short)~MMW_CON_FLAG_ALIVE;
                    return;
                }
            }

            /* Back to non-blocking mode */
            if (mmw_connection_set_nonblocking(connection) != MMW_OK)
                return;

            mmw_connection_reset(connection, srv->config.request_buffer_size, srv->config.io_buffer_size);
            connection->state = MMW_CON_STATE_WEBSOCKET;
        } break; /* MMW_WEBSOCKET */
        } /* switch */
    } /* for */
}

MMW_API void
mmw_server_update(struct mmw_server *srv)
{
    int err;
    mmw_size i, count;
    mmw_socket max_socket;
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
        max_socket = MMW_SOCK(srv->socket);
    }

    for (i = 0, count = srv->con_count; i < count; ++i) {
        mmw_socket socket = MMW_SOCK(srv->con[i].socket);
        FD_SET(socket, &read_fds);
        FD_SET(socket, &except_fds);
        if (srv->con[i].state == MMW_CON_STATE_SEND_CONTINUE)
            FD_SET(socket, &write_fds);
        if (socket > max_socket)
            max_socket = socket;
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 5;
    err = select((int)(max_socket + 1), &read_fds, &write_fds, &except_fds, &timeout);
    if (err < 0) {
        mmw_dbg(srv->config.log, "failed to select");
        return;
    }

    /* Handle incoming connections */
    if (FD_ISSET(MMW_SOCK(srv->socket), &read_fds)) {
        do {
            mmw_dbg(srv->config.log, "awake on incoming");
            err = mmw_server_on_incoming(srv);
        } while (err == 0);
    }

    /* Handle incoming connection data */
    for (i = 0, count = srv->con_count; i < count; ++i) {
        struct mmw_connection *conn = &srv->con[i];
        if (FD_ISSET(MMW_SOCK(conn->socket), &read_fds) ||
            FD_ISSET(MMW_SOCK(conn->socket), &write_fds) ||
            conn->flags & MMW_CON_FLAG_FRESH_CONNECTION)
        {
            mmw_dbg(srv->config.log, "reading from connection %d", i);
            mmw_server_update_connection(srv, conn);
        }
    }

    /* Close stale connections & compact connection array. */
    for (i = 0; i < srv->con_count; ) {
        struct mmw_connection *connection = &srv->con[i];
        if (!(connection->flags & MMW_CON_FLAG_ALIVE)) {
            mmw_size remain;
            mmw_dbg(srv->config.log, "closing connection %d (%08x)", i, connection->flags);
            if (connection->flags & MMW_CON_FLAG_WEBSOCKET)
                srv->config.ws_closed(&connection->public_data, srv->config.userdata);
            remain = srv->con_count - (mmw_size)i - 1;
            mmw_connection_close(connection);
            memmove(&srv->con[i], &srv->con[i + 1], remain*sizeof(srv->con[i]));
            --srv->con_count;
        } else ++i;
    }
}

#endif /* MMW_IMPLEMENTATION */
