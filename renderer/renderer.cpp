#include <stdio.h>
#include <string.h>

#include "utils/defines.h"
#include "utils/allocators.h"
#define MB (1024*1024)

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float u, v, w;
} TexCoord;

typedef struct {
    float x, y, z;
} Normal;

typedef struct {
    int v[3];
    int vt[3];
    int vn[3];
} Face;

typedef struct {

} Material;

typedef struct {
    Material* mat;
    Vertex* vertices;
    Face* faces;
} Object;

typedef struct {
} Group;

typedef struct {
    Object** objects;
} Model;

Arena* read_file(char* file_path, bool trim) {
    Arena* arena = arena_allocator_create(1024);
    if (!arena) {
        printf("Failed to create the initial arena\n");
        return(0);
    }

    FILE* file = fopen(file_path, "r");

    if (!file) {
        printf("Could not open file %s\n", file_path);
        arena_allocator_free(arena);
        return(0);
    }


    size_t chunk_size = 2048;
    size_t bytes_read;
    do {
        // Allocate more if needed
        if (arena_allocator_remaining(arena) < chunk_size) {
            Arena* arena2 = arena_allocator_create(2*arena->capacity);
            if (!arena2) {
                printf("Could not allocate enough memory!\n");
                arena_allocator_free(arena);
                return(0);
            }
            arena_allocator_copy(arena2, arena);
            arena_allocator_free(arena);
            arena = arena2;
        }

        bytes_read = fread(arena->buffer+arena->top, 1, chunk_size, file);
        // TODO(alex): Handle error when reading from file
        // if (bytes_read < chunk_size && ERROR?) ...
        arena->top += bytes_read;
    } while (bytes_read == chunk_size);

    int error = fclose(file);
    // TODO(alex): Handle error when closing file

    // Make it just the right size
    if (trim && arena_allocator_remaining(arena)>0) {
        Arena* arena2 = arena_allocator_create(arena->top);
        if (!arena2) {
            printf("Failed to trim the arena\n");
            arena_allocator_free(arena);
            return(0);
        }
        arena_allocator_copy(arena2, arena);
        arena_allocator_free(arena);
        arena = arena2;
    }
    return arena;
}

void print(Vertex v) {
    printf("Vertex: (%f,%f,%f)\n", v.x, v.y, v.z);
}

void parse_obj_content(Arena* arena) {
    size_t start_line = 0;
    size_t end_line = 0;
    size_t size_line = 0;
    size_t first_space_line = 0;

    size_t tmp_o_count = 0;
    size_t tmp_v_count = 0;
    size_t tmp_f_count = 0;

    Vertex vertices[400] = {};
    Face faces[600] = {};

    Model model;
    Object current_object;
    Group current_group;
    // Arena* arena = arena_allocator_create(2048);


    for (size_t i=0; i<arena->top; i++) {
        if (arena->buffer[i] == '\n') {
            // Process the line
            end_line = i;
            size_line = end_line - start_line + 1;

            // Current line
            char buffer[500];
            memcpy((void*)buffer, (void*)&arena->buffer[start_line], size_line);
            buffer[size_line] = '\0';

            char first = buffer[0];
            if (first == 'v') {                                                 // Could be v vn vt vp
                char second = buffer[1];
                if (second == ' ') {
                    // process vertex
                    Vertex v;
                    sscanf(buffer, "v %f %f %f\n", &v.x, &v.y, &v.z);
                    vertices[tmp_v_count++] = v;

                } else if (second == 'n') {
                    // process normal
                } else if (second == 't') {
                    // process texture
                } else if (second == 'p') {
                    // process vertex parameter
                }
            } else if (first == 'f') {
                Face* f = &faces[tmp_f_count++];
                sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", &f->v[0], &f->vt[0], &f->vn[0], &f->v[1], &f->vt[1], &f->vn[1], &f->v[2], &f->vt[2], &f->vn[2] );
            } else if (first == 'o') {                                          // Object
                // process object
                if (tmp_o_count++>0) break;
                printf("object: %s", buffer);
            } else if (first == 'g') {                                          // Group
                // process group
            } else if (first == 's') {                                          // smooth shading
                char third = buffer[2];
                if (third == 'o') {
                    // process shading off
                } else {
                    // process shading (intensity?)
                }
            } else if (first == 'm') {                                          // mtllib (external .mtl file name)
            } else if (first == 'u') {                                          // usemtl (material name)
            } else if (first == '\n' || first == '#') {                         // new lines or comments
            }

            start_line = i+1;
        }
    }
}

void read_model_file(char* file_path) {
    Arena* arena = read_file(file_path, true);
    printf("Size of arena = %.5f MB\n", (float)(arena->capacity/1024.0f/1024.0f));
    parse_obj_content(arena);

    arena_allocator_free(arena);
    arena = 0;


    // From wiki
    // symbol meanings:
    // # -> comments
    // v x y z [w=1] -> vertex coords
    // vn x y z -> vertex normals (seems to have n instead)
    // vt u [v=0] [w=0] -> texture coordinates 2D sometimes 3D
    // vp u [v] [w] -> vertex parameter
    // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ... -> face definition usually a triangle     index 1-based not all are present (vt vn)
    // l a b c d e -> line element
    //
    // mtllib [external .mtl file name]
    // usemtl [material name]
    //
    // o => object name
    //  g => group name
    //
    // s 1  => smooth shading
    //  s off
    //
    // Using fgets()? and sscanf()?

    /*
    unsigned char buffer[1024];
    size_t ret;
    for (int i=0;i<100;i++) {
        ret = fread(buffer, sizeof(*buffer), ARRAY_SIZE(buffer), file);

        if (ret != ARRAY_SIZE(buffer)) {
            printf("fread failed or reached end of file: %zu\n", ret);
            return;
        }
        for (int j=0; j<ARRAY_SIZE(buffer); j++) {
            if (buffer[j] != 'v' && buffer[j]!='\n' && buffer[j] != ' ' && buffer[j]>=65 && buffer[j] != 'f') {
                // printf("buffer[j]: %c\n", buffer[j]);
            }
        }
    }

    int error = fclose(file);
    */
}
