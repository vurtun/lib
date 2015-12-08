#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "meta.h"

meta_introspect struct test {
    char c; short s;
    int i; long l;
    int *pi; long al[16];
};
meta_introspect enum state {
    STATE_WAITING,
    STATE_RUNNING,
    STATE_HALTED
};
meta_introspect enum punctuation {
    PUNCT_STAR as meta("*"),
    PUNCT_COMMA as meta(","),
    PUNCT_PLUS as meta("+")
};
meta_table(weapon, const char *name; int damage; int weight; int munition) weapons {
    meta_slot(WEAPON_PISTOL, "Pistol"; 2; 10; 200),
    meta_slot(WEAPON_SHOTGUN,  "Shotgun"; 4; 20; 100),
    meta_slot(WEAPON_PLASMA, "Plasma"; 5; 25; 250),
    meta_slot(WEAPON_RAILGUN, "Railgun"; 10; 26; 100),
    meta_slot(WEAPON_MAX, NULL; 0; 0; 0)
};
meta_introspect static int function(int i, char c, long l){return i + c + l;}

#define META_IMPLEMENTATION
#include "meta.h"
int main(void)
{
    {   /* reflect */
        int *i;
        struct test t;
        void *void_ptr = &t;
        t.i = 5;
        fprintf(stdout, "i: %d\n", t.i);
        i = meta_member_ptr_from_name(void_ptr, "test", "i");
        fprintf(stdout, "i: %d\n", *i);
        *i = 10;
        fprintf(stdout, "i: %d\n", *i);
    }
    {   /* enum value from string */
        enum state s = meta_enum_value_from_string("state", "STATE_RUNNING");
        fprintf(stdout, "STATE_RUNNING(%d) == %d\n", STATE_RUNNING, s);
    }
    {   /* String enum */
        const char *star = meta_enum_str(punctuation, PUNCT_STAR);
        fprintf(stdout, "PUNCT_STAR: %s\n", star);
    }
    {   /* Table */
        const struct weapon *iter;
        for (iter = meta_query(weapons, WEAPON_PISTOL); iter->name; iter++) {
            fprintf(stdout, "%s:\n", iter->name);
            fprintf(stdout, "index: %d\n", iter->index);
            fprintf(stdout, "damage: %d\n", iter->damage);
            fprintf(stdout, "weight: %d\n", iter->weight);
            fprintf(stdout, "munition: %d\n", iter->munition);
            fprintf(stdout, "\n");
        }
    }
    return 0;
}

