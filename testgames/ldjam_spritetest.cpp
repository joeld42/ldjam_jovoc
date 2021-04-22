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

#include "tk_sprite.h"

static sg_pass_action pass_action;
bool testimgloaded = false;

typedef struct {
    glm::mat4 mvp;
} params_t;

// NOTES: sgl_begin_line_loop
static void draw();

typedef struct TestThing_s
{
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec4 tint;
    float ang;
    float rot;
    float scl;
    TKSpriteHandle sprite;
} TestThing;

#define NUM_THINGS (300)
typedef struct GameState_s
{
    TKSpriteHandle spritePlayer;
    TKSpriteHandle spriteItems[200];

    TestThing thing[NUM_THINGS];
    uint64_t ticks;

    float angleBGSprite, angleFGSprite;
} GameState;
GameState game;

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

    float pxSize = 1.0 / emsc_height();
    TKSpriteSystemDesc spriteDesc = {
        .pixelSizeX = pxSize,
        .pixelSizeY = pxSize,
    };
    tk_sprite_init_system( spriteDesc );

    // Set up game resources
    game.spritePlayer = tk_sprite_make_st( "player.png", glm::vec2( 0.0, 0.0 ), glm::vec2( 0.25, 0.5 ) );
    
    TKSpritePixelRect itemRects[] = {
    { .x = 0, .y = 322, .width = 162, .height = 94 }, /* genericItem_color_001 */ 
    { .x = 136, .y = 1231, .width = 120, .height = 110 }, /* genericItem_color_002 */ 
    { .x = 130, .y = 1791, .width = 89, .height = 45 }, /* genericItem_color_003 */ 
    { .x = 623, .y = 1826, .width = 70, .height = 88 }, /* genericItem_color_004 */ 
    { .x = 626, .y = 135, .width = 66, .height = 105 }, /* genericItem_color_005 */ 
    { .x = 366, .y = 1095, .width = 96, .height = 84 }, /* genericItem_color_006 */ 
    { .x = 628, .y = 0, .width = 60, .height = 95 }, /* genericItem_color_007 */ 
    { .x = 623, .y = 1914, .width = 68, .height = 102 }, /* genericItem_color_008 */ 
    { .x = 551, .y = 1305, .width = 76, .height = 104 }, /* genericItem_color_009 */ 
    { .x = 553, .y = 1826, .width = 70, .height = 104 }, /* genericItem_color_010 */ 
    { .x = 551, .y = 1495, .width = 75, .height = 113 }, /* genericItem_color_011 */ 
    { .x = 261, .y = 993, .width = 106, .height = 102 }, /* genericItem_color_012 */ 
    { .x = 556, .y = 174, .width = 70, .height = 119 }, /* genericItem_color_013 */ 
    { .x = 380, .y = 88, .width = 94, .height = 144 }, /* genericItem_color_014 */ 
    { .x = 552, .y = 366, .width = 74, .height = 131 }, /* genericItem_color_015 */ 
    { .x = 276, .y = 90, .width = 104, .height = 188 }, /* genericItem_color_016 */ 
    { .x = 365, .y = 1459, .width = 98, .height = 152 }, /* genericItem_color_017 */ 
    { .x = 691, .y = 1001, .width = 48, .height = 62 }, /* genericItem_color_018 */ 
    { .x = 688, .y = 393, .width = 52, .height = 68 }, /* genericItem_color_019 */ 
    { .x = 376, .y = 278, .width = 94, .height = 128 }, /* genericItem_color_020 */ 
    { .x = 134, .y = 1360, .width = 123, .height = 154 }, /* genericItem_color_021 */ 
    { .x = 464, .y = 534, .width = 87, .height = 162 }, /* genericItem_color_022 */ 
    { .x = 269, .y = 456, .width = 104, .height = 176 }, /* genericItem_color_023 */ 
    { .x = 693, .y = 461, .width = 42, .height = 74 }, /* genericItem_color_024 */ 
    { .x = 691, .y = 836, .width = 50, .height = 74 }, /* genericItem_color_025 */ 
    { .x = 692, .y = 135, .width = 42, .height = 74 }, /* genericItem_color_026 */ 
    { .x = 691, .y = 910, .width = 49, .height = 91 }, /* genericItem_color_027 */ 
    { .x = 690, .y = 1469, .width = 50, .height = 89 }, /* genericItem_color_028 */ 
    { .x = 632, .y = 812, .width = 59, .height = 130 }, /* genericItem_color_029 */ 
    { .x = 367, .y = 784, .width = 57, .height = 56 }, /* genericItem_color_030 */ 
    { .x = 146, .y = 776, .width = 116, .height = 96 }, /* genericItem_color_031 */ 
    { .x = 152, .y = 649, .width = 116, .height = 112 }, /* genericItem_color_032 */ 
    { .x = 0, .y = 1879, .width = 128, .height = 114 }, /* genericItem_color_033 */ 
    { .x = 461, .y = 1314, .width = 90, .height = 128 }, /* genericItem_color_034 */ 
    { .x = 256, .y = 1514, .width = 109, .height = 125 }, /* genericItem_color_035 */ 
    { .x = 373, .y = 436, .width = 94, .height = 98 }, /* genericItem_color_036 */ 
    { .x = 463, .y = 1442, .width = 88, .height = 124 }, /* genericItem_color_037 */ 
    { .x = 363, .y = 840, .width = 98, .height = 124 }, /* genericItem_color_038 */ 
    { .x = 382, .y = 0, .width = 90, .height = 78 }, /* genericItem_color_039 */ 
    { .x = 0, .y = 1993, .width = 111, .height = 47 }, /* genericItem_color_040 */ 
    { .x = 367, .y = 1738, .width = 72, .height = 65 }, /* genericItem_color_041 */ 
    { .x = 551, .y = 613, .width = 72, .height = 65 }, /* genericItem_color_042 */ 
    { .x = 256, .y = 1639, .width = 54, .height = 37 }, /* genericItem_color_043 */ 
    { .x = 462, .y = 1057, .width = 88, .height = 75 }, /* genericItem_color_044 */ 
    { .x = 365, .y = 1611, .width = 96, .height = 70 }, /* genericItem_color_045 */ 
    { .x = 0, .y = 532, .width = 152, .height = 88 }, /* genericItem_color_046 */ 
    { .x = 134, .y = 1514, .width = 112, .height = 80 }, /* genericItem_color_047 */ 
    { .x = 460, .y = 1738, .width = 93, .height = 117 }, /* genericItem_color_048 */ 
    { .x = 0, .y = 0, .width = 174, .height = 99 }, /* genericItem_color_049 */ 
    { .x = 0, .y = 1027, .width = 143, .height = 136 }, /* genericItem_color_050 */ 
    { .x = 0, .y = 1231, .width = 136, .height = 129 }, /* genericItem_color_051 */ 
    { .x = 162, .y = 322, .width = 114, .height = 134 }, /* genericItem_color_052 */ 
    { .x = 0, .y = 191, .width = 163, .height = 131 }, /* genericItem_color_053 */ 
    { .x = 363, .y = 1331, .width = 98, .height = 128 }, /* genericItem_color_054 */ 
    { .x = 364, .y = 1804, .width = 96, .height = 135 }, /* genericItem_color_055 */ 
    { .x = 0, .y = 1839, .width = 129, .height = 40 }, /* genericItem_color_056 */ 
    { .x = 0, .y = 736, .width = 152, .height = 40 }, /* genericItem_color_057 */ 
    { .x = 310, .y = 1639, .width = 46, .height = 38 }, /* genericItem_color_058 */ 
    { .x = 261, .y = 1186, .width = 51, .height = 38 }, /* genericItem_color_059 */ 
    { .x = 0, .y = 1163, .width = 141, .height = 68 }, /* genericItem_color_060 */ 
    { .x = 132, .y = 1686, .width = 52, .height = 13 }, /* genericItem_color_061 */ 
    { .x = 626, .y = 1501, .width = 64, .height = 88 }, /* genericItem_color_062 */ 
    { .x = 692, .y = 590, .width = 44, .height = 96 }, /* genericItem_color_063 */ 
    { .x = 688, .y = 1068, .width = 58, .height = 110 }, /* genericItem_color_064 */ 
    { .x = 688, .y = 240, .width = 58, .height = 88 }, /* genericItem_color_065 */ 
    { .x = 627, .y = 1075, .width = 60, .height = 94 }, /* genericItem_color_066 */ 
    { .x = 625, .y = 1740, .width = 66, .height = 82 }, /* genericItem_color_067 */ 
    { .x = 550, .y = 696, .width = 82, .height = 96 }, /* genericItem_color_068 */ 
    { .x = 474, .y = 0, .width = 84, .height = 98 }, /* genericItem_color_069 */ 
    { .x = 424, .y = 784, .width = 24, .height = 44 }, /* genericItem_color_070 */ 
    { .x = 626, .y = 358, .width = 62, .height = 89 }, /* genericItem_color_071 */ 
    { .x = 553, .y = 1930, .width = 70, .height = 92 }, /* genericItem_color_072 */ 
    { .x = 688, .y = 1178, .width = 58, .height = 79 }, /* genericItem_color_073 */ 
    { .x = 261, .y = 1095, .width = 105, .height = 91 }, /* genericItem_color_074 */ 
    { .x = 0, .y = 1360, .width = 134, .height = 122 }, /* genericItem_color_075 */ 
    { .x = 155, .y = 456, .width = 112, .height = 76 }, /* genericItem_color_076 */ 
    { .x = 132, .y = 1600, .width = 124, .height = 86 }, /* genericItem_color_077 */ 
    { .x = 0, .y = 99, .width = 163, .height = 92 }, /* genericItem_color_078 */ 
    { .x = 626, .y = 1409, .width = 64, .height = 92 }, /* genericItem_color_079 */ 
    { .x = 627, .y = 1279, .width = 62, .height = 99 }, /* genericItem_color_080 */ 
    { .x = 367, .y = 1681, .width = 94, .height = 57 }, /* genericItem_color_081 */ 
    { .x = 257, .y = 1449, .width = 98, .height = 63 }, /* genericItem_color_082 */ 
    { .x = 632, .y = 696, .width = 59, .height = 116 }, /* genericItem_color_083 */ 
    { .x = 257, .y = 1331, .width = 106, .height = 118 }, /* genericItem_color_084 */ 
    { .x = 550, .y = 874, .width = 82, .height = 82 }, /* genericItem_color_085 */ 
    { .x = 367, .y = 1037, .width = 38, .height = 41 }, /* genericItem_color_086 */ 
    { .x = 219, .y = 1791, .width = 31, .height = 41 }, /* genericItem_color_087 */ 
    { .x = 550, .y = 792, .width = 82, .height = 82 }, /* genericItem_color_088 */ 
    { .x = 312, .y = 1186, .width = 40, .height = 40 }, /* genericItem_color_089 */ 
    { .x = 628, .y = 95, .width = 40, .height = 40 }, /* genericItem_color_090 */ 
    { .x = 688, .y = 328, .width = 57, .height = 65 }, /* genericItem_color_091 */ 
    { .x = 688, .y = 84, .width = 51, .height = 51 }, /* genericItem_color_092 */ 
    { .x = 690, .y = 1365, .width = 50, .height = 104 }, /* genericItem_color_093 */ 
    { .x = 625, .y = 590, .width = 67, .height = 106 }, /* genericItem_color_094 */ 
    { .x = 551, .y = 506, .width = 74, .height = 107 }, /* genericItem_color_095 */ 
    { .x = 416, .y = 232, .width = 36, .height = 36 }, /* genericItem_color_096 */ 
    { .x = 626, .y = 461, .width = 38, .height = 36 }, /* genericItem_color_097 */ 
    { .x = 551, .y = 1409, .width = 75, .height = 86 }, /* genericItem_color_098 */ 
    { .x = 256, .y = 1226, .width = 108, .height = 105 }, /* genericItem_color_099 */ 
    { .x = 461, .y = 1611, .width = 91, .height = 93 }, /* genericItem_color_100 */ 
    { .x = 550, .y = 956, .width = 81, .height = 119 }, /* genericItem_color_101 */ 
    { .x = 152, .y = 532, .width = 117, .height = 117 }, /* genericItem_color_102 */ 
    { .x = 689, .y = 1257, .width = 53, .height = 108 }, /* genericItem_color_103 */ 
    { .x = 735, .y = 461, .width = 35, .height = 110 }, /* genericItem_color_104 */ 
    { .x = 625, .y = 497, .width = 68, .height = 93 }, /* genericItem_color_105 */ 
    { .x = 693, .y = 1805, .width = 33, .height = 90 }, /* genericItem_color_106 */ 
    { .x = 553, .y = 1740, .width = 72, .height = 86 }, /* genericItem_color_107 */ 
    { .x = 631, .y = 956, .width = 60, .height = 112 }, /* genericItem_color_108 */ 
    { .x = 143, .y = 1027, .width = 118, .height = 89 }, /* genericItem_color_109 */ 
    { .x = 364, .y = 1186, .width = 98, .height = 128 }, /* genericItem_color_110 */ 
    { .x = 254, .y = 1804, .width = 110, .height = 138 }, /* genericItem_color_111 */ 
    { .x = 464, .y = 696, .width = 86, .height = 86 }, /* genericItem_color_112 */ 
    { .x = 129, .y = 1839, .width = 125, .height = 194 }, /* genericItem_color_113 */ 
    { .x = 0, .y = 776, .width = 146, .height = 104 }, /* genericItem_color_114 */ 
    { .x = 470, .y = 232, .width = 86, .height = 134 }, /* genericItem_color_115 */ 
    { .x = 369, .y = 632, .width = 95, .height = 152 }, /* genericItem_color_116 */ 
    { .x = 111, .y = 2033, .width = 90, .height = 14 }, /* genericItem_color_117 */ 
    { .x = 688, .y = 0, .width = 55, .height = 84 }, /* genericItem_color_118 */ 
    { .x = 626, .y = 1175, .width = 62, .height = 104 }, /* genericItem_color_119 */ 
    { .x = 688, .y = 1589, .width = 56, .height = 121 }, /* genericItem_color_120 */ 
    { .x = 556, .y = 293, .width = 69, .height = 72 }, /* genericItem_color_121 */ 
    { .x = 691, .y = 1914, .width = 46, .height = 95 }, /* genericItem_color_122 */ 
    { .x = 691, .y = 1710, .width = 47, .height = 95 }, /* genericItem_color_123 */ 
    { .x = 145, .y = 880, .width = 117, .height = 113 }, /* genericItem_color_124 */ 
    { .x = 0, .y = 620, .width = 152, .height = 116 }, /* genericItem_color_125 */ 
    { .x = 558, .y = 0, .width = 70, .height = 135 }, /* genericItem_color_126 */ 
    { .x = 691, .y = 696, .width = 50, .height = 140 }, /* genericItem_color_127 */ 
    { .x = 130, .y = 1709, .width = 124, .height = 82 }, /* genericItem_color_128 */ 
    { .x = 734, .y = 135, .width = 28, .height = 105 }, /* genericItem_color_129 */ 
    { .x = 726, .y = 1805, .width = 27, .height = 97 }, /* genericItem_color_130 */ 
    { .x = 736, .y = 571, .width = 20, .height = 106 }, /* genericItem_color_131 */ 
    { .x = 254, .y = 1686, .width = 113, .height = 118 }, /* genericItem_color_132 */ 
    { .x = 0, .y = 880, .width = 145, .height = 147 }, /* genericItem_color_133 */ 
    { .x = 461, .y = 1855, .width = 92, .height = 132 }, /* genericItem_color_134 */ 
    { .x = 462, .y = 1132, .width = 88, .height = 140 }, /* genericItem_color_135 */ 
    { .x = 626, .y = 1589, .width = 62, .height = 146 }, /* genericItem_color_136 */ 
    { .x = 461, .y = 921, .width = 89, .height = 136 }, /* genericItem_color_137 */ 
    { .x = 174, .y = 0, .width = 109, .height = 90 }, /* genericItem_color_138 */ 
    { .x = 0, .y = 1600, .width = 132, .height = 109 }, /* genericItem_color_139 */ 
    { .x = 276, .y = 278, .width = 100, .height = 158 }, /* genericItem_color_140 */ 
    { .x = 143, .y = 1116, .width = 118, .height = 110 }, /* genericItem_color_141 */ 
    { .x = 163, .y = 99, .width = 113, .height = 202 }, /* genericItem_color_142 */ 
    { .x = 254, .y = 1942, .width = 109, .height = 98 }, /* genericItem_color_143 */ 
    { .x = 461, .y = 784, .width = 89, .height = 137 }, /* genericItem_color_144 */ 
    { .x = 0, .y = 416, .width = 155, .height = 116 }, /* genericItem_color_145 */ 
    { .x = 0, .y = 1482, .width = 134, .height = 118 }, /* genericItem_color_146 */ 
    { .x = 0, .y = 1709, .width = 130, .height = 130 }, /* genericItem_color_147 */ 
    { .x = 262, .y = 840, .width = 101, .height = 135 }, /* genericItem_color_148 */ 
    { .x = 373, .y = 534, .width = 91, .height = 89 }, /* genericItem_color_149 */ 
    { .x = 283, .y = 0, .width = 99, .height = 88 }, /* genericItem_color_150 */ 
    { .x = 550, .y = 1175, .width = 76, .height = 130 }, /* genericItem_color_151 */ 
    { .x = 467, .y = 406, .width = 85, .height = 100 }, /* genericItem_color_152 */ 
    { .x = 553, .y = 1608, .width = 73, .height = 132 }, /* genericItem_color_153 */ 
    { .x = 367, .y = 964, .width = 94, .height = 73 }, /* genericItem_color_154 */ 
    { .x = 625, .y = 293, .width = 63, .height = 65 }, /* genericItem_color_155 */ 
    { .x = 550, .y = 1075, .width = 77, .height = 100 }, /* genericItem_color_156 */ 
    { .x = 363, .y = 1942, .width = 98, .height = 102 }, /* genericItem_color_157 */ 
    { .x = 268, .y = 649, .width = 101, .height = 106 }, /* genericItem_color_158 */ 
    { .x = 405, .y = 1057, .width = 36, .height = 36 }, /* genericItem_color_159 */ 
    { .x = 380, .y = 232, .width = 36, .height = 36 }, /* genericItem_color_160 */ 
    { .x = 145, .y = 993, .width = 24, .height = 24 }, /* genericItem_color_161 */ 
    { .x = 262, .y = 761, .width = 105, .height = 79 }, /* genericItem_color_162 */ 
    { .x = 474, .y = 98, .width = 83, .height = 76 }, /* genericItem_color_163 */ 
    };

    //game.spriteItems = tk_sprite_make_px( "genericItems_spritesheet_colored.png", (TKSpritePixelRect){ .x = 0, .y = 322, .width = 162, .height = 94 } );
    int numItemSprites = sizeof(itemRects)/sizeof(itemRects[0]);
    for (int i=0; i < numItemSprites; i++)
    {
        game.spriteItems[i] = tk_sprite_make_px( "genericItems_spritesheet_colored.png",  itemRects[i] );
    }

    // Now init the things
    for (int i=0; i < NUM_THINGS; i++)
    {
        TestThing thing = {}; 
        int thingIndex = glm::linearRand( 0, numItemSprites );
        glm::vec2 p2 = glm::linearRand( glm::vec2( -1.0f, -1.0f), glm::vec2( 1.0f, 1.0f) );

        glm::vec2 vel = glm::linearRand( glm::vec2( -1.0f, -1.0f), glm::vec2( 1.0f, 1.0f) );

        thing.pos = glm::vec3( p2.x, p2.y, 0.0 );
        thing.vel = glm::vec3( vel.x, vel.y, 0.0 );
        glm::vec3 cc = glm::rgbColor( glm::vec3( glm::linearRand( 0.0f, 360.0f), 1.0f, 0.5f) );
        thing.tint = glm::vec4( cc.x, cc.y, cc.z, 1.0f );

        thing.ang = glm::linearRand( 0.0f, 360.0f );
        thing.rot = glm::linearRand( -100.0f, 100.0f );

        thing.scl = glm::linearRand( 0.7f, 1.2f );
        
        thing.sprite = game.spriteItems[thingIndex];

        game.thing[i] = thing;
    }

	emscripten_set_main_loop( draw, 0, 1 );
}

void draw()
{
    /* pump the sokol-fetch message queues, and invoke response callbacks */
    sfetch_dowork();

	/* animate our things */
    uint64_t frame_ticks = stm_laptime( &game.ticks );
    float dt = stm_sec( frame_ticks );

    for (int i=0; i < NUM_THINGS; i++)
    {
        TestThing *tt = game.thing + i;
        tt->pos = tt->pos + tt->vel * dt;
        // crappy collision
        if ((tt->pos.x > 1.0f) || (tt->pos.x < -1.0f)) {
            tt->vel.x *= -1.0f;
        }

        if ((tt->pos.y > 1.0f) || (tt->pos.y < -1.0f)) {
            tt->vel.y *= -1.0f;
        }

        tt->ang += tt->rot * dt;
    }

    game.angleFGSprite += 4.0f * dt;
    game.angleBGSprite -= 3.2f * dt;

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
    //sdtx_printf("I am a breakout game\n" );

    // sdtx_color3b( 0x20, 0x1E, 0xFF );
    // sdtx_canvas(emsc_width(), emsc_height());
    // sdtx_printf("Smol text\n" );


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
    

    // sg_apply_pipeline(pip);
    // sg_apply_bindings(&bind);
    // sg_apply_uniforms( SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
    //sg_draw(0, 6, 1);

    // draw sprites
    glm::vec4 white = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    tk_push_sprite_all( game.spritePlayer, glm::vec3( 0.0f, 0.3f, 0.0f ), 20.0f, white, game.angleBGSprite );
    
    for (int i=0; i < NUM_THINGS; i++)
    {
        TestThing *thing = game.thing + i;
        //tk_push_sprite( thing->sprite, thing->pos );
        tk_push_sprite_all( thing->sprite, thing->pos, thing->scl, thing->tint, thing->ang );
        //tk_push_sprite( game.spriteItems[i], glm::vec3( -1.0 + (0.21f * i), -0.25f, 0.0f ));
    }

    tk_push_sprite_all( game.spritePlayer, glm::vec3( 0.5f, 0.0f, 0.0f ), 10.0f, white, game.angleFGSprite );


    
    tk_sprite_drawgroups( vs_params.mvp );

    // draw sgl stuff
    sgl_draw();

    // draw sokol debug text
    sdtx_draw();

    sg_end_pass();
    sg_commit();

}