#pragma once

#include "types.h"
#include "types/matrix4.h"
#include "types/vector4.h"
#include "types/vector3.h"
#include "syscalls/syscalls.h"

typedef vector4 (*vertex_shader)(vector3 pos);
typedef argbcolor (*fragment_shader)(vector4 frag_coord, int trig_id);

typedef enum {
	primitives_none,
	primitives_pixel,
	primitives_line,
	primitives_trig,
	primitives_quad,
} primitives;

typedef enum { BUF_RAW, BUF_VERTICES, BUF_SEGMENTS, BUF_COLORS, } buffer_type;

typedef struct {
    vertex_shader vert_shader;
    fragment_shader frag_shader;
    bool do_backface_culling;
    
    matrix4x4 proj_matrix;
    primitives prim_type;
    
    u32 *fb;
    gpu_size screen_size;
    
    float* zbuf;
    size_t z_buf_size;
    
    int downscale;
} t2d_pipeline;

typedef struct {
    int id;
    int vbuf;
    int vcount;
    int sbuf;
    int scount;
    t2d_pipeline pipeline;
} t2d_encode_job;

t2d_pipeline t2d_make_pipeline(u32 width, u32 height, u32* fb);
void t2d_vert_shader(t2d_pipeline *pipeline, vertex_shader shader);
void t2d_frag_shader(t2d_pipeline *pipeline, fragment_shader shader);
int t2d_create_raw_buffer(size_t elem_size, size_t count);
void* t2d_get_buffer_ptr(int buf_index);
void t2d_make_orthographic_camera(t2d_pipeline *pipeline, float top, float bottom, float left, float right, float near, float far);
void t2d_make_perspective_camera(t2d_pipeline *pipeline, float fov, float aspect, float near, float far);
t2d_encode_job t2d_begin_encode(t2d_pipeline pipeline);
void t2d_clear(t2d_encode_job *job, argbcolor color);
void t2d_set_buffer(t2d_encode_job *job, int buf_index, int count, buffer_type);
tern t2d_commit_encode(t2d_encode_job *job);

extern vector4 default_vert_shader(vector3 pos);
extern argbcolor default_frag_shader(vector4 frag_coord, int trig_id);

static inline int t2d_create_buffer(buffer_type type, size_t count){
    size_t elem_size;
    switch (type){
        case BUF_VERTICES: elem_size = sizeof(float)*3; break;
        case BUF_SEGMENTS: elem_size = sizeof(int); break;
        case BUF_COLORS:   elem_size = sizeof(u32); break;
        case BUF_RAW: print("[T2D error]: unknown buffer type. Use t2d_create_raw_buffer to create raw buffers"); return -1;
    }
    print("Created buffer for type %i of size %i",type,count*elem_size);
    return t2d_create_raw_buffer(elem_size, count);
}