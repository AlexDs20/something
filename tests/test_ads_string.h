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
    ASSERT_EQ(s2.capacity, 0);
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

int test_string_grow_capacity(void) {
    const char* cstr = "This is test_string_grow_capacity!";
    String s = string_init_cstr(arena, cstr);
    size_t init_size = s.size;
    size_t init_capacity = s.capacity;
    size_t amount = 2;
    int r = string_grow_capacity(arena, &s, amount);

    ASSERT_EQ(r, 0);
    ASSERT_EQ(s.capacity,   init_capacity+amount);


    s = string_init_cstr(arena, cstr);
    amount = 100;
    r = string_grow_capacity(arena, &s, amount);
    ASSERT_EQ(r, 0);
    ASSERT_EQ(s.capacity,   init_capacity+amount);
    ASSERT_EQ(s.size,       init_size);
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

/*
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
*/

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

    return 0;
}

int test_string_overwrite_fmt(void) {
    const char* cstr = "This is the normal version of test_string_overwrite_fmt!";
    String s = string_init_cstr(arena, cstr);
    string_print(&s);
    string_overwrite_fmt(&s, 12, "%s", "edited");
    string_print(&s);

    return 0;
}

int test_string_overwrite_sv(void) {
    const char* cstr = "This is the normal version of test_string_overwrite_sv!";

    String s = string_init_cstr(arena, cstr);
    StringView sv = sv_from_cstr("edited");
    string_print(&s);
    string_overwrite_sv(&s, 12, sv);
    string_print(&s);

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
    printf("\n");
    const char* cstr = "This is a cstr for test_string_erase_and_insert_sv!\n";

    String s = string_init_cstr(arena, cstr);
    string_print(&s);
    StringView sv = sv_from_cstr("erased and inserted");
    string_erase_and_insert_sv(arena, &s, 8, 6, sv);
    string_print(&s);


    s = string_init_cstr(arena, cstr);
    string_print(&s);
    sv = sv_from_cstr("Inserted before: t");
    string_erase_and_insert_sv(arena, &s, 0, 1, sv);
    string_print(&s);


    s = string_init_cstr(arena, cstr);
    string_print(&s);
    sv = sv_from_cstr("inserting a lot of stuff here so that we have to touch the arena");
    string_erase_and_insert_sv(arena, &s, 8, 1, sv);
    string_print(&s);


    s = string_init_cstr(arena, cstr);
    arena_alloc_push(arena, 42);
    string_print(&s);
    sv = sv_from_cstr("inserting a lot of stuff here so that we have to touch the arena");
    string_erase_and_insert_sv(arena, &s, 8, 1, sv);
    string_print(&s);

    return 0;
}

int test_string_erase(void) {
    const char* cstr = "This is the full string test_string_erase!";
    String s = string_init_cstr(arena, cstr);
    string_print(&s);
    string_erase(&s, 11, 12);
    string_print(&s);
    return 0;
}

int test_string_clear(void) {
    const char* cstr = "This is the full string test_string_clear!";
    String s = string_init_cstr(arena, cstr);
    string_clear(&s);
    return 0;
}
