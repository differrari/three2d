#include "three2d.h"
#include "files/helpers.h"
#include "obj.h"
#include "input_keycodes.h"
#include "debug/profiler.h"

int main(int argc, char* argv[]){
    draw_ctx ctx;
    ctx.width = 1920;
    ctx.height = 1080;
    request_draw_ctx(&ctx);
    
    t2d_pipeline pipeline = t2d_make_pipeline(ctx.width, ctx.height, ctx.fb);
    
    size_t file_size = 0;
    char *file = read_full_file("/resources/Zuzie.obj",&file_size);
    mesh m = parse_obj(file, file_size, primitives_trig);
    
    t2d_vert_shader(&pipeline, default_vert_shader);
    t2d_frag_shader(&pipeline, default_frag_shader);
    
    int vertices_count = chunk_array_count(m.vertices);
    int segments_count = chunk_array_count(m.segments);
    
    int vbuf = t2d_create_buffer(BUF_VERTICES, vertices_count);
    int sbuf = t2d_create_buffer(BUF_SEGMENTS, segments_count);
    
    print("Vert buffer %i Seg buffer %i",vbuf,sbuf);
    
    print("%i segments",segments_count);
    
    t2d_make_camera(&pipeline, 72, (float)ctx.width/(float)ctx.height, 0.1f, 100);
    
    size_t num_segments = mesh_num_segments(&m);
    size_t num_verts = mesh_num_verts(&m);
    
    vector3 *vbuf_ptr = t2d_get_buffer_ptr(vbuf);
    int *sbuf_ptr = t2d_get_buffer_ptr(sbuf);
    
    for (int i = 0; i < vertices_count; i++)
        vbuf_ptr[i] = *(vector3*)chunk_array_get(m.vertices, i);
    
    for (int i = 0; i < segments_count; i++)
        sbuf_ptr[i] = *(int*)chunk_array_get(m.segments, i);
    
    profiler_init();
    u64 frames;
    u64 avg;
    while (true){
        t2d_encode_job enc = t2d_begin_encode(pipeline);
        t2d_set_buffer(&enc, vbuf, vertices_count, BUF_VERTICES);
        t2d_set_buffer(&enc, sbuf, segments_count, BUF_SEGMENTS);
        if (t2d_commit_encode(&enc) < 0){
            print("Encoding error");
            break;
        }
        
        commit_draw_ctx(&ctx);
        
        kbd_event ev = {};
        read_event(&ev);
        if (ev.key == KEY_ESC){
            destroy_draw_ctx(&ctx);
            return 0;
        } 
        
        u64 d = profiler_delta();
        avg += d;
        print("Delta %f",1.f/((double)d/1000));
        frames++;
        
    }

    destroy_draw_ctx(&ctx);
    return 0;
       
}