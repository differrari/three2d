#include "graphic_types.h"
#include "three2d.h"
#include "mesh.h"
#include "input_keycodes.h"
#include "debug/profiler.h"
#include "math/math.h"
#include "geolisp/geolisp.h"
#include "files/helpers.h"

extern vector3 camera_pos;

int main(int argc, char* argv[]){

    draw_ctx ctx;
    ctx.width = 1920;
    ctx.height = 1080;
    request_draw_ctx(&ctx);

    geolisp_init();
    size_t ssize = 0;
    char *script = read_full_file("examples/geolisp.lisp", &ssize);
    geolisp_eval((string_slice){script,ssize});
    
    t2d_pipeline pipeline = t2d_make_pipeline(ctx.width, ctx.height, ctx.fb);

    pipeline.debug_options = 0;
    
    pipeline.do_backface_culling = false;
    pipeline.downscale = 1;
    
    mesh *m = geolisp_get_mesh(0);
    if (!m) {
        print("No mesh");
        return 0;
    }
    
    t2d_vert_shader(&pipeline, default_vert_shader);
    t2d_frag_shader(&pipeline, default_frag_shader);
    
    size_t vertices_count = mesh_num_verts(m);
    size_t segments_count = chunk_array_count(m->segments);

    print("VERT COUNT %i SEG COUNT %i",vertices_count,segments_count);
    
    int vbuf = t2d_create_buffer(BUF_VERTICES, vertices_count);
    int sbuf = t2d_create_buffer(BUF_SEGMENTS, segments_count);
    
    t2d_make_perspective_camera(&pipeline, 72, (float)ctx.width/(float)ctx.height, 0.1f, 100);
    
    vector3 *vbuf_ptr = t2d_get_buffer_ptr(vbuf);
    int *sbuf_ptr = t2d_get_buffer_ptr(sbuf);
    
    for (size_t i = 0; i < vertices_count; i++){
        vbuf_ptr[i] = mesh_get_vertex(m, i);
        print("V: %f,%f,%f",vbuf_ptr[i].x,vbuf_ptr[i].y,vbuf_ptr[i].z);
    }
    
    for (size_t i = 0; i < segments_count; i++){
        sbuf_ptr[i] = mesh_get_segment(m, i);
        print("S: %i",sbuf_ptr[i]);
    }
    
    profiler_init();
    u64 frames;
    u64 avg;
    while (true){
        t2d_encode_job enc = t2d_begin_encode(pipeline);
        t2d_clear(&enc, (argbcolor){.color = 0xFFFFFFFF});
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
        print("FPS %f",1.f/((double)d/1000));
        frames++;
        
        mouse_data mouse = {};
        get_mouse_status(&mouse);
        float adj_dt = minf(0.03f,((double)d/1000));
        if (mouse_button_down(&mouse, 1)){
            camera_pos.x += mouse.raw.x * adj_dt;
            camera_pos.y += mouse.raw.y * adj_dt;
        }
        camera_pos.z -= mouse.raw.scroll * 250 * adj_dt;
        camera_pos.z = maxf(0,camera_pos.z);
    }

    destroy_draw_ctx(&ctx);
    return 0;
       
}