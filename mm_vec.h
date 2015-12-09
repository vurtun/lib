/*
    mm_vec.h - zlib - Micha Mettke

ABOUT:
    This is a ANSI C vector math library with header and implementations for
    vector, matrix, plane, sphere and AABB math. Under normal circumstances it
    is extremly awefull to use C for math since it does not allow to overload
    operators. I noticed while writing one math libraries after another
    that I had to change types and the implementation depending on the
    different C versions (C89, C99 and C11).
    Each version allows more control for example C99 has designated initializers
    and C11 allows unamed unions inside structs. But since I do not want to
    reimplement everything from scratch for each version I decided to make the
    library work only on float arrays and therefore create a way to easily
    generate the implementation independent of each vector, matrix,
    plane, quaternion, sphere, AABB type. Downside is that it is not as nice to
    use than directly defining types.

DEFINES:
    MMX_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    MMX_STATIC
        The generated implementation will stay private inside implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    MMX_INT32
    MMX_UINT32
    MMX_UINT_PTR
        If your compiler is C99 you do not need to define this.
        Otherwise, mm_sched will try default assignments for them
        and validate them at compile time. If they are incorrect, you will
        get compile errors and will need to define them yourself.

    MMS_MEMSET
        You can define this to 'memset' or your own memset replacement.
        If not, mm_sched.h uses a naive (maybe inefficent) implementation.

    MMS_MEMCPY
        You can define this to 'memcpy' or your own memset replacement.
        If not, mm_sched.h uses a naive (maybe inefficent) implementation.

    MMX_SIN
    MMX_FABS
    MMX_COS
    MMX_TAN
    MMX_ACOS
    MMX_ATAN2
    MMX_SQRT
        Overrwrite these to use your own math functions. If not defined default
        standard math library functions will be used


LICENSE: (zlib)
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


TESTED:
    + vector
    + matrix

USAGE:
    This file behaves differently depending on what symbols you define
    before including it.

    Header-File mode:
    If you do not define MMX_IMPLEMENTATION before including this file, it
    will operate in header only mode. In this mode it declares all used structs
    and the API of the library without including the implementation of the library.

    Implementation mode:
    If you define MMX_IMPLEMENTATIOn before including this file, it will
    compile the implementation of the math library. To specify the visibility
    as private and limit all symbols inside the implementation file
    you can define MMX_STATIC before including this file.
    Make sure that you only include this file implementation in *one* C or C++ file
    to prevent collisions.
*/
 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MMX_H_
#define MMX_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MMX_STATIC
#define MMX_API static
#else
#define MMX_API extern
#endif

#ifndef MMX_SQRT
#define MMX_SQRT sqrt
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#include <stdint.h>
#ifndef MMX_UINT32
#define MMX_UINT32 uint32_t
#endif
#ifndef MMX_INT32
#define MMX_INT32 int32_t
#endif
#ifndef MMX_UINT_PTR
#define MMX_UINT_PTR uintptr_t
#endif
#else
#ifndef MMX_UINT32
#define MMX_UINT32 unsigned int
#endif
#ifndef MMX_INT32
#define MMX_INT32 int
#endif
#ifndef MMX_UINT_PTR
#define MMX_UINT_PTR unsigned long
#endif
#endif

typedef unsigned char mmx_byte;
typedef MMX_UINT32 mmx_uint;
typedef MMX_INT32 mmx_int;
typedef MMX_UINT_PTR mmx_size;
typedef MMX_UINT_PTR mmx_ptr;

/* ---------------------------------------------------------------
 *                          VECTOR
 * ---------------------------------------------------------------*/
#define xv(v) ((float*)(&(v)))
#define xv_op(a,p,b,n, post) (((a)[n] p (b)[n]) post)
#define xv_applys(r,e,a,n,p,s,post) (r)[n] e ((((a)[n] p s)) post)
#define xv_expr(r,e,a,p,b,n,post) (r)[n] e ((xv_op(a,p,b,n,post)))

#define xv2_map(r,e,a,p,b,post)\
    xv_expr(r,e,a,p,b,0,post),\
    xv_expr(r,e,a,p,b,1,post)
#define xv3_map(r,e,a,p,b,post)\
    xv_expr(r,e,a,p,b,0,post),\
    xv_expr(r,e,a,p,b,1,post),\
    xv_expr(r,e,a,p,b,2,post)
#define xv4_map(r,e,a,p,b,post)\
    xv_expr(r,e,a,p,b,0,post),\
    xv_expr(r,e,a,p,b,1,post),\
    xv_expr(r,e,a,p,b,2,post),\
    xv_expr(r,e,a,p,b,3,post)

#define xv2_apply(r,e,a,p,s,post)\
    xv_applys(r,e,a,0,p,s,post),\
    xv_applys(r,e,a,1,p,s,post)
#define xv3_apply(r,e,a,p,s,post)\
    xv_applys(r,e,a,0,p,s,post),\
    xv_applys(r,e,a,1,p,s,post),\
    xv_applys(r,e,a,2,p,s,post)
#define xv4_apply(r,e,a,p,s,post)\
    xv_applys(r,e,a,0,p,s,post),\
    xv_applys(r,e,a,1,p,s,post),\
    xv_applys(r,e,a,2,p,s,post),\
    xv_applys(r,e,a,3,p,s,post)

#define xv2_dot(a,b)\
    xv_op(a,*,b,0,+0)+\
    xv_op(a,*,b,1,+0)
#define xv3_dot(a,b)\
    xv_op(a,*,b,0,+0)+\
    xv_op(a,*,b,1,+0)+\
    xv_op(a,*,b,2,+0)
#define xv4_dot(a,b)\
    xv_op(a,*,b,0,+0)+\
    xv_op(a,*,b,1,+0)+\
    xv_op(a,*,b,2,+0)+\
    xv_op(a,*,b,3,+0)

#define xv2_cross(r,a,b) (r) = (((a)[0] * (b)[1]) - ((a)[1]) * (b)[0])
#define xv3_cross(r, a, b)\
    (r)[0] = (a)[1]*(b)[2] - (a)[2]*(b)[1],\
    (r)[1] = (a)[2]*(b)[0] - (a)[0]*(b)[2],\
    (r)[2] = (a)[0]*(b)[1] - (a)[1]*(b)[0]
#define xv4_cross(r, a, b) xv3_cross(r, a, b), (r)[3] = 1

#define xv_apply(r,e,a,p,s,post)xv##dim##_apply(r,e,a,p,s,post)
#define xv_map(r,e,a,p,b,post)  xv##dim##_map(r,e,a,p,b,post)

#define xv_add(r,a,b,dim)       xv##dim##_map(r,=,a,+,b,+0)
#define xv_sub(r,a,b,dim)       xv##dim##_map(r,=,a,-,b,+0)
#define xv_addeq(r,b,dim)       xv##dim##_map(r,=,r,+,b,+0)
#define xv_subeq(r,b,dim)       xv##dim##_map(r,=,r,-,b,+0)

#define xv_muli(r,a,s,dim)      xv##dim##_apply(r,=,a,*,s,+0)
#define xv_divi(r,a,s,dim)      xv##dim##_apply(r,=,a,/,s,+0)
#define xv_addi(r,a,s,dim)      xv##dim##_apply(r,=,a,+,s,+0)
#define xv_subi(r,a,s,dim)      xv##dim##_apply(r,=,a,-,s,+0)

#define xv_mulieq(r,s,dim)      xv##dim##_apply(r,=,r,*,s,+0)
#define xv_divieq(r,s,dim)      xv##dim##_apply(r,=,r,/,s,+0)
#define xv_addieq(r,s,dim)      xv##dim##_apply(r,=,r,+,s,+0)
#define xv_subieq(r,s,dim)      xv##dim##_apply(r,=,r,-,s,+0)

#define xv_addm(r,a,b,s,dim)    xv##dim##_map(r,=,a,+,b,*s)
#define xv_subm(r,a,b,s,dim)    xv##dim##_map(r,=,a,-,b,*s)
#define xv_addmeq(r,b,s,dim)    xv##dim##_map(r,=,r,+,b,*s)
#define xv_submeq(r,b,s,dim)    xv##dim##_map(r,=,r,-,b,*s)

#define xv_addd(r,a,b,s,dim)    xv##dim##_map(r,=,a,+,b,/s)
#define xv_subd(r,a,b,s,dim)    xv##dim##_map(r,=,a,-,b,/s)
#define xv_adddeq(r,b,s,dim)    xv##dim##_map(r,=,r,+,b,/s)
#define xv_subdeq(r,b,s,dim)    xv##dim##_map(r,=,r,-,b,/s)

#define xv_adda(r,a,b,s,dim)    xv##dim##_map(r,=,a,+,b,+s)
#define xv_suba(r,a,b,s,dim)    xv##dim##_map(r,=,a,-,b,+s)
#define xv_addaeq(r,b,s,dim)    xv##dim##_map(r,=,r,+,b,+s)
#define xv_subaeq(r,b,s,dim)    xv##dim##_map(r,=,r,-,b,+s)

#define xv_adds(r,a,b,s,dim)    xv##dim##_map(r,=,a,+,b,-s)
#define xv_subs(r,a,b,s,dim)    xv##dim##_map(r,=,a,-,b,-s)
#define xv_addseq(r,b,s,dim)    xv##dim##_map(r,=,r,+,b,-s)
#define xv_subseq(r,b,s,dim)    xv##dim##_map(r,=,r,-,b,-s)

#define xv_neg(r,a,dim)         xv##dim##_apply(r,=,a,*,-1.0f,+0)
#define xv_dot(a,b,dim)         xv##dim##_dot(a,b)
#define xv_len2(a,dim)          xv##dim##_dot(a,a)
#define xv_len(a,dim)           ((float)MMX_SQRT(xv_len2(a,dim)))
#define xv_len_inv(a,dim)       xv_inv_sqrt(xv_len2(a,dim))
#define xv_cross(r,a,b,dim)     xv##dim##_cross(r, a, b)

#define xv_lerp(r,a,t,b,dim)\
    xv##dim##_apply(r,=,a,*,(1.0f - (t)), +0);\
    xv##dim##_apply(r,+=,b,*,t,+0)

#define xv_norm(o, q, dim)do{\
    float len_i_ = xv_len2(q,dim);\
    if(len_i_ > 0.00001f){\
        len_i_ = MMX_SQRT(len_i_);\
        len_i_ = 1.0f/len_i_;\
        xv_muli(o, q, len_i_, dim);\
    }}while(0)

#define xv_normeq(o, dim)do{\
    float len_i_ = xv_len2(o,dim);\
    if(len_i_ > 0.00001f){\
        len_i_ = (float)MMX_SQRT(len_i_);\
        len_i_ = 1.0f/len_i_;\
        xv_mulieq(o, len_i_, dim);\
    }}while(0)

#define xv_norm_len(len, o, q, dim)do{\
    float len_i_ = xv_len2(q,dim);\
    if(len_i_ > 0.00001f){\
        len = (float)MMX_SQRT(len_i_);\
        len_i_ = 1.0f/len;\
        xv_muli(o, q, len_i_, dim);\
    }}while(0)

#define xv_normeq_len(len, o, dim)do{\
    float len_i_ = xv_len2(o,dim);\
    if(len_i_ > 0.00001f){\
        len = (float)MMX_SQRT(len_i_);\
        len_i_ = 1.0f/len;\
        xv_mulieq(o, len_i_, dim);\
    }}while(0)

#define xv_norm_fast(o, q, dim)do{\
        float len_i_ = xv_len2(q,dim);\
        len_i_ = xv_inv_sqrt(len_i_);\
        xv_muli(o, q, len_i_, dim);\
    }}while(0)

#define xv_normeq_fast(o, dim)do{\
        float len_i_ = xv_len2(o,dim);\
        len_i_ = xv_inv_sqrt(len_i_);\
        xv_mulieq(o, len_i_, dim);\
    }}while(0)

#define xv_norm_len_fast(len, o, q, dim)do{\
        float len_i_ = xv_len2(q,dim);\
        float inv_len_i_ = xv_inv_sqrt(len_i_);\
        xv_muli(o, q, inv_len_i_,dim);\
        len = len_i_ * inv_len_i;\
    }}while(0)

#define xv_normeq_len_fast(len, o, dim)do{\
        float len_i_ = xv_len2(o,dim);\
        float inv_len_i_ = xv_inv_sqrt(len_i_);\
        xv_mulieq(o, inv_len_i_,dim);\
        len = len_i_ * inv_len_i;\
    }}while(0)

MMX_API float xv_inv_sqrt(float n);
MMX_API float xv3_angle(float *axis, const float *a, const float *b);
MMX_API void xv3_slerp(float *r, const float *a, float t, const float *b);
MMX_API void xv3_project_to_sphere(float *r, const float *v, float radius);
MMX_API void xv3_project_to_plane(float *r, const float *v, const float *normal,
                                float over_bounce);
MMX_API int xv3_project_along_plane(float *r, const float *v, const float *normal,
                                    float epsilon, float over_bounce);

/* ---------------------------------------------------------------
 *                          MATRIX
 * ---------------------------------------------------------------*/
#define XM_AXIS_X 0
#define XM_AXIS_Y 1
#define XM_AXIS_Z 2

#define xm(m) ((float*)&(m))
MMX_API void xm2_identity(float *m);
MMX_API void xm2_transpose(float *m);
MMX_API void xm2_mul(float *product, const float *a, const float *b);
MMX_API void xm2_transform(float *r, const float *m, const float *v);
MMX_API void xm2_rotate(float *m, float angle);
MMX_API void xm2_scale(float *m, float x, float y);
MMX_API float xm2_determinant(const float *m);
MMX_API int xm2_inverse_self(float *m);
MMX_API int xm2_inverse(float *r, const float *m);

MMX_API void xm3_identity(float *m);
MMX_API void xm3_transpose(float *m);
MMX_API void xm3_mul(float *product, const float *a, const float *b);
MMX_API void xm3_scale(float *m, float x, float y, float z);
MMX_API void xm3_transform(float *r, const float *m, const float *v);
MMX_API void xm3_rotate(float *m, float angle, float X, float Y, float Z);
MMX_API void xm3_rotate_x(float *m, float angle);
MMX_API void xm3_rotate_y(float *m, float angle);
MMX_API void xm3_rotate_z(float *m, float angle);
MMX_API void xm3_rotate_axis(float *m, int axis, float angle);
MMX_API void xm3_rotate_align(float *m, const float *dest, const float *src);
MMX_API float xm3_determinant(const float *m);
MMX_API int xm3_inverse_self(float *m);
MMX_API int xm3_inverse(float *r, const float *m);
MMX_API void xm3_from_quat(float *m, const float *q);
MMX_API void xm3_from_mat4(float *r, const float *m);

MMX_API void xm4_identity(float *m);
MMX_API void xm4_transpose(float *m);
MMX_API void xm4_translate(float *m, float x, float y, float z);
MMX_API void xm4_rotate(float *m, float angle, float X, float Y, float Z);
MMX_API void xm4_rotate_x(float *m, float angle);
MMX_API void xm4_rotate_y(float *m, float angle);
MMX_API void xm4_rotate_z(float *m, float angle);
MMX_API void xm4_rotate_axis(float *m, int axis, float angle);
MMX_API void xm4_mul(float *product, const float *a, const float *b);
MMX_API float xm4_determinant(const float *m);
MMX_API int xm4_inverse_self(float *m);
MMX_API int xm4_inverse(float *r, const float *m);
MMX_API void xm4_transform(float *r, const float *m, const float *v);
MMX_API void xm4_ortho(float *m, float left, float right, float bottom, float top);
MMX_API void xm4_orthographic(float *m, float left, float right, float bottom, float top, float near, float far);
MMX_API void xm4_frustum(float *m, float left, float right, float buttom, float top, float near, float far);
MMX_API void xm4_persp(float *m, float fov, float aspect, float near, float far);
MMX_API void xm4_lookat(float *m, const float *eye, const float *center, const float *up);
MMX_API void xm4_from_quat(float *m, const float *q);
MMX_API void xm4_from_quat_vec(float *m, const float *q, const float *p);
MMX_API void xm4_from_mat3(float *r, const float *m);

/* ---------------------------------------------------------------
 *                          QUATERNION
 * ---------------------------------------------------------------*/
#define xq(q) ((float*)&(q))
MMX_API void xq_from_mat3(float *q, const float *m);
MMX_API void xq_rotation(float *q, float angle, float x, float y, float z);
MMX_API void xq_transform(float *out, const float *q, const float *v);
MMX_API void xq_mul(float *r, const float *a, const float *b);
MMX_API void xq_integrate2D(float *r, const float *q, float *omega, float delta);
MMX_API void xq_integrate3D(float *r, const float *q, float *omega, float delta);
MMX_API float xq_invert(float*, const float*);
MMX_API float xq_inverteq(float*);
MMX_API float xq_get_rotation(float *axis, const float *q);
#define xq_identity(q) (q)[0] = (q)[1] = (q)[2] = 0, (q)[3] = 1.0f
#define xq_conjugate(t,f) ((t)[0] = -(f)[0],(t)[1] = -(f)[1],(t)[2] = -(f)[2], (t)[3] = (f)[3])
#define xq_norm(o, q) xv_norm(o, q, 4)
#define xq_normeq(q) xv_normeq(q, 4)
#define xq_norm_len(len, o, q) xv_norm_len(len, o, q, 4)
#define xq_normeq_len(len, q) xv_normeq_len(len, q, 4)
#define xq_len(q) xv_len(q, 4)
#define xq_lerp(r,a,t,b) xv_lerp(r,a,t,b,4)
#define xq_lerpeq(r,a,t,b) xv_lerp(a,a,t,b,4)
#define xq_lerp_norm(len, r,a,t,b) xv_lerp(r,a,t,b,4); xv_normeq(len, r)
#define xq_add(r, a, b) xv_add(r, a, b, 4)
#define xq_sub(r, a, b) xv_sub(r, a, b, 4)
#define xq_addeq(r, b) xv_addeq(r, b, 4)
#define xq_subeq(r, b) xv_subeq(r, b, 4)
#define xq_muli(r, a, s) xv_muli(r, a, s, 4)
#define xq_divi(r, a, s) xv_divi(r, a, s, 4)
#define xq_mulieq(r, s) xv_mulieq(r, s, 4)
#define xq_divieq(r, s) xv_divieq(r, s, 4)

/* ---------------------------------------------------------------
 *                          COMPLEX
 * ---------------------------------------------------------------*/
/* complex = {real, imaginary} */
#define xc(q) ((float*)&(q))
#define xc_add(r,a,b) xv_add(r,a,b,2)
#define xc_sub(r,a,b) xv_sub(r,a,b,2)
MMX_API void xc_mul(float *r, const float *a, const float *b);
MMX_API void xc_div(float *r, const float *a, const float *b);

#define xc_addeq(r,b) xv_addeq(r,b,2)
#define xc_subeq(r,b) xv_subeq(r,b,2)
#define xc_muleq(r,b) xc_mul(r, r, b)
#define xc_diveq(r,b) xc_div(r, r, b)

#define xc_addi(r,a,s) ((r)[0] = (a)[0] + (s), (r)[1] = (a)[1])
#define xc_subi(r,a,s) ((r)[0] = (a)[0] - (s), (r)[1] = (a)[1])
#define xc_muli(r,a,s) ((r)[0] = (a)[0] * (s), (r)[1] = (a)[1] * (s))
#define xc_divi(r,a,s) ((r)[0] = (a)[0] / (s), (r)[1] = (a)[1] / (s))

#define xc_addf(r,s,a) ((r)[0] = (s) + (a)[0], (r)[1] = (a)[1])
#define xc_subf(r,s,a) ((r)[0] = (s) - (a)[0], (r)[1] = -(a)[1])
#define xc_mulf(r,s,a) ((r)[0] = (s) * (a)[0], (r)[2] = (s)*(a)[1])
MMX_API void xc_divf(float *r, float a, const float *b);

#define xc_addfeq(r,s) xc_addf(r, s, r)
#define xc_subfeq(r,s) xc_subf(r, s, r)
#define xc_mulfeq(r,s) xc_mulf(r, s, r)
#define xc_divfeq(r,s) xc_divf(r, s, r)

#define xc_addieq(r,s) xc_addi(r, r, s)
#define xc_subieq(r,s) xc_subi(r, r, s)
#define xc_mulieq(r,s) xc_muli(r, r, s)
#define xc_divieq(r,s) xc_divi(r, r, s)

MMX_API void xc_sqrt(float *r, const float *c);
MMX_API float xc_abs(const float *c);
MMX_API void xc_reciprocal(float *r, const float *c);

/* ---------------------------------------------------------------
 *                          PLANE
 * ---------------------------------------------------------------*/
/* plane sides */
#define XP_FRONT    0
#define XP_BACK     1
#define XP_ON       2
#define XP_CROSS    3

/* plane = a * x + b * y + c * z + d = 0 */
#define xp(p) ((float*)&(p))
MMX_API void xp_make(float *p, const float *normal, float distance);
MMX_API int xp_from_points(float *p, const float *p1, const float *p2, const float *p3);
MMX_API int xp_from_vec(float *r, const float *v1, const float *v2, const float *p);
MMX_API void xp_translate(float *r, const float *plane, const float *translation);
MMX_API void xp_translateq(float *r, const float *translation);
MMX_API void xp_rotate(float *r, const float *plane, const float *origin, const float *m33);
MMX_API void xp_rotate_self(float *r, const float *orgin, const float *m33);
MMX_API float xp_norm(float *r, const float *p);
MMX_API float xp_norm_self(float *r);
MMX_API float xp_distance(const float *p, const float *v3);
MMX_API int xp_side(const float *p, const float *v3, float epsilon);
MMX_API int xp_intersect_line(const float *p, const float *start, const float *end);
MMX_API int xp_intersect_ray(float *scale, const float *p, const float *start, const float *dir);
MMX_API int xp_intersect_plane(float *start, float *dir, const float *p0, const float *p1);

/* ---------------------------------------------------------------
 *                          SPHERE
 * ---------------------------------------------------------------*/
/* sphere = {origin:(x,y,z),radius}*/
#define xs(p) ((float*)&(p))
MMX_API void xs_make(float *s, const float *origin, float radius);
MMX_API int xs_add_point(float *s, const float *point);
MMX_API int xs_add_sphere(float *s, const float *sphere);
MMX_API void xs_expand(float *r, const float *s, float d);
MMX_API void xs_expand_self(float *s, float d);
MMX_API void xs_translate(float *r, const float *s, const float *t);
MMX_API void xs_translate_self(float *r, const float *t);
MMX_API float xs_plane_distance(const float *s, const float *p);
MMX_API int xs_plane_side(const float *s, const float *p, float epsilon);
MMX_API int xs_contains_point(const float *s, const float *p);
MMX_API int xs_intersects_line(const float *s2, const float *start, const float *end);
MMX_API int xs_intersects_ray(float *scale0, float *scale1, const float *sphere, const float *start, const float *dir);
MMX_API int xs_intersects_sphere(const float *s2, const float *s1);
MMX_API void xs_from_box(float *sphere, const float *box);

/* ---------------------------------------------------------------
 *                          BOX
 * ---------------------------------------------------------------*/
/* Axis Aligned Bounding Box */
/* box = {min(x,y,z)},max(x,y,z)}*/
#define xb(p) ((float*)&(p))
MMX_API void xb_make(float *b, const float *min, const float *max);
MMX_API void xb_from_points(float *b, const void *verts, int num, int stride, int offset);
MMX_API int xb_add_point(float *b, const float *point);
MMX_API int xb_add_box(float *b, const float *box);
MMX_API void xb_center(float *center, const float *box);
MMX_API void xb_radius(float *radius, const float *box);
MMX_API void xb_expand(float *r, const float *b, float d);
MMX_API void xb_expand_self(float *b, float d);
MMX_API void xb_transform(float *r, const float *box, const float *origin, const float *mat33);
MMX_API void xb_translate(float *r, const float *b, const float *t);
MMX_API void xb_translate_self(float *r, const float *t);
MMX_API void xb_rotate(float *r, const float *b, const float *mat33);
MMX_API void xb_rotate_self(float *r, const float *mat33);
MMX_API void xb_intersection(float *r, const float *a, const float *b);
MMX_API void xb_intersection_self(float *r, const float *b);
MMX_API float xb_plane_distance(const float *s, const float *p);
MMX_API int xb_plane_side(const float *s, const float *p, float epsilon);
MMX_API int xb_contains_point(const float *s, const float *p);
MMX_API int xb_intersects_line(const float *s2, const float *start, const float *end);
MMX_API int xb_intersects_ray(float *scale, const float *box, const float *start, const float *dir);
MMX_API int xb_intersects_box(const float *a, const float *b);

#ifdef __cplusplus
}
#endif
#endif /* MMX_H_ */

/* ===============================================================
 *
 *                      IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MMX_IMPLEMENTATION

#define MMX_INTERN static
#define MMX_GLOBAL static
#define MMX_STORAGE static

#define MMX_MIN(a,b) (((a)<(b))?(a):(b))
#define MMX_MAX(a,b) (((a)>(b))?(a):(b))
#define MMX_DEG2RAD(a) ((a)*(MMX_PI/180.0f))
#define MMX_RAD2DEG(a) ((a)*(180.0f/MMX_PI))

/* make sure we have a correct 32-bit integer type */
typedef int mmx__check_uint32_size[(sizeof(mmx_uint) == 4) ? 1 : -1];

#ifdef __cplusplus
/* C++ hates the C align makro so have to resort to templates */
template<typename T> struct xv_alignof;
template<typename T, int size_diff> struct xv_helper{enum {value = size_diff};};
template<typename T> struct xv_helper<T,0>{enum {value = xv_alignof<T>::value};};
template<typename T> struct xv_alignof{struct Big {T x; char c;}; enum {
    diff = sizeof(Big) - sizeof(T), value = xv_helper<Big, diff>::value};};
#define MMX_ALIGNOF(t) (xv_alignof<t>::value);
#else
#define MMX_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MMX_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MMX_PTR_TO_UINT(x) ((mmx_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MMX_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MMX_PTR_TO_UINT(x) ((mmx_size)(((char*)x)-(char*)0))
#elif defined(MMX_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MMX_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MMX_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MMX_UINT_TO_PTR(x) ((void*)(x))
# define MMX_PTR_TO_UINT(x) ((mmx_size)(x))
#endif

#define MMX_MIN(a,b) (((a)<(b))?(a):(b))
#define MMX_MAX(a,b) (((a)>(b))?(a):(b))
#define MMX_CLAMP(a,v, b) MMX_MIN(b, MMX_MAX(a,v))
#define MMX_PTR_SUB(t, p, i) ((t*)((void*)((mmx_byte*)(p) - (i))))
#define MMX_ALIGN_PTR(x, mask)\
    (MMX_UINT_TO_PTR((MMX_PTR_TO_UINT((mmx_byte*)(x) + (mask-1)) & MMX_PTR_TO_UINT(~(mask-1)))))
#define MMX_ALIGN_PTR_BACK(x, mask)\
    (MMX_UINT_TO_PTR((MMX_PTR_TO_UINT((mmx_byte*)(x)) & ~(mask-1))))


#ifndef MMX_SIN
#define MMX_SIN sin
#endif

#ifndef MMX_FABS
#define MMX_FABS fabs
#endif

#ifndef MMX_COS
#define MMX_COS cos
#endif

#ifndef MMX_TAN
#define MMX_TAN tan
#endif

#ifndef MMX_ACOS
#define MMX_ACOS acos
#endif

#ifndef MMX_ATAN2
#define MMX_ATAN2 atan2
#endif

#ifndef MMX_PI
#define MMX_PI 3.141592654f
#endif

#ifndef MMX_MATRIX_EPISLON
#define MMX_MATRIX_EPISLON 1e-6
#endif

#ifndef MMX_MATRIX_INVERSE_EPISLON
#define MMX_MATRIX_INVERSE_EPISLON 1e-14
#endif

#ifndef MMX_MEMSET
#define MMX_MEMSET xv_memset
#endif

#ifndef MMX_MEMCPY
#define MMX_MEMCPY xv_memcpy
#endif

/* ---------------------------------------------------------------
 *                          UTIL
 * ---------------------------------------------------------------*/
MMX_API float
xv_inv_sqrt(float number)
{
    float x2;
    const float threehalfs = 1.5f;
    union {mmx_uint i; float f;} conv;
    conv.f = number;
    x2 = number * 0.5f;
    conv.i = 0x5f375A84 - (conv.i >> 1);
    conv.f = conv.f * (threehalfs - (x2 * conv.f * conv.f));
    return conv.f;
}

static void*
xv_memcpy(void *dst0, const void *src0, mmx_size length)
{
    mmx_ptr t;
    typedef int word;
    char *dst = dst0;
    const char *src = src0;
    if (length == 0 || dst == src)
        goto done;

    #define wsize sizeof(word)
    #define wmask (wsize-1)
    #define TLOOP(s) if (t) TLOOP1(s)
    #define TLOOP1(s) do { s; } while (--t)

    if (dst < src) {
        t = (mmx_ptr)src; /* only need low bits */
        if ((t | (mmx_ptr)dst) & wmask) {
            if ((t ^ (mmx_ptr)dst) & wmask || length < wsize)
                t = length;
            else
                t = wsize - (t & wmask);
            length -= t;
            TLOOP1(*dst++ = *src++);
        }
        t = length / wsize;
        TLOOP(*(word*)(void*)dst = *(const word*)(const void*)src; src += wsize; dst += wsize);
        t = length & wmask;
        TLOOP(*dst++ = *src++);
    } else {
        src += length;
        dst += length;
        t = (mmx_ptr)src;
        if ((t | (mmx_ptr)dst) & wmask) {
            if ((t ^ (mmx_ptr)dst) & wmask || length <= wsize)
                t = length;
            else
                t &= wmask;
            length -= t;
            TLOOP1(*--dst = *--src);
        }
        t = length / wsize;
        TLOOP(src -= wsize; dst -= wsize; *(word*)(void*)dst = *(const word*)(const void*)src);
        t = length & wmask;
        TLOOP(*--dst = *--src);
    }
    #undef wsize
    #undef wmask
    #undef TLOOP
    #undef TLOOP1
done:
    return (dst0);
}


MMX_INTERN void
xv_memset(void *ptr, int c0, unsigned long size)
{
    #define word unsigned
    #define wsize sizeof(word)
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned int c = 0;
    unsigned long t = 0;

    if ((c = (unsigned char)c0) != 0) {
        c = (c << 8) | c; /* at least 16-bits  */
        if (sizeof(unsigned int) > 2)
            c = (c << 16) | c; /* at least 32-bits*/
        if (sizeof(unsigned int) > 4)
            c = (c << 32) | c; /* at least 64-bits*/
    }

    dst = (unsigned char*)ptr;
    if (size < 3 * wsize) {
        while (size--) *dst++ = (unsigned char)c0;
        return;
    }

    if ((t = MMX_PTR_TO_UINT(dst) & wmask) != 0) {
        t = wsize -t;
        size -= t;
        do {
            *dst++ = (unsigned char)c0;
        } while (--t != 0);
    }

    t = size / wsize;
    do {
        *(word*)((void*)dst) = c;
        dst += wsize;
    } while (--t != 0);

    t = (size & wmask);
    if (t != 0) {
        do {
            *dst++ = (unsigned char)c0;
        } while (--t != 0);
    }

    #undef word
    #undef wsize
    #undef wmask
}

#define xv_zero_struct(s) xv_zero_size(&s, sizeof(s))
#define xv_zero_array(p,n) xv_zero_size(p, (n) * sizeof((p)[0]))
MMX_INTERN void
xv_zero_size(void *ptr, mmx_size size)
{
    MMX_MEMSET(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *                          VECTOR
 * ---------------------------------------------------------------*/
MMX_API float
xv3_angle(float *axis, const float *a, const float *b)
{
    float d;
    xv_cross(axis, a, b, 3);
    xv_normeq(axis, 3);
    d = xv_dot(a, b, 3);
    return (float)MMX_ACOS(MMX_CLAMP(-1.0f, d, 1.0f));
}

MMX_API void
xv3_slerp(float *r, const float *a, float t, const float *b)
{
    float t0[3], t1[3];
    float omega, cosom, sinom, scale0, scale1;
    if (t <= 0.0f) {
        r[0] = a[0];
        r[1] = a[1];
        r[2] = a[2];
        return;
    } else if (t >= 1.0f) {
        r[0] = b[0];
        r[1] = b[1];
        r[2] = b[2];
        return;
    }

    cosom = xv_dot(a,b,3);
    if ((1.0f - cosom) > 1e-6) {
        omega = (float)MMX_ACOS(cosom);
        sinom = (float)MMX_SIN(omega);
        scale0 = (float)MMX_SIN((1.0f - t) * omega)/sinom;
        scale1 = (float)MMX_SIN(t*omega)/sinom;
    } else {
        scale0 = 1.0f - t;
        scale1 = t;
    }
    xv_muli(t0, a, scale0, 3);
    xv_muli(t1, b, scale1, 3);
    xv_add(r, t0, t1, 3);
}

MMX_API void
xv3_project_to_sphere(float *r, const float *v, float radius)
{
    float rsqr = radius * radius;
    float len = xv_len(v, 3);
    r[0] = v[0]; r[1] = v[1];
    if (len < rsqr *0.5f)
        r[2] = (float)MMX_SQRT(rsqr - len);
    else r[3] = rsqr / (2.0f * (float)MMX_SQRT(len));
}

MMX_API void
xv3_project_to_plane(float *r, const float *v, const float *normal, float over_bounce)
{
    float t[3];
    float backoff = xv_dot(v, normal, 3);
    if (over_bounce != 1.0f) {
        if (backoff < 0.0f)
            backoff *= over_bounce;
        else backoff /= over_bounce;
    }
    xv_muli(t, normal, backoff, 3);
    xv_sub(r, v, t, 3);
}

MMX_API int
xv3_project_along_plane(float *r, const float *v, const float *normal,
    const float epsilon, float over_bounce)
{
    float len, temp;
    float t[3], cross[3];

    xv_cross(t, v, normal, 3);
    xv_cross(cross, t, v, 3);
    xv_normeq(cross, 3);

    len = xv_dot(normal, cross, 3);
    len = (len < 0.0f) ? -len : len;
    if (len < epsilon)
        return 0;

    temp = (over_bounce * xv_dot(normal, v, 3)) / len;
    xv_mulieq(cross, temp, 3);
    xv_sub(r, v, cross, 3);
    return 1;
}


/* ---------------------------------------------------------------
 *                          Matrix
 * ---------------------------------------------------------------*/
MMX_API void
xm2_identity(float *m)
{
    #define M(col, row) m[(col<<1)+row]
    M(0,0) = 1.0f; M(0,1) = 0.0f;
    M(0,0) = 0.0f; M(0,1) = 1.0f;
    #undef M
}

MMX_API void
xm2_transpose(float *m)
{
    #define M(col, row) m[(col<<1)+row]
    float temp = M(0,1);
    M(0,1) = M(1,0);
    M(1,0) = temp;
    #undef M
}

MMX_API void
xm2_mul(float *product, const float *a, const float *b)
{
    #define A(col, row) a[(col<<1)+row]
    #define B(col, row) b[(col<<1)+row]
    #define P(col, row) product[(col<<1)+row]
    P(0,0) = A(0,0) * B(0,0) + A(0,1) * B(1,0);
    P(0,1) = A(0,0) * B(0,1) + A(0,1) * B(1,1);
    P(1,0) = A(1,0) * B(0,0) + A(1,1) * B(1,0);
    P(1,1) = A(1,0) * B(0,1) + A(1,1) * B(1,1);
    #undef A
    #undef B
    #undef P
}

MMX_API void
xm2_transform(float *r, const float *m, const float *v)
{
    #define X(a) a[0]
    #define Y(a) a[1]
    #define M(col, row) m[(col<<1)+row]
    X(r) = M(0,0)*X(v) + M(0,1)*Y(v);
    Y(r) = M(1,0)*X(v) + M(1,1)*Y(v);
    #undef X
    #undef Y
    #undef M
}

MMX_API void
xm2_rotate(float *m, float angle)
{
    #define M(col, row) m[(col<<1)+row]
    float s = (float)MMX_SIN(MMX_DEG2RAD(angle));
    float c = (float)MMX_COS(MMX_DEG2RAD(angle));
    if (angle >= 0) {
        M(0,0) =  c; M(0,1) = s;
        M(1,0) = -s; M(1,1) = c;
    } else {
        M(0,0) =  c; M(0,1) = -s;
        M(1,0) =  s; M(1,1) =  c;
    }
    #undef M
}

MMX_API void
xm2_scale(float *m, float x, float y)
{
    #define M(col, row) m[(col<<1)+row]
    M(0,0) = x; M(0,1) = 0;
    M(0,0) = 0; M(0,1) = y;
    #undef M
}

MMX_API float
xm2_determinant(const float *m)
{
    #define M(col, row) m[(col<<1)+row]
    return M(0,0) * M(1,1) - M(0,1) * M(1,0);
    #undef M
}

MMX_API int
xm2_inverse_self(float *m)
{
    #define M(col, row) m[(col<<1)+row]
    float det, inv_det, a;
    det = M(0,0) * M(1,1) - M(0,1) * M(1,0);
    if (MMX_FABS(det) < MMX_MATRIX_INVERSE_EPISLON)
        return 0;

    inv_det = 1.0f/det;
    a = M(0,0);
    M(0,0) = M(1,1) * inv_det;
    M(0,1) = -M(0,1) * inv_det;
    M(1,0) = -M(1,0) * inv_det;
    M(1,1) = a * inv_det;
    #undef M
    return 1;
}

MMX_API int
xm2_inverse(float *r, const float *m)
{
    MMX_MEMCPY(r, m, sizeof(float) * 4);
    return xm2_inverse_self(r);
}

MMX_API void
xm3_identity(float *m)
{
    #define M(col, row) m[(col*3)+row]
    M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f;
    M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f;
    M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f;
    #undef M
}

MMX_API void
xm3_transpose(float *m)
{
    int i, j;
    #define M(col, row) m[(col*3)+row]
    for (j = 0; j < 3; ++j) {
        for (i = j+1; i < 3; ++i) {
            float t = M(i,j);
            M(i,j) = M(j,i);
            M(j,i) = t;
        }
    }
    #undef M
}

MMX_API void
xm3_rotate_x(float *m, float angle)
{
    float s = (float)MMX_SIN(MMX_DEG2RAD(angle));
    float c = (float)MMX_COS(MMX_DEG2RAD(angle));
    #define M(col, row) m[(col*3)+row]
    M(0,0) = 1; M(0,1) = 0; M(0,2) = 0;
    M(1,0) = 0; M(1,1) = c; M(1,2) =-s;
    M(2,0) = 0; M(2,1) = s; M(2,2) = c;
    #undef M
}

MMX_API void
xm3_rotate_y(float *m, float angle)
{
    float s = (float)MMX_SIN(MMX_DEG2RAD(angle));
    float c = (float)MMX_COS(MMX_DEG2RAD(angle));
    #define M(col, row) m[(col*3)+row]
    M(0,0) = c; M(0,1) = 0; M(0,2) = s;
    M(1,0) = 0; M(1,1) = 1; M(1,2) = 0;
    M(2,0) =-s; M(2,1) = 0; M(2,2) = c;
    #undef M
}

MMX_API void
xm3_rotate_z(float *m, float angle)
{
    float s = (float)MMX_SIN(MMX_DEG2RAD(angle));
    float c = (float)MMX_COS(MMX_DEG2RAD(angle));
    #define M(col, row) m[(col*3)+row]
    M(0,0) = c; M(0,1) =-s; M(0,2) = 0;
    M(1,0) = s; M(1,1) = c; M(1,2) = 0;
    M(2,0) = 0; M(2,1) = 0; M(2,2) = 1;
    #undef M
}

MMX_API void
xm3_rotate_axis(float *m, int axis, float angle)
{
    switch (axis) {
    case XM_AXIS_X: xm3_rotate_x(m, angle); break;
    case XM_AXIS_Y: xm3_rotate_y(m, angle); break;
    case XM_AXIS_Z: xm3_rotate_z(m, angle); break;
    default: xm3_identity(m); break;
    }
}

MMX_API void
xm3_rotate(float *m, float angle, float X, float Y, float Z)
{
    #define M(col, row) m[(col*3)+row]
    float c = (float)MMX_COS(MMX_DEG2RAD(angle));
    float ci = 1.0f - c;
    float s = (float)MMX_SIN(MMX_DEG2RAD(angle));

    M(0,0) = X*X + (1.0f-X*X)*c;
    M(0,1) = X*Y*ci - Z*s;
    M(0,2) = X*Z*ci + Y*s;

    M(1,0) = X*Y*ci + Z*s;
    M(1,1) = Y*Y+(1.0f-Y*Y)*c;
    M(1,2) = Y*Z*ci -X*s;

    M(2,0) = X*Z*ci - Y*s;
    M(2,1) = Y*Z*ci + X*s;
    M(2,2) = Z*Z+(1.0f - Z*Z)*c;
    #undef M
}

MMX_API void
xm3_rotate_align(float *m, const float *d, const float *z)
{
    #define X v[0]
    #define Y v[1]
    #define Z v[2]
    #define M(col, row) m[(col*3)+row]

    float v[3], c, k;
    xv_cross(v, z, d, 3);
    c = xv_dot(z,d, 3);
    k = 1.0f/(1.0f+c);

    M(0,0) = X * X * k + c; M(0,1) = Y * X * k + Z; M(0,2) = Z * X * k + Y;
    M(1,0) = X * Y * k + Z; M(1,1) = Y * Y * k + c; M(1,2) = Z * Y * k - X;
    M(2,0) = X * Z * k - Y; M(2,1) = Y * Z * k + X; M(2,2) = Z * Z * k + c;

    #undef M
    #undef X
    #undef Y
    #undef Z
}

MMX_API void
xm3_scale(float *m, float x, float y, float z)
{
    #define M(col, row) m[(col*3)+row]
    xv_zero_array(m, 9);
    M(0,0) = x;
    M(1,1) = y;
    M(2,2) = z;
    #undef M
}

MMX_API void
xm3_transform(float *r, const float *m, const float *v)
{
    #define X(a) a[0]
    #define Y(a) a[1]
    #define Z(a) a[2]
    #define M(col, row) m[(col*3)+row]
    X(r) = M(0,0)*X(v) + M(0,1)*Y(v) + M(0,2)*Z(v);
    Y(r) = M(1,0)*X(v) + M(1,1)*Y(v) + M(1,2)*Z(v);
    Z(r) = M(2,0)*X(v) + M(2,1)*Y(v) + M(2,2)*Z(v);
    #undef X
    #undef Y
    #undef Z
    #undef M
}

MMX_API void
xm3_mul(float *product, const float *a, const float *b)
{
    #define A(col, row) a[(col*3)+row]
    #define B(col, row) b[(col*3)+row]
    #define P(col, row) product[(col*3)+row]
    int i;
    for (i = 0; i < 3; ++i) {
        const float ai0 = A(i,0), ai1 = A(i,1), ai2 = A(i,2);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2);
    }
    #undef A
    #undef B
    #undef P
}

MMX_API float
xm3_determinant(const float *m)
{
    #define M(col, row) m[(col<<2)+row]
    float det2_12_01 = M(1,0) * M(2,1) - M(1,1) * M(2,0);
    float det2_12_02 = M(1,0) * M(2,2) - M(1,2) * M(2,0);
    float det2_12_12 = M(1,1) * M(2,2) - M(1,2) * M(2,1);
    return M(0,0) * det2_12_12 - M(0,1) * det2_12_02 + M(0,2) * det2_12_01;
    #undef M
}

MMX_API int
xm3_inverse_self(float *m)
{
    float i[3*3];
    float det, inv_det;
    #define M(col, row) m[(col*3)+row]
    #define I(col, row) i[(col*3)+row]
    I(0,0) = M(1,1) * M(2,2) - M(1,2) * M(2,1);
    I(1,0) = M(1,2) * M(2,0) - M(1,0) * M(2,2);
    I(2,0) = M(1,0) * M(2,1) - M(1,1) * M(2,0);
    det = M(0,0) * I(0,0) + M(0,1) * I(1,0) + M(0,2) * M(2,0);
    if (MMX_FABS(det) < MMX_MATRIX_INVERSE_EPISLON)
        return 0;

    inv_det = 1.0f / det;
    I(0,1) = M(0,2) * M(2,1) - M(0,1) * M(2,2);
    I(0,2) = M(0,1) * M(1,2) - M(0,2) * M(1,1);
    I(1,1) = M(0,0) * M(2,2) - M(0,2) * M(2,0);
    I(1,2) = M(0,2) * M(1,0) - M(0,0) * M(1,2);
    I(2,1) = M(0,1) * M(2,0) - M(0,0) * M(2,1);
    I(2,2) = M(0,0) * M(1,1) - M(0,1) * M(1,0);

    M(0,0) = I(0,0) * inv_det;
    M(0,1) = I(0,1) * inv_det;
    M(0,2) = I(0,2) * inv_det;

    M(1,0) = I(1,0) * inv_det;
    M(1,1) = I(1,1) * inv_det;
    M(1,2) = I(1,2) * inv_det;

    M(2,0) = I(2,0) * inv_det;
    M(2,1) = I(2,1) * inv_det;
    M(2,2) = I(2,2) * inv_det;

    #undef I
    #undef M
    return 1;
}

MMX_API int
xm3_inverse(float *r, const float *m)
{
    MMX_MEMCPY(r, m, sizeof(float) * 9);
    return xm3_inverse_self(r);
}

MMX_API void
xm3_from_quat(float *m, const float *q)
{
    #define M(col, row) m[(col*3)+row]
    float qx = q[0], qy = q[1], qz = q[2], qw = q[3];
    float wx, wy, wz;
    float xx, yy, yz;
    float xy, xz, zz;
    float x2, y2, z2;

    x2 = qx + qx;
    y2 = qy + qy;
    z2 = qz + qz;

    xx = qx * x2;
    xy = qx * y2;
    xz = qx * z2;

    yy = qy * y2;
    yz = qy * z2;
    zz = qz * z2;

    wx = qw * x2;
    wy = qw * y2;
    wz = qw * z2;

    M(0,0) = 1.0f - (yy + zz);
    M(0,1) = xy - wz;
    M(0,2) = xz + wy;

    M(1,0) = xy + wz;
    M(1,1) = 1.0f - (xx + zz);
    M(1,2) = yz - wx;

    M(2,0) = xz - wy;
    M(2,1) = yz + wx;
    M(2,2) = 1.0f - (xx + yy);
    #undef M
}

MMX_API void
xm3_from_mat4(float *r, const float *m)
{
    #define T(col, row) m[(col<<2)+row]
    #define M(col, row) r[(col*3)+row]
    M(0,0) = T(0,0); M(0,1) = T(0,1); M(0,2) = T(0,2); M(0,3) = 0;
    M(1,0) = T(1,0); M(1,1) = T(1,1); M(1,2) = T(1,2); M(1,3) = 0;
    M(2,0) = T(2,0); M(2,1) = T(2,1); M(2,2) = T(2,2); M(2,3) = 0;
    #undef M
    #undef T
}

MMX_API void
xm4_identity(float *m)
{
    #define M(col, row) m[(col<<2)+row]
    M(0,0) = 1; M(0,1) = 0; M(0,2) = 0; M(0,3) = 0;
    M(1,0) = 0; M(1,1) = 1; M(1,2) = 0; M(1,3) = 0;
    M(2,0) = 0; M(2,1) = 0; M(2,2) = 1; M(2,3) = 0;
    M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
    #undef M
}

MMX_API void
xm4_transpose(float *m)
{
    int i, j;
    #define M(col, row) m[(col<<2)+row]
    for (j = 0; j < 4; ++j) {
        for (i = j+1; i < 4; ++i) {
            float t = M(i,j);
            M(i,j) = M(j,i);
            M(j,i) = t;
        }
    }
    #undef M
}

MMX_API void
xm4_mul(float *product, const float *a, const float *b)
{
    #define A(col, row) a[(col << 2)+row]
    #define B(col, row) b[(col << 2)+row]
    #define P(col, row) product[(col << 2)+row]
    int i;
    for (i = 0; i < 4; ++i) {
        const float ai0 = A(i,0), ai1 = A(i,1), ai2 = A(i,2), ai3 = A(i,3);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
        P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
    #undef A
    #undef B
    #undef P
}

MMX_API float
xm4_determinant(const float *m)
{
    #define M(col, row) m[(col<<2)+row]
    float det2_01_01 = M(0,0) * M(1,1) - M(0,1) * M(1,0);
    float det2_01_02 = M(0,0) * M(1,2) - M(0,2) * M(1,0);
    float det2_01_03 = M(0,0) * M(1,3) - M(0,3) * M(1,0);
    float det2_01_12 = M(0,1) * M(1,2) - M(0,2) * M(1,1);
    float det2_01_13 = M(0,1) * M(1,3) - M(0,3) * M(1,1);
    float det2_01_23 = M(0,2) * M(1,3) - M(0,3) * M(1,2);

    float det3_201_012 = M(2,0) * det2_01_12 - M(2,1) * det2_01_02 + M(2,2) * det2_01_01;
    float det3_201_013 = M(2,0) * det2_01_13 - M(2,1) * det2_01_03 + M(2,3) * det2_01_01;
    float det3_201_023 = M(2,0) * det2_01_23 - M(2,2) * det2_01_03 + M(2,3) * det2_01_02;
    float det3_201_123 = M(2,1) * det2_01_23 - M(2,2) * det2_01_13 + M(2,3) * det2_01_12;
    return (-det3_201_123 * M (3,0) + det3_201_023 * M(3,1) - det3_201_013* M(3,2) + det3_201_012 * M(3,3));
    #undef M
}

MMX_API int
xm4_inverse_self(float *m)
{
    #define M(col, row) m[(col<<2)+row]
    float det, inv_det;

    float det2_03_01, det2_03_02, det2_03_03, det2_03_12, det2_03_13,det2_03_23;
    float det2_13_01, det2_13_02, det2_13_03, det2_13_12, det2_13_13, det2_13_23;
    float det3_203_012, det3_203_013, det3_203_023, det3_203_123;
    float det3_213_012, det3_213_013, det3_213_023, det3_213_123;
    float det3_301_012, det3_301_013, det3_301_023, det3_301_123;

    float det2_01_01 = M(0,0) * M(1,1) - M(0,1) * M(1,0);
    float det2_01_02 = M(0,0) * M(1,2) - M(0,2) * M(1,0);
    float det2_01_03 = M(0,0) * M(1,3) - M(0,3) * M(1,0);
    float det2_01_12 = M(0,1) * M(1,2) - M(0,2) * M(1,1);
    float det2_01_13 = M(0,1) * M(1,3) - M(0,3) * M(1,1);
    float det2_01_23 = M(0,2) * M(1,3) - M(0,3) * M(1,2);

    float det3_201_012 = M(2,0) * det2_01_12 - M(2,1) * det2_01_02 + M(2,2) * det2_01_01;
    float det3_201_013 = M(2,0) * det2_01_13 - M(2,1) * det2_01_03 + M(2,3) * det2_01_01;
    float det3_201_023 = M(2,0) * det2_01_23 - M(2,2) * det2_01_03 + M(2,3) * det2_01_02;
    float det3_201_123 = M(2,1) * det2_01_23 - M(2,2) * det2_01_13 + M(2,3) * det2_01_12;
    det = (-det3_201_123 * M(3,0) + det3_201_023 * M(3,1) - det3_201_013 * M(3,2) + det3_201_012 * M(3,3));
    if (MMX_FABS(det) < MMX_MATRIX_INVERSE_EPISLON)
        return 0;

    inv_det = 1.0f / det;
    det2_03_01 = M(0,0) * M(3,1) - M(0,1) * M(3,0);
    det2_03_02 = M(0,0) * M(3,2) - M(0,2) * M(3,0);
    det2_03_03 = M(0,0) * M(3,3) - M(0,3) * M(3,0);
    det2_03_12 = M(0,1) * M(3,2) - M(0,2) * M(3,1);
    det2_03_13 = M(0,1) * M(3,3) - M(0,3) * M(3,1);
    det2_03_23 = M(0,2) * M(3,3) - M(0,3) * M(3,2);

    det2_13_01 = M(1,0) * M(3,1) - M(1,1) * M(3,0);
    det2_13_02 = M(1,0) * M(3,2) - M(1,2) * M(3,0);
    det2_13_03 = M(1,0) * M(3,3) - M(1,3) * M(3,0);
    det2_13_12 = M(1,1) * M(3,2) - M(1,2) * M(3,1);
    det2_13_13 = M(1,1) * M(3,3) - M(1,3) * M(3,1);
    det2_13_23 = M(1,2) * M(3,3) - M(1,3) * M(3,2);

    det3_203_012 = M(2,0) * det2_03_12 - M(2,1) * det2_03_02 + M(2,2) * det2_03_01;
    det3_203_013 = M(2,0) * det2_03_13 - M(2,1) * det2_03_03 + M(2,3) * det2_03_01;
    det3_203_023 = M(2,0) * det2_03_23 - M(2,2) * det2_03_03 + M(2,3) * det2_03_02;
    det3_203_123 = M(2,1) * det2_03_23 - M(2,2) * det2_03_13 + M(2,3) * det2_03_12;

    det3_213_012 = M(2,0) * det2_13_12 - M(2,1) * det2_13_02 + M(2,2) * det2_13_01;
    det3_213_013 = M(2,0) * det2_13_13 - M(2,1) * det2_13_03 + M(2,3) * det2_13_01;
    det3_213_023 = M(2,0) * det2_13_23 - M(2,2) * det2_13_03 + M(2,3) * det2_13_02;
    det3_213_123 = M(2,1) * det2_13_23 - M(2,2) * det2_13_13 + M(2,3) * det2_13_12;

    det3_301_012 = M(3,0) * det2_01_12 - M(3,1) * det2_01_02 + M(3,2) * det2_01_01;
    det3_301_013 = M(3,0) * det2_01_13 - M(3,1) * det2_01_03 + M(3,3) * det2_01_01;
    det3_301_023 = M(3,0) * det2_01_23 - M(3,2) * det2_01_03 + M(3,3) * det2_01_02;
    det3_301_123 = M(3,1) * det2_01_23 - M(3,2) * det2_01_13 + M(3,3) * det2_01_12;

    M(0,0) = - det3_213_123 * inv_det;
    M(1,0) = + det3_213_023 * inv_det;
    M(2,0) = - det3_213_013 * inv_det;
    M(3,0) = + det3_213_012 * inv_det;

    M(0,1) = + det3_203_123 * inv_det;
    M(1,1) = - det3_203_023 * inv_det;
    M(2,1) = + det3_203_013 * inv_det;
    M(3,1) = - det3_203_012 * inv_det;

    M(0,2) = + det3_301_123 * inv_det;
    M(1,2) = - det3_301_023 * inv_det;
    M(2,2) = + det3_301_013 * inv_det;
    M(3,2) = - det3_301_012 * inv_det;

    M(0,3) = - det3_201_123 * inv_det;
    M(1,3) = + det3_201_023 * inv_det;
    M(2,3) = - det3_201_013 * inv_det;
    M(3,3) = + det3_201_012 * inv_det;
    #undef M
    return 1;
}

MMX_API int
xm4_inverse(float *r, const float *m)
{
    MMX_MEMCPY(r, m, sizeof(float) * 16);
    return xm4_inverse_self(r);
}

MMX_API void
xm4_translate(float *m, float x, float y, float z)
{
    #define M(col, row) m[(col<<2)+row]
    xv_zero_array(m, 16);
    M(0,0) = 1.0f;
    M(1,1) = 1.0f;
    M(2,2) = 1.0f;
    M(3,0) = x;
    M(3,1) = y;
    M(3,2) = z;
    M(3,3) = 1.0f;
    #undef M
}

MMX_API void
xm4_rotate(float *m, float angle, float X, float Y, float Z)
{
    float t[9];
    xm3_rotate(t, angle, X, Y, Z);
    xm4_from_mat3(m, t);
}

MMX_API void
xm4_rotate_x(float *m, float angle)
{
    float t[9];
    xm3_rotate_x(t, angle);
    xm4_from_mat3(m, t);
}

MMX_API void
xm4_rotate_y(float *m, float angle)
{
    float t[9];
    xm3_rotate_y(t, angle);
    xm4_from_mat3(m, t);
}

MMX_API void
xm4_rotate_z(float *m, float angle)
{
    float t[9];
    xm3_rotate_z(t, angle);
    xm4_from_mat3(m, t);
}

MMX_API void
xm4_rotate_axis(float *m, int axis, float angle)
{
    float t[9];
    xm3_rotate_axis(t, axis, angle);
    xm4_from_mat3(m, t);
}

MMX_API void
xm4_orthographic(float *m, float left, float right, float bottom, float top,
    float near, float far)
{
    #define M(col, row) m[(col<<2)+row]
    xv_zero_array(m, 16);
    M(0,0) = 2.0f/(right-left);
    M(1,1) = 2.0f/(top-bottom);
    M(2,2) = -2.0f/(far - near);
    M(3,0) = -(right+left)/(right-left);
    M(3,1) = -(top+bottom)/(top-bottom);
    M(3,2) = -(far+near)/(far-near);
    M(3,3) = 1.0f;
    #undef M
}

MMX_API void
xm4_ortho(float *m, float left, float right, float bottom, float top)
{
    #define M(col, row) m[(col<<2)+row]
    xv_zero_array(m, 16);
    M(0,0) = 2.0f/(right-left);
    M(1,1) = 2.0f/(top-bottom);
    M(2,2) = -1.0f;
    M(3,0) = -(right+left)/(right-left);
    M(3,1) = -(top+bottom)/(top-bottom);
    M(3,3) = 1.0f;
    #undef M
}

MMX_API void
xm4_persp(float *m, float fov, float aspect, float near, float far)
{
    #define M(col, row) m[(col<<2)+row]
    const float rad = MMX_DEG2RAD(fov);
    const float hfov = (float)MMX_TAN(rad/2.0f);
    xv_zero_array(m, 16);
    M(0,0) = 1.0f / (aspect * hfov);
    M(1,1) = 1.0f / hfov;
    M(2,2) = -(far + near) / (far - near);
    M(2,3) = -1.0f;
    M(3,2) = -(2.0f * far * near) / (far - near);
    #undef M
}

MMX_API void
xm4_frustum(float *m, float left, float right, float buttom, float top,
    float near, float far)
{
    #define M(col, row) m[(col<<2)+row]
    xv_zero_array(m, 16);
    M(0,0) = (2.0f * near) / (right -left);
    M(1,1) = (2.0f * near) / (top - buttom);
    M(2,0) = (right + left) / (right - left);
    M(2,1) = (top + buttom) / (top - buttom);
    M(2,2) = -(far + near) / (far - near);
    M(2,3) = -1.0f;
    M(3,2) = -(2.0f*far*near)/(far-near);
    #undef M
}

MMX_API void
xm4_lookat(float *m, const float *eye, const float *center, const float *up)
{
    float f[3], s[3], u[3];
    xv_sub(f, center, eye, 3); xv_normeq(f,3);
    xv_cross(s, f, up, 3); xv_normeq(s,3);
    xv_cross(u, s, f, 3);

    #define M(col, row) m[(col<<2)+row]
    xv_zero_array(m, 16);
    M(0,0) = s[0],  M(1,0) = s[1],  M(2,0) = s[2];
    M(0,1) = u[0],  M(1,1) = u[1],  M(2,1) = u[2];
    M(0,2) = -f[0], M(1,2) = -f[1], M(2,2) = -f[2];
    M(3,0) = -xv_dot(s, eye, 3);
    M(3,1) = -xv_dot(u, eye, 3);
    M(3,2) = xv_dot(f, eye, 3);
    M(3,3) = 1.0f;
    #undef M
}

MMX_API void
xm4_transform(float *r, const float *m, const float *v)
{
    #define X(a) a[0]
    #define Y(a) a[1]
    #define Z(a) a[2]
    #define W(a) a[3]

    #define M(col, row) m[(col<<2)+row]
    X(r) = M(0,0)*X(v) + M(0,1)*Y(v) + M(0,2)*Z(v) + M(0,3)*W(v);
    Y(r) = M(1,0)*X(v) + M(1,1)*Y(v) + M(1,2)*Z(v) + M(1,3)*W(v);
    Z(r) = M(2,0)*X(v) + M(2,1)*Y(v) + M(2,2)*Z(v) + M(2,3)*W(v);
    W(r) = M(3,0)*X(v) + M(3,1)*Y(v) + M(3,2)*Z(v) + M(3,3)*W(v);

    #undef X
    #undef Y
    #undef Z
    #undef W
    #undef M
}

MMX_API void
xm4_from_quat(float *m, const float *q)
{
    float t[3*3];
    xm3_from_quat(t, q);
    #define M(col, row) m[(col<<2)+row]
    #define T(col, row) t[(col*3)+row]
    M(0,0) = T(0,0); M(0,1) = T(0,1); M(0,2) = T(0,2); M(0,3) = 0;
    M(1,0) = T(1,0); M(1,1) = T(1,1); M(1,2) = T(1,2); M(1,3) = 0;
    M(2,0) = T(2,0); M(2,1) = T(2,1); M(2,2) = T(2,2); M(2,3) = 0;
    M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
    #undef M
    #undef T
}

MMX_API void
xm4_from_quat_vec(float *m, const float *q, const float *p)
{
    #define M(col, row) m[(col<<2)+row]
    xm4_from_quat(m, q);
    M(3,0) = p[0]; M(3,1) = p[1]; M(3,2) = p[2]; M(3,3) = 1;
    #undef M
}

MMX_API void
xm4_from_mat3(float *r, const float *m)
{
    #define M(col, row) r[(col<<2)+row]
    #define T(col, row) m[(col*3)+row]
    M(0,0) = T(0,0); M(0,1) = T(0,1); M(0,2) = T(0,2); M(0,3) = 0;
    M(1,0) = T(1,0); M(1,1) = T(1,1); M(1,2) = T(1,2); M(1,3) = 0;
    M(2,0) = T(2,0); M(2,1) = T(2,1); M(2,2) = T(2,2); M(2,3) = 0;
    M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
    #undef M
    #undef T
}

/* ---------------------------------------------------------------
 *                          Quaternion
 * ---------------------------------------------------------------*/
MMX_API void
xq_rotation(float *q, float angle, float x, float y, float z)
{
   float radians = MMX_DEG2RAD(angle);
   float sinThetaDiv2 = (float)MMX_SIN(radians/2.0f);
   q[0] = x * sinThetaDiv2;
   q[1] = y * sinThetaDiv2;
   q[2] = z * sinThetaDiv2;
   q[3] = (float)MMX_COS(radians/2.0f);
}

MMX_API float
xq_get_rotation(float *axis, const float *q)
{
    float angle = (float)MMX_ACOS(q[3]);
    float sine = (float)MMX_SIN(angle);
    if (sine >= 0.00001f) {
        xv_muli(axis, q, 1.0f/sine,3);
        return 2*angle;
    } else {
        float d = xq_len(q);
        if (d > 0.000001) {
            xv_muli(axis, q, 1.0f/d,3);
        } else {
            axis[0] = 1;
            axis[1] = axis[2] = 0;
        }
    }
    return 0;
}

MMX_API void
xq_transform(float *out, const float *q, const float *v)
{
    #define X(a) a[0]
    #define Y(a) a[1]
    #define Z(a) a[2]
    #define W(a) a[3]

    float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
    x2 = X(q) + X(q); y2 = Y(q) + Y(q); z2 = Z(q) + Z(q);
    xx = X(q) * x2; xy = X(q) * y2; xz = X(q) * z2;
    yy = Y(q) * y2; yz = Y(q) * z2; zz = Z(q) * z2;
    wx = W(q) * x2; wy = W(q) * y2; wz = W(q) * z2;

    X(out) = (1.0f - yy - zz) * X(v) + (xy - wz) * Y(v) + (xz + wy) * Z(v);
    Y(out) = (xy + wz) * X(v) + (1.0f - xx - zz) * Y(v) + (yz - wx) * Z(v);
    Z(out) = (xz - wy) * X(v) + (yz + wx) * Y(v) + (1.0f - xx - yy) * Z(v);

    #undef X
    #undef Y
    #undef Z
    #undef W
    #undef M
}

MMX_API float
xq_invert(float *to, const float *from)
{
    float len = 0;
    xq_conjugate(to, from);
    xq_normeq_len(len, to);
    return len;
}

MMX_API float
xq_inverteq(float *q)
{
    float len = 0;
    xq_conjugate(q, q);
    xq_normeq_len(len,q);
    return len;
}

MMX_API void
xq_mul(float *out, const float *q1, const float *q2)
{
    out[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
    out[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
    out[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
    out[3] = q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2];
}

MMX_API void
xq_integrate2D(float *r, const float *q, float *omega, float delta)
{
    float t[4] = {0,0,0,0};
    r[0] = omega[1] * MMX_PI * 0.5f;
    r[1] = omega[2] * MMX_PI * 0.5f;

    xq_mul(t, r, q);
    xv_mulieq(t, (0.5f * delta), 4);
    xv_add(r, q, t, 4);
    xq_normeq(r);
}

MMX_API void
xq_integrate3D(float *r, const float *q, float *omega, float delta)
{
    float magsqr, s;
    float theta[4];
    float t[4] = {0,0,0,0};

    theta[0] = omega[0];
    theta[1] = omega[1];
    theta[2] = omega[2];
    theta[3] = delta * 0.5f;

    magsqr = xv_len2(theta, 4);
    if (((magsqr * magsqr) / 24.0f) < 0.000001) {
        t[3] = 1.0f - magsqr * 0.5f;
        s = 1.0f - magsqr / 6.0f;
    } else {
        float mag = (float)MMX_SQRT(magsqr);
        t[3] = (float)MMX_COS(mag);
        s = (float)MMX_SIN(mag) / mag;
    }
    t[0] = theta[0] * s;
    t[1] = theta[1] * s;
    t[2] = theta[2] * s;
    xq_mul(r, t, q);
}

MMX_API void
xq_from_mat3(float *q, const float *m)
{
    #define M(col, row) m[(col*3)+row]
    float tr, s;
    tr = M(0,0) + M(1,1) + M(2,2);
    if (tr > 0.00001){
        s = (float)MMX_SQRT(tr + 1.0);
        q[0] = s * 0.5f; s = 0.5f / s;
        q[1] = (M(2,1) - M(1,2)) * s;
        q[2] = (M(0,2) - M(2,0)) * s;
        q[3] = (M(1,0) - M(0,1)) * s;
    } else {
        int i, j, k;
        i = 0;
        if (M(1,1) > M(0,0) ) i = 1;
        if (M(2,2) > M(i,i) ) i = 2;
        j = (i + 1) % 3;
        k = (i + 2) % 3;

        s = (float)MMX_SQRT(M(i,i) - (M(j,j) + M(k,k)) + 1.0);
        q[i] = s * 0.5f; if( s != 0.0f ) s = 0.5f / s;
        q[j] = (M(j,i) + M(i,j)) * s;
        q[k] = (M(k,i) + M(i,k)) * s;
        q[3] = (M(k,j) - M(j,k)) * s;
    }
    xq_normeq(q);
    #undef M
}

/* ---------------------------------------------------------------
 *                          COMPLEX
 * ---------------------------------------------------------------*/
MMX_API void
xc_mul(float *d, const float *a, const float *b)
{
    #define R(v) (v)[0]
    #define I(v) (v)[1]
    float r = R(a) * R(b) - I(a) * I(b);
    float i = I(a) * R(b) + R(a) * I(b);
    R(d) = r; I(d) = r;
    #undef R
    #undef I
}

MMX_API void
xc_div(float *d,const float *a, const float *b)
{
    #define R(v) (v)[0]
    #define I(v) (v)[1]
    float s, t, r, i;
    if (MMX_FABS(R(b)) >= MMX_FABS(I(b))) {
        s = I(b) / R(b);
        t = 1.0f / (R(b) + s * I(b));
        r = (R(a) + s * I(a)) * t;
        i = (I(a) - s * R(a)) * t;
    } else {
        s = R(b) / I(b);
        t = 1.0f / (s * R(b) + I(b));
        r = (R(a) *s + I(a)) * t;
        i = (I(a) *s - R(a)) * t;
    }
    R(d) = r;
    I(d) = i;
    #undef R
    #undef I
}

MMX_API void
xc_divf(float *d, float a, const float *b)
{
    #define R(v) (v)[0]
    #define I(v) (v)[1]
    float s, t, r, i;
    if (MMX_FABS(R(b)) >= MMX_FABS(I(b))) {
        s = I(b) / R(b);
        t = a / (R(b) + s * I(b));
        r = t;
        i = -s * t;
    } else {
        s = R(b) / I(b);
        t = a / (s * R(b) + I(b));
        r = s * t;
        i = -t;
    }
    R(d) = r;
    I(d) = i;
    #undef R
    #undef I
}

MMX_API void
xc_sqrt(float *d, const float *c)
{
    #define R(v) (v)[0]
    #define I(v) (v)[1]
    float x, y, w;
    if (R(c) == 0.0f && I(c) == 0.0f) {
        R(d) = 0.0f;
        I(d) = 0.0f;
        return;
    }
    x = (float)MMX_FABS(R(c));
    y = (float)MMX_FABS(I(c));
    if (x >= y) {
        w = y / x;
        w = (float)MMX_SQRT(x) * (float)MMX_SQRT(0.5f * (1.0f + (float)MMX_SQRT(1.0f + w * w)));
    } else {
        w = x / y;
        w = (float)MMX_SQRT(y) * (float)MMX_SQRT(0.5f * (w + (float)MMX_SQRT(1.0f + w * w)));
    }

    if (w == 0.0f) {
        R(d) = 0.0f;
        I(d) = 0.0f;
    } else if (R(c) >= 0.0f) {
        R(d) = w;
        I(d) = 0.5f * I(c) / w;
    } else {
        R(d) = 0.5f * y / w;
        I(d) = (I(c) >= 0.0f) ? w : -w;
    }
    #undef R
    #undef I
}

MMX_API float
xc_abs(const float *a)
{
    #define R(v) (v)[0]
    #define I(v) (v)[1]
    float x,y,t;
    x = (float)MMX_FABS(R(a));
    y = (float)MMX_FABS(I(a));
    if (x == 0.0f)
        return y;
    else if (y == 0.0f)
        return x;
    else if (x > y) {
        t = y / x;
        return x * (float)MMX_SQRT(1.0f + t * t);
    } else {
        t = x / y;
        return y * (float)MMX_SQRT(1.0f + t * t);
    }
    #undef R
    #undef I
}

MMX_API void
xc_reciprocal(float *d, const float *c)
{
    #define R(v) (v)[0]
    #define I(v) (v)[1]
    float s, t;
    if (MMX_FABS(R(c)) >= MMX_FABS(I(c))) {
        s = I(c) / R(c);
        t = 1.0f / (R(c) + s * I(c));
        R(d) = t;
        I(d) = -s * t;
    } else {
        s = R(c) / I(c);
        t = 1.0f / (s * R(c) + I(c));
        R(d) = s * t;
        I(d) = -t;
    }
    #undef R
    #undef I
}

/* ---------------------------------------------------------------
 *                          PLANE
 * ---------------------------------------------------------------*/
MMX_API void
xp_make(float *p, const float *normal, float distance)
{
    p[0] = normal[0];
    p[1] = normal[1];
    p[2] = normal[2];
    p[3] = -distance;
}

MMX_API int
xp_from_points(float *p, const float *p1, const float *p2, const float *p3)
{
    float t0[3], t1[3];
    xv_sub(t0, p1, p2, 3);
    xv_sub(t1, p3, p2, 3);
    xv_cross(p, t0, t1, 3);
    if (xp_norm_self(p) == 0.0f)
        return 0;
    p[3] = -xv_dot(p, p2, 3);
    return 1;
}

MMX_API int
xp_from_vec(float *r, const float *dir1, const float *dir2, const float *p)
{
    float t0[3];
    xv_cross(r, dir1, dir2, 3);
    if (xp_norm_self(r) == 0.0f)
        return 0;
    r[3] = -xv_dot(r, p, 3);
    return 1;
}

MMX_API void
xp_translate(float *r, const float *plane, const float *translation)
{
    r[0] = plane[0];
    r[1] = plane[1];
    r[2] = plane[2];
    r[3] = plane[3] - xv_dot(translation, plane, 3);
}

MMX_API void
xp_translateq(float *r, const float *translation)
{
    r[3] -= xv_dot(translation, r, 3);
}

MMX_API void
xp_rotate(float *r, const float *plane, const float *origin, const float *axis)
{
    xm3_transform(r, axis, plane);
    r[3] = plane[3] + xv_dot(origin, plane,3) - xv_dot(origin, r, 3);
}

MMX_API void
xp_rotate_self(float *r, const float *origin, const float *axis)
{
    float t[3];
    r[3] += xv_dot(origin, r, 3);
    xm3_transform(t, axis, r);
    r[0] = t[0]; r[1] = t[1]; r[2] = t[2];
    r[3] -= xv_dot(origin, r, 3);
}

MMX_API float
xp_norm(float *r, const float *p)
{
    float len = 0;
    xv_norm_len(len, r, p, 3);
    r[3] = p[3];
    return len;
}

MMX_API float
xp_norm_self(float *r)
{
    float len = 0;
    xv_normeq_len(len, r, 3);
    return len;
}

MMX_API float
xp_distance(const float *p, const float *v3)
{
    return xv_dot(p,v3,3) + p[3];
}

MMX_API int
xp_side(const float *p, const float *v3, float epsilon)
{
    float dist = xp_distance(p, v3);
    if (dist > epsilon)
        return XP_FRONT;
    else if (dist < -epsilon)
        return XP_BACK;
    else return XP_ON;
}

MMX_API int
xp_intersect_line(const float *p, const float *start, const float *end)
{
    float d1, d2, fraction;
    d1 = xv_dot(p, start, 3) + p[3];
    d2 = xv_dot(p, end, 3) + p[3];
    if (d1 == d2) return 0;
    if (d1 > 0.0f && d2 > 0.0f)
        return 0;
    if (d1 < 0.0f && d2 < 0.0f)
        return 0;
    fraction = (d1/(d1-d2));
    return (fraction >= 0.0f && fraction <= 1.0f);
}

MMX_API int
xp_intersect_ray(float *scale, const float *p, const float *start, const float *dir)
{
    float d1, d2;
    d1 = xv_dot(p, start, 3) + p[0];
    d2 = xv_dot(p, dir, 3);
    if (d2 == 0.0f) return 0;
    *scale = -(d1/d2);
    return 1;
}

MMX_API int
xp_intersect_plane(float *start, float *dir, const float *p0, const float *p1)
{
    float t0[3], t1[3];
    float n00, n01, n11, det, invDet, f0, f1;
    n00 = xv_len2(p0, 3);
    n01 = xv_dot(p0, p1, 3);
    n11 = xv_len2(p1, 3);
    det = n00 * n11 - n01 * n01;
    det = (det < 0.0f) ? -det : det;
    if (det < 1e-6f)
        return 0;

    invDet = 1.0f/det;
    f0 = (n01 * p1[3] - n11 * p0[3]) * invDet;
    f1 = (n01 * p0[3] - n00 * p1[3]) * invDet;

    xv_cross(dir, p0, p1, 3);
    xv_muli(t0, p0, f0, 3);
    xv_muli(t1, p1, f1, 3);
    xv_add(start, t0, t1, 3);
    return 1;
}

/* ---------------------------------------------------------------
 *                          SPHERE
 * ---------------------------------------------------------------*/
MMX_API void
xs_make(float *s, const float *origin, float radius)
{
    s[0] = origin[0];
    s[1] = origin[1];
    s[3] = origin[2];
    s[3] = radius;
}

MMX_API int
xs_add_point(float *s, const float *p)
{
    float r, t[3];
    xv_sub(t, p, s, 3);
    r = xv_len2(t, 3);
    if (r > (s[3] * s[3])) {
        r = (float)MMX_SQRT(r);
        xv_mulieq(t, 0.5f * (1.0f - s[3] / r), 3);
        xv_addeq(s, t, 3);
        s[3] += 0.5f * (r - s[3]);
        return 1;
    }
    return 0;
}

MMX_API int
xs_add_sphere(float *s0, const float *s1)
{
    float r;
    float t[3];
    xv_sub(t, s1, s0, 3);
    r = xv_len2(t, 3);
    if (r > ((s0[3] + s1[3]) * (s0[3] + s1[3]))) {
        r = (float)MMX_SQRT(r);
        xv_mulieq(t, 0.5f * (1.0f - s0[3] / (r + s1[3])), 3);
        xv_addeq(s0, t, 3);
        s0[3] += 0.5f * ((r + s1[3]) - s0[3]);
        return 1;
    }
    return 0;
}

MMX_API void
xs_expand(float *r, const float *s, float d)
{
    r[0] = s[0]; r[1] = s[1]; r[2] = s[2]; r[3] = s[3] + d;
}

MMX_API void
xs_expand_self(float *s, float d)
{
    s[3] = s[3] + d;
}

MMX_API void
xs_translate(float *r, const float *s, const float *t)
{
    r[3] = s[3];
    xv_add(r, s, t, 3);
}

MMX_API void
xs_translate_self(float *r, const float *t)
{
    xv_addeq(r, t, 3);
}

MMX_API int
xs_contains_point(const float *s, const float *p)
{
    float t[3];
    xv_sub(t, p, s, 3);
    if (xv_len2(t,3) > (s[3] * s[3]))
        return 0;
    return 1;
}

MMX_API int
xs_intersects_sphere(const float *s1, const float *s2)
{
    float t[3];
    float r = s2[3] * s1[3];
    xv_sub(t, s2, s1, 3);
    if (xv_len2(t,3) > (r * r))
        return 0;
    return 1;
}

MMX_API float
xs_plane_distance(const float *s, const float *p)
{
    float d = xp_distance(p, s);
    if (d > s[3])
        return d - s[3];
    if (d < -s[3])
        return d + s[3];
    return 0.0f;
}

MMX_API int
xs_plane_side(const float *s, const float *p, float epsilon)
{
    float d = xp_distance(p, s);
    if (d > (s[3] + epsilon))
        return XP_FRONT;
    if (d < (-s[3] - epsilon))
        return XP_BACK;
    return XP_CROSS;
}

MMX_API int
xs_intersects_line(const float *sphere, const float *start, const float *end)
{
    float a, x;
    float r[3], s[3], e[3], t[3];

    xv_sub(s, start, sphere, 3);
    xv_sub(e, end, sphere, 3);
    xv_sub(r, e, s, 3);
    a = -xv_dot(s,r,3);
    if (a <= 0)
        return (xv_dot(s,s,3) < (sphere[3] * sphere[3]));
    else if (a >= xv_dot(r,r,3))
        return (xv_dot(e,e,3) < (sphere[3] * sphere[3]));

    x = (a / xv_dot(r,r,3));
    xv_muli(t, r, x, 3);
    xv_add(r, s, t, 3);
    return (xv_dot(r,r, 3) < (sphere[3] * sphere[3]));
}

MMX_API int
xs_intersects_ray(float *scale0, float *scale1, const float *s,
    const float *start, const float *dir)
{
    float p[3];
    float a,b,c,d, sqrtd;

    xv_sub(p, start, s, 3);
    a = xv_dot(dir, dir, 3);
    b = xv_dot(dir, p, 3);
    c = xv_dot(p, p, 3) - (s[3] * s[3]);
    d = b * b - c * a;
    if (d < 0.0f) return 0;

    sqrtd = (float)MMX_SQRT(d);
    a = 1.0f / a;

    *scale0 = (-b * sqrtd) * a;
    *scale1 = (-b * sqrtd) * a;
    return 1;
}

MMX_API void
xs_from_box(float *sphere, const float *box)
{
    float t[3];
    xv_add(sphere, box, &box[3], 3);
    xv_mulieq(sphere, 0.5f, 3);
    xv_sub(t, &box[3], sphere, 3);
    sphere[3] = xv_len(t, 3);
}

/* ---------------------------------------------------------------
 *                          BOX
 * ---------------------------------------------------------------*/
MMX_API void
xb_make(float *b, const float *min, const float *max)
{
    b[0] = min[0]; b[1] = min[1]; b[2] = min[2];
    b[3] = max[0]; b[4] = max[1]; b[5] = max[2];
}

MMX_API void
xb_center(float *center, const float *box)
{
    xv_add(center, box, &box[3], 3);
    xv_mulieq(center, 0.5f, 3);
}

MMX_API void
xb_radius(float *radius, const float *b)
{
    int i;
    float total, b0, b1;
    total = 0.0f;
    for (i = 0; i < 3; ++i) {
        b0 = (float)MMX_FABS(b[i]);
        b1 = (float)MMX_FABS(b[3+i]);
        if (b0 > b1)
            total += b0 * b0;
        else total += b1 * b1;
    }
    *radius = (float)MMX_SQRT(total);
}

MMX_API void
xb_from_points(float *b, const void *verts, int num, int stride, int offset)
{
    int i = 0;
    const float *vert;
    const unsigned char *data = (const unsigned char*)verts;

    data += offset;
    vert = (const float*)(const void*)data;
    xb_make(b, vert, vert);
    data += stride;
    for (i = 0; i < num; ++i) {
        vert = (const float*)(const void*)data;
        b[0] = MMX_MIN(vert[0], b[0]);
        b[1] = MMX_MIN(vert[1], b[1]);
        b[2] = MMX_MIN(vert[2], b[2]);

        b[3] = MMX_MAX(vert[0], b[3]);
        b[4] = MMX_MAX(vert[1], b[4]);
        b[5] = MMX_MAX(vert[2], b[5]);
        data += stride;
    }
}

MMX_API int
xb_add_point(float *b, const float *point)
{
    int expanded = 0;
    if (point[0] < b[0]) {
        b[0] = point[0];
        expanded = 1;
    }
    if (point[0] > b[3]) {
        b[3] = point[0];
        expanded = 1;
    }
    if (point[1] < b[1]) {
        b[1] = point[1];
        expanded = 1;
    }
    if (point[1] > b[4]) {
        b[4] = point[1];
        expanded = 1;
    }
    if (point[2] < b[2]) {
        b[2] = point[2];
        expanded = 1;
    }
    if (point[2] > b[5]) {
        b[5] = point[2];
        expanded = 1;
    }
    return expanded;
}

MMX_API int
xb_add_box(float *b, const float *box)
{
    int expanded = 0;
    if (box[0] < b[0]) {
        b[0] = box[0];
        expanded = 1;
    }
    if (box[1] < b[1]) {
        b[1] = box[1];
        expanded = 1;
    }
    if (box[2] < b[2]) {
        b[2] = box[2];
        expanded = 1;
    }
    if (box[3] > b[3]) {
        b[3] = box[3];
        expanded = 1;
    }
    if (box[4] > b[4]) {
        b[4] = box[4];
        expanded = 1;
    }
    if (box[5] > b[5]) {
        b[5] = box[5];
        expanded = 1;
    }
    return expanded;
}

MMX_API void
xb_intersection(float *r, const float *a, const float *b)
{
    r[0] = (b[0] > a[0]) ? b[0] : a[0];
    r[1] = (b[1] > a[1]) ? b[1] : a[1];
    r[2] = (b[2] > a[2]) ? b[2] : a[2];
    r[3] = (b[3] < a[3]) ? b[3] : a[3];
    r[4] = (b[4] < a[4]) ? b[4] : a[4];
    r[5] = (b[5] < a[5]) ? b[5] : a[5];
}

MMX_API void
xb_intersection_self(float *r, const float *b)
{
    if (b[0] > r[0])
        r[0] = b[0];
    if (b[1] > r[1])
        r[1] = b[1];
    if (b[2] > r[2])
        r[2] = b[2];
    if (b[3] < r[3])
        r[3] = b[3];
    if (b[4] < r[4])
        r[4] = b[4];
    if (b[5] < r[5])
        r[5] = b[5];
}

MMX_API void
xb_expand(float *r, const float *b, float d)
{
    r[0] = b[0] - d;
    r[1] = b[1] - d;
    r[2] = b[2] - d;
    r[3] = b[3] + d;
    r[4] = b[4] + d;
    r[5] = b[5] + d;
}

MMX_API void
xb_expand_self(float *b, float d)
{
    b[0] -= d;
    b[1] -= d;
    b[2] -= d;
    b[3] += d;
    b[4] += d;
    b[5] += d;
}

MMX_API void
xb_translate(float *r, const float *b, const float *t)
{
    xv_add(r, b, t, 3);
    xv_add(&r[3], &b[3], t, 3);
}

MMX_API void
xb_translate_self(float *r, const float *t)
{
    xv_addeq(r, t, 3);
    xv_addeq(&r[3], t, 3);
}

MMX_API void
xb_transform(float *r, const float *box, const float *origin, const float *axis)
{
    int i;
    float t[3];
    float center[3];
    float extents[3];
    float rotExtents[3];

    xv_add(center, box, &box[3], 3);
    xv_mulieq(center, 0.5f, 3);
    xv_sub(extents, &box[3], center, 3);

    #define M(col, row) axis[(col*3)+row]
    rotExtents[0] = (float)(MMX_FABS(extents[0] * M(0,0)) + MMX_FABS(extents[1] * M(1,0)) + MMX_FABS(extents[2] * M(2,0)));
    rotExtents[1] = (float)(MMX_FABS(extents[0] * M(0,1)) + MMX_FABS(extents[1] * M(1,1)) + MMX_FABS(extents[2] * M(2,1)));
    rotExtents[2] = (float)(MMX_FABS(extents[0] * M(0,2)) + MMX_FABS(extents[1] * M(1,2)) + MMX_FABS(extents[2] * M(2,2)));
    #undef M

    xm3_transform(t, axis, center);
    xv_add(center, origin, t, 3);
    xv_sub(r, center, rotExtents, 3);
    xv_add(&r[3], center, rotExtents, 3);
}

MMX_API void
xb_rotate(float *r, const float *b, const float *mat33)
{
    const MMX_STORAGE float origin[] = {0,0,0};
    xb_transform(r, b, origin, mat33);
}

MMX_API void
xb_rotate_self(float *r, const float *mat33)
{
    const MMX_STORAGE float origin[] = {0,0,0};
    xb_transform(r, r, origin, mat33);
}

MMX_API int
xb_contains_point(const float *b, const float *p)
{
    if (p[0] < b[0] || p[1] < b[1] || p[2] < b[2] ||
        p[0] > b[3] || p[1] > b[4] || p[2] > b[5])
        return 0;
    return 1;
}

MMX_API float
xb_plane_distance(const float *box, const float *p)
{
    float center[3];
    float d1,d2;

    xv_add(center, box, &box[3], 3);
    xv_mulieq(center, 0.5f, 3);

    d1 = xp_distance(p, center);
    d2 = (float)(MMX_FABS((box[3] - center[0]) * p[0]) +
        MMX_FABS((box[4] - center[1]) * p[1]) +
        MMX_FABS((box[5] - center[2]) * p[2]));

    if ((d1 - d2) > 0.0f)
        return d1 - d2;
    if ((d1 + d2) < 0.0f)
        return d1 + d2;
    return 0.0f;
}

MMX_API int
xb_plane_side(const float *s, const float *p, float epsilon)
{
    float center[3];
    float d1,d2;

    xv_add(center, s, &s[3], 3);
    xv_mulieq(center, 0.5f, 3);

    d1 = xp_distance(p, center);
    d2 = (float)(MMX_FABS((s[3] - center[0]) * p[0]) +
        MMX_FABS((s[4] - center[1]) * p[1]) +
        MMX_FABS((s[5] - center[2]) * p[2]));

    if ((d1 - d2) > epsilon)
        return XP_FRONT;
    if ((d1 + d2) < 0.0f)
        return XP_BACK;
    return XP_CROSS;
}

MMX_API int
xb_intersects_box(const float *a, const float *b)
{
    if (b[3] < a[0] || b[4] < a[1] || b[5] < a[2] ||
        b[0] > a[3] || b[1] > a[4] || b[2] > a[5])
        return 0;
    return 1;
}

MMX_API int
xb_intersects_line(const float *box, const float *start, const float *end)
{
    float ld[3];
    float center[3];
    float extents[3];
    float line_dir[3];
    float line_center[3];
    float dir[3];
    float cross[3];

    xv_add(center, box, &box[3], 3);
    xv_mulieq(center, 0.5f, 3);
    xv_sub(extents, &box[3], center, 3);

    xv_sub(line_dir, end, start, 3);
    xv_mulieq(line_dir, 0.5f, 3);

    xv_add(line_center, start, line_dir, 3);
    xv_sub(dir, line_center, center, 3);

    ld[0] = (float)MMX_FABS(line_dir[0]);
    if (MMX_FABS(dir[0]) > extents[0] + ld[0])
        return 0;

    ld[1] = (float)MMX_FABS(line_dir[1]);
    if (MMX_FABS(dir[1]) > extents[1] + ld[1])
        return 0;

    ld[2] = (float)MMX_FABS(line_dir[2]);
    if (MMX_FABS(dir[2]) > extents[2] + ld[2])
        return 0;

    xv_cross(cross, line_dir, dir, 3);
    if (MMX_FABS(cross[0]) > extents[1] * ld[2] + extents[2] * ld[1])
        return 0;
    if (MMX_FABS(cross[1]) > extents[0] * ld[2] + extents[2] * ld[0])
        return 0;
    if (MMX_FABS(cross[2]) > extents[0] * ld[1] + extents[1] * ld[0])
        return 0;
    return 1;
}

MMX_API int
xb_intersects_ray(float *scale, const float *b, const float *start, const float *dir)
{
    float f;
    int i, ax0, ax1, ax2, side, inside;
    float hit[3];

    ax0 = -1;
    inside = 0;
    *scale = 0;
    for (i = 0; i < 3; ++i) {
        if (start[i] < b[i])
            side = 0;
        else if (start[i] < b[3+i])
            side = 1;
        else {
            inside++;
            continue;
        }

        if (dir[i] == 0.0f) continue;
        f = (start[i] - b[side*3+i]);
        if (ax0 < 0 || MMX_FABS(f) > MMX_FABS(*scale * dir[i])) {
            *scale = - (f/dir[i]);
            ax0 = i;
        }
    }

    if (ax0 < 0) {
        *scale = 0.0f;
        return (inside == 3);
    }

    ax1 = (ax0 + 1)%3;
    ax2 = (ax0 + 2)%3;
    hit[ax1] = start[ax1] + *scale * dir[ax1];
    hit[ax2] = start[ax2] + *scale * dir[ax2];
    return (hit[ax1] >= b[ax1] && hit[ax1] <= b[3+ax1] &&
            hit[ax2] >= b[ax2] && hit[ax2] <= b[3+ax2]);
}

#endif

