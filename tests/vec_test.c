#define MMX_STATIC
#define MMX_IMPLEMENTATION
#include <math.h>
#include "../mm_vec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define xv_test_section(desc) \
    do { \
        printf("--------------- {%s} ---------------\n", desc);\
    } while (0);

#define xv_test_assert(cond) \
    do { \
        int pass = cond; \
        printf("[%s] %s:%d: ", pass ? "PASS" : "FAIL", __FILE__, __LINE__);\
        printf((strlen(#cond) > 60 ? "%.47s...\n" : "%s\n"), #cond);\
        if (pass) pass_count++; else fail_count++; \
    } while (0)

#define xv_test_vec2(v, a, b)\
    {xv_test_assert(v.x == a);\
    xv_test_assert(v.y == b);}

#define xv_test_vec3(v, a, b, c)\
    {xv_test_assert(v.x == a);\
    xv_test_assert(v.y == b);\
    xv_test_assert(v.z == c);}

#define xv_test_result()\
    do { \
        printf("======================================================\n"); \
        printf("== Result:  %3d Total   %3d Passed      %3d Failed  ==\n", \
                pass_count  + fail_count, pass_count, fail_count); \
        printf("======================================================\n"); \
    } while (0)

struct xvec3 {float x,y,z;};
struct xmat3 {float m[3][3];};

static struct xvec3
XV3(float x, float y, float z)
{
    struct xvec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static int test_vector(void)
{
    int pass_count = 0;
    int fail_count = 0;

    struct xvec3 a, b,c;
    xv_test_section("xv_add")
    {
        a = XV3(2.0f, 3.0f, 4.0f);
        b = XV3(2.0f, 3.0f, 4.0f);
        xv_add(xv(c), xv(a), xv(b), 3);
        xv_test_vec3(c, 4.0f, 6.0f, 8.0f);
    }
    xv_test_section("xv_sub")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 9.0f, 4.0f);
        xv_sub(xv(c), xv(a), xv(b), 3);
        xv_test_vec3(c, 3.0f, -6.0f, -1.0f);
    }
    xv_test_section("xv_addeq")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 9.0f, 4.0f);
        xv_addeq(xv(a), xv(b), 3);
        xv_test_vec3(a, 7.0f, 12.0f, 7.0f);
    }
    xv_test_section("xv_subeq")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 9.0f, 4.0f);
        xv_subeq(xv(a), xv(b), 3);
        xv_test_vec3(a, 3.0f, -6.0f, -1.0f);
    }
    xv_test_section("xv_addi")
    {
        a = XV3(8.0f, 2.0f, 1.0f);
        xv_addi(xv(c), xv(a), 5.0f, 3);
        xv_test_vec3(c, 13.0f, 7.0f, 6.0f);
    }
    xv_test_section("xv_subi")
    {
        a = XV3(8.0f, 2.0f, 1.0f);
        xv_subi(xv(c), xv(a), 7.0f, 3);
        xv_test_vec3(c, 1.0f, -5.0f, -6.0f);
    }
    xv_test_section("xv_muli")
    {
        a = XV3(5.0f, 3.0f, 4.0f);
        xv_muli(xv(c), xv(a), 2.0f, 3);
        xv_test_vec3(c, 10.0f, 6.0f, 8.0f);
    }
    xv_test_section("xv_divi")
    {
        a = XV3(6.0f, 4.0f, 8.0f);
        xv_divi(xv(c), xv(a), 2.0f, 3);
        xv_test_vec3(c, 3.0f, 2.0f, 4.0f);
    }
    xv_test_section("xv_addieq")
    {
        a = XV3(8.0f, 2.0f, 1.0f);
        xv_addieq(xv(a), 5.0f, 3);
        xv_test_vec3(a, 13.0f, 7.0f, 6.0f);
    }
    xv_test_section("xv_subieq")
    {
        a = XV3(8.0f, 2.0f, 1.0f);
        xv_subieq(xv(a), 7.0f, 3);
        xv_test_vec3(a, 1.0f, -5.0f, -6.0f);
    }
    xv_test_section("xv_mulieq")
    {
        a = XV3(5.0f, 3.0f, 4.0f);
        xv_mulieq(xv(a), 2.0f, 3);
        xv_test_vec3(a, 10.0f, 6.0f, 8.0f);
    }
    xv_test_section("xv_divieq")
    {
        a = XV3(4.0f, 8.0f, 10.0f);
        xv_divieq(xv(a), 2.0f, 3);
        xv_test_vec3(a, 2.0f, 4.0f, 5.0f);
    }
    xv_test_section("xv_addm")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_addm(xv(c), xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(c, 14.0f, 10.0f, 8.0f);
    }
    xv_test_section("xv_subm")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_subm(xv(c), xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(c, 6.0f, 2.0f, 4.0f);
    }
    xv_test_section("xv_addmeq")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_addmeq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 14.0f, 10.0f, 8.0f);
    }
    xv_test_section("xv_submeq")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_submeq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 6.0f, 2.0f, 4.0f);
    }
    xv_test_section("xv_addd")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(4.0f, 3.0f, 9.0f);
        xv_addd(xv(c), xv(a), xv(b), 3.0f, 3);
        xv_test_vec3(c, 3.0f, 2.0f, 4.0f);
    }
    xv_test_section("xv_subd")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_subd(xv(c), xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(c, 2.0f, 1.0f, 4.0f);
    }
    xv_test_section("xv_adddeq")
    {
        a = XV3(5.0f, 3.0f, 3.0f);
        b = XV3(5.0f, 5.0f, 5.0f);
        xv_adddeq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 5.0f, 4.0f, 4.0f);
    }
    xv_test_section("xv_subdeq")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_subdeq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 2.0f, 1.0f, 4.0f);
    }
    xv_test_section("xv_adda")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_adda(xv(c), xv(a), xv(b), 4.0f, 3);
        xv_test_vec3(c, 12.0f, 10.0f, 14.0f);
    }
    xv_test_section("xv_suba")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_suba(xv(c), xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(c, 6.0f, 4.0f, 10.0f);
    }
    xv_test_section("xv_addaeq")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_addaeq(xv(a), xv(b), 4.0f, 3);
        xv_test_vec3(a, 12.0f, 10.0f, 14.0f);
    }
    xv_test_section("xv_subaeq")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_subaeq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 6.0f, 4.0f, 10.0f);
    }
    xv_test_section("xv_adds")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_adds(xv(c), xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(c, 6.0f, 4.0f, 8.0f);
    }
    xv_test_section("xv_subs")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_subs(xv(c), xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(c, 2.0f, 0.0f, 6.0f);
    }
    xv_test_section("xv_addseq")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_addseq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 6.0f, 4.0f, 8.0f);
    }
    xv_test_section("xv_subseq")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        b = XV3(2.0f, 2.0f, 1.0f);
        xv_subseq(xv(a), xv(b), 2.0f, 3);
        xv_test_vec3(a, 2.0f, 0.0f, 6.0f);
    }
    xv_test_section("xv_neg")
    {
        a = XV3(6.0f, 4.0f, 9.0f);
        xv_neg(xv(c), xv(a), 3);
        xv_test_vec3(c, -6.0f, -4.0f, -9.0f);
    }
    xv_test_section("xv_dot")
    {
        float dot;
        a = XV3(1.0f, 2.0f, 2.0f);
        b = XV3(3.0f, 4.0f, 2.0f);
        dot = xv_dot(xv(a), xv(b), 3);
        xv_test_assert(dot == 15.0f);
    }
    xv_test_section("xv_len2")
    {
        float len2;
        a = XV3(0.0f, 3.0f, 4.0f);
        len2 = xv_len2(xv(a), 3);
        xv_test_assert(len2 == 25.0f);
    }
    xv_test_section("xv_len")
    {
        float len;
        a = XV3(0.0f, 3.0f, 4.0f);
        len = xv_len(xv(a),3);
        xv_test_assert(len == 5.0f);
    }
    xv_test_section("xv_len")
    {
        a = XV3(0.0f, 0.0f, 0.0f);
        b = XV3(4.0f, 4.0f, 4.0f);
        xv_lerp(xv(c), xv(a), 0.5f, xv(b), 3);
        xv_test_vec3(c, 2.0f, 2.0f, 2.0f);
    }
    xv_test_result();
    return fail_count;
}

static int
test_matrix3(void)
{
    int pass_count = 0;
    int fail_count = 0;

    struct xmat3 a, b, r;
    xv_test_section("xm3_identity")
    {
        xm3_identity(xm(a));
        xv_test_assert(a.m[0][0] == 1.0f);
        xv_test_assert(a.m[0][1] == 0.0f);
        xv_test_assert(a.m[0][2] == 0.0f);
        xv_test_assert(a.m[1][0] == 0.0f);
        xv_test_assert(a.m[1][1] == 1.0f);
        xv_test_assert(a.m[1][2] == 0.0f);
        xv_test_assert(a.m[2][0] == 0.0f);
        xv_test_assert(a.m[2][1] == 0.0f);
        xv_test_assert(a.m[2][2] == 1.0f);
    }
    xv_test_section("xm3_scale")
    {
        xm3_scale(xm(a), 2.0f, 3.0f, 0.5f);
        xv_test_assert(a.m[0][0] == 2.0f);
        xv_test_assert(a.m[0][1] == 0.0f);
        xv_test_assert(a.m[0][2] == 0.0f);
        xv_test_assert(a.m[1][0] == 0.0f);
        xv_test_assert(a.m[1][1] == 3.0f);
        xv_test_assert(a.m[1][2] == 0.0f);
        xv_test_assert(a.m[2][0] == 0.0f);
        xv_test_assert(a.m[2][1] == 0.0f);
        xv_test_assert(a.m[2][2] == 0.5f);
    }

    xv_test_section("xm3_mul identity * identity")
    {
        xm3_identity(xm(a));
        xm3_identity(xm(b));
        xm3_mul(xm(r), xm(a), xm(b));

        xv_test_assert(r.m[0][0] == 1.0f);
        xv_test_assert(r.m[0][1] == 0.0f);
        xv_test_assert(r.m[0][2] == 0.0f);
        xv_test_assert(r.m[1][0] == 0.0f);
        xv_test_assert(r.m[1][1] == 1.0f);
        xv_test_assert(r.m[1][2] == 0.0f);
        xv_test_assert(r.m[2][0] == 0.0f);
        xv_test_assert(r.m[2][1] == 0.0f);
        xv_test_assert(r.m[2][2] == 1.0f);
    }

    xv_test_section("xm3_mul matrix * identity ")
    {
        xm3_identity(xm(a));
        xm3_scale(xm(a), 2.0f, 3.0f, 0.5f);
        xm3_mul(xm(r), xm(a), xm(b));
        xv_test_assert(r.m[0][0] == 2.0f);
        xv_test_assert(r.m[0][1] == 0.0f);
        xv_test_assert(r.m[0][2] == 0.0f);
        xv_test_assert(r.m[1][0] == 0.0f);
        xv_test_assert(r.m[1][1] == 3.0f);
        xv_test_assert(r.m[1][2] == 0.0f);
        xv_test_assert(r.m[2][0] == 0.0f);
        xv_test_assert(r.m[2][1] == 0.0f);
        xv_test_assert(r.m[2][2] == 0.5f);
    }

    xv_test_section("xm3_mul matrix * matrix ")
    {
        a.m[0][0] = 1; a.m[0][1] = 2;
        a.m[0][2] = 3; a.m[1][0] = 3;
        a.m[1][1] = 2; a.m[1][2] = 1;
        a.m[2][0] = 2; a.m[2][1] = 1;
        a.m[2][2] = 3;

        b.m[0][0] = 4; b.m[0][1] = 5;
        b.m[0][2] = 6; b.m[1][0] = 6;
        b.m[1][1] = 5; b.m[1][2] = 4;
        b.m[2][0] = 4; b.m[2][1] = 6;
        b.m[2][2] = 5;
        xm3_mul(xm(r), xm(a), xm(b));

        xv_test_assert(r.m[0][0] == 28.0f);
        xv_test_assert(r.m[0][1] == 33.0f);
        xv_test_assert(r.m[0][2] == 29.0f);
        xv_test_assert(r.m[1][0] == 28.0f);
        xv_test_assert(r.m[1][1] == 31.0f);
        xv_test_assert(r.m[1][2] == 31.0f);
        xv_test_assert(r.m[2][0] == 26.0f);
        xv_test_assert(r.m[2][1] == 33.0f);
        xv_test_assert(r.m[2][2] == 31.0f);
    }

#if 0
    MMX_API void xm3_rotate(float *m, float angle, float X, float Y, float Z);
    MMX_API void xm3_rotate_x(float *m, float angle);
    MMX_API void xm3_rotate_y(float *m, float angle);
    MMX_API void xm3_rotate_z(float *m, float angle);
    MMX_API void xm3_rotate_axis(float *m, int axis, float angle);
    MMX_API void xm3_transpose(float *m);
    MMX_API void xm3_transform(float *r, const float *m, const float *v);
    MMX_API void xm3_rotate_align(float *m, const float *d, const float *z);
    MMX_API void xm3_from_quat(float *m, const float *q);
#endif


    xv_test_result();
    return fail_count;
}

int
main(void)
{
    test_vector();
    test_matrix3();
    return 0;
}

