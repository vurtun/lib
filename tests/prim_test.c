/*
    Copyright (c) 2015 Micha Mettke

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1.  The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2.  Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3.  This notice may not be removed or altered from any source distribution.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>

#define VX_STATIC
#define VX_USE_ASSERT
#define VX_IMPLEMENTATION
#include "vtx.h"

#define XV_PI 3.141592654f
#define DEG2RAD(a) ((a)*(XV_PI/180.0f))
#define RAD2DEG(a) ((a)*(180.0f/XV_PI))
#define UNUSED(x) ((void)x)

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

struct vertex {
    float pos[3];
    float col[4];
};

struct device {
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_normal;
    GLint attrib_col;
    GLint uniform_proj;
    GLint uniform_mdl;
    GLint uniform_view;
};

#define vec_op(a,p,b,n, post) (((a)[n] p (b)[n]) post)
#define vec_applys(r,e,a,n,p,s,post) (r)[n] e ((((a)[n] p s)) post)
#define vec_expr(r,e,a,p,b,n,post) (r)[n] e ((vec_op(a,p,b,n,post)))

#define vec_map(r,e,a,p,b,post)\
    vec_expr(r,e,a,p,b,0,post),\
    vec_expr(r,e,a,p,b,1,post),\
    vec_expr(r,e,a,p,b,2,post)
#define vec_apply(r,e,a,p,s,post)\
    vec_applys(r,e,a,0,p,s,post),\
    vec_applys(r,e,a,1,p,s,post),\
    vec_applys(r,e,a,2,p,s,post)
#define vec_dot(a,b)\
    vec_op(a,*,b,0,+0)+\
    vec_op(a,*,b,1,+0)+\
    vec_op(a,*,b,2,+0)

#define vec_cross(r, a, b)\
    (r)[0] = (a)[1]*(b)[2] - (a)[2]*(b)[1],\
    (r)[1] = (a)[2]*(b)[0] - (a)[0]*(b)[2],\
    (r)[2] = (a)[0]*(b)[1] - (a)[1]*(b)[0]

#define vec_add(r,a,b)       vec_map(r,=,a,+,b,+0)
#define vec_sub(r,a,b)       vec_map(r,=,a,-,b,+0)
#define vec_addeq(r,b)       vec_map(r,=,r,+,b,+0)
#define vec_subeq(r,b)       vec_map(r,=,r,-,b,+0)

#define vec_muli(r,a,s)      vec_apply(r,=,a,*,s,+0)
#define vec_divi(r,a,s)      vec_apply(r,=,a,/,s,+0)
#define vec_addi(r,a,s)      vec_apply(r,=,a,+,s,+0)
#define vec_subi(r,a,s)      vec_apply(r,=,a,-,s,+0)

#define vec_mulieq(r,s)      vec_apply(r,=,r,*,s,+0)
#define vec_divieq(r,s)      vec_apply(r,=,r,/,s,+0)
#define vec_addieq(r,s)      vec_apply(r,=,r,+,s,+0)
#define vec_subieq(r,s)      vec_apply(r,=,r,-,s,+0)

#define vec_addm(r,a,b,s)    vec_map(r,=,a,+,b,*s)
#define vec_subm(r,a,b,s)    vec_map(r,=,a,-,b,*s)
#define vec_addmeq(r,b,s)    vec_map(r,=,r,+,b,*s)
#define vec_submeq(r,b,s)    vec_map(r,=,r,-,b,*s)

#define vec_addd(r,a,b,s)    vec_map(r,=,a,+,b,/s)
#define vec_subd(r,a,b,s)    vec_map(r,=,a,-,b,/s)
#define vec_adddeq(r,b,s)    vec_map(r,=,r,+,b,/s)
#define vec_subdeq(r,b,s)    vec_map(r,=,r,-,b,/s)

#define vec_adda(r,a,b,s)    vec_map(r,=,a,+,b,+s)
#define vec_suba(r,a,b,s)    vec_map(r,=,a,-,b,+s)
#define vec_addaeq(r,b,s)    vec_map(r,=,r,+,b,+s)
#define vec_subaeq(r,b,s)    vec_map(r,=,r,-,b,+s)

#define vec_adds(r,a,b,s)    vec_map(r,=,a,+,b,-s)
#define vec_subs(r,a,b,s)    vec_map(r,=,a,-,b,-s)
#define vec_addseq(r,b,s)    vec_map(r,=,r,+,b,-s)
#define vec_subseq(r,b,s)    vec_map(r,=,r,-,b,-s)

#define vec_neg(r,a)         vec_apply(r,=,a,*,-1.0f,+0)
#define vec_len2(a)          vec_dot(a,a)
#define vec_len(a)           ((float)sqrt(vec_len2(a)))

#define vec_normeq(o)do{\
    float len_i_ = vec_len2(o);\
    if(len_i_ > 0.00001f){\
        len_i_ = (float)sqrt(len_i_);\
        len_i_ = 1.0f/len_i_;\
        vec_mulieq(o, len_i_);\
    }}while(0)

static void
m4_identity(float *m)
{
    #define M(row, col) m[(col<<2)+row]
    M(0,0) = 1; M(0,1) = 0; M(0,2) = 0; M(0,3) = 0;
    M(1,0) = 0; M(1,1) = 1; M(1,2) = 0; M(1,3) = 0;
    M(2,0) = 0; M(2,1) = 0; M(2,2) = 1; M(2,3) = 0;
    M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
    #undef M
}

static void
m4_persp(float *m, float fov, float aspect, float near, float far)
{
    #define M(row, col) m[(col<<2)+row]
    const float rad = DEG2RAD(fov);
    const float hfov = (float)tan(rad/2.0f);
    memset(m,0, 16*sizeof(float));
    M(0,0) = 1.0f / (aspect * hfov);
    M(1,1) = 1.0f / hfov;
    M(2,2) = -(far + near) / (far - near);
    M(2,3) = -1.0f;
    M(3,2) = -(2.0f * far * near) / (far - near);
    #undef M
}

static void
m4_lookat(float *m, const float *eye, const float *center, const float *up)
{
    float f[3], s[3], u[3];
    vec_sub(f, center, eye); vec_normeq(f);
    vec_cross(s, f, up); vec_normeq(s);
    vec_cross(u, s, f);

    #define M(row, col) m[(col<<2)+row]
    memset(m,0, 16*sizeof(float));
    M(0,0) = s[0],  M(1,0) = s[1],  M(2,0) = s[2];
    M(0,1) = u[0],  M(1,1) = u[1],  M(2,1) = u[2];
    M(0,2) = -f[0], M(1,2) = -f[1], M(2,2) = -f[2];
    M(3,0) = -vec_dot(s, eye);
    M(3,1) = -vec_dot(u, eye);
    M(3,2) = vec_dot(f, eye);
    M(3,3) = 1.0f;
    #undef M
}

static void
device_init(struct device *dev)
{
    GLint status;
    static const GLchar *vertex_shader =
        "#version 300 es\n"
        "uniform mat4 proj;\n"
        "uniform mat4 mdl;\n"
        "uniform mat4 view;\n"
        "in vec3 Position;\n"
        "in vec4 Color;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = proj * view * mdl * vec4(Position, 1);\n"
        "}\n";
    static const GLchar *fragment_shader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color;\n"
        "}\n";

    dev->prog = glCreateProgram();
    dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(dev->vert_shdr);
    glCompileShader(dev->frag_shdr);

    glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    glAttachShader(dev->prog, dev->vert_shdr);
    glAttachShader(dev->prog, dev->frag_shdr);

    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniform_proj = glGetUniformLocation(dev->prog, "proj");
    dev->uniform_view = glGetUniformLocation(dev->prog, "view");
    dev->uniform_mdl = glGetUniformLocation(dev->prog, "mdl");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct vertex);
        size_t vp = offsetof(struct vertex, pos);
        size_t vc = offsetof(struct vertex, col);

        const struct vertex vertexes[3] = {
            {{-0.5f,0.0f,0.5f}, {1,0,0,1}},
            {{0.5f,0.0f,0.5f}, {1,0,0,1}},
            {{0.0f,0.0f,-0.5f}, {1,0,0,1}}
        };

        glGenBuffers(1, &dev->vbo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STREAM_DRAW);

        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);
        glVertexAttribPointer((GLuint)dev->attrib_pos, 3, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_FLOAT, GL_FALSE, vs, (void*)vc);

    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void
device_shutdown(struct device *dev)
{
    glDetachShader(dev->prog, dev->vert_shdr);
    glDetachShader(dev->prog, dev->frag_shdr);
    glDeleteShader(dev->vert_shdr);
    glDeleteShader(dev->frag_shdr);
    glDeleteProgram(dev->prog);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
}

static void
device_draw(struct device *dev, int width, int height)
{
    GLfloat persp[4][4];
    GLfloat view[4][4];
    GLfloat mdl[4][4];

    const GLfloat eye[3] = {0,15,-15};
    const GLfloat target[3] = {0,0,0};
    const GLfloat up[3] = {0,1,0};

    m4_persp(&persp[0][0], 45.0f, 4.0f/3.0f, 0.1f, 1000.0f);
    m4_lookat(&view[0][0], eye, target, up);
    m4_identity(&mdl[0][0]);

    /* setup global state */
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* setup buffer */
    glBindVertexArray(dev->vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);

    /* setup program */
    glUseProgram(dev->prog);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &persp[0][0]);
    glUniformMatrix4fv(dev->uniform_view, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(dev->uniform_mdl, 1, GL_FALSE, &mdl[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void
on_error(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
        const GLchar *msg, const void *data)
{
    UNUSED(source);
    UNUSED(type);
    UNUSED(id);
    UNUSED(severity);
    UNUSED(length);
    UNUSED(msg);
    UNUSED(data);
    fprintf(stdout, "[GL]: %s\n", msg);
}

int main(void)
{
    /* Platform */
    const char *font_path;
    SDL_Window *win;
    SDL_GLContext glContext;
    int width = 0, height = 0;
    int running = 1;
    struct device dev;
    GLuint unused = 0;

    /* SDL */
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow("Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &width, &height);

    /* OpenGL */
    glViewport(0, 0, width, height);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK)
        die("Failed to setup GLEW\n");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(on_error, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
        GL_DONT_CARE, 0, &unused, 1);

    device_init(&dev);
    while (running) {
        /* Input */
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_WINDOWEVENT) {
                if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
                    glViewport(0, 0, evt.window.data1, evt.window.data2);
            } else if (evt.type == SDL_QUIT) goto cleanup;
        }

        /* Draw */
        SDL_GetWindowSize(win, &width, &height);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        device_draw(&dev, width, height);
        SDL_GL_SwapWindow(win);
    }

cleanup:
    /* Cleanup */
    device_shutdown(&dev);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

