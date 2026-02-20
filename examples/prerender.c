#include "three2d.h"
#include "files/helpers.h"
#include "mesh.h"
#include "obj.h"
#include "input_keycodes.h"
#include "image/bmp.h"

static vector3 camera = {0,0,4};

extern t2d_pipeline *current_pipeline;

vector4 pixel_vert(vector3 pos){
    vector4 cam_pos = (vector4){pos.x + camera.x, pos.y - camera.y, pos.z - camera.z, 1};
    
    vector4 out_v = matrix4_mul(cam_pos, current_pipeline->proj_matrix);
    
    return out_v;
}

int main(int argc, char *argv[]){
    gpu_size img_size = {1920,1080};
    
    size_t data_offset = sizeof(bmp_header) + 10;
    size_t bmp_size = (sizeof(u32) * img_size.width * img_size.height) + data_offset;
    
    bmp_header *header = allocate_bmp_file(img_size.width, img_size.height);

    void *buffer = (void*)((uintptr_t)header + header->data_offset);
    
    t2d_pipeline pipeline = t2d_make_pipeline(img_size.width, img_size.height, buffer);
    
    pipeline.downscale = 3;
    
    size_t file_size = 0;
    char *file = read_full_file("/resources/Computer.obj",&file_size);
    mesh m = parse_obj(file, file_size, primitives_trig);
    
    t2d_vert_shader(&pipeline, pixel_vert);
    t2d_frag_shader(&pipeline, default_frag_shader);
    
    size_t vertices_count = mesh_num_verts(&m);
    size_t segments_count = chunk_array_count(m.segments);
    
    int vbuf = t2d_create_buffer(BUF_VERTICES, vertices_count);
    int sbuf = t2d_create_buffer(BUF_SEGMENTS, segments_count);
    
    // t2d_make_orthographic_camera(&pipeline, 0, img_size.height, 0, img_size.width, 0.1f, 100);
    t2d_make_perspective_camera(&pipeline, 72, (float)img_size.width/(float)img_size.height, 0.1f, 100);
    
    vector3 *vbuf_ptr = t2d_get_buffer_ptr(vbuf);
    int *sbuf_ptr = t2d_get_buffer_ptr(sbuf);
    
    for (size_t i = 0; i < vertices_count; i++){//CRED: increments
        vbuf_ptr[i] = mesh_get_vertex(&m, i);
    }
    
    for (size_t i = 0; i < segments_count; i++){//CRED: increments
        sbuf_ptr[i] = mesh_get_segment(&m, i);
    }
    
    t2d_encode_job enc = t2d_begin_encode(pipeline);
    t2d_set_buffer(&enc, vbuf, vertices_count, BUF_VERTICES);
    t2d_set_buffer(&enc, sbuf, segments_count, BUF_SEGMENTS);
    if (t2d_commit_encode(&enc) < 0){
        print("Encoding error");
    }
    
    write_full_file("test.bmp", header, header->file_size);
    
    return 0;
}