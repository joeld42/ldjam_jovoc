
#include <string.h>
#include <stdlib.h>

#include "glm/gtc/random.hpp"
#include "glm/gtx/color_space.hpp"


#include "sokol_fetch.h"
#include "sokol_gfx.h"

#include "sokol_debugtext.h"

#include "stb_image.h"

#include "tk_sprite.h"
#include "tk_tilemap.h"

#define TKSPRITE_INITIAL_VERTBUFFSIZE (1024 * 1024)
#define TKSPRITE_INITIAL_INDEXBUFFSIZE (1024 * 128)

// TODO make growable
#define TKSPRITE_MAX_BATCHES (100)

static uint8_t tksprite_fetch_buffer[1024 * 1024 * 6];

typedef struct TKSpriteDrawVert_s
{
	glm::vec3 pos;	
	glm::vec4 color;
	glm::vec4 st;	
} TKSpriteDrawVert;

typedef struct TKSpriteGroup_s
{
	// Sprite atlas
	char name[256];
	uint32_t index;
	sg_image atlas;
	uint32_t width;
	uint32_t height;
	bool is_ui_sprite;

} TKSpriteGroup;

typedef struct TKSpriteDef_s
{
	uint32_t sg_ndx; // sprite group index
	glm::vec4 st0;
	glm::vec4 st1;
	glm::vec2 pivot;
	TKSpritePixelRect pxrect; 
} TKSpriteDef;


typedef struct {
    glm::mat4 mvp;
} TKDefaultSpriteShaderParams;

struct TKSpriteBatch
{
	uint32_t groupIndex;
	uint32_t startIndex;
	uint32_t numIndices; 
};


struct TKSpriteSystem
{
	TKSpriteSystemDesc info;	
	TKSpriteGroup spritegroups[TK_MAX_SPRITEGROUPS];
	int numSpriteGroups;

	TKSpriteDef spritedefs[TK_MAX_SPRITEDEFS];
	int numSpriteDefs;

	sg_pipeline pipe;
	sg_bindings bind;
	sg_shader defaultSpriteShader;

	// Buffers
	uint32_t capacityVertexBuff;
	uint32_t capacityIndexBuff;	

	TKSpriteDrawVert *drawverts;
	size_t numverts;	
	size_t currVert;

	uint16_t *indices;
	size_t numIndices;

	TKSpriteBatch spriteBatches[TKSPRITE_MAX_BATCHES];
	TKSpriteBatch *currBatch;
	int numBatches;	
};
struct TKSpriteSystem g_defaultSpriteSystem = {};

void tk_sprite_init_system( TKSpriteSystemDesc info )
{
	TKSpriteSystem *sys = &g_defaultSpriteSystem;

	sys->info = info;

	if (sys->info.pixelSizeX <= 0.0f) {
		sys->info.pixelSizeX = 1.0f;
	}

	if (sys->info.pixelSizeY <= 0.0f) {
		sys->info.pixelSizeY = 1.0f;
	}

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
            //"  vec3 cc = texture2D(tex, uv).rgb * color.rgb;\n"
            //"  gl_FragColor = vec4( cc, 0.8 );\n"
            "}\n";
    shader_desc.attrs[0].name = "position";
	shader_desc.attrs[1].name = "color0";
	shader_desc.attrs[2].name = "texcoord0";

	shader_desc.fs.images[0] = { .name = "tex", .type=SG_IMAGETYPE_2D };

	shader_desc.vs.uniform_blocks[0] = {
		.size = sizeof(TKDefaultSpriteShaderParams)
	};
	shader_desc.vs.uniform_blocks[0].uniforms[0] = { .name="mvp", .type = SG_UNIFORMTYPE_MAT4 };

	sys->defaultSpriteShader = sg_make_shader(&shader_desc);

	// Set up the pipeline for our sprites
	sg_pipeline_desc sprite_pipe_desc = {
        .layout = { },
        .shader = sys->defaultSpriteShader,
        .index_type = SG_INDEXTYPE_UINT16,
        .blend = {
        	.enabled = true,
        	.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
    		.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    		.src_factor_alpha = SG_BLENDFACTOR_ZERO,
    		.dst_factor_alpha = SG_BLENDFACTOR_ONE,
        }
    };
    sprite_pipe_desc.layout.attrs[0].format=SG_VERTEXFORMAT_FLOAT3;
    sprite_pipe_desc.layout.attrs[1].format=SG_VERTEXFORMAT_FLOAT4;
    sprite_pipe_desc.layout.attrs[2].format=SG_VERTEXFORMAT_FLOAT4;
    sys->pipe = sg_make_pipeline(&sprite_pipe_desc);

    // Initialize the buffers    
	sys->capacityVertexBuff = TKSPRITE_INITIAL_VERTBUFFSIZE;
	sys->drawverts = (TKSpriteDrawVert*)malloc( sys->capacityVertexBuff );
	sg_buffer_desc vert_buffer_desc = {
    	.usage = SG_USAGE_STREAM,
	};
	vert_buffer_desc.size = sys->capacityVertexBuff;
	sys->bind.vertex_buffers[0] = sg_make_buffer(&vert_buffer_desc);

	sys->capacityIndexBuff = TKSPRITE_INITIAL_INDEXBUFFSIZE;
	sys->indices = (uint16_t*)malloc( sys->capacityIndexBuff );
	sg_buffer_desc index_buffer_desc = {        	
    	.type = SG_BUFFERTYPE_INDEXBUFFER,
    	.usage = SG_USAGE_STREAM,
	};    	
	index_buffer_desc.size = sys->capacityIndexBuff;
	sys->bind.index_buffer = sg_make_buffer(&index_buffer_desc);
}

// Update the sdef when with and height are known
void tk_sprite_update_sdef( TKSpriteDef *sdef, int width, int height)
{
	// if the sdef already has a pxrect defined, then we can calculate STs
	if ((sdef->pxrect.width > 0) && (sdef->pxrect.height > 0)) {
		// update sts from pxrect
		sdef->st0 = glm::vec4( (float)sdef->pxrect.x / (float)width, 
									  (float)sdef->pxrect.y / (float)height, 
									  0.0, 0.0 );

		sdef->st1 = glm::vec4( ((float)sdef->pxrect.width / (float)width) + sdef->st0.x, 
									  ((float)sdef->pxrect.height / (float)height) + sdef->st0.y, 
									  0.0, 0.0 );
	} else {
		// update pxrect from STs
		sdef->pxrect.x = sdef->st0.x * width;
		sdef->pxrect.y = sdef->st0.y * height;
		sdef->pxrect.width = (sdef->st1.x * width) - sdef->pxrect.x;
		sdef->pxrect.height = (sdef->st1.y * height) - sdef->pxrect.y;
	}
}


static void tksprite_fetch_callback(const sfetch_response_t* response) {

	printf("in tksprite_fetch_callback\n");
	
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

			TKSpriteGroup *sgroup = *(TKSpriteGroup**)(response->user_data);

			sg_init_image( sgroup->atlas, &image_desc );
            
			sg_image_info info = sg_query_image_info( sgroup->atlas );
            sgroup->width = info.width;
            sgroup->height = info.height;

            // Update STs for sprites created with this image
            TKSpriteSystem *sys = &g_defaultSpriteSystem;
            for (int i=0; i < sys->numSpriteDefs; i++) {
            	TKSpriteDef *sdef = sys->spritedefs + i;
            	if (sdef->sg_ndx == sgroup->index) {
            		tk_sprite_update_sdef( sdef, sgroup->width, sgroup->height );
            	}
            }

            printf("Fetch succeeded for image %s (%dx%d)\n", response->path, info.width, info.height );
        }
    }
    else if (response->failed) {
        // TODO indicate error better
        printf("fetch failed for %s (%d)\n", response->path, response->error_code );

    }
}


int tk_sprite_make_sdef( const char *path )
{
	TKSpriteSystem *sys = &g_defaultSpriteSystem;

	// See if we already have the sprite group (atlas) for this loaded
	uint16_t sgIndex = TK_MAX_SPRITEGROUPS;
	for (int i=0; i < sys->numSpriteGroups; i++)
	{
		TKSpriteGroup *sg = sys->spritegroups + i;
		if (!strcmp( sg->name, path )) {
			sgIndex = i;
			break;
		}
	}

	// Make a new spritegroup for it if we didn't find it
	if (sgIndex == TK_MAX_SPRITEGROUPS) 
	{
		assert( sys->numSpriteGroups < TK_MAX_SPRITEGROUPS);
		sgIndex = sys->numSpriteGroups++;
		TKSpriteGroup *sgroup = sys->spritegroups + sgIndex;
		sgroup->index = sgIndex;
		
		strncpy( sgroup->name, path, 255 );
		sgroup->atlas = sg_alloc_image();

		// Start our sfetch request
	    sfetch_request_t fetch_request = {
	        //.path = "genericItems_spritesheet_colored.png",
	        .callback = tksprite_fetch_callback,
	        .buffer_ptr = tksprite_fetch_buffer,
	        .buffer_size = sizeof(tksprite_fetch_buffer)
	    };
		fetch_request.path = sgroup->name;	    
	    fetch_request.user_data_ptr = &sgroup;
	    fetch_request.user_data_size = sizeof(sgroup);

	    printf("sending fetch request for sgroup %s (index %d)\n", sgroup->name, sgIndex );
	 	sfetch_send(&fetch_request);
	} else {
		printf("Found sprite group at index %d\n", sgIndex );
	}

	// Make a new sdef for it now
	assert( sys->numSpriteDefs < TK_MAX_SPRITEDEFS);
	uint32_t sdef_ndx = sys->numSpriteDefs++;
	TKSpriteDef *sdef = sys->spritedefs + sdef_ndx;	
	sdef->sg_ndx = sgIndex;	
	sdef->pxrect = {};

	return sdef_ndx;
}

void tk_sprite_mark_ui( const char *path )
{
	TKSpriteSystem *sys = &g_defaultSpriteSystem;

	// Find the named spritegroup
	uint16_t sgIndex = TK_MAX_SPRITEGROUPS;
	for (int i=0; i < sys->numSpriteGroups; i++)
	{
		TKSpriteGroup *sg = sys->spritegroups + i;
		if (!strcmp( sg->name, path )) {
			sg->is_ui_sprite = true;
			break;
		}
	}
}


TKSpriteHandle tk_sprite_make_st( const char *path, glm::vec2 st0, glm::vec2 st1 )
{
	TKSpriteSystem *sys = &g_defaultSpriteSystem;

	uint32_t sdef_ndx = tk_sprite_make_sdef( path );
	TKSpriteDef *sdef = sys->spritedefs + sdef_ndx;	

	sdef->st0 = glm::vec4( st0.x, st0.y, 0.0, 0.0 );
	sdef->st1 = glm::vec4( st1.x, st1.y, 1.0, 1.0 );

	TKSpriteGroup *sgroup = sys->spritegroups + sdef->sg_ndx;	
	if ((sgroup->width > 0) && (sgroup->height > 0)) {
		tk_sprite_update_sdef( sdef, sgroup->width, sgroup->height );
	}

	TKSpriteHandle result = {
		.sdef_ndx = sdef_ndx
	};

	return  result;
}

TKSpriteHandle tk_sprite_make( const char *path )
{
	return tk_sprite_make_st( path, glm::vec2( 0.0, 0.0 ), glm::vec2( 1.0, 1.0 ) );
}

TKSpriteHandle tk_sprite_make_px( const char *path, TKSpritePixelRect pxrect )
{
	TKSpriteSystem *sys = &g_defaultSpriteSystem;

	uint32_t sdef_ndx = tk_sprite_make_sdef( path );
	TKSpriteDef *sdef = sys->spritedefs + sdef_ndx;	
	sdef->pxrect = pxrect;

	TKSpriteGroup *sgroup = sys->spritegroups + sdef->sg_ndx;	
	if ((sgroup->width > 0) && (sgroup->height > 0)) {
		tk_sprite_update_sdef( sdef, sgroup->width, sgroup->height );
	}

	TKSpriteHandle result = {
		.sdef_ndx = sdef_ndx
	};

	return  result;
}

uint16_t _tk_sprite_pushvvert( TKSpriteSystem *sys, TKSpriteGroup *sg, glm::vec3 pos, glm::vec4 st, glm::vec4 color )
{

	(void)sg; // unused currently

	// TODO: grow drawvert buffer if needed
	uint32_t ndx = sys->numverts++;
	assert( sys->numverts * sizeof(TKSpriteDrawVert) < sys->capacityVertexBuff );

	TKSpriteDrawVert *v = sys->drawverts + ndx;

	v->pos = pos;
	v->st = st;
	v->color = color;

	return ndx;
}

void _tk_sprite_pushtri( TKSpriteSystem *sys, TKSpriteGroup *sg, uint32_t ndxA, uint32_t ndxB, uint32_t ndxC )
{
	(void)sg; // unused currently

	assert( sys->indices );
	assert( sys->numIndices * sizeof(uint16_t) < sys->capacityIndexBuff );
	// TODO: grow indexbuffer when needed	

	uint16_t *triIndices = sys->indices + sys->numIndices;
	
	triIndices[0] = ndxA;
	triIndices[1] = ndxB;
	triIndices[2] = ndxC;	

	sys->numIndices += 3;
	sys->currBatch->numIndices += 3;
}

void tk_push_sprite( TKSpriteHandle sh, glm::vec3 pos )
{
	tk_push_sprite_all( sh, pos, 1.0f, glm::vec4( 1.0f ), 0.0f );
}
void tk_push_sprite_scaled( TKSpriteHandle sh, glm::vec3 pos, float scale )
{
	tk_push_sprite_all( sh, pos, scale, glm::vec4( 1.0f ), 0.0f );
}

void tk_push_sprite_tint( TKSpriteHandle sh, glm::vec3 pos, glm::vec4 color )
{
	tk_push_sprite_all( sh, pos, 1.0f, color, 0.0f );
}

void tk_sprite_update_batch(TKSpriteSystem *sys, TKSpriteDef *sdef, TKSpriteGroup *sg)
{
	// Can this sprite be appended to the current batch or do we need a new one?	
	if ((!sys->currBatch) || (sys->currBatch->groupIndex != sdef->sg_ndx))  {		
		//sdtx_printf("pushsprite: new batch for sdef %d (group %d)\n", sdefIndex, sdef->sg_ndx );
		sys->currBatch = sys->spriteBatches + sys->numBatches++;
		sys->currBatch->groupIndex = sdef->sg_ndx;
		sys->currBatch->startIndex = sys->numIndices;
		sys->currBatch->numIndices = 0;
	}	else {
		//sdtx_printf("pushsprite: adding to current batch, sdef %d (group %d)\n", sdefIndex, sdef->sg_ndx );
	}
}

void tk_push_sprite_all( TKSpriteHandle sh, glm::vec3 pos, float scale, glm::vec4 color, float angle_deg )
{
	// Get the sprite group
	TKSpriteSystem *sys = &g_defaultSpriteSystem;
	TKSpriteDef *sdef = sys->spritedefs + sh.sdef_ndx;
	TKSpriteGroup *sg = sys->spritegroups + sdef->sg_ndx;

	tk_sprite_update_batch( sys, sdef, sg );

	//float sz = 0.2f * scale;
	// TODO: precalc this size in sdef
	float szX = sys->info.pixelSizeX * sdef->pxrect.width * scale;
	float szY = sys->info.pixelSizeY * sdef->pxrect.height * scale;
	
	glm::vec3 dir_right( szX, 0.0f, 0.0f );
	glm::vec3 dir_up( 0.0f, szY, 0.0f );
	if (fabs(angle_deg) > 0.0001f) {
		float angle_rad = angle_deg * (M_PI / 360.0f);
		float cs = cos(angle_rad);
		float sn = sin(angle_rad);

		dir_right = glm::vec3( dir_right.x * cs - dir_right.y * sn,
						  dir_right.x * sn + dir_right.y * cs, 0.0f );

		dir_up = glm::vec3( dir_up.x * cs - dir_up.y * sn,
						  dir_up.x * sn + dir_up.y * cs, 0.0f );
		
	}
	
	// Upper quad
	glm::vec4 st00 = sdef->st0;
	glm::vec4 st11 = sdef->st1;
	glm::vec4 st10 = glm::vec4( st11.x, st00.y, st11.z, st00.w );		
	glm::vec4 st01 = glm::vec4( st00.x, st11.y, st00.z, st11.w );	
	//glm::vec3 pA = pos + glm::vec3( -szX,  szY, 0.0f ); 
	glm::vec3 pA = pos - dir_right + dir_up;
	glm::vec4 stA = st00;
	//glm::vec3 pB = pos + glm::vec3(  szX,  szY, 0.0f );
	glm::vec3 pB = pos + dir_right + dir_up;
	glm::vec4 stB = st10;
	//glm::vec3 pC = pos + glm::vec3( -szX, -szY, 0.0f );
	glm::vec3 pC = pos - dir_right - dir_up;
	glm::vec4 stC = st01;
	//glm::vec3 pD = pos + glm::vec3(  szX, -szY, 0.0f );
	glm::vec3 pD = pos + dir_right - dir_up;
	glm::vec4 stD = st11;

	uint32_t ndxA = _tk_sprite_pushvvert( sys, sg, pA, stA, color );
	uint32_t ndxB = _tk_sprite_pushvvert( sys, sg, pB, stB, color );
	uint32_t ndxC = _tk_sprite_pushvvert( sys, sg, pC, stC, color );
	uint32_t ndxD = _tk_sprite_pushvvert( sys, sg, pD, stD, color );

	_tk_sprite_pushtri( sys, sg, ndxC, ndxB, ndxA );
	_tk_sprite_pushtri( sys, sg, ndxB, ndxC, ndxD );
}

void tk_push_sprite_tilemap( TKSpriteHandle sh, RoomInfo *map, glm::vec3 roomPos )
{
	// Get the sprite group
	TKSpriteSystem *sys = &g_defaultSpriteSystem;
	TKSpriteDef *sdef = sys->spritedefs + sh.sdef_ndx;
	TKSpriteGroup *sg = sys->spritegroups + sdef->sg_ndx;

	tk_sprite_update_batch( sys, sdef, sg );

	// Now push all the tiles
	float szX = 1.0f;
	float szY = 1.0f;
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	
	for (int i=0; i < map->numTiles; i++)
	{
		TileInfo *tt = map->tiles + i;

		//glm::vec4 color = glm::vec4( glm::rgbColor( glm::vec3( sin((float)(i * 1723.45)) * 360.0f, 0.3f, 0.9f) ), 1.0f) ;

		int tx = tt->tx;
		int ty = -tt->ty; // FIXME: ty is negative in data 
		int ndx = ty * 16 + tx;
		
		// if (map->collision[ndx]) {
		// 	color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
		// }

		glm::vec3 pos = glm::vec3( (float)tt->tx, (float)tt->ty, 0.0f ) + roomPos;

		glm::vec4 st00 = glm::vec4( tt->st0.x, tt->st0.y, 0.0f, 0.0f );
		glm::vec4 st11 = glm::vec4( tt->st1.x, tt->st1.y, 1.0f, 1.0f );
		glm::vec4 st10 = glm::vec4( st11.x, st00.y, st11.z, st00.w );		
		glm::vec4 st01 = glm::vec4( st00.x, st11.y, st00.z, st11.w );		

		glm::vec3 pA = pos + glm::vec3( 0,  0.0f, 0.0f ); 
		glm::vec4 stA = st00;
		
		glm::vec3 pB = pos + glm::vec3(  szX,  0.0f, 0.0f );		
		glm::vec4 stB = st10;
		
		glm::vec3 pC = pos + glm::vec3( 0.0f, -szY, 0.0f );		
		glm::vec4 stC = st01;
		
		glm::vec3 pD = pos + glm::vec3(  szX, -szY, 0.0f );		
		glm::vec4 stD = st11;

		uint32_t ndxA = _tk_sprite_pushvvert( sys, sg, pA, stA, color );
		uint32_t ndxB = _tk_sprite_pushvvert( sys, sg, pB, stB, color );
		uint32_t ndxC = _tk_sprite_pushvvert( sys, sg, pC, stC, color );
		uint32_t ndxD = _tk_sprite_pushvvert( sys, sg, pD, stD, color );

		_tk_sprite_pushtri( sys, sg, ndxC, ndxB, ndxA );
		_tk_sprite_pushtri( sys, sg, ndxB, ndxC, ndxD );
	}
}

void tk_sprite_drawgroups( glm::mat4 mvp, glm::mat4 ui_matrix )
{
	TKSpriteSystem *sys = &g_defaultSpriteSystem;
	
	TKDefaultSpriteShaderParams params = {};
	params.mvp = mvp;

	sdtx_printf("--- Have %d groups, %d batches\n", sys->numSpriteGroups, sys->numBatches );

	sg_update_buffer( sys->bind.vertex_buffers[0], sys->drawverts, sys->numverts * sizeof(TKSpriteDrawVert));
	sg_update_buffer( sys->bind.index_buffer, sys->indices, sys->numIndices * sizeof(uint16_t) );
	
	sg_apply_pipeline(sys->pipe);
	for (int i=0; i < sys->numBatches; i++)
	{		
		TKSpriteBatch *batch = sys->spriteBatches + i;
		TKSpriteGroup *sgroup = sys->spritegroups + batch->groupIndex;
	    
	    sys->bind.fs_images[ 0 ] = sgroup->atlas;

	    if (sgroup->is_ui_sprite) {
	    	params.mvp = ui_matrix;
	    } else {
	    	params.mvp = mvp;	    
	    }

		sg_apply_bindings( &(sys->bind) ); 
	    sg_apply_uniforms( SG_SHADERSTAGE_VS, 0, &params, sizeof(params));

	    //sdtx_printf("batch %d, start %d num %d\n", i, batch->startIndex, batch->numIndices );
	    sg_draw( batch->startIndex, batch->numIndices, 1);
	}

	// reset the system for the next frame
	sys->numverts = 0;
    sys->numIndices = 0;
    sys->numBatches = 0;
    sys->currBatch = NULL;
}

