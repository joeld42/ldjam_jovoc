//#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define SOKOL_IMPL
#define SOKOL_GLES2
#include "sokol_gfx.h"
#include "sokol_fetch.h"
#include "sokol_time.h"
#include "sokol_gl.h"
#include "sokol_debugtext.h"
#include "emsc.h"

#include "stb_image.h"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/color_space.hpp"

#include <stdio.h>  // needed by fontstash's IO functions even though they are not used
#define FONTSTASH_IMPLEMENTATION
#if defined(_MSC_VER )
#pragma warning(disable:4996)   // strncpy use in fontstash.h
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#define FONTSTASH_IMPLEMENTATION
#include "fontstash/fontstash.h"
#define SOKOL_FONTSTASH_IMPL
#include "sokol_fontstash.h"

#include "tk_sprite.h"
#include "tk_tilemap.h"

static sg_pass_action pass_action;
bool testimgloaded = false;

typedef struct {
    glm::mat4 mvp;
} params_t;

typedef struct FontInfo_s {
    FONScontext* fons;
    //float dpi_scale;
    int font_normal;
    uint8_t font_normal_data[256 * 1024];
} FontInfo;
static FontInfo font;


// Include level data
#include "level-data.cpp"

// NOTES: sgl_begin_line_loop
static void draw();

typedef struct GameState_s
{
    TKSpriteHandle spritePlayer;
    TKSpriteHandle spriteTilemap;

    uint64_t ticks;

    glm::vec3 camFocus;
    glm::vec3 camTarget;

    RoomInfo *currRoom;

    float angleBGSprite, angleFGSprite;
} GameState;
GameState game;

static void font_normal_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        font.font_normal = fonsAddFontMem( font.fons, (const char *)"sans", (unsigned char *)response->buffer_ptr, response->fetched_size,  false);
    }
}

int main()
{
	// setup WebGL1 context, no antialias
	emsc_init("#canvas", EMSC_NONE );

    // setup timer
    stm_setup();

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
		.max_requests = 4,
        .num_channels = 1,
        .num_lanes = 1
	};
    sfetch_setup(&sfetch_desc);

    // Setup fonts
    int atlas_dim = 512;
    FONScontext* fons_context = sfons_create(atlas_dim, atlas_dim, FONS_ZERO_TOPLEFT);
    font.fons = fons_context;
    font.font_normal = FONS_INVALID;

    sfetch_request_t fontRequest = {     
        .path = "bobcaygr.ttf",
        .callback = font_normal_loaded,
        .buffer_ptr = font.font_normal_data,
        .buffer_size = sizeof(font.font_normal_data)
    };
    sfetch_send(&(fontRequest));

    float pxSize = 1.0 / emsc_height();
    TKSpriteSystemDesc spriteDesc = {
        .pixelSizeX = pxSize,
        .pixelSizeY = pxSize,
    };
    tk_sprite_init_system( spriteDesc );

    // Set up game resources
    game.spritePlayer = tk_sprite_make_st( "player.png", glm::vec2( 0.0, 0.0 ), glm::vec2( 0.25, 0.5 ) );

    game.spriteTilemap = tk_sprite_make( "dreams_tileset.png");

    game.currRoom = &room_Level_0;

	emscripten_set_main_loop( draw, 0, 1 );
}

void draw()
{
    /* pump the sokol-fetch message queues, and invoke response callbacks */
    sfetch_dowork();

    /* text rendering via fontstash.h */
    float sx, sy, dx, dy, lh = 0.0f;
    uint32_t white32 = sfons_rgba(255, 255, 255, 255);
    uint32_t black32 = sfons_rgba(0, 0, 0, 255);
    uint32_t brown32 = sfons_rgba(192, 128, 0, 128);
    uint32_t blue32  = sfons_rgba(0, 192, 255, 255);
    fonsClearState(font.fons);

    FONScontext* fs = font.fons;

	// ======================================================================
    //    Update the things  
    // ======================================================================
    uint64_t frame_ticks = stm_laptime( &game.ticks );
    float dt = stm_sec( frame_ticks );

    game.angleFGSprite += 4.0f * dt;
    game.angleBGSprite -= 3.2f * dt;

    if (game.currRoom)
    {
        game.camTarget = glm::vec3( (float)game.currRoom->worldX + 8.0f,
                                    (float)game.currRoom->worldY - 5.5f,
                                    0.0f );
    }
    game.camFocus = glm::mix( game.camFocus, game.camTarget,  0.05f );

    // ======================================================================
    //    Render the things  
    // ======================================================================
	params_t vs_params = {};
	
	float aspect = (float)emsc_width() / (float)emsc_height();
    float viewHite = 20.5f;
	glm::mat4 proj = glm::ortho( -viewHite * aspect, viewHite * aspect, -viewHite, viewHite, -1.0f, 1.0f );
    glm::mat4 cam = glm::translate( proj, -game.camFocus );


	vs_params.mvp = cam;
    // printf("Mvp is %f %f %f %f\n%f %f %f %f\n",
    // 	vs_params.mvp[0].x, vs_params.mvp[0].y, vs_params.mvp[0].z, vs_params.mvp[0].w,
    // 	vs_params.mvp[1].x, vs_params.mvp[1].y, vs_params.mvp[1].z, vs_params.mvp[1].w );

    sdtx_canvas(emsc_width() * 0.5f, emsc_height() * 0.5f);
    sdtx_origin(3.0f, 3.0f);
    sdtx_color3b( 0x20, 0xFF, 0xFF );
    //sdtx_printf("I am a breakout game\n" );

    // sdtx_color3b( 0x20, 0x1E, 0xFF );
    // sdtx_canvas(emsc_width(), emsc_height());
    // sdtx_printf("Smol text\n" );


	sg_begin_default_pass(&pass_action, emsc_width(), emsc_height());
        
    sgl_defaults();
    sgl_load_matrix( glm::value_ptr( cam ));

    sgl_c3f( 1.0f, 0.0f, 1.0f );
    sgl_begin_line_strip();
    //float m = 0.95f;
    sgl_v3f( 0.0f, 0.0f, 0.0f );
    sgl_v3f( 16.0f, 0.0f, 0.0f );
    sgl_v3f( 16.0f, -11.0f, 0.0f );
    sgl_v3f( 0.0f, -11.0f, 0.0f );
    
    sgl_v3f( 0.0f, 0.0f, 0.0f );

    sgl_end();
    

    // sg_apply_pipeline(pip);
    // sg_apply_bindings(&bind);
    // sg_apply_uniforms( SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
    //sg_draw(0, 6, 1);

    // draw sprites
    glm::vec4 white = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    tk_push_sprite_all( game.spritePlayer, glm::vec3( 0.0f, 0.3f, 0.0f ), 20.0f, white, game.angleBGSprite );
        
    
    for (int i=0; i < MAX_ROOMS; i++)
    {
        RoomInfo* room = world.rooms[i];
        tk_push_sprite_tilemap( game.spriteTilemap, room, glm::vec3( room->worldX, room->worldY, 0.0f ));
    }
    
    
    //tk_push_sprite( game.spriteTilemap, glm::vec3( 0.0f, 0.0f, 0.0f ));
    tk_push_sprite_all( game.spritePlayer, glm::vec3( 0.5f, 0.0f, 0.0f ), 10.0f, white, game.angleFGSprite );

    tk_sprite_drawgroups( vs_params.mvp );

    sgl_ortho(0.0f, (float)emsc_width(), (float)emsc_height(), 0.0f, -1.0f, +1.0f);

    //draw text
    float cx, cy;
    cx = 2.0f;
    cy = 150.0f;

    sgl_c3f( 0.0f, 1.0f, 1.0f );
    sgl_begin_line_strip();
    
    sgl_v3f( 0, 0, 0.0f );
    sgl_v3f(  10, 0, 0.0f );
    sgl_v3f(  10,  10, 0.0f );
    sgl_v3f( 0,  10, 0.0f );

    sgl_v3f( 0, 0, 0.0f );
    sgl_end();
    
    
    if (font.font_normal != FONS_INVALID) {
        fonsSetFont(fs, font.font_normal);
        fonsSetSize(fs, 100.0f );
        fonsSetColor(fs, white32 );
        cx = fonsDrawText(fs, cx, cy, "Game Text", NULL);
        sdtx_printf("Drawing game text\n");
    }

    // draw sgl stuff
    sgl_draw();

    // draw sokol debug text
    sdtx_draw();

    /* flush fontstash's font atlas to sokol-gfx texture */
    sfons_flush(fs);

    sg_end_pass();
    sg_commit();

}