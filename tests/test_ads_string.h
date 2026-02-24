#include "memory/allocators.h"
#include "libs/ads_string.h"

int test_string_init_empty(void) {
    String s1 = string_init_empty(arena, 42);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, 0);
    ASSERT_EQ(s1.capacity, 42);

    //------------------------------

    String s2 = string_init_empty(arena, 0);
    ASSERT_NOT_NULL(s2.buffer);
    ASSERT_EQ(s2.size, 0);
    ASSERT_EQ(s2.capacity, 1);

    String s_null = {0};
    return 0;
}

int test_string_init_fmt() {
    String s = string_init_fmt(arena, "This is a %s string with some numbers %.3f\n", "FORMATTED", 3.141562f);

    ASSERT_NOT_NULL(s.buffer);
    ASSERT_GE(s.size, 44);
    ASSERT_GE(s.capacity, 44);

    //------------------------------

    s = string_init_fmt(arena, "", "formatted", 3.141562f);

    ASSERT_NOT_NULL(s.buffer);
    ASSERT_EQ(s.size, 0);
    ASSERT_GT(s.capacity, 0);
    return 0;
}

int test_string_init_sv() {
    const char* cstr = "This is my stringView!\n";
    StringView sv = sv_from_cstr(cstr);
    String s = string_init_sv(arena, sv);

    ASSERT_NOT_NULL(s.buffer);
    ASSERT_EQ(s.size, strlen(cstr));
    ASSERT_GE(s.capacity, strlen(cstr)+1);

    //------------------------------

    const char* cstr_null = NULL;
    StringView sv2 = sv_from_cstr(cstr_null);
    String s2 = string_init_sv(arena, sv2);

    ASSERT_NULL(s2.buffer);
    ASSERT_EQ(s2.size, 0);
    ASSERT_EQ(s2.capacity, 0);

    //------------------------------

    const char* cstr_empty = "";
    StringView sv3 = sv_from_cstr(cstr_empty);
    String s3 = string_init_sv(arena, sv3);

    ASSERT_NOT_NULL(s3.buffer);
    ASSERT_EQ(s3.size, 0);
    ASSERT_GE(s3.capacity, 0);

    return 0;
}

int test_string_init_cstr() {
    const char* cstr = "THIS IS A CONST CHAR*";
    String s1 = string_init_cstr(arena, cstr);

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr));
    ASSERT_GE(s1.capacity, strlen(cstr));

    //------------------------------

    const char* cstr2 = NULL;
    String s2 = string_init_cstr(arena, cstr2);

    ASSERT_NULL(s2.buffer);
    ASSERT_EQ(s2.size, 0);
    ASSERT_EQ(s2.capacity, 0);
    return 0;
}

int test_string_init_buffer(void) {
    const char* cstr = "THIS IS A CONST CHAR*";
    String s1 = string_init_buffer(arena, cstr, 5);

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, 5);
    ASSERT_GE(s1.capacity, 5);

    //------------------------------

    s1 = string_init_buffer(arena, cstr, strlen(cstr));

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr));
    ASSERT_GE(s1.capacity, strlen(cstr));

    //------------------------------

    const char* cstr2 = NULL;
    String s2 = string_init_buffer(arena, cstr2, 0);

    ASSERT_NULL(s2.buffer);
    ASSERT_EQ(s2.size, 0);
    ASSERT_EQ(s2.capacity, 0);

    //------------------------------

    s2 = string_init_buffer(arena, cstr2, 15);

    ASSERT_NULL(s2.buffer);
    ASSERT_EQ(s2.size, 0);
    ASSERT_EQ(s2.capacity, 0);
    return 0;
}

int test_string_append_fmt(void) {
    const char* cstr =  "This is test_string_append_fmt";
    String s1 = string_init_cstr(arena, cstr);
    const char* post =  "APPENDING WITH A ! in the end!";
    int r = string_append_fmt(arena, &s1, "%s", post);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(post));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(post));

    //------------------------------
    s1 = string_init_cstr(arena, cstr);
    r = string_append_fmt(arena, &s1, NULL);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr));
    ASSERT_GE(s1.capacity,  strlen(cstr));

    return 0;
}

int test_string_append_sv(void) {
    const char* cstr =  "This is test_string_append_sv";
    String s1 = string_init_cstr(arena, cstr);
    const char* post =  " APPENDING 1! in the end!";
    StringView sv1 = sv_from_cstr(post);
    int r = string_append_sv(arena, &s1, sv1);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv1.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv1.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    const char* post2 =  " APPENDING VERY MUCH STUFF IN THE END TO ABOVE THE CAPACITY WITH A ! in the end!";
    StringView sv2 = sv_from_cstr(post2);
    r = string_append_sv(arena, &s1, sv2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv2.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    r = string_append_sv(arena, &s1, sv2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv2.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    StringView sv3 = {.buffer=NULL, .size=0};
    r = string_append_sv(arena, &s1, sv3);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr));
    ASSERT_GE(s1.capacity, strlen(cstr));

    return 0;
}

int test_string_append_string(void) {
    const char* cstr =  "This is test_string_append_string";
    String s1 = string_init_cstr(arena, cstr);
    String s2 = string_init_cstr(arena, " APPENDING 1! in the end!");

    int r = string_append_string(arena, &s1, &s2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+s2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+s2.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    s2 = string_init_cstr(arena, " APPENDING VERY MUCH STUFF IN THE END TO ABOVE THE CAPACITY WITH A ! in the end!");
    r = string_append_string(arena, &s1, &s2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,     strlen(cstr)+s2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+s2.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    s2 = string_init_cstr(arena, " APPENDING VERY MUCH STUFF IN THE END TO ABOVE THE CAPACITY WITH A ! in the end!");

    r = string_append_string(arena, &s1, &s2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,     strlen(cstr)+s2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+s2.size);
    return 0;
}

int test_string_append_cstr(void) {
    const char* cstr =  "This is test_string_append_cstr";
    const char* post = " APPENDING VERY MUCH STUFF IN THE END TO ABOVE THE CAPACITY WITH A ! in the end!";
    String s1 = string_init_cstr(arena, cstr);
    int r = string_append_cstr(arena, &s1, post);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+strlen(post));
    ASSERT_GE(s1.capacity, strlen(cstr)+strlen(post));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_append_cstr(arena, &s1, NULL);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    return 0;
}

int test_string_append_buffer(void) {
    const char* cstr =  "This is test_string_append_sv";
    String s1 = string_init_cstr(arena, cstr);
    const char* post =  " APPENDING 1! in the end!";
    string_append_buffer(arena, &s1, post, 10);

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+10);
    ASSERT_GE(s1.capacity, strlen(cstr)+10);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);

    const char* post2 =  " APPENDING VERY MUCH STUFF IN THE END TO ABOVE THE CAPACITY WITH A ! in the end!";
    string_append_buffer(arena, &s1, post2, strlen(post2)-1);

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+strlen(post2)-1);
    ASSERT_GE(s1.capacity, strlen(cstr)+strlen(post2)-1);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);

    string_append_buffer(arena, &s1, post2, strlen(post2));

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+strlen(post2));
    ASSERT_GE(s1.capacity, strlen(cstr)+strlen(post2));
    return 0;
}

int test_string_append_char(void) {
    const char* cstr =  "This is test_string_append_sv";
    String s1 = string_init_cstr(arena, cstr);

    string_append_char(arena, &s1, '!');

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+1);
    ASSERT_GE(s1.capacity, strlen(cstr)+1);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    string_append_char(arena, &s1, '!');

    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+1);
    ASSERT_GE(s1.capacity, strlen(cstr)+1);
    return 0;
}

int test_string_prepend_fmt(void) {
    const char* cstr =  "This is test_string_prepend_fmt\n";
    String s1 = string_init_cstr(arena, cstr);
    const char* pre =  "PREPENDING WITH A ! in the end!";
    int r = string_prepend_fmt(arena, &s1, "%s", pre);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(pre));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(pre));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_prepend_fmt(arena, &s1, NULL);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr));
    ASSERT_GE(s1.capacity,  strlen(cstr));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    const char* pre_super_long =  "THIS HAS TO GO THROUGH THE SHORT PATH THIS HAS TO GO THROUGH THE SHORT PATH THIS HAS TO GO THROUGH THE SHORT PATH THIS HAS TO GO THROUGH THE SHORT PATH";
    r = string_prepend_fmt(arena, &s1, "%s", pre_super_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(pre_super_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(pre_super_long));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    const char* pre_long =  "THIS IS A VERY LONG THING I WANT TO PREPEND TO HAVE TO INCREASE THE CAPACITY! ";
    r = string_prepend_fmt(arena, &s1, "%s", pre_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(pre_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(pre_long));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    r = string_prepend_fmt(arena, &s1, "%s", pre_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(pre_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(pre_long));

    return 0;
}

int test_string_prepend_sv(void) {
    const char* cstr =  "This is test_string_prepend_sv";
    String s1 = string_init_cstr(arena, cstr);
    const char* pre =  "PREPENDING 1! in the end!";
    StringView sv1 = sv_from_cstr(pre);
    int r = string_prepend_sv(arena, &s1, sv1);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv1.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv1.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    const char* pre2 =  "PREPENDING VERY MUCH STUFF IN THE END TO ABOVE THE CAPACITY WITH A ! in the end!";
    StringView sv2 = sv_from_cstr(pre2);
    r = string_prepend_sv(arena, &s1, sv2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv2.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    r = string_prepend_sv(arena, &s1, sv2);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv2.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv2.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    StringView sv3 = {.buffer=NULL, .size=0};
    r = string_prepend_sv(arena, &s1, sv3);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr));
    ASSERT_GE(s1.capacity, strlen(cstr));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    StringView sv4 = sv_slice_string(s1, 8, s1.size-8);
    r = string_prepend_sv(arena, &s1, sv4);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+sv4.size);
    ASSERT_GE(s1.capacity, strlen(cstr)+sv4.size);

    return 0;
}

int test_string_insert_fmt(void) {
    const char* cstr =  "This is test_string_insert_fmt\n";
    String s1 = string_init_cstr(arena, cstr);
    const char* ins =  "INSERTING WITH A ! in the end! ";
    int r = string_insert_fmt(arena, &s1, 8, "%s", ins);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(ins));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(ins));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_insert_fmt(arena, &s1, 8, NULL);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr));
    ASSERT_GE(s1.capacity,  strlen(cstr));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    const char* ins_long =  "THIS IS A VERY LONG THING I WANT TO INSERT TO HAVE TO INCREASE THE CAPACITY! ";
    r = string_insert_fmt(arena, &s1, 8, "%s", ins_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(ins_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(ins_long));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    r = string_insert_fmt(arena, &s1, 8, "%s", ins_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(ins_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(ins_long));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_insert_fmt(arena, &s1, 0, "%s", ins_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(ins_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(ins_long));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_insert_fmt(arena, &s1, strlen(cstr), "%s", ins_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + strlen(ins_long));
    ASSERT_GE(s1.capacity,  strlen(cstr) + strlen(ins_long));

    return 0;
}

int test_string_insert_sv(void) {
    const char* cstr =  "This is test_string_insert_sv\n";
    String s1 = string_init_cstr(arena, cstr);
    const char* ins =  "INSERTING WITH A ! in the end! ";
    StringView sv = sv_from_cstr(ins);
    int r = string_insert_sv(arena, &s1, 8, sv);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + sv.size);
    ASSERT_GE(s1.capacity,  strlen(cstr) + sv.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    sv = {.buffer=NULL, .size=0};
    r = string_insert_sv(arena, &s1, 8, sv);

    ASSERT_EQ(r, -1);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr));
    ASSERT_GE(s1.capacity,  strlen(cstr));

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    const char* ins_long =  "THIS IS A VERY LONG THING I WANT TO INSERT TO HAVE TO INCREASE THE CAPACITY! ";
    StringView sv_long = sv_from_cstr(ins_long);
    r = string_insert_sv(arena, &s1, 8, sv_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + sv_long.size);
    ASSERT_GE(s1.capacity,  strlen(cstr) + sv_long.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    r = string_insert_sv(arena, &s1, 8, sv_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + sv_long.size);
    ASSERT_GE(s1.capacity,  strlen(cstr) + sv_long.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_insert_sv(arena, &s1, 0, sv_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + sv_long.size);
    ASSERT_GE(s1.capacity,  strlen(cstr) + sv_long.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    r = string_insert_sv(arena, &s1, strlen(cstr), sv_long);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + sv_long.size);
    ASSERT_GE(s1.capacity,  strlen(cstr) + sv_long.size);

    //------------------------------

    s1 = string_init_cstr(arena, cstr);
    sv = sv_slice_string(s1, 7, 22);
    r = string_insert_sv(arena, &s1, 7, sv);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size,      strlen(cstr) + sv.size);
    ASSERT_GE(s1.capacity,  strlen(cstr) + sv.size);

    return 0;
}

/*
int test_string_insert_buffer() {
    const char* cstr =  "This is my initial string\n";

    String s1 = string_init_cstr(arena, cstr);
    int r = string_insert_buffer(arena, &s1, 10, " INSERTED AND MORE AND MORE AND MORE", 9);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s1.buffer);
    ASSERT_EQ(s1.size, strlen(cstr)+9);
    ASSERT_GE(s1.capacity, strlen(cstr)+9);

    //------------------------------

    String s2 = string_init_cstr(arena, cstr);
    int r2 = string_insert_buffer(arena, &s2, 10, " INSERTED AND MORE AND MORE AND MORE", 36);

    ASSERT_LT(strlen(cstr), 36);
    ASSERT_EQ(r2, 0);
    ASSERT_EQ(s2.size, strlen(cstr)+36);
    ASSERT_GE(s2.capacity, strlen(cstr)+36);

    //------------------------------

    String s3 = string_init_cstr(arena, cstr);
    // To not be at the end of the arena
    arena_alloc_push(arena, 42);
    int r3 = string_insert_buffer(arena, &s3, 10, " INSERTED AND MORE AND MORE AND MORE", 36);

    ASSERT_LT(strlen(cstr), 36);
    ASSERT_EQ(r3, 0);
    ASSERT_EQ(s3.size, strlen(cstr)+36);
    ASSERT_GE(s3.capacity, strlen(cstr)+36);

    //------------------------------
    // NULL

    String s4 = string_init_cstr(arena, cstr);
    int r4 = string_insert_buffer(arena, &s4, 10, NULL, 9);

    ASSERT_EQ(r, 0);
    ASSERT_NOT_NULL(s4.buffer);
    ASSERT_EQ(s4.size, strlen(cstr)+9);
    ASSERT_GE(s4.capacity, strlen(cstr)+9);

    //------------------------------

    String s5 = string_init_cstr(arena, cstr);
    int r5 = string_insert_buffer(arena, &s5, 10, NULL, 36);

    ASSERT_LT(strlen(cstr), 36);
    ASSERT_EQ(r5, 0);
    ASSERT_EQ(s5.size, strlen(cstr)+36);
    ASSERT_GE(s5.capacity, strlen(cstr)+36);

    //------------------------------

    String s6 = string_init_cstr(arena, cstr);
    // To not be at the end of the arena
    arena_alloc_push(arena, 42);
    int r6 = string_insert_buffer(arena, &s6, 10, NULL, 36);

    ASSERT_LT(strlen(cstr), 36);
    ASSERT_EQ(r6, 0);
    ASSERT_EQ(s6.size, strlen(cstr)+36);
    ASSERT_GE(s6.capacity, strlen(cstr)+36);

    return 0;
}
*/

int test_string_overwrite_fmt(void) {
    const char* cstr = "This is the normal version of test_string_overwrite_fmt!";
    String s = string_init_cstr(arena, cstr);
    string_overwrite_fmt(arena, &s, 12, "%s", "super edited");


    s = string_init_cstr(arena, cstr);
    string_overwrite_fmt(arena, &s, 12, "%s", "super edited version of this and a lot of stuff........................................\n");

    return 0;
}

int test_string_overwrite_sv(void) {
    const char* cstr = "This is the normal version of test_string_overwrite_sv!";

    String s = string_init_cstr(arena, cstr);
    StringView sv = sv_from_cstr("edited");
    string_overwrite_sv(arena, &s, 12, sv);


    s = string_init_cstr(arena, cstr);
    sv = sv_from_cstr("edited with a very long but not so long as we need to realloc");
    string_overwrite_sv(arena, &s, 12, sv);


    const char* cstr2 = "Short test_string_overwrite_sv";

    s = string_init_cstr(arena, cstr2);
    sv = sv_from_cstr("edited with a very long StringView, so long as we need to realloc");
    string_overwrite_sv(arena, &s, 12, sv);


    s = string_init_cstr(arena, cstr2);
    arena_alloc_push(arena, 42);
    sv = sv_from_cstr("edited with a very long StringView, so long as we need to realloc");
    string_overwrite_sv(arena, &s, 12, sv);

    s = string_init_cstr(arena, cstr);
    sv = sv_slice_string(s, 12, 6);
    string_overwrite_sv(arena, &s, 14, sv);

    s = string_init_cstr(arena, cstr);
    sv = sv_slice_string(s, 12, s.size-12);
    string_overwrite_sv(arena, &s, 30, sv);

    return 0;
}
/*

int test_string_overwrite_buffer(void) {
    const char* cstr =  "This  is test_string_overwrite_buffer";
    String s1 = string_init_cstr(arena, cstr);
    string_overwrite_buffer(arena, &s1, 5, "was", 3);


    String s2 = string_init_cstr(arena, cstr);
    string_overwrite_buffer(arena, &s2, 5, NULL, 3);


    String s3 = string_init_cstr(arena, cstr);
    string_overwrite_buffer(arena, &s3, 20, "77777777777777777777", 20);

    String s4 = string_init_cstr(arena, cstr);
    string_overwrite_buffer(arena, &s4, 20, NULL, 20);

    return 0;
}
*/

int test_string_erase_and_insert_sv(void) {
    const char* cstr = "This is a cstr for test_string_erase_and_insert_sv!\n";

    String s = string_init_cstr(arena, cstr);
    StringView sv = sv_from_cstr("erased and inserted");
    string_erase_and_insert_sv(arena, &s, 8, 6, sv);


    s = string_init_cstr(arena, cstr);
    sv = sv_from_cstr("Inserted before: t");
    string_erase_and_insert_sv(arena, &s, 0, 1, sv);


    s = string_init_cstr(arena, cstr);
    sv = sv_from_cstr("inserting a lot of stuff here so that we have to touch the arena");
    string_erase_and_insert_sv(arena, &s, 8, 1, sv);


    s = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    sv = sv_from_cstr("inserting a lot of stuff here so that we have to touch the arena");
    string_erase_and_insert_sv(arena, &s, 8, 1, sv);


    s = string_init_cstr(arena, cstr);
    sv = sv_slice_string(s, 10, 41);
    string_erase_and_insert_sv(arena, &s, 19, 4, sv);

    return 0;
}

int test_string_erase_and_insert_fmt(void) {
    const char* cstr = "This is a cstr for test_string_erase_and_insert_fmt!\n";

    String s = string_init_cstr(arena, cstr);
    string_erase_and_insert_fmt(arena, &s, 8, 6, "%s", "erased and inserted");


    s = string_init_cstr(arena, cstr);
    string_erase_and_insert_fmt(arena, &s, 0, 1, "%s", "Inserted before: t");


    s = string_init_cstr(arena, cstr);
    string_erase_and_insert_fmt(arena, &s, 8, 1, "%s", "inserting a lot of stuff here so that we have to touch the arena");


    s = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    string_erase_and_insert_fmt(arena, &s, 8, 1, "%s", "inserting a lot of stuff here so that we have to touch the arena");

    return 0;
}

int test_string_erase(void) {
    const char* cstr = "This is the full string test_string_erase!";
    String s = string_init_cstr(arena, cstr);
    string_erase(&s, 11, 12);
    return 0;
}

int test_string_clear(void) {
    const char* cstr = "This is the full string test_string_clear!";
    String s = string_init_cstr(arena, cstr);
    string_clear(&s);
    return 0;
}

int test_string_reserve(void) {
    const char* cstr = "This is the full string test_string_reserve!";

    String s = string_init_cstr(arena, cstr);
    size_t old_cap = s.capacity;
    size_t new_cap = 3*old_cap;
    int r = string_reserve(arena, &s, new_cap);

    ASSERT_EQ(r, 0);
    ASSERT_EQ(s.capacity, new_cap);

    s = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    old_cap = s.capacity;
    new_cap = 3*old_cap;
    r = string_reserve(arena, &s, new_cap);

    ASSERT_EQ(r, 0);
    ASSERT_EQ(s.capacity, new_cap);

    return 0;
}

int test_string_shrink_to_fit(void) {
    const char* cstr = "This is the full string test_string_shrink_to_fit!";

    String s = string_init_cstr(arena, cstr);
    size_t old_cap = s.capacity;
    size_t new_cap = 3*old_cap;
    int r = string_reserve(arena, &s, new_cap);
    ASSERT_EQ(r, 0);

    r = string_shrink_to_fit(arena, &s);
    ASSERT_EQ(r, 0);
    ASSERT_EQ(s.capacity, s.size+1);


    s = string_init_cstr(arena, cstr);
    old_cap = s.capacity;
    new_cap = 3*old_cap;
    r = string_reserve(arena, &s, new_cap);
    ASSERT_EQ(r, 0);
    arena_alloc_push(arena, 42);

    r = string_shrink_to_fit(arena, &s);
    // TODO: Think about what would be mose natural here
    // ASSERT_EQ(r, 0);
    // Capacity not changed because not on top of arena
    ASSERT_EQ(s.capacity, new_cap);

    return 0;
}

int test_sv_from_buffer(void) {
    const char* buffer = "This is a test of test_sv_from_buffer";
    StringView sv = sv_from_buffer(buffer, strlen(buffer));

    ASSERT_NOT_NULL(sv.buffer);
    ASSERT_EQ(sv.buffer, buffer);

    sv = sv_from_buffer(buffer, 10);
    ASSERT_EQ(sv.buffer, buffer);
    ASSERT_EQ(sv.size, 10);

    return 0;
}

int test_sv_from_cstr(void) {
    const char* cstr = "This is a test of test_sv_from_cstr";
    StringView sv = sv_from_cstr(cstr);

    ASSERT_NOT_NULL(sv.buffer);
    ASSERT_EQ(sv.buffer, cstr);

    return 0;
}

int test_sv_slice_sv(void) {
    const char* cstr = "This is not slice of test_sv_slice_sv!";
    StringView slice;
    StringView sv = sv_from_cstr(cstr);

    slice = sv_slice_sv(sv, 12, sv.size-12);
    ASSERT_EQ(sv_equal(slice, sv_from_cstr("slice of test_sv_slice_sv!")), true);


    slice = sv_slice_sv(sv, 12, 5);
    ASSERT_EQ(sv_equal(slice, sv_from_cstr("slice")), true);

    return 0;
}

int test_sv_slice_string(void) {
    const char* cstr = "This is not slice of test_sv_slice_string!";
    StringView slice;
    String str = string_init_cstr(arena, cstr);

    slice = sv_slice_string(str, 12, str.size-12);
    ASSERT_TRUE(sv_equal(slice, sv_from_cstr("slice of test_sv_slice_string!")));

    slice = sv_slice_string(str, 12, 5);
    ASSERT_TRUE(sv_equal(slice, sv_from_cstr("slice")));

    return 0;
}

int test_sv_truncate_front(void) {
    const char* cstr = "This is test_sv_truncate_front!";

    StringView sv = sv_from_cstr(cstr);

    sv = sv_truncate_front(sv, 8);
    ASSERT_TRUE(sv_equal(sv, sv_from_cstr("test_sv_truncate_front!")));

    sv = sv_truncate_front(sv, strlen(cstr));
    ASSERT_NULL(sv.buffer);

    sv = sv_truncate_front(sv, strlen(cstr)+1);
    ASSERT_NULL(sv.buffer);
    return 0;
}

int test_sv_truncate_back(void) {
    const char* cstr = "This is test_sv_truncate_back!";

    StringView sv = sv_from_cstr(cstr);

    sv = sv_truncate_back(sv, 26);
    ASSERT_TRUE(sv_equal(sv, sv_from_cstr("This")));

    sv = sv_truncate_back(sv, strlen(cstr));
    ASSERT_NULL(sv.buffer);

    sv = sv_truncate_back(sv, strlen(cstr)+1);
    ASSERT_NULL(sv.buffer);
    return 0;
}

int test_sv_trim_front(void) {
    const char* cstr = "This is without spaces at the front test_sv_trim_front!";
    StringView sv = sv_from_cstr(cstr);
    StringView r;

    r = sv_trim_front(sv);
    ASSERT_TRUE(sv_equal(r, sv));

    const char* cstr2 = "   \t This is with spaces at the front test_sv_trim_front!";
    sv = sv_from_cstr(cstr2);

    r = sv_trim_front(sv);
    ASSERT_TRUE(sv_equal(r, sv_from_cstr("This is with spaces at the front test_sv_trim_front!")));
    return 0;
}

int test_sv_trim_back(void) {
    const char* cstr = "This is without spaces at the back test_sv_trim_back!";
    StringView sv = sv_from_cstr(cstr);
    StringView r;

    r = sv_trim_back(sv);
    ASSERT_TRUE(sv_equal(r, sv));

    const char* cstr2 = "   \t This is with spaces at the back test_sv_trim_back! \v \f";
    sv = sv_from_cstr(cstr2);

    r = sv_trim_back(sv);
    ASSERT_TRUE(sv_equal(r, sv_from_cstr("   \t This is with spaces at the back test_sv_trim_back!")));
    return 0;
}

int test_sv_chop_by_delim_sv(void) {
    const char* cstr = "This, is, with, a, bunch, of, commas, within, test_sv_chop_by_delim_sv!";
    StringView sv = sv_from_cstr(cstr);

    StringView delim = sv_from_cstr(",");
    while (1) {
        StringView part = sv_chop_by_delim_sv(&sv, delim);
        if (part.size == 0) break;
    }

    delim = sv_from_cstr("with");
    sv = sv_from_cstr(cstr);
    while (1) {
        StringView part = sv_chop_by_delim_sv(&sv, delim);
        if (part.size == 0) break;
    }

    sv = sv_from_cstr(cstr);
    while (1) {
        StringView part = sv_chop_by_delim_char(&sv, ',');
        if (part.size == 0) break;
    }

    sv = sv_from_cstr(cstr);
    String delim_str = string_init_cstr(arena, ",");
    while (1) {
        StringView part = sv_chop_by_delim_string(&sv, delim_str);
        if (part.size == 0) break;
    }

    sv = sv_from_cstr(cstr);
    const char* buf = "with";
    printf("\n");
    sv_print(sv);
    while (1) {
        StringView part = sv_chop_by_delim_buffer(&sv, buf, 4);
        printf("\n");
        sv_print(part);
        printf("\n");
        sv_print(sv);
        if (part.size == 0) break;
    }
    return 0;
}

int test_sv_equal(void) {
    const char* cstr = "This is test_sv_equal!";
    StringView sv1 = sv_from_cstr(cstr);
    StringView sv2 = sv_from_cstr(cstr);

    bool r = sv_equal(sv1, sv2);

    ASSERT_EQ(r, true);

    StringView sv3 = sv_from_cstr("This is another cstr");
    r = sv_equal(sv1, sv3);
    ASSERT_EQ(r, false);

    return 0;
}

int test_sv_compare(void) {
    const char* cstr = "This is test_sv_equal!";
    int r;
    StringView sv1 = sv_from_cstr(cstr);
    StringView sv2 = sv_from_cstr(cstr);

    r = sv_compare(&sv1, &sv2);
    ASSERT_EQ(r, 0);

    StringView sv3 = sv_from_cstr("This is another cstr");
    r = sv_compare(&sv1, &sv3);
    ASSERT_GE(r, 1);

    r = sv_compare(&sv3, &sv1);
    ASSERT_LE(r, -1);

    String s = string_init_cstr(arena, cstr);
    int m = string_append_cstr(arena, &s, " with some extras");
    ASSERT_EQ(m, 0);
    StringView sv4 = sv_from_string(s);

    r = sv_compare(&sv1, &sv4);
    ASSERT_LE(r, -1);

    r = sv_compare(&sv4, &sv1);
    ASSERT_GE(r, 1);

    return 0;
}

int test_sv_starts_with(void) {
    const char* cstr = "This is test_sv_starts_with!";
    bool r;

    StringView sv1 = sv_from_cstr(cstr);

    r = sv_starts_with(sv1, sv_from_cstr("This"));
    ASSERT_EQ(r, true);

    r = sv_starts_with(sv1, sv_from_cstr("This#"));
    ASSERT_EQ(r, false);

    r = sv_starts_with(sv1, (StringView){0});
    ASSERT_EQ(r, false);

    r = sv_starts_with((StringView){0}, sv_from_cstr("This"));
    ASSERT_EQ(r, false);

    r = sv_starts_with((StringView){0}, (StringView){0});
    ASSERT_EQ(r, false);

    return 0;
}

int test_sv_ends_with(void) {
    const char* cstr = "This is test_sv_ends_with!";
    bool r;

    StringView sv1 = sv_from_cstr(cstr);

    r = sv_ends_with(sv1, sv_from_cstr("with!"));
    ASSERT_EQ(r, true);

    r = sv_ends_with(sv1, sv_from_cstr("#with!"));
    ASSERT_EQ(r, false);

    r = sv_ends_with(sv1, (StringView){0});
    ASSERT_EQ(r, false);

    r = sv_ends_with((StringView){0}, sv_from_cstr("with!"));
    ASSERT_EQ(r, false);

    r = sv_ends_with((StringView){0}, (StringView){0});
    ASSERT_EQ(r, false);

    return 0;
}

int test_sv_find(void) {
    const char* cstr = "This is a test to find a needle in a haystack test_sv_find!";
    StringView sv = sv_from_cstr(cstr);
    size_t r;

    r = sv_find(sv, sv_from_cstr("This"));
    ASSERT_EQ(r, 0);

    r = sv_find(sv, sv_from_cstr("!"));
    ASSERT_EQ(r, sv.size-1);

    r = sv_find(sv, sv_from_cstr("test"));
    ASSERT_EQ(r, 10);
    ASSERT_TRUE(sv_starts_with(sv_truncate_front(sv, r), sv_from_cstr("test")));

    return 0;
}

int test_sv_rfind(void) {
    const char* cstr = "This is a test to find a needle in a haystack test_sv_rfind!";
    StringView sv = sv_from_cstr(cstr);
    size_t r;

    r = sv_rfind(sv, sv_from_cstr("This"));
    ASSERT_EQ(r, 0);

    r = sv_rfind(sv, sv_from_cstr("rfind!"));
    ASSERT_TRUE(sv_starts_with(sv_truncate_front(sv, r), sv_from_cstr("rfind!")));

    r = sv_rfind(sv, sv_from_cstr("!"));
    ASSERT_EQ(r, sv.size-1);

    r = sv_rfind(sv, sv_from_cstr("T"));
    ASSERT_EQ(r, 0);

    r = sv_rfind(sv, sv_from_cstr("test"));
    ASSERT_EQ(r, 46);
    ASSERT_TRUE(sv_starts_with(sv_truncate_front(sv, r), sv_from_cstr("test")));

    return 0;
}
