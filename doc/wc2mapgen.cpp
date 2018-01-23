//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name map_random.cpp - Random map generator */
//
//      (c) Copyright 2010 by Exa.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "iolib.h"


struct linear_congruential_data {
	uint32_t seed;
	uint32_t m; // modulo
	uint32_t a; // multiplicative term
	uint32_t b; // additive term
} lcg_data;

// implements seed' = a*seed+b mod m without incurring unsigned wraparound
uint32_t lcg() {
  uint64_t newseed = lcg_data.seed;
  newseed *= lcg_data.a;
  newseed += lcg_data.b;
  lcg_data.seed = newseed%lcg_data.m;
  return newseed;
}

void seed_lcg(uint32_t seed) {	
	lcg_data.seed = seed;
	lcg_data.m = 2147483647;
	lcg_data.a = 16807; // other choices: 48271, 69621 
	lcg_data.b = 0;
}



static int min(int a, int b) {return a < b ? a : b;}

static int nears[9][2] = {{-1,-1},  { 0,-1},  { 1,-1},
                          {-1, 0},  { 0, 0},  { 1, 0},
                          {-1, 1},  { 0, 1},  { 1, 1}};


typedef struct Tile Tile;
typedef struct TileType TileType;


struct TileType {
	int typeId;
	char const *name;	// printable name
	int base;		// tile type, on which tiles of this type can be placed
	Tile *tiles[100];	// tiles that belong to this type
	int numTiles;
};


#define MAX_TILE_IDS 8
struct Tile {
	int ids[MAX_TILE_IDS];	// stratagus ids, several for alternating tiles
	int idsUsed;
	int type;		// common identifier for similiar tiles
	int sides[3][3];	// types of this tile for different directions
};


#define GRASS        0x01
#define FOREST       0x02
#define ROCK         0x04
#define COAST        0x08
#define WATER        0x10
#define DEEP         0x20
#define LAND         (GRASS|FOREST|ROCK|COAST)
#define WILDCARD     (GRASS|FOREST|ROCK|COAST|WATER|DEEP)


static char const *tilesets[] = {
	"scripts/tilesets/summer.lua",
	"scripts/tilesets/swamp.lua",
	"scripts/tilesets/wasteland.lua",
	"scripts/tilesets/winter.lua"
};

static int tilesetsNum = 4;
static char const *tileset;

static TileType tileTypes[256];

#define MAX_TILES 1000
static Tile tiles[MAX_TILES];
static int tilesUsed = 0;
static int refixNeeded; // maps needs another refixation cycle
static int numFixes;


typedef struct Point Point;
typedef struct Cell Cell;
typedef struct Unit Unit;



typedef struct Zone Zone;
typedef struct ZoneNeighbor ZoneNeighbor;

struct Unit {
	char const *name;
	int size;
	int owner;
	int resources;
	Cell *cell;
	int baseCenter;
	int goldMine;
};

#define MAX_UNITS 500
static Unit units[MAX_UNITS];
static int unitsUsed;

struct Point {int x,y;};

struct Cell {
	Tile *tile;
	Unit *unit;
	int x,y;
	Zone *zone;
	int visited; // for traversal alghorithms
};

static int w,h;
static int realW, realH;
static Cell map[512*512];


#define MAX_REWRITERS  50
#define MAX_MATRICES   10

#define MAT_DIM 3

typedef struct Rewriter Rewriter;
struct Rewriter {
	int mats[MAX_MATRICES][MAT_DIM*MAT_DIM]; 
	int numMats;
};

static Rewriter rewriters[MAX_REWRITERS];
static int rewritersUsed;

#define MIRROR_V     1
#define MIRROR_H     2
#define MIRROR_D     (MIRROR_V|MIRROR_H)


static int land;
static int landCount;
static int deep,water,coast,grass,trees,rock;

#define MAX_ISLANDS 101

typedef struct Island Island;
struct Island {
	int x,y;
};

static Island islands[MAX_ISLANDS];
static int islandCount;

static int terraCount[256];


// I use term "emitters", but they also called "axioms"
static int rockEmitters;
static int forestEmitters;
static int coastEmitters;

static int difficulty;

static int goldMines;
static int goldPerGoldMine;
static int oilPatches;
static int oilPerPatch;

#define MAX_GOLD_MINES 200
static Point goldMinesLocs[MAX_GOLD_MINES];
static int goldMinesUsed;


#define MAX_OIL_PATCHES 200
static Point oilPatchesLocs[MAX_OIL_PATCHES];
static int oilPatchesUsed;




#define HUMAN 0
#define ORC   1

typedef struct Player Player;
struct Player {
	int num;
	int race;
	int human;
	int x,y; // start location
	int gold, wood, oil;
	int numBases;
	int basesPlaced;
	int guardian;
};

#define MAX_PLAYERS 32
static Player players[MAX_PLAYERS];
static int numPlayers;
static int userRace;
static int userPlayerNum;

#define ZONE_UNKNOWN  0x00
#define ZONE_LAND     0x01
#define ZONE_FOREST   0x02
#define ZONE_ROCK     0x04
#define ZONE_COAST    0x08
#define ZONE_WATER    0x10

struct ZoneNeighbor {
	Zone *n;
	ZoneNeighbor *next;
};

struct Zone {
	int type;
	int center_x, center_y;
	int size;
	int has;
	int players;
	int goldMines;
	int oilPatches;
	ZoneNeighbor *neighbors;
};


#define MAX_NEIGHBORS      (256*256)
#define MAX_ZONES          (128*128)

static ZoneNeighbor neighbors[MAX_NEIGHBORS];
static int neighborsUsed;

static Zone zones[MAX_ZONES];
static int zonesUsed;






static int metric(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	return dx*dx + dy*dy;
}


static int Isis(int x, int y) {
	return 0 <= x && x < w && 0 <= y && y < h;
}

static Cell *xy(int x, int y) {
	return Isis(x,y) ? &map[x + y*w] : 0;
}

// returns type of tile at x,y as it seen from fx,fy
static int xyt(int fx, int fy, int x, int y) {
	Cell *c = xy(x,y);
	Cell *f = xy(fx,fy);
	int r;
	assert(f);
	if(c) {
		Tile *t = c->tile;
		r = t->sides[1 + fy-y][1 + fx-x];
		// HACK: forest-coast border creation should be done differently
		if(f->tile->type == FOREST && r == COAST) {
			r = GRASS;
		}
	} else {
		r = f->tile->type;
	}

	return r;
}







static int canPlace(int x, int y, int size, int land) {
	int i,j;

	if(x < 0 || x+size >= realW || y < 0 || y >= realH+size) return 0;

	for(i = 0; i < size; i++)
		for(j = 0; j < size; j++) {
			if(xy(x+j,y+i)->unit) return 0;
			if(land) {
				if(xy(x+j,y+i)->tile->type != GRASS) return 0;
			} else {
				if(xy(x+j,y+i)->tile->type != WATER) return 0;
				if(xy(x+j,y+i)->tile->ids[0] != 16) return 0; // only open water
			}
		}
	return 1;
}

static void placeUnit(int x, int y, Unit *u) {
	int i,j;

	u->cell = xy(x,y);

	for(i = 0; i < u->size; i++)
		for(j = 0; j < u->size; j++)
			xy(x+j,y+i)->unit = u;
}




static void addNeighbor(Zone *z, Zone *neighbor) {
	ZoneNeighbor *zn;
	for(zn = z->neighbors; zn; zn = zn->next)
		if(zn->n == neighbor)
			return;

	assert(neighborsUsed < MAX_NEIGHBORS);
	zn = &neighbors[neighborsUsed++];
	zn->n = neighbor;
	zn->next = z->neighbors;
	z->neighbors = zn;
}




static int zoneType(Cell *c) {
	switch(c->tile->type) {
	case GRASS: return ZONE_LAND;
	case COAST: return ZONE_LAND;
	case WATER: return ZONE_WATER;
	case FOREST: return ZONE_FOREST;
	case ROCK: return ZONE_ROCK;
	default: return ZONE_UNKNOWN;
	}
}

static void markZoneRecursive(Zone *z, int x, int y) {
	Cell *c = xy(x,y);
	if(!c || x >= realW || y >= realH || c->zone || zoneType(c) != z->type) return;

	c->zone = z;
	z->size++;

	for(int i = 0; i < 9; i++)
		if(nears[i][0] && nears[i][1])
			markZoneRecursive(z, x+nears[i][0],y+nears[i][1]);
}


static void discoverZoneNeighbors(Zone *z, int x, int y) {
	Cell *c = xy(x,y);
	if(!c) return;

	if(c->visited || zoneType(c) != z->type) {
		if(c->zone != z) {
			addNeighbor(z, c->zone);
			z->has |= zoneType(c);
		}
		return;
	}

	c->visited = 1;

	for(int i = 0; i < 9; i++)
		if(nears[i][0] && nears[i][1])
			markZoneRecursive(z, x+nears[i][0],y+nears[i][1]);
}

static void discoverZoneWorldNeighbors() {
	int i;
	for(i = 0; i < w*h; i++) map[i].visited = 0;

	for(i = 0; i < zonesUsed; i++) {
		Zone *z = &zones[i];
		discoverZoneNeighbors(z, z->center_x, z->center_y);
	}
}

static void markZone(Cell *c) {
	Zone *z;
	assert(zonesUsed < MAX_ZONES);

	z = &zones[zonesUsed++];
	z->type = zoneType(c);
	z->size = 0;
	z->has = 0;
	z->neighbors = 0;
	z->center_x = c->x;
	z->center_y = c->y;
	markZoneRecursive(z, z->center_x, z->center_y);
}

static void buildZoneWorld() {
	int x, y;
	neighborsUsed = 0;
	zonesUsed = 0;

	for(y = 0; y < realH; y++) {
		for(x = 0; x < realW; x++) {
			Cell *c = xy(x,y);
			if(c->zone) continue; // already marked
			markZone(c);
		}
	}
	discoverZoneWorldNeighbors();
}

static void printZones() {
	int i;

	for(i = 0; i < zonesUsed; i++) {
		char const *t;
		switch(zones[i].type) {
		case ZONE_LAND: t = "land"; break;
		case ZONE_COAST: t = "coast"; break;
		case ZONE_WATER: t = "water"; break;
		case ZONE_FOREST: t = "forest"; break;
		case ZONE_ROCK: t = "rock"; break;
		default: t = "unknown"; break;
		}
		printf("Zone%d: size:%d type:%s\n", i, zones[i].size, t);
	}
	printf("Total Zones: %d\n", zonesUsed);
}


static Unit *placerUnits;
static int placerNumUnits;
static int placerPlaced;
static int placerLand;
static void zonePlaceRecursive(Zone *z, int x, int y) {
	Cell *c = xy(x,y);
	if(placerPlaced == placerNumUnits || !c) goto end;
	if(c->visited || zoneType(c) != z->type) goto end;
	c->visited = 1;

	if(!c->unit && canPlace(x,y,placerUnits[placerPlaced].size,placerLand)) {
		placeUnit(x,y,&placerUnits[placerPlaced++]);
	}

	for(int i = 0; i < 9; i++)
		if(nears[i][0] && nears[i][1])
			zonePlaceRecursive(z, x+nears[i][0],y+nears[i][1]);
end:;
}


static int zonePlace(Zone *z, Unit *units, int numUnits, int range, int land) {
	placerUnits = units;
	placerNumUnits = numUnits;
	placerLand = land;
	placerPlaced = 0;
	zonePlaceRecursive(z, z->center_x, z->center_y);
	return placerPlaced;
}




static void addTileSet(int typeId, int base, char const *name) {
	assert(typeId < 256);
	tileTypes[typeId].typeId = typeId;
	tileTypes[typeId].name = name;
	tileTypes[typeId].base = base;
	tileTypes[typeId].numTiles = 0;
}

static Tile *addTile(int id, int nw, int n   , int ne,
                     int w , int type, int e ,
                     int sw, int s   , int se) {
	Tile *t;
	assert(tilesUsed < MAX_TILES);

	t = &tiles[tilesUsed++];
	tileTypes[type].tiles[tileTypes[type].numTiles++] = t;

	t->ids[0] = id;
	t->idsUsed = 1;
	t->type = type;

	t->sides[0][0] = nw;
	t->sides[0][1] = n;
	t->sides[0][2] = ne;
	t->sides[1][0] = w;
	t->sides[1][1] = type;
	t->sides[1][2] = e;
	t->sides[2][0] = sw;
	t->sides[2][1] = s;
	t->sides[2][2] = se;
	return t;
}


static void initGrassTileSet() {
	Tile *t;
	addTileSet(GRASS, GRASS, "grass");

	t = addTile(80, GRASS, GRASS, GRASS,
	                GRASS, GRASS, GRASS,
	                GRASS, GRASS, GRASS);
	t->ids[t->idsUsed++] = 81;
}

static void initForestTileSet() {
	Tile *t;
	addTileSet(FOREST, GRASS, "forest");

	t = addTile(112, FOREST, FOREST, FOREST,
	                 FOREST, FOREST, FOREST,
	                 FOREST, FOREST, FOREST);
	t->ids[t->idsUsed++] = 113;
	t->ids[t->idsUsed++] = 114;


	// Tiles with one corner removed
	addTile(2000, GRASS , FOREST, FOREST,
	              FOREST, FOREST, FOREST,
	              FOREST, FOREST, FOREST);

	addTile(1984, FOREST, FOREST, GRASS ,
	              FOREST, FOREST, FOREST,
	              FOREST, FOREST, FOREST);

	addTile(1888, FOREST, FOREST, FOREST,
	              FOREST, FOREST, FOREST,
	              FOREST, FOREST, GRASS );

	addTile(1952, FOREST, FOREST, FOREST,
	              FOREST, FOREST, FOREST,
	              GRASS , FOREST, FOREST);

	// Tiles with two corners removed
	addTile(1872, GRASS , FOREST, FOREST,
	              FOREST, FOREST, FOREST,
	              FOREST, FOREST, GRASS);

	addTile(1920, FOREST, FOREST, GRASS ,
	              FOREST, FOREST, FOREST,
	              GRASS , FOREST, FOREST);

	// Tiles with upto three corners removed
	addTile(1968, GRASS|FOREST,  GRASS, GRASS|FOREST,
	                 FOREST   , FOREST,    FOREST   ,
	                 FOREST   , FOREST,    FOREST   );

	addTile(1856, FOREST, FOREST, GRASS|FOREST,
	              FOREST, FOREST,    GRASS    ,
	              FOREST, FOREST, GRASS|FOREST);

	addTile(1824,    FOREST   , FOREST,    FOREST   ,
	                 FOREST   , FOREST,    FOREST   ,
	              GRASS|FOREST,  GRASS, GRASS|FOREST);

	addTile(1936, GRASS|FOREST, FOREST, FOREST,
	                 GRASS    , FOREST, FOREST,
	              GRASS|FOREST, FOREST, FOREST);

	// Tiles with upto five corners removed
	addTile(1904, GRASS|FOREST,  GRASS, GRASS|FOREST,
	                 GRASS    , FOREST,    FOREST   ,
	              GRASS|FOREST, FOREST,    FOREST   );

	addTile(1840, GRASS|FOREST,   GRASS, GRASS|FOREST,
	                 FOREST,     FOREST,    GRASS    ,
	                 FOREST,     FOREST, GRASS|FOREST);

	addTile(1792,    FOREST   , FOREST, GRASS|FOREST,
	                 FOREST   , FOREST,    GRASS    ,
	              GRASS|FOREST,  GRASS, GRASS|FOREST);

	addTile(1808, GRASS|FOREST, FOREST,    FOREST   ,
	                 GRASS    , FOREST,    FOREST   ,
	              GRASS|FOREST,  GRASS, GRASS|FOREST);
}

static void initCoastTileSet() {
	Tile *t;
	addTileSet(COAST, GRASS, "coast");

	t = addTile(48, COAST, COAST, COAST,
	                COAST, COAST, COAST,
	                COAST, COAST, COAST);
	t->ids[t->idsUsed++] = 49;
	t->ids[t->idsUsed++] = 50;


	// Tiles with one corner removed
	addTile(1488, GRASS, COAST, COAST,
	              COAST, COAST, COAST,
	              COAST, COAST, COAST);

	addTile(1472, COAST, COAST, GRASS,
	              COAST, COAST, COAST,
	              COAST, COAST, COAST);

	addTile(1376, COAST, COAST, COAST,
	              COAST, COAST, COAST,
	              COAST, COAST, GRASS);

	addTile(1440, COAST, COAST, COAST,
	              COAST, COAST, COAST,
	              GRASS, COAST, COAST);

	// Tiles with two corners removed
	addTile(1360, GRASS, COAST, COAST,
	              COAST, COAST, COAST,
	              COAST, COAST, GRASS);

	addTile(1408, COAST, COAST, GRASS,
	              COAST, COAST, COAST,
	              GRASS, COAST, COAST);

	// Tiles with upto three corners removed
	addTile(1456, GRASS|COAST, GRASS, GRASS|COAST,
	                 COAST   , COAST,    COAST   ,
	                 COAST   , COAST,    COAST   );

	addTile(1344, COAST, COAST, GRASS|COAST,
	              COAST, COAST,    GRASS    ,
	              COAST, COAST, GRASS|COAST);

	addTile(1312,    COAST   , COAST,    COAST   ,
	                 COAST   , COAST,    COAST   ,
	              GRASS|COAST, GRASS, GRASS|COAST);

	addTile(1424, GRASS|COAST, COAST, COAST,
	                 GRASS   , COAST, COAST,
	              GRASS|COAST, COAST, COAST);

	// Tiles with upto five corners removed
	addTile(1392, GRASS|COAST, GRASS, GRASS|COAST,
	                 GRASS   , COAST,    COAST   ,
	              GRASS|COAST, COAST,    COAST   );

	addTile(1328, GRASS|COAST,  GRASS, GRASS|COAST,
	                 COAST,     COAST,    GRASS   ,
	                 COAST,     COAST, GRASS|COAST);

	addTile(1280,    COAST   , COAST, GRASS|COAST,
	                 COAST   , COAST,    GRASS   ,
	              GRASS|COAST, GRASS, GRASS|COAST);

	addTile(1296, GRASS|COAST, COAST,    COAST   ,
	                 GRASS   , COAST,    COAST   ,
	              GRASS|COAST, GRASS, GRASS|COAST);
}

static void initWaterTileSet() {
	Tile *t;
	addTileSet(WATER, COAST, "water");
	

	t = addTile(16, WATER, WATER, WATER,
	                WATER, WATER, WATER,
	                WATER, WATER, WATER);
	t->ids[t->idsUsed++] = 17;

	// Tiles with one corner removed
	addTile(720, COAST, WATER, WATER,
	             WATER, WATER, WATER,
	             WATER, WATER, WATER);

	addTile(704, WATER, WATER, COAST,
	             WATER, WATER, WATER,
	             WATER, WATER, WATER);

	addTile(608, WATER, WATER, WATER,
	             WATER, WATER, WATER,
	             WATER, WATER, COAST);

	addTile(672, WATER, WATER, WATER,
	             WATER, WATER, WATER,
	             COAST, WATER, WATER);

	// Tiles with two corners removed
	addTile(592, COAST, WATER, WATER,
	             WATER, WATER, WATER,
	             WATER, WATER, COAST);

	addTile(640, WATER, WATER, COAST,
	             WATER, WATER, WATER,
	             COAST, WATER, WATER);

	// Tiles with upto three corners removed
	addTile(688, COAST|WATER, COAST, COAST|WATER,
	                WATER   , WATER,    WATER   ,
	                WATER   , WATER,    WATER   );

	addTile(576, WATER, WATER, COAST|WATER,
	             WATER, WATER,    COAST    ,
	             WATER, WATER, COAST|WATER);

	addTile(544,    WATER   , WATER,    WATER   ,
	                WATER   , WATER,    WATER   ,
	             COAST|WATER, COAST, COAST|WATER);

	addTile(656, COAST|WATER, WATER, WATER,
	                COAST   , WATER, WATER,
	             COAST|WATER, WATER, WATER);

	// Tiles with upto five corners removed
	addTile(624, COAST|WATER, COAST, COAST|WATER,
	                COAST   , WATER,    WATER   ,
	             COAST|WATER, WATER,    WATER   );

	addTile(560, COAST|WATER,  COAST, COAST|WATER,
	                WATER,     WATER,    COAST   ,
	                WATER,     WATER, COAST|WATER);

	addTile(512,    WATER   , WATER, COAST|WATER,
	                WATER   , WATER,    COAST   ,
	             COAST|WATER, COAST, COAST|WATER);

	addTile(528, COAST|WATER, WATER,    WATER   ,
	                COAST   , WATER,    WATER   ,
	             COAST|WATER, COAST, COAST|WATER);
}

static void initRockTileSet() {
	Tile *t;
	addTileSet(ROCK, COAST, "rock");

	t = addTile(128, ROCK, ROCK, ROCK,
	                 ROCK, ROCK, ROCK,
	                 ROCK, ROCK, ROCK);

	t->ids[t->idsUsed++] = 129;
	t->ids[t->idsUsed++] = 130;
	t->ids[t->idsUsed++] = 131;

	// Tiles with one corner removed
	addTile(1232, COAST, ROCK, ROCK,
	              ROCK,  ROCK, ROCK,
	              ROCK,  ROCK, ROCK);

	addTile(1216, ROCK, ROCK, COAST,
	              ROCK, ROCK,  ROCK,
	              ROCK, ROCK,  ROCK);

	addTile(1120, ROCK, ROCK,  ROCK,
	              ROCK, ROCK,  ROCK,
	              ROCK, ROCK, COAST);

	addTile(1184, ROCK,  ROCK, ROCK,
	              ROCK,  ROCK, ROCK,
	              COAST, ROCK, ROCK);

	// Tiles with two corners removed
	addTile(1104, COAST, ROCK,  ROCK,
	              ROCK,  ROCK,  ROCK,
	              ROCK,  ROCK, COAST);

	addTile(1152, ROCK,  ROCK, COAST,
	              ROCK,  ROCK,  ROCK,
	              COAST, ROCK,  ROCK);

	// Tiles with upto three corners removed
	addTile(1200, COAST|ROCK, COAST, COAST|ROCK,
	                 ROCK   ,  ROCK,    ROCK   ,
	                 ROCK   ,  ROCK,    ROCK   );

	addTile(1088, ROCK, ROCK, COAST|ROCK,
	              ROCK, ROCK,    COAST    ,
	              ROCK, ROCK, COAST|ROCK);

	addTile(1056,   ROCK   ,  ROCK,    ROCK   ,
	                ROCK   ,  ROCK,    ROCK   ,
	             COAST|ROCK, COAST, COAST|ROCK);

	addTile(1168, COAST|ROCK, ROCK, ROCK,
	                 COAST  , ROCK, ROCK,
	              COAST|ROCK, ROCK, ROCK);

	// Tiles with upto five corners removed
	addTile(1136, COAST|ROCK, COAST, COAST|ROCK,
	                 COAST  , ROCK ,    ROCK   ,
	              COAST|ROCK, ROCK ,    ROCK   );

	addTile(1072, COAST|ROCK,  COAST, COAST|ROCK,
	                 ROCK   ,   ROCK,    COAST  ,
	                 ROCK   ,   ROCK, COAST|ROCK);

	addTile(1024,   ROCK   ,  ROCK, COAST|ROCK,
	                ROCK   ,  ROCK,    COAST  ,
	             COAST|ROCK, COAST, COAST|ROCK);

	addTile(1040, COAST|ROCK,  ROCK,    ROCK    ,
	                 COAST  ,  ROCK,    ROCK    ,
	              COAST|ROCK, COAST, COAST|ROCK);
}

static void initTileset() {
	if(tilesUsed) return;

	initGrassTileSet();
	initForestTileSet();
	initCoastTileSet();
	initWaterTileSet();
	initRockTileSet();
}

static Tile *defaultTileOf(int type) {
	return tileTypes[type].tiles[0];
}

static void fixTile(int x, int y) {
	int i,j,k, sides[3][3];
	Tile *tile = xy(x,y)->tile;
	TileType *type = &tileTypes[tile->type];
	sides[0][0] = xyt(x,y,x-1,y-1);
	sides[0][1] = xyt(x,y,x+0,y-1);
	sides[0][2] = xyt(x,y,x+1,y-1);

	sides[1][0] = xyt(x,y,x-1,y+0);
	sides[1][1] = xyt(x,y,x+0,y+0);
	sides[1][2] = xyt(x,y,x+1,y+0);

	sides[2][0] = xyt(x,y,x-1,y+1);
	sides[2][1] = xyt(x,y,x+0,y+1);
	sides[2][2] = xyt(x,y,x+1,y+1);

	for(i = 0; i < type->numTiles; i++) {
		for(j = 0; j < 3; j++) for(k = 0; k < 3; k++) {
			int (*tsides)[3] = type->tiles[i]->sides;
			if(!(tsides[j][k]&sides[j][k]) && (j != k || k != 1)) {
				goto top_continue;
			}
		}
		// got a match!
		xy(x,y)->tile = type->tiles[i];
		goto end;
top_continue:;
	}

	// no match at all, degrade...
	if(defaultTileOf(tile->type) != tile) {
		xy(x,y)->tile = defaultTileOf(tile->type);
		refixNeeded = 1;
		numFixes++;
	} else if(defaultTileOf(type->base) != tile) {
		//printf("fixer fail:%d,%d\n", x,y);
		xy(x,y)->tile = defaultTileOf(type->base);
		refixNeeded = 1;
		numFixes++;
	}
end:;
}

static void fixTilesOfType(int type) {
	int x,y;

	for(y = 0; y < h; y++)
		for(x = 0; x < w; x++) {
			if(xyt(x,y,x,y) == type)
				fixTile(x,y);
		}

}

static void fixTiles() {
	do {
		numFixes = 0;
		do {refixNeeded = 0; fixTilesOfType(ROCK);} while(refixNeeded);
		do {refixNeeded = 0; fixTilesOfType(FOREST);} while(refixNeeded);
		do {refixNeeded = 0; fixTilesOfType(WATER);} while(refixNeeded);
		do {refixNeeded = 0; fixTilesOfType(COAST);} while(refixNeeded);
	} while(numFixes);
}

static void savePlayer(CFile &f, int index) {
	for (int i = 0; i < numPlayers; i++) {
		Player *p = &players[i];
		if (p->num != index) continue;
		// The order of these functions is important!!!
		f.printf("SetStartView(%d, %d, %d)\n\n", p->num, p->x, p->y);
		f.printf("SetPlayerData(%d, \"Resources\", \"wood\", %d)\n", p->num, p->wood);
		f.printf("SetPlayerData(%d, \"Resources\", \"gold\", %d)\n", p->num, p->gold);
		f.printf("SetPlayerData(%d, \"Resources\", \"oil\", %d)\n", p->num, p->oil);
		f.printf("SetPlayerData(%d, \"RaceName\", \"%s\")\n",
		         p->num, p->race == 0 ? "human" : "orc");
		f.printf("SetAiType(%d, \"generic\")\n");
		return;
	}
}

static void save(char const *filename) {
	int i,x,y;
	CFile f;
	char fnbuf[1024];
	int pt[16];

	sprintf(fnbuf, "%s.sms", filename);
	if (f.open(fnbuf, CL_WRITE_GZ | CL_OPEN_WRITE) == -1) {
		printf("Couldn't create %s\n", fnbuf);
		abort();
	}

	for(i = 0; i < 16; i++) savePlayer(f, i);

	f.printf("LoadTileModels(\"%s\")\n\n", tileset);
          

	for(y = 0; y < realH; y++)
		for(x = 0; x < realW; x++) {
			Tile *t = xy(x, y)->tile;
			f.printf("SetTile(%d,%d,%d,0)\n",t->ids[lcg()%t->idsUsed],x,y);
		}
	f.printf("\n");


	for(i = 0; i < unitsUsed; i++) {
		Unit *u = &units[i];
		if(!u->cell) {
			printf("save(): unit with no location: %s\n", u->name);
			continue;
		}
		f.printf("unit = CreateUnit(\"%s\",%d,{%d,%d})\n",
		           u->name,players[u->owner].num,u->cell->x,u->cell->y);
		if(u->resources) {
			f.printf("SetResourcesHeld(unit, %d)\n", u->resources);
		}
	}
	f.printf("\n");

	f.close();




	sprintf(fnbuf, "%s.smp", filename);
	if (f.open(fnbuf, CL_WRITE_GZ | CL_OPEN_WRITE) == -1) {
		printf("Couldn't create %s\n", fnbuf);
		abort();
	}

	memset(pt, 0, sizeof(int)*16);
	for(i = 1; i < numPlayers; i++)
		pt[players[i].num] = players[i].human ? 2 : 1;

	f.printf("DefinePlayerTypes(");
	int numPlayers = 0;
	for(i = 0; i < 8; i++) {
		if(i) f.printf(", ");
		if(pt[i] == 0) f.printf("\"nobody\"");
		else if(pt[i] == 1) f.printf("\"computer\"");
		else {
			f.printf("\"person\"");
			numPlayers++;
		}
	}
	f.printf(")\n");

	// last parameter is Map UID. It is unused.
	f.printf("PresentMap(\"Random world Generator by Exa.\", %d, %d, %d, 2)\n",
	         numPlayers, realW, realH);
	f.close();
}

static void printWorld() {
	int x,y;
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) switch(xy(x,y)->tile->type) {
		case DEEP:   printf("D "); break;
		case WATER:  printf("W "); break;
		case COAST:  printf("C "); break;
		case GRASS:  printf("G "); break;
		case FOREST: printf("F "); break;
		case ROCK:   printf("R "); break;
		default:   printf("U "); break;
		}
		printf("\n");
	}
}





static void compileRewriter(Rewriter *r, char const *p, int n) {
	int i, j, k, m, v, t;
	r->numMats = n;
	for(i = 0; i < MAT_DIM; i++) {
		for(j = 0; j < n; j++) {
			for(k = 0; k < MAT_DIM; k++) {
				m = p[k*2 + j*2*MAT_DIM + i*n*2*MAT_DIM];
				v = p[k*2 + j*2*MAT_DIM + i*n*2*MAT_DIM + 1];
				t = WILDCARD;
				switch(v) {
				case 'w': t = WATER; break;
				case 'c': t = COAST; break;
				case 'd': t = DEEP; break;
				case 'l': t = LAND; break;
				case 'g': t = GRASS; break;
				case 'r': t = ROCK; break;
				case 'f': t = FOREST; break;
				case 'u': t = WILDCARD; break;
				case '*': t = WILDCARD; break;
				default: t = WILDCARD; break;
				}
				if(m == '!') t = ~t;
				r->mats[j][k + i*MAT_DIM] = t;
			}
		}
	}
}



static void mirrorRewriter(Rewriter *mirror, Rewriter *source, int flags) {
	int i, mx, my, sx, sy;
	mirror->numMats = source->numMats;
	for(i = 0; i < source->numMats; i++) {
		int *m = mirror->mats[i];
		int *s = source->mats[i];
		for(my = 0; my < MAT_DIM; my++) {
			if(flags&MIRROR_V) sy = MAT_DIM-1 - my;
			else sy = my;
			for(mx = 0; mx < MAT_DIM; mx++) {
				if(flags&MIRROR_H) sx = MAT_DIM-1 - mx;
				else sx = mx;
				m[mx + my*MAT_DIM] = s[sx + sy*MAT_DIM];
			}
		}
	}
}

static void loadRewriteRules(char const **rewriteRules) {
	int i, l, n;
	Rewriter *r;
	char const *p, *o;

	rewritersUsed = 0;

	for(i = 0; rewriteRules[i]; i++);
	assert(!(i%2));

	for(i = 0; rewriteRules[i]; i+=2) {
		assert(rewritersUsed < MAX_REWRITERS);
		o = rewriteRules[i];
		p = rewriteRules[i + 1];
		l = strlen(p);
		n = l/(MAT_DIM*MAT_DIM*2);
		assert(l);
		assert(l%(MAT_DIM*MAT_DIM) == 0);
		assert(1 < n && n <= MAX_MATRICES);
		r = &rewriters[rewritersUsed++];
		compileRewriter(r, p, n);
		if(strchr(o,'v')) mirrorRewriter(&rewriters[rewritersUsed++], r, MIRROR_V);
		if(strchr(o,'h')) mirrorRewriter(&rewriters[rewritersUsed++], r, MIRROR_H);
		if(strchr(o,'m')) mirrorRewriter(&rewriters[rewritersUsed++], r, MIRROR_D);
	}
}

static void printRewriteRules() {
	int i,j,m;
	for(m = 0; m < rewriters[0].numMats; m++) {
		for(i = 0; i < MAT_DIM; i++) {
			for(j = 0; j < MAT_DIM; j++) {
				int v = rewriters[0].mats[m][j + i*MAT_DIM];
				switch(v) {
				case WILDCARD:   printf("* "); break;
				case LAND:       printf("L "); break;
				case WATER:      printf("W "); break;
				case DEEP:       printf("D "); break;
				case COAST:      printf("C "); break;
				case GRASS:      printf("G "); break;
				case FOREST:     printf("F "); break;
				case ROCK:       printf("R "); break;
				default:         printf("U "); break;
				}
			}
			printf("\n");
		}
		if(m == 0) printf("----------\n");
		else printf("\n");
	}
}

static int matchRewriter(int x, int y, Rewriter *r) {
	int mx, my, *m;
	m = r->mats[0];
	for(my = 0; my < MAT_DIM; my++)
		for(mx = 0; mx < MAT_DIM; mx++) {
			Cell *c = xy(x+mx,y+my);
			Tile *t = c->tile;
			int type = m[mx + my*MAT_DIM];
			if(c->unit) return 0;
			if(type == WILDCARD) continue;
			if((t->type&type) != t->type) {
				return 0;
			}
		}
	return 1;
}

static void applyRewriter(int x, int y, Rewriter *r) {
	int mx, my, *m;
	m = r->mats[1 + lcg()%(r->numMats-1)];
	for(my = 0; my < MAT_DIM; my++)
		for(mx = 0; mx < MAT_DIM; mx++) {
			Cell *c = xy(x+mx,y+my);
			if(!c) continue;
			int new_type = m[mx + my*MAT_DIM];
			int old_type = c->tile->type;
			if(new_type == WILDCARD) continue;
			if(old_type&LAND) landCount--;
			terraCount[old_type]--;
			terraCount[new_type]++;
			c->tile = defaultTileOf(new_type);
		}
}

static struct {
	int x,y;
	Rewriter *r;
} rewriteStack[512*512];

static int rewriteSP;

static void rewrite() {
	int i, j, k, x, y;

	rewriteSP = 0;

	for(i = 0; i < MAT_DIM; i++) {
		for(y = i; y < h-MAT_DIM; y+=MAT_DIM) {
			for(k = 0; k < MAT_DIM; k++) {
				for(x = k; x < w-MAT_DIM; x+=MAT_DIM) {
					for(j = 0; j < rewritersUsed; j++) {
						if(matchRewriter(x,y,&rewriters[j]))  {
							rewriteStack[rewriteSP].x = x;
							rewriteStack[rewriteSP].y = y;
							rewriteStack[rewriteSP].r = &rewriters[j];
							rewriteSP++;
							break;
						}
					}
				}
			}
		}
	}

	while(rewriteSP--) {
		x = rewriteStack[rewriteSP].x;
		y = rewriteStack[rewriteSP].y;
		applyRewriter(x, y, rewriteStack[rewriteSP].r);
	}
}



static void clearWorldTo(int type) {
	int x,y;
	Tile *t = defaultTileOf(type);

	memset(map, 0, w*h*sizeof(Cell));
	memset(terraCount, 0, 256*sizeof(int));
	memset(zones, 0, MAX_ZONES*sizeof(Zone));

	terraCount[type] = w*h;

	for(y = 0; y < h; y++)
		for(x = 0; x < w; x++) {
			Cell *c = xy(x,y);
			c->tile = t;
			c->x = x;
			c->y = y;
			c->zone = 0;
		}
}


static char const *islandRules1[] = {
	"vhm",
	" * * *" " * * *" " * g *" " * * *" " * * *" " * * g" " * * *" " g * *" " * * *"
	" * g *" " g * *" " * * *" " * * *" " * * g" " * * *" " * * *" " * * *" " * * *"
	" * * *" " * * *" " * * *" " * g *" " * * *" " * * *" " g * *" " * * *" " * * g"
	,

	0
};

static char const *islandRules2[] = {
	"vhm",
	" * * *" " * * *" " * g *" " * * *" " * * *" " * * g" " * * *"
	" * g *" " g * *" " * * *" " * * *" " * * g" " * * *" " * * *"
	" * * *" " * * *" " * * *" " * g *" " * * *" " * * *" " g * *"
	,

	0
};

static char const *islandRules3[] = {
	"vhm",
	" * * *" " * * *" " * g *" " * * *" " * * *" " * * *" " g * *"
	" * g *" " g * *" " * * *" " * * *" " * * g" " * * *" " * * *"
	" * * *" " * * *" " * * *" " * g *" " * * *" " * * g" " * * *"
	,

	0
};

static char const *forestRules[] = {
	"vhm",
	"!w!w!w" " * * *" " * f *" " * * *" " * * *"
	"!w f!w" " f * *" " * * *" " * * *" " * * f"
	"!w!w!w" " * * *" " * * *" " * f *" " * * *"
	,

	0
};

static char const *rockRules[] = {
	"vhm",
	"!w!w!w" " * * *" " * r *" " * * r" " * * *"
	"!w r!w" " * * *" " * * *" " * * *" " * * *"
	"!w!w!w" " r * *" " * * r" " * r *" " * * r"
	,

	"",
	" * g *" " * r *"
	" g r g" " r * r"
	" r * r" " * * *"
	,


	0
};

Unit *addUnit(char const *name, int owner, int size) {
	Unit *u;
	assert(unitsUsed < MAX_UNITS);
	u = &units[unitsUsed++];
	u->name = name;
	u->owner = owner;
	u->size = size;
	u->resources = 0;
	u->cell = 0;
	return u;
}

typedef struct Place Place;
struct Place {
	int x, y;
	int size;
	int used;
};

#define MAX_PLACES 100
static Place places[MAX_PLACES];
static int placesFound;


static int findPlaces(int x, int y, int dim, int land) {
	int i,j,k,l,m;
	placesFound = 0;

	for(i = y; i < y+dim-4; i++) {
		if(i >= realH-5) break;
		for(j = x; j < x+dim-4; j++) {
			if(j >= realW-5) break;
			for(k = i; k < i+4; k++) {
				for(l = j; l < j+4; l++) {
					Cell *c = xy(l,k);
					if(!c || c->unit || c->tile->type != (land ? GRASS : WATER)) {
						goto terra_check_end;
					}
					for(m = 0; m < placesFound; m++) {
						if(places[m].x <= l && l < places[m].x+places[m].size &&
						   places[m].y <= k && k < places[m].y+places[m].size)
							goto terra_check_end;
					}
				}
			}
		terra_check_end:;
			m = min(l-j, k-i);
			if(!m) continue;
			places[placesFound].size = m;
			places[placesFound].x = j;
			places[placesFound].y = i;
			places[placesFound].used = 0;
			placesFound++;
			if(placesFound >= MAX_PLACES) goto end;
		}
	}
end:
	return placesFound;
}


#define SCAN_DIM    16
static void placeUnits() {
	int i,j,k,l,m,race;
	Cell *c;
	Player *p;
	Place *goldPlace, *hallPlace;
	Unit *u;
	int foundSizes[10];
	int goldMinesPlaced = 0;

	memset(players, 0, sizeof(Player)*MAX_PLAYERS);
	memset(units, 0, sizeof(Unit)*MAX_UNITS);

	unitsUsed = 0;

	numPlayers++;

	players[0].num = 15;

	for(i = 1; i < numPlayers; i++) players[i].num = -1;

	// assign random colors
	for(i = 1; i < numPlayers; i++) {
	color_again:
		k = lcg()%8;
		for(j = 1; j < i; j++)
			if(players[j].num == k) goto color_again;
		p = &players[i];
		p->num = k;
		p->gold = 2000;
		p->wood = 1000;
		p->oil  = 10000;
		p->numBases = 1;
		p->basesPlaced = 0;
		p->race = (lcg()&1) ? ORC : HUMAN;
		p->human = 0;
	}

	// make random player a human player
	userPlayerNum = 1 + lcg()%(numPlayers-1);
	players[userPlayerNum].human = 1;
	players[userPlayerNum].race = userRace;
	players[userPlayerNum].oil = 1000;


	// place oil patches and criters
	oilPatches = 0;
	for(i = 0; i < realH-4; i++) {
		for(j = 0; j < realW-4; j ++) {
			if(!canPlace(j,i,5,0)) {
				if(((lcg()%h) != i && (lcg()%w) != j) || (lcg()&3)) continue;
				if(!canPlace(j,i,1,1)) continue;
				placeUnit(j, i, addUnit("unit-critter", 0, 1));
				continue;
			}
			if((lcg()%h) != i && (lcg()%w) != j) continue;

			for(k = 0; k < unitsUsed; k++) {
				u = &units[k];
				m = metric(j, i, u->cell->x, u->cell->y);
				if(m < metric(0,0,30,0)) goto oil_next;
			}
			u = addUnit("unit-oil-patch", 0, 3);
			u->resources = oilPerPatch;
			//u->oilPatch = 1;
			placeUnit(j, i, u);
			c = xy(j, i);
			c->zone->oilPatches++;
			oilPatches++;
		oil_next:;
		}
	}

	for(i = 0; i < realH; i+= SCAN_DIM/4) {
		for(j = 0; j < realW; j += SCAN_DIM/4) {
			if(!findPlaces(j, i, SCAN_DIM, 1)) continue;
			memset(foundSizes, 0, 10*sizeof(int));
			for(k = 0; k < placesFound; k++) foundSizes[places[k].size]++;
			if(!foundSizes[3] && !foundSizes[4] && !foundSizes[5]) continue;


			for(k = 0; k < placesFound; k++) {
				if(places[k].size < 4) continue;
				c = xy(places[k].x,places[k].y);
				for(l = 0; l < placesFound; l++) {
					if(k == l || places[l].size < 3) continue;

					// zone map is really useful!
					if(xy(places[l].x, places[l].y)->zone != c->zone) continue;

					m = metric(places[l].x+1,places[l].y+1,
					           places[k].x+2,places[k].y+2);
					if(m > metric(0,0,12,12)) continue;
					if(m <= metric(0,0,6,6)) continue;
					goto hall_found;
				}
			}
			continue;
			hall_found:;

			hallPlace = &places[k];
			goldPlace = &places[l];
			goldPlace->used = 1;
			hallPlace->used = 1;

			for(k = 0; k < unitsUsed; k++) {
				u = &units[k];
				if(u->goldMine) {
					m = metric(goldPlace->x, goldPlace->y, u->cell->x, u->cell->y);
					if(m < metric(0,0,32,0)) goto skip_placement;
				} else if(u->baseCenter) {
					m = metric(hallPlace->x, hallPlace->y, u->cell->x, u->cell->y);
					if(m < metric(0,0,32,0)) goto skip_placement;
					if(hallPlace) {
						m = metric(hallPlace->x, hallPlace->y, u->cell->x, u->cell->y);
						if(m < metric(0,0,32,0)) goto skip_placement;
					}
				}
			}

			for(k = 1; k < numPlayers; k++)
				if(players[k].basesPlaced < players[k].numBases)
					break;


			if(goldMinesPlaced < goldMines) {
				u = addUnit("unit-gold-mine", 0, 3);
				u->resources = goldPerGoldMine;
				u->goldMine = 1;
				placeUnit(goldPlace->x, goldPlace->y, u);
				c = xy(goldPlace->x, goldPlace->y);
				c->zone->goldMines++;
				goldMinesPlaced++;
			}

			//printf("%d/%d, %p\n", k, numPlayers, hallPlace);			
			if(!hallPlace || k == numPlayers) {
				if(goldMinesPlaced >= goldMines && k == numPlayers) goto unit_place_end;
				continue;
			}

			players[k].x = hallPlace->x+1;
			players[k].y = hallPlace->y+1;
			players[k].basesPlaced++;

			race = players[k].race;
			u = addUnit(race==ORC ? "unit-great-hall":"unit-town-hall", k, 4);
			u->baseCenter = 1;
			placeUnit(hallPlace->x, hallPlace->y, u);

			for(l = 0; l < placesFound; l++) {
				if(places[l].used) continue;
				u = addUnit(race==ORC ? "unit-peon":"unit-peasant", k, 1);
				placeUnit(places[l].x, places[l].y, u);
				break;
			}

		skip_placement:;
		}
	}
unit_place_end:;

	/*for(i = 0; i < w*h; i++) map[i].visited = 0;

	

	for(i = 0; i < zonesUsed; i++) {
		if(zones[i].size < 400 || zones[i].type != ZONE_LAND) continue;
		if(zonePlace(&zones[i],units, 1, 32, 1)) {
			break;
		}
	}*/
}

static void terragen() {
	int i,j,x,y,d;
	char const **islandRules = (lcg()&1) ? islandRules1 : (lcg()&1) ? islandRules2 : islandRules3;

	assert(islandCount <= MAX_ISLANDS);


	// HACK to allow matrix to cross bottom and left borders
	realW = w;
	realH = h;
	w += MAT_DIM;
	h += MAT_DIM;

	land = land*w*h / 100;
	landCount = 0;

	trees = trees * land/100;
	rock = rock * land/100;
	coast = coast * (w*h - land)/100;

	//printf("trees will take %d cells of %d land cells\n", trees, land);
	//printf("rock will take %d cells of %d land cells\n", rock, land);
	//printf("coast will take %d cells of %d sea cells\n", rock, (w*h - land));


	//clearWorldTo(GRASS);
	//buildZoneWorld();
	//placeUnits();
	//return;

	if(islandCount > 3) {
		clearWorldTo(WATER);

		d = metric(0,0,w/2/islandCount,h/2/islandCount);

		for(i = 0; i < islandCount; i++) {
		again:
			x = lcg()%w;
			y = lcg()%h;
			for(j = 0; j < i; j++) {
				if(metric(x,y,islands[j].x,islands[j].y) < d)
					goto again;
			}
			xy(x,y)->tile = defaultTileOf(GRASS);
			islands[i].x = x;
			islands[i].y = y;
		}

		loadRewriteRules(islandRules);
		for(i = 0; terraCount[GRASS] < land && i < 50; i++) rewrite();
	} else {
		clearWorldTo(GRASS);
		islandCount = 1;
		islands[0].x = w/2;
		islands[0].y = h/2;
	}

	// this also fixes holes
	fixTiles();


	for(i = 0; i < rockEmitters; i++) {
	rock_again:
		x = (lcg()%w) * ((lcg()&1) ? 1 : -1);
		y = (lcg()%h) * ((lcg()&1) ? 1 : -1);
		Cell *c = xy(x, y);
		if(!c) goto rock_again;
		if(c->tile->type != GRASS) goto rock_again;
		c->tile = defaultTileOf(ROCK);
	}

	loadRewriteRules(rockRules);
	for(i = 0; terraCount[ROCK] < rock && i < 50; i++) rewrite();



	for(i = 0; i < islandCount; i++)
		xy(islands[i].x, islands[i].y)->tile = defaultTileOf(FOREST);

	for(; i < forestEmitters; i++) {
	forest_again:
		x = (lcg()%w) * ((lcg()&1) ? 1 : -1);
		y = (lcg()%h) * ((lcg()&1) ? 1 : -1);
		Cell *c = xy(x, y);
		if(!c) goto forest_again;
		if(c->tile->type != GRASS) goto forest_again;
		c->tile = defaultTileOf(FOREST);
	}

	loadRewriteRules(forestRules);
	for(i = 0; terraCount[FOREST] < trees && i < 50; i++) rewrite();


	fixTiles();
}


void generate_map(char const *filename, unsigned int seed,
		int race, int players, int gold, int oil,
		int landmass, int numLandEmitters) {
	seed_lcg(seed);

	w = 128;
	h = 128;

	tileset = tilesets[lcg()%tilesetsNum];
	//tileset = "scripts/tilesets/swamp.lua";
	//tileset = "scripts/tilesets/summer.lua";
	//tileset = "scripts/tilesets/grassland.lua";

	userRace = race == 0 ? HUMAN : ORC;
	numPlayers = 6;
	goldMines = 80;

	goldPerGoldMine = gold;
	oilPerPatch = oil;


	land = landmass;
	islandCount = numLandEmitters;

	deep = 25;
	water = 70;
	coast = 5;
	grass = 30;
	trees = 50;
	rock = 50;


	rockEmitters = 40;
	forestEmitters = 40;
	coastEmitters = 2;


	initTileset();
	terragen();
	buildZoneWorld(); // required for correct placement
	//printZones();;
	placeUnits();
	//mapPrint();
	save(filename);
}


















#if 0
static void printRect(int x, int y, int w, int h) {
	int i, j;
	for(i = y; i < y+h; i++) {
		for(j = x; j < x+w; j++) {
			//Tile *tile = xy(x,y);
			//TileType *type = &tileTypes[tile->type];
			printf("%d ", xyt(j,i,j,i) == GRASS ? 0 : 1);
		}
		printf("\n");
	}
}


static void txy2(int x, int y) {
	xy(x,y)->tile = defaultTileOf(WATER);
}

static void txy(int x, int y, int nw, int n, int ne,
                      int w , int c, int e ,
                      int sw, int s, int se) {
	if(nw) txy2(x+0,y+0);
	if(n)  txy2(x+1,y+0);
	if(ne) txy2(x+2,y+0);
	if(w)  txy2(x+0,y+1);
	if(c)  txy2(x+1,y+1);
	if(e)  txy2(x+2,y+1);
	if(sw) txy2(x+0,y+2);
	if(s)  txy2(x+1,y+2);
	if(se) txy2(x+2,y+2);
}

static void test() {
	/*txy(1,1,  1, 1, 1,
	          1, 1, 1,
	          1, 1, 1);

	//fixTile(1,1);
	//printf("%d\n", xyt(0,0,1,1));
	//printRect(0,0,16,16);*/

	//fixTiles();
}

#endif