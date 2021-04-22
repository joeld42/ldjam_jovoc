//#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define SOKOL_IMPL
#define SOKOL_GLES2
#include "sokol_gfx.h"
#include "sokol_fetch.h"
#include "sokol_gl.h"
#include "sokol_debugtext.h"
#include "emsc.h"

#include "stb_image.h"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

static sg_pipeline pip;
static sg_bindings bind;
static sg_pass_action pass_action;

typedef struct {
    glm::mat4 mvp;
} params_t;

// NOTES: sgl_begin_line_loop


static uint8_t file_buffer[512 * 1024];
static void draw();
static void fetch_callback(const sfetch_response_t*);

int main()
{
	// setup WebGL1 context, no antialias
	emsc_init("#canvas", EMSC_NONE );

	// setup sokol_gfx
	sg_desc desc = {};
	sg_setup( &desc );
	//assert( gl_isvalid() );

    /* setup sokol-gl */
    sgl_desc_t sgl_desc ={
        .sample_count = 1
    };

    sgl_setup(&sgl_desc);

    sdtx_desc_t sdtx_desc = {};
    sdtx_desc.fonts[0] = sdtx_font_c64();
    sdtx_setup( &sdtx_desc );
    sdtx_font(0);

	/* setup sokol-fetch with the minimal "resource limits" */
	sfetch_desc_t sfetch_desc = {
		.max_requests = 1,
        .num_channels = 1,
        .num_lanes = 1
	};
    sfetch_setup(&sfetch_desc);

    // Start our sfetch request
    sfetch_request_t fetch_request = {
		.path = "player.png",
        //.path = "genericItems_spritesheet_colored.png",
        .callback = fetch_callback,
        .buffer_ptr = file_buffer,
        .buffer_size = sizeof(file_buffer)
    };
 	sfetch_send(&fetch_request);

	pass_action = {};
	pass_action.colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.0f, 0.0f, 0.0f, 1.0f } };

	/* a vertex buffer */
    float vertices[] = {
        // positions            colors                    // texcoord
        -0.5f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f,  0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f
    };
    
    sg_buffer_desc vert_buffer_desc = {
        .size = sizeof(vertices),
        //.content = vertices,    
    };
    bind.vertex_buffers[0] = sg_make_buffer(&vert_buffer_desc);

    /* an index buffer */
    uint16_t indices[] = {
        0, 1, 2,    // first triangle
        0, 2, 3,    // second triangle        
    };
    sg_buffer_desc index_buffer_desc = {
        .size = sizeof(indices),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        //.content = indices,
    };
    bind.index_buffer = sg_make_buffer(&index_buffer_desc);
    

    /* create a shader */
    sg_shader_desc shader_desc = {};
    shader_desc.vs.source =
			"uniform mat4 mvp;\n"
            "attribute vec4 position;\n"
            "attribute vec4 color0;\n"
            "attribute vec2 texcoord0;\n"
            "varying vec4 color;\n"
            "varying vec2 uv;"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
            //"  gl_Position = position;\n"
            "  color = color0;\n"
            "  uv = texcoord0;\n"
            "}\n";
	shader_desc.fs.source =
            "precision mediump float;\n"
            "uniform sampler2D tex;\n"
            "varying vec4 color;\n"
            "varying vec2 uv;\n"
            "void main() {\n"
            "  gl_FragColor = texture2D(tex, uv) * color;\n"
            //"  gl_FragColor = vec4( uv.x, uv.y, 0.5, 1.0 );\n"
            "}\n";
    shader_desc.attrs[0].name = "position";
	shader_desc.attrs[1].name = "color0";
	shader_desc.attrs[2].name = "texcoord0";

	shader_desc.fs.images[0] = { .name = "tex", .type=SG_IMAGETYPE_2D };

	shader_desc.vs.uniform_blocks[0] = {
		.size = sizeof(params_t)
	};
	shader_desc.vs.uniform_blocks[0].uniforms[0] = { .name="mvp", .type = SG_UNIFORMTYPE_MAT4 };

	sg_shader shd = sg_make_shader(&shader_desc);

	/* create a checkerboard texture */
    uint32_t pixels[4*4] = {
        0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
        0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
        0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
    };
    sg_image_desc image_desc = {
        .width = 4,
        .height = 4,
    };
    image_desc.content.subimage[0][0] = {
            .ptr = pixels,
            .size = sizeof(pixels)    	
    };
    bind.fs_images[0] = sg_make_image(&image_desc);

    
    /* a pipeline object (default render state is fine) */
    sg_pipeline_desc pipeline_desc = {
        .layout = { },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16
    };
    pipeline_desc.layout.attrs[0].format=SG_VERTEXFORMAT_FLOAT3;
    pipeline_desc.layout.attrs[1].format=SG_VERTEXFORMAT_FLOAT4;
    pipeline_desc.layout.attrs[2].format=SG_VERTEXFORMAT_FLOAT2;
    pip = sg_make_pipeline(&pipeline_desc);


	emscripten_set_main_loop( draw, 0, 1 );
}

static void fetch_callback(const sfetch_response_t* response) {
    if (response->fetched) {
        /* the file data has been fetched, since we provided a big-enough
           buffer we can be sure that all data has been loaded here
        */
        int png_width, png_height, num_channels;
        const int desired_channels = 4;
        stbi_uc* pixels = stbi_load_from_memory(
            (stbi_uc const *)response->buffer_ptr,
            (int)response->fetched_size,
            &png_width, &png_height,
            &num_channels, desired_channels);
        if (pixels) {            
            /* ok, time to actually initialize the sokol-gfx texture */
            sg_image_desc image_desc = {
                .width = png_width,
                .height = png_height,
                .pixel_format = SG_PIXELFORMAT_RGBA8,
                .min_filter = SG_FILTER_NEAREST,
                .mag_filter = SG_FILTER_NEAREST,
            };
            image_desc.content.subimage[0][0].ptr = pixels;
            image_desc.content.subimage[0][0].size = (size_t)(png_width * png_height * 4);

            sg_init_image(bind.fs_images[ 0 ], &image_desc );

            // sg_init_image(bind.fs_images[SLOT_tex], &(sg_image_desc){
            //     .width = png_width,
            //     .height = png_height,
            //     .pixel_format = SG_PIXELFORMAT_RGBA8,
            //     .min_filter = SG_FILTER_LINEAR,
            //     .mag_filter = SG_FILTER_LINEAR,
            //     .data.subimage[0][0] = {
            //         .ptr = pixels,
            //         .size = (size_t)(png_width * png_height * 4),
            //     }
            //});
            stbi_image_free(pixels);
        }
    }
    else if (response->failed) {
        // TODO: indicate error 

    }
}
void draw()
{
    /* pump the sokol-fetch message queues, and invoke response callbacks */
    sfetch_dowork();

	/* animate clear colors */
	// float g = pass_action.colors[0].val[1] + 0.01f;
	// if (g > 1.0f) g = 0.0f;
	// pass_action.colors[0].val[1] = g;

	/* draw one frame */
	// sg_begin_default_pass( &pass_action, emsc_width(), emsc_height() );
	// sg_end_pass();
	// sg_commit();

	params_t vs_params = {};
	
	float aspect = (float)emsc_width() / (float)emsc_height();
	glm::mat4 proj = glm::ortho( -1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f, -1.0f, 1.0f );

	vs_params.mvp = glm::mat4(1.0) * proj;
    // printf("Mvp is %f %f %f %f\n%f %f %f %f\n",
    // 	vs_params.mvp[0].x, vs_params.mvp[0].y, vs_params.mvp[0].z, vs_params.mvp[0].w,
    // 	vs_params.mvp[1].x, vs_params.mvp[1].y, vs_params.mvp[1].z, vs_params.mvp[1].w );

    sdtx_canvas(emsc_width() * 0.5f, emsc_height() * 0.5f);
    sdtx_origin(3.0f, 3.0f);
    sdtx_color3b( 0x20, 0xFF, 0xFF );
    sdtx_printf("Hello there\n" );

    sdtx_color3b( 0x20, 0x1E, 0xFF );
    sdtx_canvas(emsc_width(), emsc_height());
    sdtx_printf("Smol text\n" );


	sg_begin_default_pass(&pass_action, emsc_width(), emsc_height());
        
    sgl_defaults();
    sgl_load_matrix( glm::value_ptr( proj ));

    sgl_c3f( 1.0f, 0.0f, 1.0f );
    sgl_begin_line_strip();
    float m = 0.95f;
    sgl_v3f( -m, -m, 0.0f );
    sgl_v3f(  m, -m, 0.0f );
    sgl_v3f(  m,  m, 0.0f );
    sgl_v3f( -m,  m, 0.0f );

    sgl_v3f( -m, -m, 0.0f );
    sgl_end();
    

    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_apply_uniforms( SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
    sg_draw(0, 6, 1);
    
    // draw sgl stuff
    sgl_draw();

    // draw sokol debug text
    sdtx_draw();

    sg_end_pass();
    sg_commit();

}