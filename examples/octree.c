#include "three2d.h"
#include "mesh.h"
#include "input_keycodes.h"
#include "debug/profiler.h"
#include "math/math.h"

extern vector3 camera_pos;

mesh make_voxel_mesh(){
    mesh m = {};
    m.vertices = chunk_array_create(sizeof(vector3), 1000);
    m.segments = chunk_array_create(sizeof(int), 1000);
    m.primitive_type = primitives_trig;
    return m;
}

void push_trig(chunk_array_t *segments, int a, int b, int c){
    // print("TRIG %i, %i, %i",a,b,c);
    chunk_array_push(segments, &a);
    chunk_array_push(segments, &b);
    chunk_array_push(segments, &c);
}

void make_cube(mesh *m, vector3 base, float size, int num_segments){
    int max_seg = num_segments-1;
    
    int current = mesh_num_verts(m);
    for (int z = 0; z <= max_seg; z++){
        //Front & Back
        push_trig(m->segments, current + (z == 0 ? 2 : 0), current + 1, current + (z == 0 ? 0 : 2));
        push_trig(m->segments, current + (z == 0 ? 1 : 3), current + 2, current + (z == 0 ? 3 : 1));
        if (z > 0){//Sides
            push_trig(m->segments, current - 2, current - 4, current);
            push_trig(m->segments, current + 2, current - 2, current);
            
            push_trig(m->segments, current + 1, current - 4 + 1, current - 2 + 1);
            push_trig(m->segments, current + 1, current - 2 + 1, current + 2 + 1);
        }
        if (z < max_seg){//Top & Bottom
            push_trig(m->segments, current, current + 1, current + 4);
            push_trig(m->segments, current + 4 + 1, current + 4, current+ 1);
            
            push_trig(m->segments, current + 2 + 4, current + 2 + 1, current + 2);
            push_trig(m->segments, current + 2 + 1, current + 2 + 4, current + 2 + 4 + 1);
        }
        for (int y = 0; y <= max_seg; y++){
            for (int x = 0; x <= max_seg; x++){
                current = chunk_array_push(m->vertices, &(vector3){ base.x + x * size, base.y + y * size, base.z + z * size}) + 1;
            }
        }
    }
}

void make_octree(mesh *m, vector3 base, int depth){
    float size = (float)(1 << depth);
    
    if (depth == 0){
        make_cube(m, base, size, 2);
        return;
    }
    
    float nsize = (float)(1 << (depth - 1));
    for (int z = 0; z <= 1; z++)
        for (int y = 0; y <= 1; y++)
            for (int x = 0; x <= 1; x++){
                vector3 loc = (vector3){base.x + x * nsize,base.y + y * nsize,base.z + z * nsize};
                if (!x && !y && z)
                    make_octree(m, loc, depth-1);
                else
                    make_cube(m, loc, nsize, 2);
            }
}

int main(int argc, char* argv[]){
    draw_ctx ctx;
    ctx.width = 1920;
    ctx.height = 1080;
    request_draw_ctx(&ctx);
    
    t2d_pipeline pipeline = t2d_make_pipeline(ctx.width, ctx.height, ctx.fb);
    
    pipeline.do_backface_culling = true;
    
    mesh m = make_voxel_mesh();
    make_octree(&m, (vector3){0,0,-20}, 5);
    
    t2d_vert_shader(&pipeline, default_vert_shader);
    t2d_frag_shader(&pipeline, default_frag_shader);
    
    size_t vertices_count = mesh_num_verts(&m);
    size_t segments_count = chunk_array_count(m.segments);
    
    int vbuf = t2d_create_buffer(BUF_VERTICES, vertices_count);
    int sbuf = t2d_create_buffer(BUF_SEGMENTS, segments_count);
    
    t2d_make_perspective_camera(&pipeline, 72, (float)ctx.width/(float)ctx.height, 0.1f, 100);
    
    vector3 *vbuf_ptr = t2d_get_buffer_ptr(vbuf);
    int *sbuf_ptr = t2d_get_buffer_ptr(sbuf);
    
    for (size_t i = 0; i < vertices_count; i++){
        vbuf_ptr[i] = mesh_get_vertex(&m, i);
    }
    
    for (size_t i = 0; i < segments_count; i++){
        sbuf_ptr[i] = mesh_get_segment(&m, i);
    }
    
    profiler_init();
    u64 frames;
    u64 avg;
    while (true){
        t2d_encode_job enc = t2d_begin_encode(pipeline);
        t2d_clear(&enc, (argbcolor){});
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
        // print("Delta %f",1.f/((double)d/1000));
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