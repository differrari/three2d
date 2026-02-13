#include "syscalls/syscalls.h"
#include "types/matrix4.h"
#include "types/vector4.h"
#include "math/vector.h"
#include "memory/memory.h"
#include "three2d.h"
#include "debug/assert.h"
#include "data/struct/chunk_array.h"

bool do_backface_culling = true;

static inline bool should_clip(vector4 v){
    return v.x <= -v.w || v.x >= v.w ||
           v.y <= -v.w || v.y >= v.w || 
           v.z <= -v.w || v.z >= 0   ||
           v.w <= 0;
}

static inline float triangle_area(vector3 a, vector3 b, vector3 c){
    return -0.5f*((b.y-a.y)*(b.x+a.x) + (c.y-b.y)*(c.x+b.x) + (a.y-c.y)*(a.x+c.x));
}

static inline float ceilf(float val){
    int64_t whole = (uint64_t)val;
    float frac = val - (float)whole;

    return (float)(frac > 0 ? whole + 1 : whole);
}

static inline float floorf(float val){
    return (float)(int64_t)val;
}

void rasterize_triangle(vector3 v0, vector3 v1, vector3 v2, int trig_id, int downscale, gpu_size screen, u32 *fb, float *zbuf, fragment_shader shader){
    float min_x = floorf(minf(v0.x,minf(v1.x,v2.x)));
    float min_y = floorf(minf(v0.y,minf(v1.y,v2.y)));
    float max_x = ceilf(maxf(v0.x,maxf(v1.x,v2.x)));
    float max_y = ceilf(maxf(v0.y,maxf(v1.y,v2.y)));
    
    min_x = clampf(min_x, 0, screen.width-1);
    min_y = clampf(min_y, 0, screen.height-1);
    max_x = clampf(max_x, 0, screen.width-1);
    max_y = clampf(max_y, 0, screen.height-1);
    
    float total_area = triangle_area(v0, v1, v2);
    float area_sign = total_area < 0 ? -1.0f : 1.0f;
    total_area *= area_sign;

    if ((do_backface_culling && (area_sign<0)) || (total_area < 1)) return;

    for (float y = min_y; y <= max_y; y += downscale){
        for (float x = min_x; x <= max_x; x += downscale){
            vector3 p = (vector3){x,y,0};
            float alpha = area_sign * triangle_area(p, v1, v2);
            if (alpha < 0) continue;
            float beta = area_sign * triangle_area(p, v2, v0);
            if (beta < 0) continue;
            float gamma = area_sign * triangle_area(p, v0, v1);
            if (gamma < 0) continue;
            
            float new_inv_z = (alpha * v0.z + beta * v1.z + gamma * v2.z)/total_area;
            if (new_inv_z <= -1.0f || new_inv_z >= 0.0f) continue;
            
            u8 depth_color = (u8)(255 * (-new_inv_z));
            uint32_t color = shader((vector4){x,y,0,0},trig_id).color;//(0xFF << 24) | (depth_color << 16) | (depth_color << 8) | depth_color;
            
            for (int yy = 0; yy < downscale && y + yy < screen.height; yy++)
                for (int xx = 0; xx < downscale  && x + xx < screen.width; xx++) {
                    int offs = ((int)(y + yy) * screen.width) + (int)(x + xx);

                    float current_inv_z = zbuf[offs];
                    if (new_inv_z > current_inv_z) continue;

                    fb[offs] = color;
                    zbuf[offs] = new_inv_z;
                }
        }
    }
    
    // fb_draw_line(&ctx, min_x, min_y, max_x, min_y, 0xFFFF0000);
    // fb_draw_line(&ctx, min_x, min_y, min_x, max_y, 0xFFFF0000);
    // fb_draw_line(&ctx, min_x, max_y, max_x, max_y, 0xFFFF0000);
    // fb_draw_line(&ctx, max_x, min_y, max_x, max_y, 0xFFFF0000);
    
    // fb_draw_line(&ctx, v0.x, v0.y, v1.x, v1.y, 0xFFB4DD13);
    // fb_draw_line(&ctx, v1.x, v1.y, v2.x, v2.y, 0xFFB4DD13);
    // fb_draw_line(&ctx, v2.x, v2.y, v0.x, v0.y, 0xFFB4DD13);
    
}

void rasterize_quad(vector3 v0, vector3 v1, vector3 v2, vector3 v3){
    //Stub
}

tern draw(int segment_index, t2d_encode_job *job, vector2 origin, gpu_size screen, u32 *fb){
    int *segments = t2d_get_buffer_ptr(job->sbuf);
    vector3 *verts = t2d_get_buffer_ptr(job->vbuf);
    
    int i0 = segments[segment_index+0];
    int i1 = segments[segment_index+1];
    int i2 = job->pipeline.prim_type > primitives_line ? segments[segment_index+2] : -1;
    int i3 = job->pipeline.prim_type > primitives_trig ? segments[segment_index+3] : -1;
    assert_true(i0 >= 0 && (uint64_t)i0 < job->vcount, "Wrong i0 %i at seg %i. Num verts %i",segment_index+0,i0,job->vcount);
    assert_true(i1 >= 0 && (uint64_t)i1 < job->vcount, "Wrong i1 %i at seg %i. Num verts %i",segment_index+1,i1,job->vcount);
    if (i2 != -1) assert_true(i2 >= 0 && (uint64_t)i2 < job->vcount, "Wrong i2 %i at seg %i. Num verts %i",segment_index+2,i2,job->vcount);
    if (i3 != -1) assert_true(i3 >= 0 && (uint64_t)i3 < job->vcount, "Wrong i3 %i at seg %i. Num verts %i",segment_index+3,i3,job->vcount);
    
    vector4 c0 = job->pipeline.vert_shader(verts[i0]);
    vector4 c1 = job->pipeline.vert_shader(verts[i1]);
    vector4 c2 = i2 > -1 ? job->pipeline.vert_shader(verts[i2]) : (vector4){};
    vector4 c3 = i3 > -1 ? job->pipeline.vert_shader(verts[i3]) : (vector4){};
    
    if (should_clip(c0) && should_clip(c1)
        && (i2 > -1 ? should_clip(c2) : true)
        && (i3 > -1 ? should_clip(c3) : true)
    ) return false;
    
    //NDC
    vector3 v0 = {c0.x/c0.w,c0.y/c0.w,c0.z/c0.w};
    vector3 v1 = {c1.x/c1.w,c1.y/c1.w,c1.z/c1.w};
    vector3 v2 = i2 < 0 ? (vector3){} : (vector3){c2.x/c2.w,c2.y/c2.w,c2.z/c2.w};
    vector3 v3 = i3 < 0 ? (vector3){} : (vector3){c3.x/c3.w,c3.y/c3.w,c3.z/c3.w};
    
    vector3 s0 = {(v0.x+1)*0.5f*(screen.width-1),(1-((v0.y+1)*0.5f))*(screen.height-1),v0.z};
    vector3 s1 = {(v1.x+1)*0.5f*(screen.width-1),(1-((v1.y+1)*0.5f))*(screen.height-1),v1.z};
    
    float min_depth = min(c0.w,c1.w);
    
    if (i2 >= 0){
        vector3 s2 = {(v2.x+1)*0.5f*(screen.width-1),(1-((v2.y+1)*0.5f))*(screen.height-1),v2.z};
        min_depth = min(min_depth,c2.w);
        if (i3 >= 0){
            min_depth = min(min_depth,c3.w);
            vector3 s3 = {(v3.x+1)*0.5f*(screen.width-1),(1-((v3.y+1)*0.5f))*(screen.height-1),v3.z};
            rasterize_quad(s0,s1,s2,s3);
        } else {
            rasterize_triangle(s0,s1,s2,segment_index, 1 + floor(min_depth/50), screen, job->pipeline.fb, job->pipeline.zbuf, job->pipeline.frag_shader);
        }
    }
    
    return true;
}

t2d_pipeline t2d_make_pipeline(u32 width, u32 height, u32 *fb){
    t2d_pipeline pipeline = {
        .do_backface_culling = true,
        .fb = fb,
        .screen_size = (gpu_size){ width, height},
        .prim_type = primitives_trig,
        .z_buf_size = width * height * sizeof(float),
    };
    pipeline.zbuf = zalloc(pipeline.z_buf_size);
    return pipeline;
}

void t2d_clear(t2d_encode_job *job, argbcolor color){
    u32 *fb = job->pipeline.fb;
    u32 w = job->pipeline.screen_size.width;
    u32 h = job->pipeline.screen_size.height;
    for (u32 y = 0; y < h; y++)
        for (u32 x = 0; x < w; x++)
            fb[(y * w) + x] = color.color;
}

chunk_array_t *buffers;

int t2d_create_raw_buffer(size_t elem_size, size_t count){
    if (!buffers){
        buffers = chunk_array_create(sizeof(void*), 16);
    }
    void *ptr = zalloc(elem_size * count);
    return chunk_array_push(buffers, &ptr);
}

void* t2d_get_buffer_ptr(int buf_index){
    if (!buffers) return 0;
    if (chunk_array_count(buffers) <= buf_index) return 0;
    
    return *(void**)chunk_array_get(buffers, buf_index);
}

int job_id = 1;

t2d_encode_job t2d_begin_encode(t2d_pipeline pipeline){
    memset(pipeline.zbuf, 0, pipeline.z_buf_size);
    return (t2d_encode_job){
        .id = job_id++,
        .vbuf = 0,
        .sbuf = 0,
        .pipeline = pipeline
    };
}

void t2d_set_buffer(t2d_encode_job *job, int buf_index, int count, buffer_type type){
    switch (type) {
    case BUF_VERTICES:
        job->vbuf = buf_index;
        job->vcount = count;
        break;
    case BUF_SEGMENTS:
        job->sbuf = buf_index;
        job->scount = count;
        break;
    default:
      break;
    }
}

vector2 mid;

extern t2d_pipeline *current_pipeline;

tern t2d_commit_encode(t2d_encode_job *job){
    current_pipeline = &job->pipeline;
    if (!mid.x || !mid.y) mid = (vector2){job->pipeline.screen_size.width/2.f,job->pipeline.screen_size.height/2.f};
    for (size_t i = 0; i < job->scount/job->pipeline.prim_type; i++){
        if (draw((i * job->pipeline.prim_type), job, mid, (gpu_size){job->pipeline.screen_size.width,job->pipeline.screen_size.height}, job->pipeline.fb) == -1) return -1;
    }
    return true;
}