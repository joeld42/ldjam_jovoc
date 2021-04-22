#ifndef TK_SPRITE_H
#define TK_SPRITE_H

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define TK_MAX_SPRITEGROUPS (128)
#define TK_MAX_SPRITEDEFS (1024)

// sprite system params, all zeros are reasonable defaults
typedef struct TKSpriteSystemDesc_s
{
	float pixelSizeX; // Pixel size in world space. If this is 0 it will
	float pixelSizeY; // assume 1.0
} TKSpriteSystemDesc;

typedef struct TkSpritePixelRect_s {
	int x;
	int y;
	int width;
	int height;
} TKSpritePixelRect;

typedef struct TKSpriteHandle_s
{
	uint32_t sdef_ndx;
} TKSpriteHandle;

void tk_sprite_init_system( TKSpriteSystemDesc info );

TKSpriteHandle tk_sprite_make( const char *path );
TKSpriteHandle tk_sprite_make_st( const char *path, glm::vec2 st0, glm::vec2 st1 );
TKSpriteHandle tk_sprite_make_px( const char *path, TKSpritePixelRect pxrect );

void tk_push_sprite( TKSpriteHandle sh, glm::vec3 pos );
void tk_push_sprite_scaled( TKSpriteHandle sh, glm::vec3 pos, float scale );
void tk_push_sprite_tint( TKSpriteHandle sh, glm::vec3 pos, glm::vec4 color );
void tk_push_sprite_all( TKSpriteHandle sh, glm::vec3 pos, float scale, glm::vec4 color );


void tk_sprite_drawgroups( glm::mat4 mvp );

#endif