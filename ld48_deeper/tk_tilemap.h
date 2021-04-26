#ifndef TK_TILEMAP_H
#define TK_TILEMAP_H

#include "glm/mat4x4.hpp"

typedef struct TileInfo_s {
    int tx;
    int ty;
    glm::vec2 st0;
    glm::vec2 st1;
} TileInfo;

typedef struct TileRect_s {
	int tx, ty, w, h;
} TileRect;

typedef struct JournalText_s {
	TileRect rect;
	char *text;
	bool viewed;
} JournalText;

typedef struct SleepZone_s {
	TileRect rect;
	bool asleepHere;
} SleepZone;

typedef struct ActorInfo_s {
	const char name[16];
	int tx;
	int ty;	
	const char *phrase[5];
	int npcIndex;
} ActorInfo;

#define MAX_TILES_PER_ROOM (500)
typedef struct RoomInfo_s {
    const char name[32];
    int worldX;
    int worldY;
    int collision[ 16*11 ];

    // NOTE: NOT in grid order, may be multiple layers too
    int numTiles;
    TileInfo tiles[ MAX_TILES_PER_ROOM ];

    JournalText journal;
    SleepZone sleeps[5];
    ActorInfo actor;
} RoomInfo;



#define MAX_ROOMS (100)
typedef struct WorldMap_s {
	RoomInfo *rooms[MAX_ROOMS];
} WorldMap;


#endif