//#include <stdio.h>
#include <ctype.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define SOKOL_IMPL
#define SOKOL_GLES2

#include "sokol_app.h"
#include "sokol_glue.h"

#include "sokol_gfx.h"
#include "sokol_fetch.h"
#include "sokol_time.h"
#include "sokol_gl.h"
#include "sokol_debugtext.h"
//#include "emsc.h"

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
    int font_dream;
    uint8_t font_dream_data[256 * 1024];
    uint8_t font_normal_data[256 * 1024];
} FontInfo;
static FontInfo font;


// Include level data
#include "level-data.cpp"

// NOTES: sgl_begin_line_loop
static void draw();

typedef struct Actor_s 
{
    TKSpriteHandle sprHead;
    TKSpriteHandle sprBody;
    TKSpriteHandle sprHand;

    glm::vec3 pos;

    float travel; // for anim

    ActorInfo *info;
} Actor;

typedef struct DreamWord_s
{
    char real[16];
    bool learned;
} DreamWord;

#define CAM_HITE_ZOOMED ( 7.0f )
#define CAM_HITE_WIDE ( 30.0f )

enum {
    ACTION_None,

    ACTION_Journal,
    ACTION_Sleep,
    ACTION_Wake,
    ACTION_Talk,
    ACTION_Inspect,
};

#define MAX_KEYCODE (400)
typedef struct GameState_s
{
    TKSpriteHandle spriteTilemap;

    Actor player;

    int numNPCs;
    Actor npc[10];

    // UI Sprites
    TKSpriteHandle spriteUIJournal;
    TKSpriteHandle spriteUIHalftone;
    TKSpriteHandle spriteUIRoundRect;
    TKSpriteHandle spriteUIWaves;

    TKSpriteHandle btnOkay;
    TKSpriteHandle btnJournal;
    TKSpriteHandle btnSleep;
    TKSpriteHandle btnWake;
    TKSpriteHandle btnTalk;
    TKSpriteHandle btnInspect;

    uint64_t ticks;
    float gameTime;

    glm::vec3 camFocus;
    glm::vec3 camTarget;
    float camHite;
    float camHiteTarget;

    // UI stuff
    int availAction;
    bool show_journal;    
    bool show_wordlist;
    bool show_dialog;

    int dreamLevel;

    glm::vec3 inputDir;
    RoomInfo *currRoom;
    SleepZone *currSleep; // sleep zone you're standing on
    DreamWordInfo *currWord;
    ActorInfo *talkableNPC;

    bool keydown[MAX_KEYCODE];

    float angleBGSprite, angleFGSprite;

} GameState;
GameState game;

char g_cipher[27] = "BCDEFGHIJKLMNOPQRSTUVWXYZA";
DreamWord g_words[] = {
    { .real = "ANSWER" },
    { .real = "BOAT" },
    { .real = "CAPTAIN" },
    { .real = "FISH" },
    { .real = "GENERATOR" },
    { .real = "GLASS" },
    { .real = "MAP" },
    { .real = "MAYBE" },
    { .real = "RAIN" },
    { .real = "SEABIRD" },
    { .real = "SUNFLOWER" },
    { .real = "TOWN" },
    { .real = "VIOLENCE" },
    { .real = "WATER" },
    { .real = "WINE" },
    { .real = "WORLD" },

    { .real = "DREAMER" },
    { .real = "FIND" },
    { .real = "PATH" },
    { .real = "MOONLIGHT" },
    { .real = "PUNISHMENT" },
    { .real = "DAWN" },
    { .real = "BEFORE" },
    { .real = "REST" },
    { .real = "WORLD" },
};

static void font_normal_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        printf("Font loaded...\n");
        font.font_normal = fonsAddFontMem( font.fons, (const char *)"sans", (unsigned char *)response->buffer_ptr, response->fetched_size,  false);
    }
}

static void font_dream_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        printf("Dream Font loaded...\n");
        font.font_dream = fonsAddFontMem( font.fons, (const char *)"dream", (unsigned char *)response->buffer_ptr, response->fetched_size,  false);
    }
}

void setup_sprites( Actor *actor, int xx, int yy )
{
    glm::vec2 offs = glm::vec2(  xx * (1.0/4.0f), yy * (1.0/3.0f) );
    actor->sprHead = tk_sprite_make_st( "chars_new.png", glm::vec2( 64.0 / 1024.0, 21.0 / 1024.0 ) + offs, glm::vec2( 195.0 / 1024.0, 146.0 / 1024.0 ) + offs );
    actor->sprBody = tk_sprite_make_st( "chars_new.png", glm::vec2( 64.0 / 1024.0, 155.0 / 1024.0 ) + offs, glm::vec2( 195.0 / 1024.0, 1.0 / 3.0 ) + offs );
    actor->sprHand = tk_sprite_make_st( "chars_new.png", glm::vec2( 0.0 / 1024.0, 117.0 / 1024.0 ) + offs, glm::vec2( 57.0 / 1024.0, 179.0 / 1024.0 ) + offs );
}

static void init()
{
	// setup WebGL1 context, no antialias
	//emsc_init("#canvas", EMSC_NONE );
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
		.max_requests = 8,
        .num_channels = 1,
        .num_lanes = 1
	};
    sfetch_setup(&sfetch_desc);

    float canv_width = (float)sapp_width();
    float canv_height = (float)sapp_height();
    (void)canv_width;

    // Setup fonts
    int atlas_dim = 512;
    FONScontext* fons_context = sfons_create(atlas_dim, atlas_dim, FONS_ZERO_TOPLEFT);
    font.fons = fons_context;
    font.font_normal = FONS_INVALID;

    sfetch_request_t fontRequest = {     
        //.path = "epistolar.ttf",
        
        .path = "AveriaSerif-Light.ttf",
        .callback = font_normal_loaded,
        .buffer_ptr = font.font_normal_data,
        .buffer_size = sizeof(font.font_normal_data)
    };
    sfetch_send(&(fontRequest));

    sfetch_request_t fontDreamRequest = {     
        //.path = "epistolar.ttf",
        .path = "ACGuanche-Lite.ttf",
        .callback = font_dream_loaded,
        .buffer_ptr = font.font_dream_data,
        .buffer_size = sizeof(font.font_dream_data)
    };
    sfetch_send(&(fontDreamRequest));

    float pxSize = 1.0f / canv_height;
    TKSpriteSystemDesc spriteDesc = {
        .pixelSizeX = pxSize,
        .pixelSizeY = pxSize,
    };
    tk_sprite_init_system( spriteDesc );

    // Set up game resources
    //game.spritePlayer = tk_sprite_make_st( "player.png", glm::vec2( 0.0, 0.0 ), glm::vec2( 0.25, 0.5 ) );

    //game.player.sprHead = tk_sprite_make_st( "chars_new.png", glm::vec2( 64.0 / 1024.0, 21.0 / 1024.0 ), glm::vec2( 195.0 / 1024.0, 146.0 / 1024.0 ) );
    //game.player.sprBody = tk_sprite_make_st( "chars_new.png", glm::vec2( 64.0 / 1024.0, 155.0 / 1024.0 ), glm::vec2( 195.0 / 1024.0, 1.0 / 3.0 ) );
    //game.player.sprHand = tk_sprite_make_st( "chars_new.png", glm::vec2( 0.0 / 1024.0, 117.0 / 1024.0 ), glm::vec2( 57.0 / 1024.0, 179.0 / 1024.0 ) );
    setup_sprites( &game.player, 0, 0 );

    game.spriteTilemap = tk_sprite_make( "dreams_tileset.png");

    game.spriteUIJournal = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 0.0f, 0.0f ), glm::vec2( 691.0 / 1024.0, 602.0 / 1024.0) );
    game.spriteUIWaves = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 0.0f, 857.0 / 1024.0 ), glm::vec2( 1.0f, 1.0f) );
    game.spriteUIRoundRect = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 0.0f, 748.0 / 1024.0 ), glm::vec2( 1.0, 844.0 / 1024.0) );
    game.spriteUIHalftone = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 0.0f, 645.0 / 1024.0 ), glm::vec2( 1.0, 717.0 / 1024.0) );
    
    float buttonSz = 72.0f;
    game.btnOkay = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 748.0 / 1024.0, 0.0f), glm::vec2( 1.0f, 72.0f / 1024.0 ) );
    game.btnJournal = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 748.0 / 1024.0, (buttonSz * 1.0f) / 1024.0f), glm::vec2( 1.0f, (buttonSz * 2.0f) / 1024.0f ) );
    game.btnSleep = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 748.0 / 1024.0, (buttonSz * 2.0f) / 1024.0f), glm::vec2( 1.0f, (buttonSz * 3.0f) / 1024.0f ) ); 
    game.btnWake = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 748.0 / 1024.0, (buttonSz * 3.0f) / 1024.0f), glm::vec2( 1.0f, (buttonSz * 4.0f) / 1024.0f ) );
    game.btnTalk = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 748.0 / 1024.0, (buttonSz * 4.0f) / 1024.0f), glm::vec2( 1.0f, (buttonSz * 5.0f) / 1024.0f ) );
    game.btnInspect = tk_sprite_make_st( "ui_stuff.png", glm::vec2( 748.0 / 1024.0, (buttonSz * 5.0f) / 1024.0f), glm::vec2( 1.0f, (buttonSz * 6.0f) / 1024.0f ) );

    tk_sprite_mark_ui( "ui_stuff.png" );

    game.camHite = CAM_HITE_WIDE;
    game.camHiteTarget = CAM_HITE_ZOOMED;
    game.currRoom = &room_LivingRoom;    
    game.currSleep = NULL;

    game.player.pos = glm::vec3( 9.5f, -5.5f, 0.0f );

    game.dreamLevel = 1;

    // set up actors for world
    for (int i=0; i < MAX_ROOMS; i++)
    {
        RoomInfo* room = world.rooms[i];
        if (!room) break;

        if (strlen(room->actor.name) > 0) {
            Actor *npc = game.npc + game.numNPCs;
            
            // TODO: use different sprites
            // npc->sprHead = game.player.sprHead;
            // npc->sprBody = game.player.sprBody;
            // npc->sprHand = game.player.sprHand;
            int sx=0;
            int sy = 0;
            if (!strcmp(room->actor.name, "Arthur")) {
                sx = 1;
            } else if (!strcmp(room->actor.name, "Janet")) {
                sx = 2;
            } else if (!strcmp(room->actor.name, "Tulio")) {
                sx = 3;
            } else if (!strcmp(room->actor.name, "Esme")) {
                sx = 0; sy = 1;                
            } else if (!strcmp(room->actor.name, "Jamal")) {
                sx = 1; sy = 1;                                
            } else if (!strcmp(room->actor.name, "Sandy")) {
                sx = 2; sy = 1;
            } else if (!strcmp(room->actor.name, "CAPTAIN")) {
                sx = 3; sy = 1;                                                                
            }
            setup_sprites( npc, sx, sy );

            npc->pos = glm::vec3( 
                room->worldX + room->actor.tx, 
                room->worldY - room->actor.ty, 0.0f );

            // links
            npc->info = &(room->actor);
            room->actor.npcIndex = game.numNPCs;

            game.numNPCs++;
        }
    }


	//emscripten_set_main_loop( draw, 0, 1 );
}

void draw_actor( Actor *actor, glm::vec3 pos )
{
    float head_bob = fabs( sin( actor->travel ) );
    float hand_bob = sin( actor->travel * 2.0f );
    tk_push_sprite_scaled( actor->sprBody, pos + glm::vec3( 0.0f, 0.5f , 0.0f), 4.0f );
    
    tk_push_sprite_scaled( actor->sprHead, pos + glm::vec3( 0.0f, 1.2f + 0.2f * head_bob, 0.0f), 3.0f );

    tk_push_sprite_scaled( actor->sprHand, pos + glm::vec3( -0.6f, 0.35f + 0.1f * hand_bob, 0.0f), 3.0f );
    tk_push_sprite_scaled( actor->sprHand, pos + glm::vec3(  0.5f, 0.35f - 0.1f * hand_bob, 0.0f), 3.0f );
}

RoomInfo *room_for_world_pos( glm::vec3 pos )
{
    for (int i=0; i < MAX_ROOMS; i++)
    {
        RoomInfo* room = world.rooms[i];
        if (!room) break;

        glm::vec3 roomPos =  glm::vec3( room->worldX, room->worldY, 0.0f );
        
        // see if the pos is in this room
        if ( (pos.x > roomPos.x) && (pos.y > roomPos.y - 11.0) &&
             (pos.x < roomPos.x + 16.0f) && (pos.y < roomPos.y) ) {
            return room;
        }
    }

    return NULL;
}

float emit_norm_text( FONScontext* fs, char *text, float cx, float cy, float fontSize )
{

    //sdtx_printf("EMIT: %3.0f %3.0f %s\n", cx, cy, text);

    uint32_t black32 = sfons_rgba(0, 0, 0, 255);
    fonsSetFont(fs, font.font_normal);
    fonsSetSize(fs, fontSize );
    fonsSetColor(fs, black32 );
    return fonsDrawText(fs, cx, cy, text, NULL);
}

float emit_dream_text( FONScontext* fs, char *text, float cx, float cy, float fontSize )
{
    char dreamword[16];    
    char *dst = dreamword;    
    char *src = text;
    int count = 0;
    do {
        *dst++ = toupper(*src++);
        count++;
    } while (*src);
    *dst = '\0';

    // see if this word is in our list and needs to be encrypted
    bool shouldEncrypt = false;
    for (int i=0; i < sizeof(g_words) / sizeof(g_words[0]); i++ ) {
        if (!strcmp( g_words[i].real, dreamword )) {
            shouldEncrypt = !g_words[i].learned;
            break;
        }
    }

    if (shouldEncrypt) {
        for (char *ch = dreamword; *ch; ch++) {
            *ch = g_cipher[ *ch - 'A' ];
        }
    }

    uint32_t blue32  = sfons_rgba(0, 192, 255, 255); 
    fonsSetFont(fs, font.font_dream);
    fonsSetSize(fs, fontSize );
    fonsSetColor(fs, blue32 );

    // measure the unencrypted word    
    float result = fonsDrawText(fs, cx, cy, dreamword, NULL);    

    // float dx = result - cx;
    // sdtx_printf("AVG: %3.2f\n", dx / (float)count );

    if (shouldEncrypt) {
        result = cx + count * 12.0;
    }
    return result;
}

void wrap_dream_text( float x0, float x1, float y, char *text, float fontSize )
{
    FONScontext* fs = font.fons;
    float cx = x0;
    float cy = y;    

    // TODO randomize color
    uint32_t blue32  = sfons_rgba(0, 192, 255, 255); 

    fonsSetFont(fs, font.font_normal);
    fonsSetSize(fs, fontSize );
    

    char buff[1024];
    char *ch = text;
    char *dst = buff;
    while (*ch) {
        
        // See if we need to wrap if we hit a space
        if (*ch==' ')
        {

            *dst = '\0';
            float nextx = cx + fonsTextBounds( fs, cx, cy, buff, dst, NULL);
            //sdtx_printf( "NX: %3.2f %s\n", nextx, buff );
            if (nextx > x1) {

                *dst = '\0';
                cx = emit_norm_text( fs, buff, cx, cy, fontSize );
                
                dst = buff;

                // This will wrap
                cy += fontSize;
                cx = x0;
                ch++;
                continue;                
            }
        }

        // newline
        if (*ch=='\n') {
            *dst = '\0';
            cx = emit_norm_text( fs, buff, cx, cy, fontSize );
            dst = buff;
            cy += fontSize * 1.5;
            cx = x0;
            ch++;
            continue;                
        }

        // append char
        if (*ch == '[')
        {
            *dst = '\0';
            cx = emit_norm_text( fs, buff, cx, cy, fontSize );
            dst = buff;
        }
        else if (*ch == ']')
        {
            *dst = '\0';
            cx = emit_dream_text( fs, buff, cx, cy, fontSize );
            dst = buff;
        }
        else 
        {
            *dst++ = *ch;
            // check for wrap
        }
        ch++;
    }

    if (strlen(buff)) {
        *dst = '\0';
        emit_norm_text( fs, buff, cx, cy, fontSize );
    }

    /*
    fonsSetFont(fs, font.font_normal);
    fonsSetSize(fs, 36.0f );
    fonsSetColor(fs, black32 );
    cx = fonsDrawText(fs, cx, cy, "The language of ", NULL);

    fonsSetFont(fs, font.font_dream);
    fonsSetSize(fs, 36.0f );
    fonsSetColor(fs, blue32 );
    cx = fonsDrawText(fs, cx, cy, "Dreams", NULL);

    fonsSetFont(fs, font.font_normal);
    fonsSetSize(fs, 36.0f );
    fonsSetColor(fs, black32 );
    cx = fonsDrawText(fs, cx, cy, " is hidden from us.", NULL);
    */
}

bool inside_tile_rect( int tx, int ty, TileRect rect )
{
    if ( (tx >= rect.tx) && (tx < rect.tx + rect.w) &&
         (ty >= rect.ty) && (ty < rect.ty + rect.h) ) {
        return true;
    } else {
        return false;
    }
}

void show_dialog_box( FONScontext *fs)
{
    const char *title = "Message";
    const char *message = "Message [Text] goes here.";
    // "The language of [Dreams] is hidden from us. We are counting down to the [blast],"
    //     " but instead of your [Numbers] I use a list of [Seabirds]: [Tern], [Awk], [Seagull], [albatross]. "
    //     "How a [Ship] having passed the [Line] was driven by [storms] to the cold [Country] towards the South Pole; "
    //     "and how from thence she made her course to the tropical Latitude of the Great Pacific Ocean; and of " 
    //     "the strange things that befell; and in what manner the Ancyent Marinere came back to his own Country.";

    char buff[1024];

    if (game.talkableNPC)
    {    
        ActorInfo *nfo = game.talkableNPC;
        title = nfo->name;
        message = nfo->phrase[ game.dreamLevel ];
    } 
    else if (game.currWord)
    {        
        title = game.currWord->word;
        if (game.currWord->message) {
            sprintf( buff, "%s You learned the word [%s] in the language of dreams.", 
                game.currWord->message,
                game.currWord->word );
        } else {
            sprintf( buff, "You learned the word [%s] in the language of dreams.", game.currWord->word );
        }
        message = buff;
    }


    // Character Title
    fonsSetFont(fs, font.font_normal);
    fonsSetSize(fs, 50.0f );
    fonsSetColor(fs, 0xFFFFFFFF );
    fonsDrawText(fs, 20, 462, title, NULL);

    wrap_dream_text( 30.0f, 700.0f, 495.0f, 
        (char *)message,
        24.0f );

}

void frame()
{
    /* pump the sokol-fetch message queues, and invoke response callbacks */
    sfetch_dowork();

    /* text rendering via fontstash.h */
    float sx, sy, dx, dy, lh = 0.0f;
    uint32_t white32 = sfons_rgba(255, 255, 255, 255);    
    uint32_t brown32 = sfons_rgba(192, 128, 0, 128);    
    fonsClearState(font.fons);

    FONScontext* fs = font.fons;

	// ======================================================================
    //    Update the things  
    // ======================================================================
    uint64_t frame_ticks = stm_laptime( &game.ticks );
    float dt = stm_sec( frame_ticks );

    game.gameTime += dt;

    game.angleFGSprite += 4.0f * dt;
    game.angleBGSprite -= 3.2f * dt;

    // update our cipher once a frame
    int ndxA = rand() % 26;
    int ndxB = rand() % 26;
    char t = g_cipher[ndxA];
    g_cipher[ndxA] = g_cipher[ndxB];
    g_cipher[ndxB] = t;





    // Player movement
    if ((!game.show_dialog) && (!game.show_journal) && (!game.show_wordlist))
    {
        game.inputDir = glm::vec3( 0.0f );
        if (game.keydown[ SAPP_KEYCODE_UP ]) {
            game.inputDir += glm::vec3( 0.0f, 1.0f, 0.0f );
        }
        if (game.keydown[ SAPP_KEYCODE_DOWN ]) {
            game.inputDir += glm::vec3( 0.0f, -1.0f, 0.0f );
        }
        if (game.keydown[ SAPP_KEYCODE_LEFT ]) {
            game.inputDir += glm::vec3( -1.0f, 0.0f, 0.0f );
        }
        if (game.keydown[ SAPP_KEYCODE_RIGHT ]) {
            game.inputDir += glm::vec3( 1.0f, 0.0f, 0.0f );
        }

        glm::vec3 newPlayerPos = game.player.pos + game.inputDir * (dt * 6.0f );

        
        float dtrav = glm::distance( game.player.pos, newPlayerPos );
        game.player.travel += dtrav; // increase trav even if we're walking nowhere, so we animate into walls

        // check collision
        int tx = 0, ty = 0;
        RoomInfo *room = room_for_world_pos( newPlayerPos );
        bool didCollide = true;
        if (room) {        
            // get tile collision       
            if (newPlayerPos.x >= 0.0f) {
                tx = (int)newPlayerPos.x - room->worldX;
            } else {
                tx = (int)(newPlayerPos.x - 1) - room->worldX;
            }

            if (newPlayerPos.y < 0.0f) {
                //tx = (int)newPlayerPos.x - room->worldX;
                ty = -((int)newPlayerPos.y - room->worldY);
            } else {
                //tx = (int)newPlayerPos.x - room->worldX;
                ty = -((int)(newPlayerPos.y + 1) - room->worldY);
            }
            sdtx_printf( "pxy %3.2f %3.2f txty %d %d\n", newPlayerPos.x, newPlayerPos.y, tx, ty );
            
            didCollide = room->collision[ty*16+tx];

        }

        if (!didCollide)
        {
            game.player.pos = newPlayerPos;

            game.availAction = ACTION_None;

            // Check if we're on a journal square
            if (room && room->journal.text)
            {
                if (inside_tile_rect( tx, ty, room->journal.rect)) {
            
                    if (!room->journal.viewed)
                    {
                        game.show_journal = true;
                    } else {
                        // Let player view it again
                        game.availAction = ACTION_Journal;
                    }

                } else if (game.availAction == ACTION_Journal) {
                    game.availAction = ACTION_None;
                }
            }

            // Check sleeps
            if ((room) && (game.availAction != ACTION_Journal))
            {                
                game.availAction = ACTION_None;
                game.currSleep = NULL;
                for (int i=0; i < 5; i++)
                {
                    SleepZone *sleep = room->sleeps + i;
                    if ((sleep->rect.w > 0) && (sleep->rect.h > 0))
                    {
                        //sdtx_printf("Checking sleep.... %d\n", i);
                        if (inside_tile_rect( tx, ty, sleep->rect )) {
                            game.currSleep = sleep;
                            if (sleep->asleepHere) {
                                game.availAction = ACTION_Wake;
                            } else if (game.dreamLevel < 4) {
                                game.availAction = ACTION_Sleep;
                            }
                            break;
                        }
                    }
                }
            }

            if ((room) && (game.availAction != ACTION_Journal))
            {
                for (int i=0; i < 10; i++)
                {
                    DreamWordInfo *dw = room->dreamWords + i;
                    if ((dw->rect.w > 0) && (dw->rect.h > 0))
                    {
                        if (inside_tile_rect( tx, ty, dw->rect )) {
                            game.availAction = ACTION_Inspect;
                            game.currWord = dw;
                            break;
                        }
                    }
                }
            }

            if ((room) && (game.availAction != ACTION_Journal))
            {
                // See if we are near enough to talk to somebody
                game.talkableNPC = NULL;
                for (int i=0; i < game.numNPCs; i++)
                {
                    Actor *npc = game.npc + i;                    
                    float d = glm::length( npc->pos - game.player.pos);
                    if (d < 2.0f) {
                        game.availAction = ACTION_Talk;
                        game.talkableNPC = npc->info;
                        break;
                    }
                }
            }
        }

    }
    

    if (game.currRoom)
    {
        game.camTarget = glm::vec3( (float)game.currRoom->worldX + 8.0f,
                                    (float)game.currRoom->worldY - 5.5f,
                                    0.0f );
    }
    game.camFocus = glm::mix( game.camFocus, game.camTarget,  0.05f );
    game.camHite = glm::mix( game.camHite, game.camHiteTarget, 0.05f );



    // ======================================================================
    //    Render the things  
    // ======================================================================
	params_t vs_params = {};
	
    // Don't use real width, assume an 800x600 virtual canvas
    float canv_width_real = (float)sapp_width();
    float canv_height_real = (float)sapp_height();
    float canv_width = 800;
    float canv_height = 600;

	//float aspect = (float)emsc_width() / (float)emsc_height();
    float aspect = canv_width / canv_height;
    float viewHite = game.camHite;
	glm::mat4 proj = glm::ortho( -viewHite * aspect, viewHite * aspect, -viewHite, viewHite, -1.0f, 1.0f );
    glm::mat4 cam = glm::translate( proj, -(game.camFocus + glm::vec3( 0.0f, -1.0f, 0.0f) ) );


	vs_params.mvp = cam;
    // printf("Mvp is %f %f %f %f\n%f %f %f %f\n",
    // 	vs_params.mvp[0].x, vs_params.mvp[0].y, vs_params.mvp[0].z, vs_params.mvp[0].w,
    // 	vs_params.mvp[1].x, vs_params.mvp[1].y, vs_params.mvp[1].z, vs_params.mvp[1].w );

    //sdtx_canvas(emsc_width() * 0.5f, emsc_height() * 0.5f);
    sdtx_canvas(canv_width * 0.5f, canv_height * 0.5f);
    sdtx_origin(3.0f, 3.0f);
    sdtx_color3b( 0x20, 0xFF, 0xFF );
    //sdtx_printf("I am a breakout game\n" );

    // sdtx_color3b( 0x20, 0x1E, 0xFF );
    // sdtx_canvas(emsc_width(), emsc_height());
    // sdtx_printf("Smol text\n" );


	sg_begin_default_pass(&pass_action, canv_width_real, canv_height_real );
        
    sgl_defaults();
    sgl_load_matrix( glm::value_ptr( cam ));

    //sdtx_printf("TILE: %d   %d\n", tx, ty );

/*
    float rx = 0.0f;
    float ry = 0.0f;
    if (game.currRoom) {
        sgl_c3f( 1.0f, 0.0f, 1.0f );    
        rx = game.currRoom->worldX;
        ry = game.currRoom->worldY;
    } else {
        sgl_c3f( 1.0f, 1.0f, 0.0f );
    }
    sgl_begin_line_strip();
    //float m = 0.95f;
    sgl_v3f( rx + 0.0f,  ry + 0.0f, 0.0f );
    sgl_v3f( rx + 16.0f, ry + 0.0f, 0.0f );
    sgl_v3f( rx + 16.0f, ry - 11.0f, 0.0f );
    sgl_v3f( rx + 0.0f,  ry - 11.0f, 0.0f );
    
    sgl_v3f( rx + 0.0f,  ry + 0.0f, 0.0f );

    sgl_end();

    sgl_c3f( 1.0f, 0.0f, 1.0f );    
    sgl_begin_lines();

    sgl_v3f( game.player.pos.x - 0.3f, game.player.pos.y - 0.3f, 0.0f );
    sgl_v3f( game.player.pos.x + 0.3f, game.player.pos.y + 0.3f, 0.0f );

    sgl_v3f( game.player.pos.x + 0.3f, game.player.pos.y - 0.3f, 0.0f );
    sgl_v3f( game.player.pos.x - 0.3f, game.player.pos.y + 0.3f, 0.0f );
    sgl_end();
*/
    // sg_apply_pipeline(pip);
    // sg_apply_bindings(&bind);
    // sg_apply_uniforms( SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
    //sg_draw(0, 6, 1);

    // draw sprites
    glm::vec4 white = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    //tk_push_sprite_all( game.spritePlayer, glm::vec3( 0.0f, 0.3f, 0.0f ), 20.0f, white, game.angleBGSprite );
        
    
    for (int i=0; i < MAX_ROOMS; i++)
    {
        RoomInfo* room = world.rooms[i];
        if (!room) break;

        glm::vec3 roomPos =  glm::vec3( room->worldX, room->worldY, 0.0f );
        tk_push_sprite_tilemap( game.spriteTilemap, room, roomPos );

        // see if the player is in this room and set it current
        if ( (game.player.pos.x > roomPos.x) && (game.player.pos.y > roomPos.y - 11.0) &&
             (game.player.pos.x < roomPos.x + 16.0f) && (game.player.pos.y < roomPos.y) ) {
            game.currRoom = room;
        }
    }
    
    draw_actor( &(game.player), game.player.pos );

    for (int i=0; i < game.numNPCs; i++)
    {
        Actor *npc = game.npc + i;
        draw_actor( npc, npc->pos );
        npc->travel += dt * 0.2;
    }

    //tk_push_sprite( game.spriteTilemap, glm::vec3( 0.0f, 0.0f, 0.0f ));
    //tk_push_sprite_scaled( game.spritePlayer, game.playerPos + glm::vec3( 0.0f, 0.5f, 0.0f), 30.0f );
    
    // Draw UI sprites on top
    if ((game.show_journal) || (game.show_wordlist))
    {
        tk_push_sprite_scaled( game.spriteUIJournal, glm::vec3( 400.0f, 300.0f, 0.0f ), 350.0f );

        //tk_push_sprite_scaled( game.btnOkay, glm::vec3( canv_width / 3.0f, canv_height / 4.0f, 0.0f ), 500.0 );
        tk_push_sprite_scaled( game.btnOkay, glm::vec3( (canv_width / 2.0f) + 130.0f, 120.0f, 0.0f ), 200.0 );
    }
    else if (game.show_dialog)
    {
        //void tk_push_sprite_all( TKSpriteHandle sh, glm::vec3 pos, float scale, glm::vec4 color, float angle_deg );
        
        // oops wave is not tall enough, draw it twice to cover the gap
        tk_push_sprite_all( game.spriteUIWaves, glm::vec3( canv_width / 2.0f, 50.0f, 0.0f), 295.0f, 
            glm::vec4( 168.0/255.0f, 236.0/255.0f, 240.0/255.0f, 1.0f), 0.0f );

        tk_push_sprite_all( game.spriteUIWaves, glm::vec3( canv_width / 2.0f, 100.0f, 0.0f), 295.0f, 
            glm::vec4( 168.0/255.0f, 236.0/255.0f, 240.0/255.0f, 1.0f), 0.0f );

        // Name of speaker
        tk_push_sprite_all( game.spriteUIRoundRect, glm::vec3( 80.0f, 150.0f, 0.0f), 230.0f, 
            glm::vec4( 100.0/255.0f, 30.0/255.0f, 250.0/255.0f, 1.0f), 0.0f );

    } 
    else 
    {
        // No UI is up, show avail action if there is one
        if (game.availAction != ACTION_None) {
            TKSpriteHandle btnAction;

            if (game.availAction == ACTION_Journal) {
                btnAction = game.btnJournal;
            } else if (game.availAction == ACTION_Sleep) {
                btnAction = game.btnSleep;
            } else if (game.availAction == ACTION_Wake) {
                btnAction = game.btnWake;                
            } else if (game.availAction == ACTION_Talk) {
                btnAction = game.btnTalk;
            } else if (game.availAction == ACTION_Inspect) {                
                btnAction = game.btnInspect;
            }            

            tk_push_sprite_scaled( btnAction, glm::vec3( (canv_width / 2.0f), 80.0f, 0.0f ), 400.0 );       
        }
    }

    float textSz = 800.0f;// 800 "virtual" pixels
    glm::mat4 ui_mat = glm::ortho( 0.0f, 800.0f, 0.0f, 600.0f, -1.0f, +1.0f);

    tk_sprite_set_dream_level( game.dreamLevel, game.gameTime );
    tk_sprite_drawgroups( vs_params.mvp, ui_mat );
    
    
    
    sgl_load_identity();
    //sgl_ortho(0.0f, textSz * aspect, textSz, 0.0f, -1.0f, +1.0f);
    sgl_ortho( 0.0f, 800.0f, 600.0f, 0.0f, -1.0f, +1.0f );

    sdtx_color3b( 0x6c, 0x17, 0xff );
    sdtx_printf("\n\n\n\nDream Level: %d\n", game.dreamLevel - 1 );
    sdtx_printf("Avail: %d\n", game.availAction );

    sdtx_color3b( 0x20, 0xFF, 0xFF );
    
    uint32_t black32 = sfons_rgba(0, 0, 0, 255);
    if ((font.font_normal != FONS_INVALID) && (font.font_dream != FONS_INVALID))
    {
    
        if (game.show_journal) {
            float col_journalstart = 120.0f;
            float col_journalend = 360.0f;
            wrap_dream_text( col_journalstart, col_journalend, 85.0f, game.currRoom->journal.text, 20.0f );

        } else if (game.show_wordlist) {
            float col_dreamword = 130.0f;
            float col_realword = 350.0f;
            float col_sep = (col_realword + col_dreamword) / 2.0 + 20.0;
            float rowstart = 120;

    
            // fonsSetFont(fs, font.font_dream);
            // fonsSetSize(fs, 24.0f );
            // fonsSetColor(fs, black32 );            
            float cy = rowstart;
            // only show the Act I words
            for (int i=0; i < 16; i++ ) 
            {
                DreamWord *dw = g_words+i;

                float offs = 0;
                if (i > 9) {
                    offs = (i - 9) * 5;
                }


                emit_dream_text( fs, dw->real, col_dreamword + offs, cy, 24.0f );
                cy += 24.0f;
            }

            fonsSetFont(fs, font.font_normal );
            fonsSetSize(fs, 24.0f );
            fonsSetColor(fs, black32 );
            fonsSetAlign( fs, FONS_ALIGN_BASELINE | FONS_ALIGN_RIGHT );
            cy = rowstart;

            for (int i=0; i < 16; i++ ) 
            {
                DreamWord *dw = g_words+i;
                fonsDrawText(fs, col_sep, cy, "...", NULL);
                fonsDrawText(fs, col_realword, cy, dw->learned?dw->real:"????", NULL);
                cy += 24.0f;
            }

            fonsSetAlign( fs, FONS_ALIGN_BASELINE | FONS_ALIGN_LEFT );
        }

        if (game.show_dialog) {

            show_dialog_box( fs );
               
            
        }
    } else {
        sdtx_printf("Fons not loaded\n");
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

void learn_word( DreamWordInfo *dw )
{
    for (int i=0; i < sizeof(g_words) / sizeof(g_words[0]); i++ ) {
        if (!strcmp( g_words[i].real, dw->word )) {
            g_words[i].learned = true;
            break;
        }
    }
}

static void event(const sapp_event* e) {

    assert((e->type >= 0) && (e->type < _SAPP_EVENTTYPE_NUM));
    if ((e->type == SAPP_EVENTTYPE_KEY_DOWN) || (e->type == SAPP_EVENTTYPE_KEY_UP))
    {
        //printf("event .. keydownup %d code %d...\n", e->type, e->key_code );
        if (e->key_code < MAX_KEYCODE) {
            game.keydown[e->key_code] = (e->type == SAPP_EVENTTYPE_KEY_DOWN);
        }

        // HERE handle key down/ups where we want to react to the pressed event
        if ((e->type == SAPP_EVENTTYPE_KEY_DOWN) && (!e->key_repeat))
        {
            if (e->key_code == SAPP_KEYCODE_TAB)
            {
                if (game.camHiteTarget > (CAM_HITE_ZOOMED + CAM_HITE_WIDE) / 2.0f ) {
                    game.camHiteTarget = CAM_HITE_ZOOMED;
                } else {
                    game.camHiteTarget = CAM_HITE_WIDE;
                }
            }
            else if ((e->key_code == SAPP_KEYCODE_X) || (e->key_code == SAPP_KEYCODE_Z) || (e->key_code == SAPP_KEYCODE_SPACE)) {

                if (game.show_journal) {
                    game.currRoom->journal.viewed = true;
                    game.show_journal = false;
                } else if (game.show_dialog) {
                    game.show_dialog = false;
                } else {
                    // Do avail actions
                    if (game.availAction == ACTION_Journal) 
                    {
                        game.currRoom->journal.viewed = false;
                        game.show_journal = true;
                    } else if (game.availAction == ACTION_Sleep) {
                        
                        game.dreamLevel += 1;
                        if (game.currSleep) {
                            game.currSleep->asleepHere = true;
                        }
                    } else if (game.availAction == ACTION_Wake) {
                        game.dreamLevel -= 1;
                        if (game.currSleep) {
                            game.currSleep->asleepHere = false;
                        }
                    } else if (game.availAction == ACTION_Inspect) {

                        learn_word( game.currWord );
                        game.show_dialog = true;

                    } else if (game.availAction == ACTION_Talk) {
                        game.show_dialog = true;
                    }
                }

            }
            else if (e->key_code == SAPP_KEYCODE_W) {
                game.show_wordlist = !game.show_wordlist;
            }

            // DEBUG KEYS
            else if (e->key_code == SAPP_KEYCODE_J) {
                game.show_journal = !game.show_journal;
            }
            else if (e->key_code == SAPP_KEYCODE_D) {                
                game.show_dialog = !game.show_dialog;
            }
        }
    }
    // } else {
    //     printf("event... %d\n", e->type );
    // }

}


static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    sapp_desc desc = { };
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.event_cb = event;
    desc.cleanup_cb = cleanup;
    desc.width = 832;
    desc.height = 600;
    desc.window_title = "LD48-deeper";
    desc.user_cursor = true;
    desc.gl_force_gles2 = true;
    desc.enable_clipboard = true;
    desc.enable_dragndrop = true;
    //desc.max_dropped_files = max_dropped_files;
    return desc;
}
