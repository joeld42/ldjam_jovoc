#ifndef TK_TILEMAP_H
#define TK_TILEMAP_H

#include "glm/mat4x4.hpp"

typedef struct TileInfo_s {
    int tx;
    int ty;
    glm::vec2 st0;
    glm::vec2 st1;
} TileInfo;


#define MAX_TILES_PER_ROOM (500)
typedef struct RoomInfo_s {
    const char name[32];
    int worldX;
    int worldY;
    int collision[ 16*11 ];

    // NOTE: NOT in grid order, may be multiple layers too
    int numTiles;
    TileInfo tiles[ MAX_TILES_PER_ROOM ];
} RoomInfo;

#define MAX_ROOMS (100)
typedef struct WorldMap_s {
	RoomInfo *rooms[MAX_ROOMS];
} WorldMap;


#endif