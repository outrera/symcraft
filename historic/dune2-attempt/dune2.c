/*

TODO:
 * attack in guard range
 * auto-targeting missiles
 * Rest of graphics
 * Random Map Generator
 * optimise attributes in draw-loop
 * fix pathfinding to reverse direction on map borders
 * fix damage absortion
 * game ballance
 * camera that follows unit
 * fix transporter collisions
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_image.h>

typedef int8_t   s1;
typedef int16_t  s2;
typedef int32_t  s4;
typedef int64_t  s8;
typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef s1       bool;

#define false    0
#define true     1

/*****************************************************************
*
*                          UTILS
*
*****************************************************************/
u1 prng_seed = 0xd5;

u1 prng() {
  u1 a = prng_seed&0xb8;
  u1 x = 5;
  u1 y = 0;

  do {
    if(a&0x80) y++;
    a <<= 1;
  } while(--x);
  prng_seed = (prng_seed<<1) | (y&1);
  return prng_seed;
}

void prng_add(u1 v, u1 bits) {
  while(bits--) {
    prng_seed = (prng_seed << 1) | (v&1);
    v >>= 1;
  }
  if(!prng_seed) prng_seed = 0x73;
}


u1 min8(u1 a, u1 b) {
  return a < b ? a : b;
}


u1 sign8(s1 v) {
  if(v < 0) return -1;
  if(v > 0) return 1;
  return 0;
}

u1 o_add45(u1 o) {return (o+1)&0x7;}
u1 o_sub45(u1 o) {return (o-1)&0x7;}

u1 o_add(char o, char v) {
  return (o+v)&0x7;
}

char o_diff(u1 a, u1 b) {
  char d = a - b;
  if(abs(d) > 4) d += d > 0 ? -8 : 8;
  return d;
}

u1 sqrt8tbl[255] = {
 0,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3, 
 4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5, 
 5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6, 
 6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, 
 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, 
 8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 
 9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 
10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 
11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 
12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 
12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 
13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 
14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

u1 sqrt8(u1 v) {
  return sqrt8tbl[v];
}

// 16-bit 2nd power of a 8-bit number
u2 sq16tbl[256] = {
    0,    1,    4,    9,   16,   25,   36,   49,
   64,   81,  100,  121,  144,  169,  196,  225,
  256,  289,  324,  361,  400,  441,  484,  529,
  576,  625,  676,  729,  784,  841,  900,  961,
 1024, 1089, 1156, 1225, 1296, 1369, 1444, 1521,
 1600, 1681, 1764, 1849, 1936, 2025, 2116, 2209,
 2304, 2401, 2500, 2601, 2704, 2809, 2916, 3025,
 3136, 3249, 3364, 3481, 3600, 3721, 3844, 3969,
 4096, 4225, 4356, 4489, 4624, 4761, 4900, 5041,
 5184, 5329, 5476, 5625, 5776, 5929, 6084, 6241,
 6400, 6561, 6724, 6889, 7056, 7225, 7396, 7569,
 7744, 7921, 8100, 8281, 8464, 8649, 8836, 9025,
 9216, 9409, 9604, 9801,10000,10201,10404,10609,
10816,11025,11236,11449,11664,11881,12100,12321,
12544,12769,12996,13225,13456,13689,13924,14161,
14400,14641,14884,15129,15376,15625,15876,16129,
16384,16641,16900,17161,17424,17689,17956,18225,
18496,18769,19044,19321,19600,19881,20164,20449,
20736,21025,21316,21609,21904,22201,22500,22801,
23104,23409,23716,24025,24336,24649,24964,25281,
25600,25921,26244,26569,26896,27225,27556,27889,
28224,28561,28900,29241,29584,29929,30276,30625,
30976,31329,31684,32041,32400,32761,33124,33489,
33856,34225,34596,34969,35344,35721,36100,36481,
36864,37249,37636,38025,38416,38809,39204,39601,
40000,40401,40804,41209,41616,42025,42436,42849,
43264,43681,44100,44521,44944,45369,45796,46225,
46656,47089,47524,47961,48400,48841,49284,49729,
50176,50625,51076,51529,51984,52441,52900,53361,
53824,54289,54756,55225,55696,56169,56644,57121,
57600,58081,58564,59049,59536,60025,60516,61009,
61504,62001,62500,63001,63504,64009,64516,65025
};

u2 sq16(u1 v) {
  return sq16tbl[v];
}

u1 abs8(char v) {
  return v < 0 ? -v : v;
}

u2 abs16(s2 v) {
  return v < 0 ? -v : v;
}

s2 max16(s2 a, s2 b) {
  return a > b ? a : b;
}

char sign16(s2 v) {
  if(v < 0) return -1;
  if(v > 0) return 1;
  return 0;
}


/*****************************************************************
*
*                          PPU
*
*****************************************************************/

u1 ppu_pal[64][3] = {
  {0x80,0x80,0x80}, {0x00,0x00,0xBB}, {0x37,0x00,0xBF}, {0x84,0x00,0xA6},
  {0xBB,0x00,0x6A}, {0xB7,0x00,0x1E}, {0xB3,0x00,0x00}, {0x91,0x26,0x00},
  {0x7B,0x2B,0x00}, {0x00,0x3E,0x00}, {0x00,0x48,0x0D}, {0x00,0x3C,0x22},
  {0x00,0x2F,0x66}, {0x00,0x00,0x00}, {0x05,0x05,0x05}, {0x05,0x05,0x05},

  {0xC8,0xC8,0xC8}, {0x00,0x59,0xFF}, {0x44,0x3C,0xFF}, {0xB7,0x33,0xCC},
  {0xFF,0x33,0xAA}, {0xFF,0x37,0x5E}, {0xFF,0x37,0x1A}, {0xD5,0x4B,0x00},
  {0xC4,0x62,0x00}, {0x3C,0x7B,0x00}, {0x1E,0x84,0x15}, {0x00,0x95,0x66},
  {0x00,0x84,0xC4}, {0x11,0x11,0x11}, {0x09,0x09,0x09}, {0x09,0x09,0x09},

  {0xFF,0xFF,0xFF}, {0x00,0x95,0xFF}, {0x6F,0x84,0xFF}, {0xD5,0x6F,0xFF},
  {0xFF,0x77,0xCC}, {0xFF,0x6F,0x99}, {0xFF,0x7B,0x59}, {0xFF,0x91,0x5F},
  {0xFF,0xA2,0x33}, {0xA6,0xBF,0x00}, {0x51,0xD9,0x6A}, {0x4D,0xD5,0xAE},
  {0x00,0xD9,0xFF}, {0x66,0x66,0x66}, {0x0D,0x0D,0x0D}, {0x0D,0x0D,0x0D},

  {0xFF,0xFF,0xFF}, {0x84,0xBF,0xFF}, {0xBB,0xBB,0xFF}, {0xD0,0xBB,0xFF},
  {0xFF,0xBF,0xEA}, {0xFF,0xBF,0xCC}, {0xFF,0xC4,0xB7}, {0xFF,0xCC,0xAE},
  {0xFF,0xD9,0xA2}, {0xCC,0xE1,0x99}, {0xAE,0xEE,0xB7}, {0xAA,0xF7,0xEE},
  {0xB3,0xEE,0xFF}, {0xDD,0xDD,0xDD}, {0x11,0x11,0x11}, {0x11,0x11,0x11}
};

#define PPU_SPRITE_LIMIT 64

// frame buffer dimensions
#define PPU_FB_WIDTH    256
#define PPU_FB_HEIGHT   240

// name table dimensions
#define PPU_NT_WIDTH    32
#define PPU_NT_HEIGHT   30

// memory map
#define PPU_PT          0x0000 /* pattern tables */
#define PPU_PT0         0x0000 /* pattern table 0 */
#define PPU_PT1         0x1000 /* pattern table 1 */
#define PPU_PT_SIZE     0x1000 /* size of pattern table */
#define PPU_NT          0x2000 /* name tables */
#define PPU_NT_SIZE     0x0400 /* size of name table */
#define PPU_CLUT        0x3F00 /* palettes */
#define PPU_CLUT_SIZE   0x0020

#define PPU_VFLIP       0x80
#define PPU_HFLIP       0x40
#define PPU_PRIO        0x20

typedef struct PPU {
  u1 ctrl1;
  u1 ctrl2;
  u1 status;
  u2 adr;
  u1 read_cache;
  u1 adr2005[2]; // $2005
  u1 adr2006[2]; // $2006
  u2 temp;
  u1 ram[16*1024];
  u1 spr_ram[256];
  u1 fb[PPU_FB_WIDTH*PPU_FB_HEIGHT*4];
} PPU;

PPU ppu;


void ppu_advance_temp() {
  if((ppu.temp&0x1f) == 31) {
    if(ppu.temp&(1<<10)) {
      ppu.temp &= ~0x1f;
      if(((ppu.temp>>5)&0xff) == 29) {
        ppu.temp &= ~(0x1f<<5);
        ppu.temp ^= 1<<11;
      } else if(((ppu.temp>>5)&0xff) == 31) {
        ppu.temp &= ~(0x1f<<5);
      } else {
        ppu.temp += 1<<5;
      }
    }
    ppu.temp ^= 1<<10;
  } else {
    ppu.temp++;
  }
}

void ppu_init() {
  ppu.ctrl1 = 0;
  ppu.ctrl2 = 0;
  ppu.status = 0;
  ppu.temp = 0;
  memset(ppu.ram, 0, 16*1024);
  memset(ppu.spr_ram, 0, 256);
}

u1 ppu_read_ctrl1() {return ppu.ctrl1;}
void ppu_write_ctrl1(u1 v) {ppu.ctrl1 = v;}
u1 ppu_read_ctrl2() {return ppu.ctrl2;}
void ppu_write_ctrl2(u1 v) {ppu.ctrl2 = v;}
u1 ppu_read_status() {return ppu.status;}

void ppu_write_spr_ram(u1 *sprites) {
  u1 i = 0;
  do {ppu.spr_ram[i] = sprites[i];} while(++i);
}

u1 ppu_read() {
  u1 r = ppu.read_cache;
  ppu.read_cache = ppu.ram[ppu.adr++];
  return r;
}

void ppu_set_adr(u2 adr) {
  ppu.adr = adr;
}

void ppu_write(u1 v) {
  u2 adr = ppu.adr++ & 0x3FFF;
  if(0x3F20 < adr && adr < 0x3FFF)
    adr &= 0xFF1F;

  // 0x2000:0x2EFF = 0x3000:0x3EFF
  // 0x4000:0xC000 = 0x0000:0x3FFF
  if(adr < 0x2000) {
    ppu.ram[adr] = v;
  } else {
    if(0x3F00 <= adr && adr <= 0x3F1F && !(adr&3)) { // clut mirrors
      if(adr != 0x3F00) return;
      ppu.ram[adr+0x00] = v;
      ppu.ram[adr+0x04] = v;
      ppu.ram[adr+0x08] = v;
      ppu.ram[adr+0x0C] = v;
      ppu.ram[adr+0x10] = v;
      ppu.ram[adr+0x14] = v;
      ppu.ram[adr+0x18] = v;
      ppu.ram[adr+0x1C] = v;
      return;
    }
    ppu.ram[adr+0] = v;
    if(adr <= 0x2EFF)
      ppu.ram[adr + 0x1000] = v;
    else if(0x3000 <= adr && adr <= 0x3EFF)
      ppu.ram[adr - 0x1000] = v;
  }
}


u1 ppu_get_color(u2 pt, u1 t, u1 x, u1 y) {
  u1 *p = ppu.ram + PPU_PT + pt + t*16;
  return ((p[  y]>>x)&1) |
         (((p[8+y]>>x)&1)<<1);
}

void ppu_set_color(u2 pt, u1 t, u1 x, u1 y, u1 c) {
  u1 *p = ppu.ram + PPU_PT + pt + t*16;
  p[  y] = (p[  y]&~(1<<x)) | ((c&1)<<x);
  p[8+y] = (p[8+y]&~(1<<x)) | (((c&2)>>1)<<x);
}

//0 1 | 4 5
//2 3 | 6 7
//---------
//8 9 | C D
//A B | E F
#define ATTRIB_INDEX ((y/4)*8 + (x/4))
#define ATTRIB_SHIFT (((y&3)/2)*4 + ((x&3)/2)*2)

u1 ppu_get_attrib(u1 x, u1 y) {
  u1 *p = ppu.ram + PPU_NT + 32*30;
  u1 i = ATTRIB_INDEX;
  u1 s = ATTRIB_SHIFT;
  return (p[i] >> s) & 3;
}

void ppu_set_attrib(u1 x, u1 y, u1 c) {
  u1 *p = ppu.ram + PPU_NT + 32*30;
  u1 i = ATTRIB_INDEX;
  u1 s = ATTRIB_SHIFT;
  //printf("x=%d,y=%d: i=%d,s=%d\n", x,y,i,s);
  p[i] = (p[i] & ~(3<<s)) | (c << s);
}

void ppu_render_color(int x, int y, u1 c, bool fg) {
  u1 *p = ppu.fb + (y*PPU_FB_WIDTH + x)*4;
  c = ppu.ram[PPU_CLUT + (fg*16) + c]&0x3f;
  p[0] = ppu_pal[c][0];
  p[1] = ppu_pal[c][1];
  p[2] = ppu_pal[c][2];
  p[3] = 0x00;
}

void ppu_render_bg() {
  u1 x, y, fx=0, fy=0; // x, y, fine x, fine y
  for(y = 0; y < PPU_NT_HEIGHT; ) {
    for(x = 0; x < PPU_NT_WIDTH; ) {
      u1 t = ppu.ram[PPU_NT + y*PPU_NT_WIDTH + x];
      u1 c = (ppu_get_attrib(x,y)<<2) |
             ppu_get_color(PPU_PT0,t,fx,fy);
      ppu_render_color(((int)x<<3)|fx, ((int)y<<3)|fy, c, false);
      x += (++fx & (1<<3)) >> 3;
      fx &= 0x7;
    }
    y += (++fy & (1<<3)) >> 3;
    fy &= 0x7;
  }
}

void ppu_render_sprite_line(u1 t, u1 a, u1 x, u1 y, u1 sy,
                            u1 vflip, u1 hflip, u1 priority) {
  u1 i, sx, c;
  if(vflip) sy = 15-sy;
  a <<= 2;
  t += (sy&0x8)>>3;
  sy &= 0x7;
  for(i = 0; i < 8; i++) {
    sx = hflip ? 7-i : i;
    c = ppu_get_color(PPU_PT1,t,sx,sy);
    if(c && x+i < PPU_FB_WIDTH) ppu_render_color(x+i, y, a|c, true);
  }
}

void ppu_render_fg() {
  u1 s, c, y;
  for(y = 0; y < 240; y++) {
    s = 0;
    c = 0;
    do {
      u1 *p = ppu.spr_ram+s;
      if(p[0] <= y && y <= p[0]+15) {
        ppu_render_sprite_line(p[1], p[2]&3, p[3], y, y-p[0],
                               p[2]&PPU_VFLIP, p[2]&PPU_HFLIP, p[2]&PPU_PRIO);
        c++;
      }
      s += 4;
    } while(s && c != PPU_SPRITE_LIMIT);
  }
}

void ppu_render() {
  ppu_render_bg();
  ppu_render_fg();
}


/*****************************************************************
*
*                        SPRITE/TILE DECLARATIONS
*
*****************************************************************/

#define PAL_DUNE        0
#define PAL_SPICE       1
#define PAL_HIGHLIGHT   2
#define PAL_STRC        3


#define PAL_ALLIED      0
#define PAL_ENEMY       1
#define PAL_BULLET      3


// converts x,y to tile offset
#define XYT(x,y) ( (((y)&~8)<<1) + (((x)>>3)<<1) + (((y)>>3)&1) )

/* orientaions
 7 0 1
  \|/
6--*--2
  /|\
 5 4 3 
*/
#define O_000              0
#define O_045              1
#define O_090              2
#define O_135              3
#define O_180              4
#define O_225              5
#define O_270              6
#define O_315              7


enum {
  SPR_8x16,
  SPR_16x16,
  SPR_24x16,
  SPR_32x16,
  SPR_16x32,
  SPR_24x32,
  SPR_32x32
};


#define SPR_NORMAL         0x0
#define SPR_MIRROR         0x1
#define SPR_DIRECTED       0x2

#define SPR_MIRROR_H       0x10
#define SPR_MIRROR_V       0x20

// flags for sprite parts
#define SPR_RAND           0x10

#define SPR_ZEROED 0,0,0,0,0,0

enum {
  SPR_ROCKET,
  SPR_EXPL1,
  SPR_EXPL2,
  SPR_EXPL3,
  SPR_EXPL4,
  SPR_EXPL5,
  SPR_HARVESTER,
  SPR_TRANSPORTER,
  SPR_SCOUT,
  SPR_HEAVY_TANK,
  SPR_MISSILE_TANK,
  SPR_SONIC_TANK,
  SPR_TOTAL
};

u1 sprites[SPR_TOTAL][10] = {
  // rocket
  { SPR_DIRECTED,       // description (SPR_NORMAL/SPR_DIRECTED)
                        // and flags (like SPR_RANDOMIZE)
     0,112,SPR_8x16,    // vertical
     8,112,SPR_8x16,    // diagonal
    16,112,SPR_16x16},  // horizontal

  // explosion 1
  {SPR_NORMAL|SPR_RAND, 64,112, SPR_8x16, SPR_ZEROED},

  // explosion 2
  {SPR_NORMAL|SPR_RAND, 56,112, SPR_8x16, SPR_ZEROED},

  // explosion 3
  {SPR_MIRROR|SPR_RAND, 48,112, SPR_8x16, SPR_ZEROED},

  // explosion 4
  {SPR_MIRROR|SPR_RAND, 40,112, SPR_8x16, SPR_ZEROED},

  // explosion 5 (smoke)
  {SPR_MIRROR|SPR_RAND, 32,112, SPR_8x16, SPR_ZEROED},

  // harvester
  { SPR_DIRECTED,
     0,32,SPR_16x32,
    16,32,SPR_24x32,
    40,32,SPR_32x16},

  // transporter
  { SPR_DIRECTED,
     0,64,SPR_16x32|SPR_MIRROR_H,
    16,64,SPR_32x32,
    40,48,SPR_32x16|SPR_MIRROR_V},

  // scout vehicle
  { SPR_DIRECTED,
     72,112,SPR_16x16,
     88,112,SPR_16x16,
    104,112,SPR_16x16},

  // heavy tank
  { SPR_DIRECTED,
     0,0,SPR_24x32,
    24,0,SPR_24x32,
    48,0,SPR_24x32},

  // missile tank
  { SPR_DIRECTED,
    72, 0,SPR_16x32,
    88, 0,SPR_24x32,
    72,48,SPR_24x16},

  // sonic tank
  { SPR_DIRECTED,
    112, 0,SPR_16x32,
     96,32,SPR_24x32,
     72,32,SPR_24x16},
};


enum {
  TILE_ROCK,
  TILE_TEST1x1,
  TOTAL_TILES1x1
};

u1 tiles1x1[TOTAL_TILES1x1][4*4] = {
  // TILE_ROCK
  {XYT(96,0),XYT(104,0),XYT(96,0),XYT(104,0),
   XYT(96,8),XYT(104,8),XYT(96,8),XYT(104,8),
   XYT(96,0),XYT(104,0),XYT(96,0),XYT(104,0),
   XYT(96,8),XYT(104,8),XYT(96,8),XYT(104,8)},

  // TILE_TEST1x1
  {XYT( 0,48),XYT( 8,48),XYT(8,48),XYT(16,48),
   XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),
   XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),
   XYT( 0,64),XYT( 8,64),XYT(8,64),XYT(16,64)}
};

enum {
  TILE_TEST2x2,
  TILE_CYARD,
  TILE_REFINERY,
  TILE_FACTORY,
  TOTAL_TILES2x2
};



u1 tiles2x2[TOTAL_TILES2x2][8*8] = {
  // TILE_TEST2x2
  {XYT( 0,48),XYT( 8,48),XYT(8,48),XYT(16,48),XYT( 0,48),XYT( 8,48),XYT(8,48),XYT(16,48),
   XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),
   XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),
   XYT( 0,64),XYT( 8,64),XYT(8,64),XYT(16,64),XYT( 0,64),XYT( 8,64),XYT(8,64),XYT(16,64),
   XYT( 0,48),XYT( 8,48),XYT(8,48),XYT(16,48),XYT( 0,48),XYT( 8,48),XYT(8,48),XYT(16,48),
   XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),
   XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),XYT( 0,56),XYT( 8,56),XYT(8,56),XYT(16,56),
   XYT( 0,64),XYT( 8,64),XYT(8,64),XYT(16,64),XYT( 0,64),XYT( 8,64),XYT(8,64),XYT(16,64)},

  // TILE_CYARD
  {XYT(24,48),XYT(32,48),XYT(72,96),XYT(72,96),XYT(72,96),XYT(72,96),XYT(72,96),XYT(72,96),
   XYT(24,56),XYT(32,56),XYT(32,56),XYT( 0,72),XYT( 8,72),XYT(16,72),XYT(24,72),XYT(72,96),
   XYT(24,56),XYT(32,56),XYT(32,56),XYT( 0,80),XYT( 8,80),XYT(16,80),XYT(24,80),XYT(72,96),
   XYT(24,56),XYT(32,56),XYT(32,56),XYT( 0,88),XYT( 8,88),XYT(16,88),XYT(24,88),XYT(72,96),
   XYT(24,56),XYT(32,56),XYT(32,56),XYT( 0,96),XYT( 8,96),XYT(16,96),XYT(24,96),XYT(72,96),
   XYT(24,56),XYT(48,88),XYT(56,88),XYT(64,88),XYT(32,56),XYT(32,56),XYT(32,56),XYT(72,96),
   XYT(24,56),XYT(48,96),XYT(56,96),XYT(64,96),XYT(32,56),XYT(32,56),XYT(32,56),XYT(40,56),
   XYT(24,64),XYT(32,64),XYT(32,64),XYT(32,64),XYT(32,64),XYT(32,64),XYT(32,64),XYT(40,64)},

  // TILE_REFINERY
  {XYT(72,96),XYT(72,96),XYT(72, 96),XYT(72, 96),XYT(72, 96),XYT(72, 96),XYT(72, 96),XYT( 72,96),
   XYT(72,96),XYT(0,112),XYT( 8,112),XYT( 8,112),XYT( 8,112),XYT( 8,112),XYT(16,112),XYT( 72,96),
   XYT(72,96),XYT(0,120),XYT( 8,120),XYT( 8,120),XYT( 8,120),XYT( 8,120),XYT(16,120),XYT( 72,96),
   XYT(72,96),XYT(16,96),XYT(64, 48),XYT(72, 48),XYT(112, 0),XYT(120,16),XYT(120,16),XYT(120, 0),
   XYT(48,48),XYT(56,48),XYT(64, 56),XYT(72, 56),XYT(  8,56),XYT(120,16),XYT(120,16),XYT(  8,56),
   XYT(48,56),XYT(56,56),XYT(64, 48),XYT(72, 48),XYT(120,16),XYT(120,16),XYT(120,16),XYT(120,16),
   XYT(48,48),XYT(56,48),XYT(64, 56),XYT(72, 56),XYT(  8,56),XYT(120,16),XYT(120,16),XYT(  8,56),
   XYT(48,56),XYT(56,56),XYT(72, 72),XYT(72, 72),XYT(112, 8),XYT(120,16),XYT(120,16),XYT(120, 8)},

  // TILE_FACTORY
  {XYT( 0,48),XYT( 8,48),XYT( 8,48),XYT( 8,48),XYT(16,48),XYT(72, 72),XYT(72, 72),XYT(72,64),
   XYT( 0,56),XYT(80,72),XYT(88,72),XYT(96,72),XYT(16,56),XYT(88, 96),XYT(96, 96),XYT(72,88),
   XYT( 0,56),XYT(80,80),XYT(88,80),XYT(96,80),XYT(16,56),XYT(88,104),XYT(96,104),XYT(72,88),
   XYT( 0,56),XYT(80,88),XYT(88,88),XYT(96,88),XYT(16,56),XYT(32, 56),XYT(32, 56),XYT(32,56),
   XYT( 0,64),XYT( 8,64),XYT( 8,64),XYT( 8,64),XYT(16,64),XYT(32, 72),XYT(40, 72),XYT(32,56),
   XYT(72,96),XYT(32,56),XYT(32,72),XYT(40,72),XYT(32,56),XYT(32, 80),XYT(40, 80),XYT(72,88),
   XYT(72,96),XYT(32,56),XYT(32,80),XYT(40,80),XYT(32,56),XYT(32, 56),XYT(32, 56),XYT(72,88),
   XYT(72,96),XYT(72,96),XYT(72,96),XYT(72,72),XYT(72,72),XYT(72,72),XYT(72, 72),XYT(72,80)},

};


// terra tile declarator
#define TT(x0,y0,x1,y1,x2,y2,x3,y3) {                          \
  XYT(x0+0,y0+0),XYT(x0+8,y0+0),XYT(x1+0,y1+0),XYT(x1+8,y1+0), \
  XYT(x0+0,y0+8),XYT(x0+8,y0+8),XYT(x1+0,y1+8),XYT(x1+8,y1+8), \
  XYT(x2+0,y2+0),XYT(x2+8,y2+0),XYT(x3+0,y3+0),XYT(x3+8,y3+0), \
  XYT(x2+0,y2+8),XYT(x2+8,y2+8),XYT(x3+0,y3+8),XYT(x3+8,y3+8)} \

// cell transition combinations. we use 4-corners scheme.
#define CT(x,y) {                              \
  TT(x+ 0,y+ 0,x+ 0,y+ 0,x+ 0,y+ 0,x+ 0,y+ 0), \
  TT(x+ 0,y-16,x+ 0,y-16,x+ 0,y+ 0,x+ 0,y+ 0), \
  TT(x+ 0,y+ 0,x+16,y+ 0,x+ 0,y+ 0,x+16,y+ 0), \
  TT(x+ 0,y-16,x+16,y-16,x+ 0,y+ 0,x+16,y+ 0), \
  TT(x+ 0,y+ 0,x+ 0,y+ 0,x+ 0,y+16,x+ 0,y+16), \
  TT(x+ 0,y-16,x+ 0,y-16,x+ 0,y+16,x+ 0,y+16), \
  TT(x+ 0,y+ 0,x+16,y+ 0,x+ 0,y+16,x+16,y+16), \
  TT(x+ 0,y-16,x+16,y-16,x+ 0,y+16,x+16,y+16), \
  TT(x-16,y+ 0,x+ 0,y+ 0,x-16,y+ 0,x+ 0,y+ 0), \
  TT(x-16,y-16,x+ 0,y-16,x-16,y+ 0,x+ 0,y+ 0), \
  TT(x-16,y+ 0,x+16,y+ 0,x-16,y+ 0,x+16,y+ 0), \
  TT(x-16,y-16,x+16,y-16,x-16,y+ 0,x+16,y+ 0), \
  TT(x-16,y+ 0,x+ 0,y+ 0,x-16,y+16,x+ 0,y+16), \
  TT(x-16,y-16,x+ 0,y-16,x-16,y+16,x+ 0,y+16), \
  TT(x-16,y+ 0,x+16,y+ 0,x-16,y+16,x+16,y+16), \
  TT(x-16,y-16,x+16,y-16,x-16,y+16,x+16,y+16)} \

enum {
  CTC_SAND,
  CTC_SPICE,
  TOTAL_CTC
};

u1 ctc[TOTAL_CTC][16][4*4] = {
  CT(16,16), // CTC_SAND
  CT(64,16)  // CTC_SPICE
};


/*****************************************************************
*
*                        OBJECT LOGIC
*
*****************************************************************/


#define OBJ_LIM_HUMAN     32
#define OBJ_LIM_AI        48
#define MAX_OBJS          (OBJ_LIM_HUMAN+OBJ_LIM_AI)

struct {
  u2 credits;
  u2 power;
  u1 preq;     // prequisites cache
  u1 nunits;
  u1 nstrcs;
  u1 nharvs;
  u1 sbl[16];
  u1 sblsz;
  u1 ubl[16];
  u1 ublsz;
} players[2];


enum {
  TERRA_ROCK,
  TERRA_SAND,
  TERRA_SPICE,
  NONE,

  UNITS_START,
  UNIT_HARVESTER=UNITS_START,
  UNIT_TRANSPORTER,
  UNIT_SCOUT,
  UNIT_HEAVY_TANK,
  UNIT_MISSILE_TANK,
  UNIT_SONIC_TANK,
  UNITS_END,

  UNUSED,

  STRCS_START,
  STRC_TEST1x1=STRCS_START,
  STRC_TEST2x2,
  STRC_CYARD,
  STRC_REFINERY,
  STRC_FACTORY,
  STRCS_END,

  TERRA_TYPES=NONE,
  UNIT_TYPES =UNITS_END-UNITS_START,
  STRC_TYPES =STRCS_END-STRCS_START
};

#define HUMAN_OBJS       UNITS_START
#define HUMAN_OBJS_END   (HUMAN_OBJS+OBJ_LIM_HUMAN)
#define AI_OBJS          HUMAN_OBJS_END
#define AI_OBJS_END      (AI_OBJS+OBJ_LIM_AI)

#define TOTAL_OBJECTS    (TERRA_TYPES+MAX_OBJS)



#define MAX_BULLETS       16

enum {
  BUL_TANK,
  BUL_ROCKET,
  BUL_EXPL1,
  BUL_EXPL2,
  BUL_EXPL3,
  BUL_EXPL4,
  BUL_EXPL5,
  BUL_TOTAL,
  BUL_NONE=0xFF
};


struct {
  u1 damage;
  u1 speed;     // speed of projectile (pixels per frame)
  u1 sprite;
  //u1 nframes;
} btypes[BUL_TOTAL] = {
  {5,8,SPR_EXPL1},    // BUL_TANK
  {5,6,SPR_ROCKET},
  {0,0,SPR_EXPL1},
  {0,0,SPR_EXPL2},
  {0,0,SPR_EXPL3},
  {0,0,SPR_EXPL4},
  {0,0,SPR_EXPL5},
};

struct {
  u1 d;     // type descriptor
  s2 x, y;  // fine coords
  s2 tx, ty; 
} bullets[MAX_BULLETS];



// types of terrain
struct {
  u1 passable;
  u1 pal;      // palette
  u1 trans;    // terrain, with which it may be combined
  u1 tile;
} terra[TERRA_TYPES] = {
  {1, PAL_DUNE ,TERRA_ROCK,CTC_SAND},    // sand
  {1, PAL_DUNE ,TERRA_ROCK,TILE_ROCK},   // rock
  {1, PAL_SPICE,TERRA_SAND,CTC_SPICE},   // spice
};



#define PREQ_NONE        0x00
#define PREQ_NA          0x01
#define PREQ_PPLANT      0x02
#define PREQ_REFINERY    0x04
#define PREQ_OUTPOST     0x08
#define PREQ_FACTORY     0x10
#define PREQ_HITECH      0x40
#define PREQ_BARACKS     0x80


enum {
  GOAL_BUILD,
  GOAL_GUARD,
  GOAL_MOVE,
  GOAL_FOLLOW,
  GOAL_ATTACK,
  GOAL_HARVEST,
  GOAL_HARVEST2,
  GOAL_UNLOAD,
  GOAL_CARGO,
  GOAL_PICKUP,
  GOAL_FLY,
  GOAL_RAMPANT
};


#define HARV_CAPACITY  8


struct {
  u2 shield;
  u1 weapon;
  u1 range;      // fire range
  u1 reload;     // reload time
  u1 speed;
  u2 cost;       // cost/time to build
  u1 sprite;
  u1 preq;
} utypes[UNIT_TYPES] = {
  {10, BUL_NONE,   0, 20, 8,20,SPR_HARVESTER,    PREQ_NONE},
  {10, BUL_NONE,   0, 20, 9,20,SPR_TRANSPORTER,  PREQ_NONE},
  {10, BUL_ROCKET, 4, 20, 8,20,SPR_SCOUT,        PREQ_NONE},
  {10, BUL_TANK,   4, 60, 8,20,SPR_HEAVY_TANK,   PREQ_NONE},
  {10, BUL_ROCKET, 4, 60, 8,20,SPR_MISSILE_TANK, PREQ_NONE},
  {10, BUL_ROCKET, 4, 60, 8,20,SPR_SONIC_TANK,   PREQ_NONE},
};


#define TILE_1x1    0
#define TILE_2x2    1
#define TILE_3x2    2
#define TILE_3x3    3


struct {
  u2 shield;
  u1 size;
  u2 cost;
  u1 tile;
  u1 preq; // which prequisites it requires
  u1 prov; // which prequisites it provides
} stypes[STRC_TYPES] = {
  {7, TILE_1x1, 20,TILE_TEST1x1 ,PREQ_NA       ,PREQ_NONE},
  {7, TILE_2x2, 20,TILE_TEST2x2 ,PREQ_NA       ,PREQ_NONE},
  {7, TILE_2x2, 20,TILE_CYARD   ,PREQ_NA       ,PREQ_NONE},
  {7, TILE_2x2, 20,TILE_REFINERY,PREQ_NONE     ,PREQ_REFINERY},
  {7, TILE_2x2, 20,TILE_FACTORY ,PREQ_REFINERY ,PREQ_NONE},
};


u1 obj[TOTAL_OBJECTS][16];

u1 get_type(u1 o) {return obj[o][0];}
void set_type(u1 o, u1 v) {obj[o][0] = v;}
u1 get_terra(u1 o) {return obj[o][1];}
void set_terra(u1 o, u1 v) {obj[o][1] = v;}
u1 get_owner(u1 o) {return o < (OBJ_LIM_HUMAN+TERRA_TYPES) ? 0 : 1;}
u1 get_x(u1 o) {return obj[o][2];}
void set_x(u1 o, u1 v) {obj[o][2] = v;}
u1 get_y(u1 o) {return obj[o][3];}
void set_y(u1 o, u1 v) {obj[o][3] = v;}
u2 get_shield(u1 o) {return *(u2*)(obj[o]+4);}
void set_shield(u1 o, u2 v) {*(u2*)(obj[o]+4) = v;}

u2 get_progress(u1 o) {return *(u2*)(obj[o]+6);}
void set_progress(u1 o, u2 v) {*(u2*)(obj[o]+6) = v;}
u1 get_build_state(u1 o) {return obj[o][8];}
void set_build_state(u1 o, u1 v) {obj[o][8] = v;}
u2 get_cost(u1 t) {
  if(t < STRCS_START) return utypes[t-UNITS_START].cost;
  return stypes[t-STRCS_START].cost;
}

u1 get_orientation(u1 o) {return obj[o][6]&0x7;}
void set_orientation(u1 o, u1 v) {obj[o][6] = (obj[o][6]&~0x7) | v;}
u1 get_obstacle(u1 o) {return obj[o][6]>>3;}
void set_obstacle(u1 o, u1 v) {obj[o][6] = (obj[o][6]&0x7) | (v<<3);}
u1 get_gx(u1 o) {return obj[o][8];}
void set_gx(u1 o, u1 v) {obj[o][8] = v;}
u1 get_gy(u1 o) {return obj[o][9];}
void set_gy(u1 o, u1 v) {obj[o][9] = v;}
u1 get_wait(u1 o) {return obj[o][10];}
void set_wait(u1 o, u1 v) {obj[o][10] = v;}
u1 get_fine_move(u1 o) {return obj[o][11]&0xf;}
void set_fine_move(u1 o, u1 v) {obj[o][11] = (obj[o][11]&~0xf)|v;}
u1 harv_get_credits(u1 o) {return obj[o][11]>>4;}
void harv_set_credits(u1 o, u1 v) {obj[o][11] = (obj[o][11]&0xf)|(v<<4);}

u1 get_x0(u1 o) {return obj[o][12];}
void set_x0(u1 o, u1 v) {obj[o][12] = v;}
u1 get_y0(u1 o) {return obj[o][13];}
void set_y0(u1 o, u1 v) {obj[o][13] = v;}
u1 get_x1(u1 o) {return obj[o][14];}
void set_x1(u1 o, u1 v) {obj[o][14] = v;}
u1 get_y1(u1 o) {return obj[o][15];}
void set_y1(u1 o, u1 v) {obj[o][15] = v;}
void clear_trace_flag(u1 o) {set_x0(o, 0xff);}
u1 is_trace(u1 o) {return get_x0(o) != 0xff;}

u1 get_goal(u1 o) {return obj[o][7];}
void set_goal(u1 o, u1 v) {
  clear_trace_flag(o);
  obj[o][7] = v;
}

void mark_inactive(u1 o) {set_x(o, 96);}
bool is_active(u1 o) {return get_x(o) != 96;}

u1 get_speed(u1 o) {return utypes[get_type(o)-UNITS_START].speed;}
u1 get_reload(u1 o) {return utypes[get_type(o)-UNITS_START].reload;}
u1 get_weapon(u1 t) {return utypes[t-UNITS_START].weapon;}
u1 get_range(u1 o) {return utypes[get_type(o)-UNITS_START].range;}
u1 get_sprite(u1 t) {return utypes[t-UNITS_START].sprite;}

void get_fine_xy(u1 u, s2 *fx, s2 *fy) {
  s2 f = -(get_fine_move(u)<<1);
  *fx = ((u2)get_x(u)<<5) + 16;
  *fy = ((u2)get_y(u)<<5) + 16;
  switch(get_orientation(u)) {
  case O_000:       ;*fy-=f;break;
  case O_045: *fx+=f;*fy-=f;break;
  case O_090: *fx+=f;       break;
  case O_135: *fx+=f;*fy+=f;break;
  case O_180:        *fy+=f;break;
  case O_225: *fx-=f;*fy+=f;break;
  case O_270: *fx-=f;       break;
  case O_315: *fx-=f;*fy-=f;break;
  }
}


u1 get_size(u1 t) {return stypes[t-STRCS_START].size;}
u1 get_prov(u1 t) {return stypes[t-STRCS_START].prov;}
u1 get_tile(u1 t) {return stypes[t-STRCS_START].tile;}


u1 is_occupied(o) {return get_type(o) >= TERRA_TYPES;}
u1 is_unit(u1 o) {return UNITS_START <= get_type(o) && get_type(o) < UNITS_END;}
u1 is_strc(u1 o) {return STRCS_START <= get_type(o);}
#define for_each_obj(i) for(i = TERRA_TYPES; i < TOTAL_OBJECTS; i++)
#define for_each_unit(i) for_each_obj(i) if(get_type(i) < UNUSED)
#define for_each_strc(i) for_each_obj(i) if(get_type(i) > UNUSED)

#define for_each_obj_of(p,i) \
  i = p ? AI_OBJS : HUMAN_OBJS; \
  for(; i < (p ? AI_OBJS_END : HUMAN_OBJS_END); i++)

#define for_each_unit_of(p,i) for_each_obj_of(p,i) if(get_type(i) < UNUSED)
#define for_each_strc_of(p,i) for_each_obj_of(p,i) if(get_type(i) > UNUSED)



void cancel_actions_involving_object(u1 o) {
  u1 i;
  for_each_unit(i) {
    switch(get_goal(i)) {
    case GOAL_ATTACK: case GOAL_FOLLOW:
      if(get_gx(i) == o) set_goal(i, GOAL_GUARD);
    }
  }
}

u1 get_free_slot_for_player(u1 player) {
  u1 i;

  for_each_obj_of(player, i)
    if(get_type(i) == UNUSED)
      return i;

  return NONE;
}

void collect_preqs_for(u1 p) {
  u1 i, preq = 0;
  for_each_strc_of(p,i)
    if(get_x(i) != 0xff) preq |= get_prov(get_type(i));
  players[p].preq = preq;

  players[p].ublsz = 0;
  for(i = 0; i < UNIT_TYPES; i++)
    if((utypes[i].preq&players[p].preq) == utypes[i].preq)
      players[p].ubl[players[p].ublsz++] = i+UNITS_START;

  players[p].sblsz = 0;
  for(i = 0; i < STRC_TYPES; i++)
    if((stypes[i].preq&players[p].preq) == stypes[i].preq) 
      players[p].sbl[players[p].sblsz++] = i+STRCS_START;
}


void order_move(u1 u, u1 x, u1 y) {
  set_goal(u,GOAL_MOVE);
  set_gx(u,x);
  set_gy(u,y);
}

void order_fly(u1 u, u1 x, u1 y) {
  set_goal(u,GOAL_FLY);
  set_gx(u,x);
  set_gy(u,y);
}


void order_harvest(u1 u, u1 x, u1 y) {
  set_goal(u,GOAL_HARVEST);
  set_gx(u,x);
  set_gy(u,y);
}

void order_attack(u1 u, u1 t) {
  if(get_weapon(get_type(u)) != BUL_NONE) {
    set_goal(u, GOAL_ATTACK);
    set_gx(u, t);
  }
}


void create_bullet(u1 d, u2 x, u2 y, u2 tx, u2 ty) {
  u1 i;

  for(i = 0; i < MAX_BULLETS; i++)
    if(bullets[i].d == BUL_NONE) {
      bullets[i].d = d;
      bullets[i].x = x;
      bullets[i].y = y;
      bullets[i].tx = tx;
      bullets[i].ty = ty;
      break;
    }
}

u1 create_unit(u1 t, u1 orientation, u1 side) {
  u1 s;

  s = get_free_slot_for_player(side);
  if(s == NONE) return NONE; // object limit reached

  set_type(s, t);
  set_orientation(s, orientation);
  set_gx(s,0);
  set_gy(s,0);
  set_goal(s,GOAL_BUILD);
  clear_trace_flag(s);
  set_wait(s,0);
  set_shield(s,utypes[t-UNITS_START].shield);

  players[side].nunits++;

  mark_inactive(s);

  if(t == UNIT_HARVESTER) {
    harv_set_credits(s,0);
    players[side].nharvs++;
  }

  return s;
}


u1 create_strc(u1 t, u1 side) {
  u1 i = get_free_slot_for_player(side);

  if(i == NONE) return NONE;

  set_type(i, t);
  set_shield(i, stypes[t-STRCS_START].shield);
  set_build_state(i, NONE);

  players[side].nstrcs++;
  mark_inactive(i);

  return i;
}

void map_remove_strc(u1 o);
void map_remove_unit(u1 o);

void destroy(u1 o) {
  cancel_actions_involving_object(o);
  if(is_strc(o)) {
    u1 t = get_build_state(o);
    if(t != NONE) set_type(t, UNUSED);
    if(is_active(o)) map_remove_strc(o);
    players[get_owner(o)].nstrcs--;
    collect_preqs_for(get_owner(o));
  } else {
    if(is_active(o)) map_remove_unit(o);
    if(get_type(o) == UNIT_HARVESTER) players[get_owner(o)].nharvs--;
    players[get_owner(o)].nunits--;
  }
  set_type(o,UNUSED);
}

void absord_damage(u1 o, u1 d) {
  if(get_shield(o) && get_shield(o) <= d) destroy(o);
  else set_shield(o, get_shield(o)-d);
}

void start_build(u1 s, u1 t) {
  set_build_state(s, t);
  set_progress(s, get_cost(t));
}

void cancel_build(u1 s) {
  u1 t = get_build_state(s);
  if(t != NONE) {
    players[get_owner(s)].credits += get_cost(get_type(t))-get_progress(s);
    set_type(t, UNUSED);
    set_build_state(s, NONE);
  }
}



/*****************************************************************
*
*                          MAP LOGIC
*
*****************************************************************/
#define MAP_SIZE        32

u1 map[MAP_SIZE*MAP_SIZE];         // MAP_SIZE/BITS_PER_CELL*MAP_SIZE
u1 map_fow[MAP_SIZE*MAP_SIZE/8];   // state of map exploration
u1 map_occ[MAP_SIZE*MAP_SIZE/8];   // state of map occupation

u1 view_x, view_y;     // viewport coordinates
u1 cur_x, cur_y;       // cursor coordinates
u1 sel;                // selection

void screen_to_map(u1 *mx, u1 *my, u1 sx, u1 sy) {
  *mx = ((view_x&~1)+(sx>>3))>>2;
  *my = ((view_y&~1)+(sy>>3))>>2;
}

void map_init() {
  u1 i;
  cur_x = 126;
  cur_y = 120;
  view_x = 0;
  view_y = 0;
  sel = NONE;

  memset(map_fow, 0, MAP_SIZE*MAP_SIZE/8);
  memset(map_occ, 0, MAP_SIZE*MAP_SIZE/8);

  for(i = 0; i < TERRA_TYPES; i++) {
    set_type(i,i);
    set_terra(i,i);
  }

  set_type(NONE,NONE);

  for_each_obj(i) set_type(i, UNUSED);

  for(i = 0; i < MAX_BULLETS; i++)
    bullets[i].d = BUL_NONE;

  for(i = 0; i < 2; i++) {
    players[i].credits = 0;
    players[i].power = 0;
  }
}

u1 map_get(u1 x, u1 y) {
  if(x < MAP_SIZE && y < MAP_SIZE)
    return map[(u2)y*MAP_SIZE + x];

  return NONE;
}

void map_set(u1 x, u1 y, u1 t) {
  if(x < MAP_SIZE && y < MAP_SIZE)
    map[(u2)y*MAP_SIZE + x] = t;
}


u1 map_get_occ(u1 x, u1 y) {
  return (map_occ[y*(MAP_SIZE/8) + x/8]>>(x&7))&1;
}

void map_set_occ(u1 x, u1 y, u1 v) {
  u1 *p = map_occ + y*(MAP_SIZE/8) + x/8;
  if(v) *p |= 1<<(x&7);
  else *p &= ~(1<<(x&7));
}

u1 map_is_explored(u1 x, u1 y) {
  return 1;
}


u1 map_get_terra(u1 x, u1 y) {
  u1 t = get_terra(map_get(x, y));
  if(t < TERRA_TYPES) return t;
  return get_terra(t);
}


u1 is_passable(u1 x, u1 y) {
  u1 t = get_type(map_get(x,y));
  if(t == STRC_REFINERY) {
    t = map_get(x,y);
    if(x == get_x(t)+1 && y == get_y(t)+1)
      return 1;
    return 0;
  }
  return x < MAP_SIZE && y < MAP_SIZE && t < TERRA_TYPES;
}


void map_place_unit(u1 x, u1 y, u1 u) {
  set_terra(u, map_get(x,y));
  set_x(u,x);
  set_y(u,y);
  map_set(x, y, u);
  map_set_occ(x, y, 1);
}

#define UNIT_PLACE_RANGE 16
void map_place_unit_safe(u1 x, u1 y, u1 u) {
  u1 r, n, i;
  s1 dx, dy;

  if(is_passable(x,y)) {
    map_place_unit(x,y,u);
    return;
  }

  r = 0;
  n = 0;
  i = 0;
  x++;

  for(;;) {
    if(!i--) {
      i = r;
      if((n&1) && ++r == UNIT_PLACE_RANGE*2) return;
      switch(n++&3) {
      case 0: dx= 1;dy= 0;break;
      case 1: dx= 0;dy= 1;break;
      case 2: dx=-1;dy= 0;break;
      case 3: dx= 0;dy=-1;break;
      }
    }

    if(is_passable(x,y)) {
      map_place_unit(x,y,u);
      return;
    }
    x += dx;
    y += dy;
  }
}


void map_remove_unit(u1 u) {
  map_set(get_x(u), get_y(u), get_terra(u));
  map_set_occ(get_x(u), get_y(u), 0);
}

u1 map_create_unit(u1 x, u1 y, u1 t, u1 side) {
 u1 u = create_unit(t, O_000, side);
 map_place_unit(x, y, u);
 if(t == UNIT_TRANSPORTER) set_goal(u, GOAL_FLY);
 else set_goal(u, GOAL_GUARD);
 return u;
}

void map_unit_move(u1 nx, u1 ny, u1 u) {
  map_remove_unit(u);
  map_place_unit(nx,ny,u);
}

void map_place_strc(u1 x, u1 y, u1 s) {
  set_x(s, x);
  set_y(s, y);

  if(get_size(get_type(s)) == TILE_1x1) {
    map_set(x,y,s);
    map_set_occ(x,y,1);
  } else {
    map_set(x+1,y+0,s);
    map_set(x+0,y+1,s);
    map_set(x+1,y+1,s);
    map_set(x+0,y+0,s);
    map_set_occ(x+1,y  ,1);
    map_set_occ(x  ,y+1,1);
    map_set_occ(x+1,y+1,1);
    map_set_occ(x  ,y  ,1);
  }
}

void map_remove_strc(u1 s) {
  u1 x = get_x(s);
  u1 y = get_y(s);
  u1 t = TERRA_ROCK;
  if(get_size(get_type(s)) == TILE_1x1) {
    map_set(x,y,t);
    map_set_occ(x,y,0);
  } else {
    map_set(x  ,y  ,t);
    map_set(x+1,y  ,t);
    map_set(x  ,y+1,t);
    map_set(x+1,y+1,t);
    map_set_occ(x+1,y  ,0);
    map_set_occ(x  ,y+1,0);
    map_set_occ(x+1,y+1,0);
    map_set_occ(x  ,y  ,0);
  }
}

/*****************************************************************
*
*                          GAME LOGIC
*
*****************************************************************/
u1 anim_time;
u1 pad_state = 0;

enum {
  UI_NORMAL,
  UI_DEFEAT,
  UI_PAUSE,
  UI_BUILDING,
  UI_PLACEMENT,
};

u1 ui_state;
u1 placement;


// selected structure's build list
u1 *bl;
u1 blsz;
u1 blsel;


#define AI_DEFENCE              1 /* number of units ai should keep in reserve */
#define AI_ATTACK               0 /* number to units to send in attack */
#define AI_MAX_UNITS            (AI_DEFENCE+2*AI_ATTACK)
#define AI_MIN_HARVS            0

u1 find_nearest_enemy(u1 o) {
  u1 i, j;
  u2 r = 64*64*2;
  u1 best = NONE;
  s1 x = get_x(o);
  s1 y = get_y(o);
  for_each_strc_of(!get_owner(o), i) {
    j = sq16(abs8(get_x(i)-x)) + sq16(abs8(get_y(i)-y));
    if(j < r) {
      r = j;
      best = i;
    }
  }

  if(best != NONE) return best;

  for_each_unit_of(!get_owner(o), i) {
    j = sq16(abs8(get_x(i)-x)) + sq16(abs8(get_y(i)-y));
    if(j < r) {
      r = j;
      best = i;
    }
  }

  return best;
}

void update_ai() {
  u1 i, e, p;
  p = players[1].nunits-players[1].nharvs;
  if(p >= AI_ATTACK+AI_DEFENCE) {
    p -= AI_DEFENCE;
    e = NONE;
    for_each_unit_of(1, i) {
      if(get_weapon(get_type(i)) == BUL_NONE) continue;
      if(!is_active(i)) continue;

      if(!p--) break;

      if(e == NONE) {
        e = find_nearest_enemy(i);
        if(e == NONE) return; // player is probably defeated
      }

      if(get_goal(i) != GOAL_ATTACK) {
        order_attack(i,e);
      }
    }
    return;
  }
}

void ai_build(u1 s) {
  u1 i;

  if(get_type(s) != STRC_FACTORY &&
     get_type(s) != STRC_CYARD)
    return;

  if(get_type(s) == STRC_CYARD) return;

  if(players[1].nharvs < AI_MIN_HARVS) {
    start_build(s, UNIT_HARVESTER);
    return;
  }

  if((players[1].nunits - players[1].nharvs) >= AI_MAX_UNITS) return;

  for(i = 0; i < players[1].ublsz; i++)
    if(get_weapon(players[1].ubl[i]) != BUL_NONE && (prng()&1)) {
      start_build(s, players[1].ubl[i]);
      break;
    }
}



u1 scan_for_enemies(u1 u) {
  u1 x,y,b,t,
    ux = get_x(u),
    uy = get_y(u),
    r = get_range(u),
    sx = (ux - min8(ux,r))/8,
    sy = uy - min8(uy,r),
    ex = min8(ux + (r+7)/8*2, MAP_SIZE-1),
    ey = min8(uy + r*2, MAP_SIZE-1)
  ;

  for(y = sy; y < ey; y++) {
    for(x = sx; x < ex; x++) {
      u1 v = map_occ[y*(MAP_SIZE/8) + x];
      if(!v) continue;
      for(b = 0; b < 8; b++) {
        if(!(v&(1<<b))) continue;
        t = map_get(x*8 + b, y);
        //if(t != u) printf("%d,%d\n", get_x(t), get_y(t));
        if(get_type(t) < TERRA_TYPES || get_owner(u) == get_owner(t))
          continue;
        
        if(sq16(abs8(x*8+b - ux)) + sq16(abs8(y-uy)) <= sq16(r))
          return t;
      }
    }
  }
  return NONE;
}

// vetors for each orientation
s1 ovec[9][2] = {
  { 0,-1}, //   0
  { 1,-1}, //  45
  { 1, 0}, //  90
  { 1, 1}, // 135
  { 0, 1}, // 180
  {-1, 1}, // 225
  {-1, 0}, // 270
  {-1,-1}, // 315
  { 0, 0}  // origin (useful in some loops)
};

// orientation to direction vector
#define o_to_v(x,y,o) do{    \
  switch(o) {                \
  case O_000:    ;y--;break; \
  case O_045: x++;y--;break; \
  case O_090: x++;    break; \
  case O_135: x++;y++;break; \
  case O_180:     y++;break; \
  case O_225: x--;y++;break; \
  case O_270: x--;    break; \
  case O_315: x--;y--;break; \
  }                          \
}while(0)

// direction vector to orientation
u1 v_to_o(char dx, char dy) {
  u1 o;
  if(dx > 0) {
    if       (dy < 0) o = O_045;
    else if  (dy > 0) o = O_135;
    else              o = O_090;
  } else if(dx < 0) {
    if       (dy > 0) o = O_225;
    else if  (dy < 0) o = O_315;
    else              o = O_270;
  } else {
    if       (dy < 0) o = O_000;
    else if  (dy > 0) o = O_180;
    else              o = O_000;
  }
  return o;
}


// TODO: add angular turns for flying units
void update_move(u1 u, s1 dx, s1 dy) {
  s1 x, y, nx, ny, i, j, o, b;

  x = get_x(u);
  y = get_y(u);
  o = get_orientation(u);

  if(is_trace(u)) { // tracing obstacle?
    nx = get_x1(u);
    ny = get_y1(u);
    if(abs8(nx-x)<=1 && abs8(ny-y)<=1) {
      if(x == nx && y == ny) {
        // got around obstacle
        clear_trace_flag(u);
        goto normal_move;
      }
      goto commit_move;
    }

trace_begin:
    b = get_obstacle(u);
    for(i=0;;) {
      nx = x+ovec[b][0];
      ny = y+ovec[b][1];
      if(is_passable(nx,ny)) break;
      b = o_add45(b);
      if(++i == 8) { // we got circled?
        clear_trace_flag(u);
        //set_goal(u,GOAL_GUARD);
        return;
      }
    }
    if(!i) { // obstacle itself moved from our way?
      clear_trace_flag(u);
      goto normal_move;
    }

    i = v_to_o(nx-x, ny-y);
    if(i != o) goto turn;

    if(nx == get_x0(u) && ny == get_y0(u)) {
      // moving in circles; move x1,y1 one ring future.
      i = get_x1(u);
      j = get_y1(u);
      dx = sign8(x+dx-get_x0(u));
      dy = sign8(y+dy-get_y0(u));
      do {i += dx, j += dy;} while(is_passable(i,j));
      do {i += dx, j += dy;} while(!is_passable(i,j));
      set_x1(u,i);
      set_y1(u,j);
    }

    // keep hand on the obstacle
    b = (b-3)&7;
    while(is_passable(nx+ovec[b][0],ny+ovec[b][1])) b = o_add45(b);

    set_obstacle(u, b);
    goto commit_move2;
  }

normal_move:
  dx = sign8(dx);
  dy = sign8(dy);
  nx = x + dx;
  ny = y + dy;

  if(!is_passable(nx,ny)) {
    if(abs8(dx) == abs8(dy)) {
      if(is_passable(x+dx,y)) {
        ny -= dy;
        goto commit_move;
      }
      if(is_passable(x,y+dy)) {
        nx -= dx;
        goto commit_move;
      }
    }
    do {nx += dx, ny += dy;} while(!is_passable(nx,ny));
    set_x0(u,x);
    set_y0(u,y);
    set_x1(u,nx);
    set_y1(u,ny);
    set_obstacle(u, v_to_o(dx,dy));
    goto trace_begin;
  }


commit_move:
  i = v_to_o(nx-x, ny-y);

  if(o != i) {
turn:
    set_orientation(u,o_add(o,sign8(o_diff(i,o))));
  } else {
commit_move2:
    map_unit_move(nx, ny, u);
    set_fine_move(u, 0xf);
  }

  if(get_speed(u) <= 7) set_wait(u, 7-get_speed(u));
}

void update_fly(u1 u, s1 dx, s1 dy) {
  s1 x,y,o,i;
  x = get_x(u);
  y = get_y(u);
  o = get_orientation(u);

  i = v_to_o(dx, dy);
  if(o != i) {
    if(abs8(o-i) < 2 || sq16(dx) + sq16(dy) > sq16(6)) {
      o = o_add(o,sign8(o_diff(i,o)));
      set_orientation(u,o);
    }
  }
  dx = ovec[o][0];
  dy = ovec[o][1];

  map_unit_move(x+dx, y+dy, u);
  set_fine_move(u, 0xf);
  if(get_speed(u) <= 7) set_wait(u, 7-get_speed(u));
}


u1 get_closest_refinery(u1 u) {
  u1 i;
  s1 ux = get_x(u);
  s1 uy = get_y(u);
  u1 r = NONE;
  for_each_obj_of(get_owner(u), i)
    if(get_type(i) == STRC_REFINERY)
      if(r == NONE ||
         abs8(get_x(r)-ux+get_y(r)-uy) > abs8(get_x(i)-ux+get_y(i)-uy))
        r = i;

  return r;
}

void fire_at(u1 u, u1 t) {
  s2 ftx, fty;
  u1 o, no;
  u1 x = get_x(u);
  u1 y = get_y(u);
  u1 tx = get_x(t);
  u1 ty = get_y(t);
  s1 dx = tx - x;
  s1 dy = ty - y;

  o = get_orientation(u);
  no = v_to_o(dx, dy);
  if(o != no) {
    set_orientation(u, o_add(o,sign8(o_diff(no,o))));
    if(get_speed(u) <= 7) set_wait(u, 7-get_speed(u));
    return;
  }

  if(is_strc(t)) {
    ftx = ((u2)tx<<5) + 16;
    fty = ((u2)ty<<5) + 16;
  } else {
    get_fine_xy(t, &ftx, &fty);
  }

  create_bullet(get_weapon(get_type(u)),(u2)x*32+16,(u2)y*32+16,ftx,fty);
  set_wait(u, get_reload(u));
}

void update_objects() {
  u1 u, t, o;
  s1 x, y, tx, ty, dx, dy;
  u2 c;
  for_each_obj(u) {
    if(get_type(u) == UNUSED) continue;

    if(is_strc(u)) {
      if(!is_active(u)) continue;

      o = get_owner(u);
      t = get_build_state(u);
      if(t == NONE) {
        if(o == 1) ai_build(u);
        continue;
      }
      c = get_progress(u);

      if(c) {
        if(!players[o].credits) continue;
        players[o].credits--;
        set_progress(u, c-1);
        continue;
      }

      if(t >= STRCS_START) {
        continue;
      }

      set_build_state(u, NONE);

      t = create_unit(t, prng()&0x7, o);
      if(t == NONE) continue; // object limit reached

      map_place_unit_safe(get_x(u)+1, get_y(u)+1, t);
      if(get_type(t) == UNIT_TRANSPORTER) set_goal(t, GOAL_FLY);
      else set_goal(t, GOAL_GUARD);
      continue;
    }

    if(get_wait(u)) {
      set_wait(u, get_wait(u)-1);
      continue;
    }

    t = get_fine_move(u);
    if(t) { // in process of moving between cells?
      if(get_speed(u) <= 7) {
        set_wait(u, 7-get_speed(u));
        t -= 1;
      } else {
        t -= min8(t, get_speed(u)-7);
      }
      set_fine_move(u, t);
      continue;
    }

    x = get_x(u);
    y = get_y(u);

    switch(get_goal(u)) {
    case GOAL_BUILD:
      break;

    case GOAL_GUARD:
      t = scan_for_enemies(u);
      if(t == NONE) continue;
      fire_at(u, t);
      break;

    case GOAL_CARGO:
      if(get_type(u) != UNIT_TRANSPORTER) break;
      break;

    case GOAL_PICKUP:
      break;

    case GOAL_FLY:
      tx = get_gx(u);
      ty = get_gy(u);

      if(x == tx && y == ty) {
        do {
          dx = prng()&7;
          dy = prng()&7;
          if(prng()&1) dx = -dx;
          if(prng()&1) dy = -dy;
          tx = x + dx;
          ty = y + dy;
          if(tx < 0) tx = 0;
          else if(tx >= MAP_SIZE) tx = MAP_SIZE-1;
          if(ty < 0) ty = 0;
          else if(ty >= MAP_SIZE) ty = MAP_SIZE-1;
        } while(x == tx && y == ty);

        set_gx(u, tx);
        set_gy(u, ty);
      } else {
        dx = tx - x;
        dy = ty - y;
      }
      update_fly(u,dx,dy);
      break;

    case GOAL_MOVE:
      tx = get_gx(u);
      ty = get_gy(u);
      dx = tx - x;
      dy = ty - y;
      if(dx || dy) update_move(u, dx, dy);
      else set_goal(u, GOAL_GUARD);
      break;

    case GOAL_ATTACK:
      t = get_gx(u);
      tx = get_x(t);
      ty = get_y(t);
      dx = tx - x;
      dy = ty - y;
      if(sq16(abs8(dx)) + sq16(abs8(dy)) > sq16(get_range(u))) {
        update_move(u, dx, dy);
        break;
      }
      fire_at(u, t);
      break;

    case GOAL_HARVEST:
      tx = get_gx(u);
      ty = get_gy(u);
      dx = tx - x;
      dy = ty - y;
      if(harv_get_credits(u) == HARV_CAPACITY) {
        set_goal(u,GOAL_UNLOAD);
        break;
      }
      if(dx || dy) {
        update_move(u, dx, dy);
        break;
      }
      // one spice cell equals to 100 credits
      if(get_terra(u) == TERRA_SPICE) {
        set_goal(u, GOAL_HARVEST2);
        set_wait(u, get_reload(u));
        break;
      } else {
        #define HARV_SCAN_RANGE 6
        u1 r = 0;
        s1 dx, dy;
        u1 n = 0;
        u1 i = 0;

        for(;;) {
          if(!i--) {
            i = r;
            if(n&1) {
              if(++r == HARV_SCAN_RANGE*2) {
                // nothing to harvest around
                set_goal(u, harv_get_credits(u) ? GOAL_UNLOAD : GOAL_GUARD);
                break;
              }
            }
            switch(n++&3) {
            case 0: dx= 1;dy= 0;break;
            case 1: dx= 0;dy= 1;break;
            case 2: dx=-1;dy= 0;break;
            case 3: dx= 0;dy=-1;break;
            }
          }

          if(map_get(x,y) == TERRA_SPICE) {
            set_gx(u,x);
            set_gy(u,y);
            break;
          }
          x += dx;
          y += dy;
        }
      }
      break;

    case GOAL_HARVEST2:
      harv_set_credits(u, harv_get_credits(u)+1);
      set_terra(u, TERRA_SAND);
      set_goal(u, GOAL_HARVEST);
      break;

    case GOAL_UNLOAD:
      t = get_closest_refinery(u);
      if(t == NONE) { // player dont have any refinery
        set_goal(u, GOAL_GUARD);
        break;
      }
      dx = get_x(t)+1 - x;
      dy = get_y(t)+1 - y;
      if(dx || dy) update_move(u, dx, dy);
      else {
        players[get_owner(u)].credits += 100;
        harv_set_credits(u, harv_get_credits(u)-1);
        if(!harv_get_credits(u)) set_goal(u,GOAL_HARVEST);
        set_wait(u, get_reload(u)>>2);
      }
      break;

    default:
      break;
    }
  }
}


void bullet_emit_damage(s2 ex, s2 ey, u1 damage) {
  s2 ux, uy, d;
  u1 cx = ex >> 5;
  u1 cy = ey >> 5;
  u1 i;
  if(!damage) return;
  for(i = 0; i < 9; i++) {
    u1 c = map_get(cx+ovec[i][0],cy+ovec[i][1]);
    if(!is_occupied(c)) continue;
    if(is_strc(c)) {
      ux = ((s2)(cx+ovec[i][0])<<5) + 16;
      uy = ((s2)(cy+ovec[i][1])<<5) + 16;
      d = max16(abs16(ux-ex),abs16(uy-ey));
      if(d >= 16) continue;
      absord_damage(c, damage);
      continue;
    }
    get_fine_xy(c, &ux, &uy);
    d = max16(abs16(ux-ex),abs16(uy-ey));
    if(d >= 16) continue;
    absord_damage(c, d < damage ? damage-d : 1);
  }
}

void bullet_detonate(u1 i) {
  u1 d = bullets[i].d;
  if(BUL_EXPL1 <= d && d < BUL_EXPL5) {
    bullets[i].d = d+1;
    return;
  }

  bullet_emit_damage(bullets[i].x, bullets[i].y, btypes[d].damage);
  if(d == BUL_TANK || d == BUL_ROCKET) {
    bullets[i].d = BUL_EXPL1;
  } else {
    bullets[i].d = BUL_NONE;
  }
}

s2 min16(s2 a, s2 b) {
  return a < b ? a : b;
}

void update_bullets() {
  u1 i;
  s2 speed;
  s2 x,y,tx,ty;
  s2 dx, dy;
  s2 incx, incy;
  s2 balance;

  for(i = 0; i < MAX_BULLETS; i++) {
    if(bullets[i].d == BUL_NONE) continue;

    x = bullets[i].x;
    y = bullets[i].y;
    tx = bullets[i].tx;
    ty = bullets[i].ty;

    dx = tx - x;
    dy = ty - y;

    if(tx >= x) {
      dx = tx - x;
      incx = 1;
    } else {
      dx = x - tx;
      incx = -1;
    }

    if(ty >= y) {
      dy = ty - y;
      incy = 1;
    } else {
      dy = y - ty;
      incy = -1;
    }

    speed = btypes[bullets[i].d].speed;

    if(!dx && !dy /*dx <= speed && dy <= speed*/) {
       bullet_detonate(i);
      continue;
    }

    if(dx >= dy) {
      speed = min16(speed, dx);
      dy <<= 1;
      balance = dy - dx;
      dx <<= 1;

      while(speed-- /*x != tx*/) {
        if(balance >= 0) {
          y += incy;
          balance -= dx;
        }
        balance += dy;
        x += incx;
      }
    } else {
      speed = min16(speed, dy);
      dx <<= 1;
      balance = dx - dy;
      dy <<= 1;

      while(speed-- /*y != ty*/) {
        if(balance >= 0) {
          x += incx;
          balance -= dy;
        }
        balance += dx;
        y += incy;
      }
    }
    bullets[i].x = x;
    bullets[i].y = y;
  }
}

// cursor precission
#define CUR_PREC 8

// our view follows cursor
void update_view() {
  if(cur_x > 128+63 && view_x < ((MAP_SIZE-8)<<2)) {
    view_x+=1;
    cur_x -= CUR_PREC;  
  } else if(cur_x < 128-64 && view_x > 0) {
    view_x-=1;
    cur_x += CUR_PREC;
  }

  if(cur_y > 120+59 && view_y < ((MAP_SIZE-8)<<2)) {
    view_y+=1;
    cur_y -= CUR_PREC;  
  } else if(cur_y < 120-60 && view_y > 0) {
    view_y-=1;
    cur_y += CUR_PREC;
  }
}


u1 sel_pal[16] = {
  0x0C, 0x1C, 0x2C, 0x3C,
  0x1C, 0x2C, 0x3C, 0x0C,
  0x2C, 0x3C, 0x0C, 0x1C,
  0x3C, 0x0C, 0x1C, 0x2C
};


// we use palette animation for little special effects
void update_palette() {
  u1 i = ((anim_time>>2)&3) << 2;
  ppu_set_adr(PPU_CLUT+0x10+8+1);
  ppu_write(sel_pal[i+1]);
  ppu_write(sel_pal[i+2]);
  ppu_write(sel_pal[i+3]);
  anim_time++;
}


#define PAD_RIGHT   0x01
#define PAD_LEFT    0x02
#define PAD_UP      0x04
#define PAD_DOWN    0x08
#define PAD_B       0x10
#define PAD_A       0x20
#define PAD_SELECT  0x40
#define PAD_START   0x80

void pad_up() {
  if(ui_state == UI_BUILDING) {
    if(get_build_state(sel) != NONE) return;
    if(!blsel) blsel = blsz - 1;
    else blsel--;
    pad_state&=~PAD_UP;
    return;
  }

  if(cur_y > CUR_PREC) cur_y -= CUR_PREC;
  else cur_y = 0; 
}

void pad_left() {
  if(ui_state == UI_BUILDING) {
    return;
  }

  if(cur_x > CUR_PREC) cur_x -= CUR_PREC;
  else cur_x = 0;
}

void pad_down() {
  if(ui_state == UI_BUILDING) {
    if(get_build_state(sel) != NONE) return;
    if(++blsel == blsz) blsel = 0;
    pad_state&=~PAD_DOWN;
    return;
  }

  if(cur_y < 240) cur_y += CUR_PREC;
}

void pad_right() {
  if(ui_state == UI_BUILDING) {
    return;
  }

  if(cur_x < 255-CUR_PREC) cur_x += CUR_PREC;
}


u1 get_sprite_owner_at(u1 x, u1 y);

void pad_b() {
  u1 i, j;

  if(ui_state == UI_BUILDING) {
    ui_state = UI_NORMAL;
    sel = NONE;
    return;
  }

  if(ui_state == UI_PLACEMENT) {
    // cancel placement
    ui_state = UI_BUILDING;
    set_type(placement, UNUSED);
    return;
  }

  sel = get_sprite_owner_at(cur_x, cur_y);
  if(sel != NONE) {
    /*if(get_owner(sel) != 0) {
      sel = NONE;
    }*/
    return;
  }

  screen_to_map(&i, &j, cur_x, cur_y);
  if(!is_strc(sel = map_get(i, j))) {
    sel = NONE;
    return;
  }

  ui_state = UI_BUILDING;

  if(get_type(sel) == STRC_FACTORY) {
    blsz = players[0].ublsz;
    bl = players[0].ubl;
  } else if(get_type(sel) == STRC_CYARD) {
    blsz = players[0].sblsz;
    bl = players[0].sbl;
  }

  j = get_build_state(sel);
  for(i = 0; i < blsz; i++)
    if(bl[i] == j) {
      blsel = i;
      return;
    }


  blsel = 0;
}

bool is_friendy_strc(u1 x, u1 y, u1 p) {
  u1 t = map_get(x, y);
  return get_type(t) >= STRC_TYPES && get_owner(t) == p;
}

bool can_place(u1 x, u1 y, u1 s) {
  u1 o = get_owner(s);

  if(get_size(get_type(s)) == TILE_1x1)
    return map_get(x,y) == TERRA_ROCK &&
           (is_friendy_strc(x+0, y-1, o) ||
            is_friendy_strc(x+1, y-1, o) ||
            is_friendy_strc(x+1, y+0, o) ||
            is_friendy_strc(x+1, y+1, o) ||
            is_friendy_strc(x+0, y+1, o) ||
            is_friendy_strc(x-1, y+1, o) ||
            is_friendy_strc(x-1, y+0, o) ||
            is_friendy_strc(x-1, y-1, o));

  return map_get(x  ,y  ) == TERRA_ROCK &&
         map_get(x+1,y  ) == TERRA_ROCK &&
         map_get(x  ,y+1) == TERRA_ROCK &&
         map_get(x+1,y+1) == TERRA_ROCK &&
         (is_friendy_strc(x+0, y-1, o) ||
          is_friendy_strc(x+1, y-1, o) || 
          is_friendy_strc(x+2, y-1, o) ||
          is_friendy_strc(x+2, y+0, o) ||
          is_friendy_strc(x+2, y+1, o) ||
          is_friendy_strc(x+2, y+2, o) ||
          is_friendy_strc(x+1, y+2, o) ||
          is_friendy_strc(x+0, y+2, o) ||
          is_friendy_strc(x-1, y+2, o) ||
          is_friendy_strc(x-1, y+1, o) ||
          is_friendy_strc(x-1, y+0, o) ||
          is_friendy_strc(x-1, y-1, o));
}

void pad_a() {
  u1 t, u, x, y;
  if(sel == NONE) return;

  if(ui_state == UI_PLACEMENT) {
    t = placement;
    screen_to_map(&x, &y, cur_x, cur_y);
    if(can_place(x,y,t)) {
      map_place_strc(x,y,t);
      if(get_type(t) == STRC_REFINERY) {
        u = create_unit(UNIT_HARVESTER, O_180, get_owner(t));
        if(u != NONE) map_place_unit(x+1, y+1, u);
      }

      collect_preqs_for(get_owner(t));

      set_build_state(sel, NONE);
      sel = NONE;
      ui_state = UI_NORMAL;
    }
    return;
  }

  if(ui_state == UI_BUILDING) {
    if(!blsz) return;
    t = get_build_state(sel);
    if(t == NONE) {
      start_build(sel, bl[blsel]);
      return;
    }

    if(t < STRCS_START) return;

    if(!get_progress(sel)) {
      placement = create_strc(t, get_owner(sel));
      if(placement == NONE) return; // handle object limit
      ui_state = UI_PLACEMENT;
    }
    return;
  }


  t = get_sprite_owner_at(cur_x, cur_y);
  if(t != NONE) {
    if(get_owner(sel) != get_owner(t))
      order_attack(sel, t);
    return;
  }

  x = (view_x>>2) + (cur_x>>5);
  y = (view_y>>2) + (cur_y>>5);
  t = map_get(x,y);

  if(!is_occupied(t)) {
    if(get_type(sel) == UNIT_HARVESTER && t == TERRA_SPICE) {
      order_harvest(sel,x,y);
      return;
    } else if(get_type(sel) == UNIT_TRANSPORTER) {
      order_fly(sel,x,y);
      return;
    }
    order_move(sel,x,y);
    return;
  }

  if(is_strc(t) && get_owner(sel) != get_owner(t))
    order_attack(sel, t);
}

void pad_start() {
}

void pad_select() {
  if(ui_state == UI_BUILDING) {
    cancel_build(sel);
    return;
  }
}




void handle_input() {
  // handle b/a/start/select, because may require
  // cursor data from previous update
  if(pad_state&PAD_B)      pad_b(), pad_state&=~PAD_B;
  if(pad_state&PAD_A)      pad_a(), pad_state&=~PAD_A;
  if(pad_state&PAD_START)  pad_start(), pad_state&=~PAD_START;
  if(pad_state&PAD_SELECT) pad_select(), pad_state&=~PAD_SELECT;

  if(pad_state&PAD_RIGHT)  pad_right();
  if(pad_state&PAD_LEFT)   pad_left();
  if(pad_state&PAD_UP)     pad_up();
  if(pad_state&PAD_DOWN)   pad_down();
}


u1 place_area[4];

void show_placement() {
  u1 x,y;
  u1 s = placement;
  screen_to_map(&x,&y,cur_x,cur_y);

  if(get_size(get_type(s)) == TILE_1x1) {
    place_area[0] = map_get(x,y);
  } else {
    place_area[0] = map_get(x  ,y  );
    place_area[1] = map_get(x+1,y  );
    place_area[2] = map_get(x  ,y+1);
    place_area[3] = map_get(x+1,y+1);
  }

  map_place_strc(x, y, s);
}

void hide_placement() {
  u1 x,y;
  u1 s = placement;
  screen_to_map(&x,&y,cur_x,cur_y);

  if(get_size(get_type(s)) == TILE_1x1) {
    place_area[0] = map_get(x,y);
  } else {
    map_set(x  ,y  , place_area[0]);
    map_set(x+1,y  , place_area[1]);
    map_set(x  ,y+1, place_area[2]);
    map_set(x+1,y+1, place_area[3]);
  }
  set_x(s, 0xff); // keep it out of calculations
}

void update_world() {
  handle_input();
  update_palette();
  update_view();

  switch(ui_state) {
  case UI_NORMAL:
    update_bullets();
    update_objects();
    update_ai();
    break;

  case UI_PAUSE:
    break;

  case UI_BUILDING:
    break;

  case UI_PLACEMENT:
    show_placement();
    break;
  }
}



/*****************************************************************
*
*                     VIEW DRAWING ROUTINES
*
*****************************************************************/

#define MAX_OBJECT_SPRITES 63

u1 cursor_sprite;
u1 spr[256];
u1 spr_cnt;
u1 spr_owners[MAX_OBJECT_SPRITES]; // note that we may pack this into 40 bytes

u1 get_sprite_owner(u1 s) {return spr_owners[s];}
void set_sprite_owner(u1 s, u1 o) {spr_owners[s] = o;}


u1 get_sprite_owner_at(u1 x, u1 y) {
  u1 i;
  u1 *p = spr;
  for(i = 0; i < 64; i++, p += 4) {
    if(p[3] <= x && x <= p[3]+7 &&
       p[0] <= y && y <= p[0]+15 &&
       i != cursor_sprite) {
      return get_sprite_owner(i);
    }
  }
  return NONE;
}



void draw_spr(u1 x, u1 y, u1 t, u1 a, u1 o) {
  u1 i;

  /*if(spr_cnt == MAX_OBJECT_SPRITES) {
    // consider overwriting random sprite instead of returning
    return;
  }*/

  i = spr_cnt<<2;
  spr[i] = y;
  spr[i+1] = t;
  spr[i+2] = a;
  spr[i+3] = x;
  set_sprite_owner(spr_cnt, o);
  spr_cnt=(spr_cnt+17)&0x3f;
}

u1 sprdim[][2] = {
  { 8, 16}, {16, 16}, {24, 16}, {32, 16},
  {16, 32}, {24, 32}, {32, 32}
};

void sprite_draw(u1 spr, u1 pal, u1 x, u1 y, u1 o, u1 owner) {
  u1 t;
  u1 a = pal;
  u1 sx, sy;
  u1 ix,cx,cy,i,j,w,h;
  s1 dx,dy;

  switch(sprites[spr][0]&0x7) {
  case SPR_NORMAL:
    if(sprites[spr][0]&SPR_RAND)
      a |= ((prng()&1)?PPU_VFLIP:0)|((prng()&1)?PPU_HFLIP:0);
    t = XYT(sprites[spr][1], sprites[spr][2]);
    draw_spr(x-4,y-8,t,a,owner);
    break;

  case SPR_MIRROR:
    if(sprites[spr][0]&SPR_RAND) a |= ((prng()&1)?PPU_VFLIP:0);
    t = XYT(sprites[spr][1], sprites[spr][2]);
    draw_spr(x-8,y-8,t,a,owner);
    if(sprites[spr][0]&SPR_RAND) a ^= ((prng()&1)?PPU_VFLIP:0);
    draw_spr(x  ,y-8,t,a|PPU_HFLIP,owner);
    break;

  case SPR_DIRECTED:
    switch(o) {
    case O_000: t=1;                       break;
    case O_045: t=4;                       break;
    case O_090: t=7;                       break;
    case O_135: t=4;a|=PPU_VFLIP;          break;
    case O_180: t=1;a|=PPU_VFLIP;          break;
    case O_225: t=4;a|=PPU_HFLIP|PPU_VFLIP;break;
    case O_270: t=7;a|=PPU_HFLIP;          break;
    case O_315: t=4;a|=PPU_HFLIP;          break;
    }

    i = sprites[spr][t+2]&0xf;
    w = sprdim[i][0];
    h = sprdim[i][1];


    switch(sprites[spr][t+2]&0xf0) {
    case SPR_MIRROR_V: x -= w/2; y -=   h; break;
    case SPR_MIRROR_H: x -=   w; y -= h/2; break;
    default:           x -= w/2; y -= h/2; break;
    }
    ix = x;
    cy = y;

    if(a&PPU_VFLIP) {
      dy = -16;
      cy += h-16;
      if(a&PPU_HFLIP) dx = -8, ix += w-8;
      else dx = 8;
    } else {
      dy = 16;
      if(a&PPU_HFLIP) dx = -8, ix += w-8;
      else dx = 8;
    }

    sy = sprites[spr][t+1];
    for(j = h/16; j; j--, cy+=dy, sy+=16) {
      sx = sprites[spr][t+0];
      for(i = w/8, cx = ix; i; i--, cx+=dx, sx+=8)
        draw_spr(cx,cy,XYT(sx,sy),a,owner);
    }

    ix = x;
    cy = y;
    switch(sprites[spr][t+2]&0xf0) {
    case SPR_MIRROR_V:         cy += h;a ^= PPU_VFLIP; break;
    case SPR_MIRROR_H: ix += w;        a ^= PPU_HFLIP; break;
    default: return;
    }

    if(a&PPU_VFLIP) {
      dy = -16;
      cy += h-16;
      if(a&PPU_HFLIP) dx = -8, ix += w-8;
      else dx = 8;
    } else {
      dy = 16;
      if(a&PPU_HFLIP) dx = -8, ix += w-8;
      else dx = 8;
     }

    sy = sprites[spr][t+1];
    for(j = h/16; j; j--, cy+=dy, sy+=16) {
      sx = sprites[spr][t+0];
      for(i = w/8, cx = ix; i; i--, cx+=dx, sx+=8)
        draw_spr(cx,cy,XYT(sx,sy),a,owner);
    }
    break;
  }
}


void draw_bullets() {
  u1 i;
  s2 bx, by, vx, vy;

  for(i = 0; i < MAX_BULLETS; i++) {
     if(bullets[i].d == BUL_NONE) continue;
     vx = ((u2)view_x<<3)&~0x0f;
     vy = ((u2)view_y<<3)&~0x0f;
     bx = bullets[i].x;
     by = bullets[i].y;
     if(vx <= bx && bx < vx + 8*32 &&
        vy <= by && by < vy + 7*32) {
       char dx = sign16(bullets[i].tx - bx);
       char dy = sign16(bullets[i].ty - by);
       sprite_draw(btypes[bullets[i].d].sprite, PAL_BULLET,
                   bx-vx, by-vy, v_to_o(dx, dy), NONE);
     }
  }
}

void draw_cursor() {
  // TODO: cursor shouldn't flicker, so we must kill 8th sprite on line
  cursor_sprite = spr_cnt;
  draw_spr(cur_x, cur_y, 254, PAL_ALLIED, NONE);
}

void draw_unit(u1 x, u1 y, u1 u) {
  u1 o = get_orientation(u);
  u1 m = get_fine_move(u)<<1;
  u1 pal = u == sel ? PAL_HIGHLIGHT : get_owner(u);

  x = x*8+16;
  y = y*8+16;

  switch(o) {
  case O_000:      y+=m;break;
  case O_045: x-=m;y+=m;break;
  case O_090: x-=m;     break;
  case O_135: x-=m;y-=m;break;
  case O_180:      y-=m;break;
  case O_225: x+=m;y-=m;break;
  case O_270: x+=m;     break;
  case O_315: x+=m;y+=m;break;
  }

  // cheap roughness-like animation for rock terrain
  // (prng() gives better results)
  if(m && !(prng()&3) && get_terra(u) == TERRA_ROCK &&
     get_type(u) != UNIT_TRANSPORTER /*&& !(anim_time&3)*/) {
    u1 r = (o+2)&3;
    x -= ovec[r][0];
    y -= ovec[r][1];
  }

  sprite_draw(get_sprite(get_type(u)), pal, x, y, o, u);
}

void set_cell_palette(u1 x, u1 y, u1 p) {
  x *= 2;
  y *= 2;
  ppu_set_attrib(x+0, y+0, p);
  ppu_set_attrib(x+0, y+1, p);
  ppu_set_attrib(x+1, y+0, p);
  ppu_set_attrib(x+1, y+1, p);
}

u1 sub1sat(u1 v) {return v ? v - 1 : v;}
u1 add1sat(u1 v) {return v < 63 ? v + 1 : v;}
u1 get_cell_transition(u1 x, u1 y, u1 to) {
  u1 r;
  r  = map_get_terra(sub1sat(x),        y )==to; r<<=1;
  r |= map_get_terra(        x ,add1sat(y))==to; r<<=1;
  r |= map_get_terra(add1sat(x),        y )==to; r<<=1;
  r |= map_get_terra(        x ,sub1sat(y))==to;
  return r;
}


void draw_bg() {
  u1 vx = view_x&~1;
  u1 vy = view_y&~1;
  u1 stx,sty;
  u1 x, y, cx, cy, map_x, map_y, s, t, *p;

  ppu_set_adr(PPU_NT);

  cy = vy&3;
  for(y = 0; y < 30; y++, cy++) {
    map_y = (vy+y)>>2;
    cx = vx&3;
    for(x = 0; x < 32; x++, cx++) {
      map_x = (vx+x)>>2;

      /*if(!map_is_explored(map_x, map_y)) {
        t = TERRA_ROCK;
        continue;
      }*/
      t = map_get(map_x, map_y);

again:
      if(!is_occupied(t)) {
        if(terra[t].trans == t) { // can only make transition to itself?
          p = tiles1x1[terra[t].tile];
        } else {
          u1 tran = get_cell_transition(map_x, map_y, terra[t].trans);
          p = ctc[terra[t].tile][tran];
        }
        set_cell_palette(x/2, y/2, terra[t].pal);
        ppu_write(p[(cy&3)*4 + (cx&3)]);
      } else if(is_strc(t)) {
        stx = (map_x-get_x(t))*4+(cx&3);
        sty = (map_y-get_y(t))*4+(cy&3);
        set_cell_palette(x/2, y/2, PAL_STRC);
        t = get_type(t);
        switch(get_size(t)) {
        case TILE_3x3:
          ppu_write(tiles1x1[get_tile(t)][(cy%12)*12 + (cx%12)]);
          break;

        case TILE_3x2:
          ppu_write(tiles1x1[get_tile(t)][(cy&7)*12 + (cx%12)]);
          break;

        case TILE_2x2:
          ppu_write(tiles2x2[get_tile(t)][sty*8 + stx]);
          break;

        case TILE_1x1:
          ppu_write(tiles1x1[get_tile(t)][(cy&3)*4 + (cx&3)]);
          break;
        }
      } else {
        s = get_terra(t);
        if(!(x&3) && !(y&3)) {
          if(is_unit(s)) {
            draw_unit(x-(vx&3), y-(vy&3), s);
            s = get_terra(s);
          }
          draw_unit(x-(vx&3), y-(vy&3), t);
        }
        t = s;
        goto again;
      }
    }
  }
}


void draw_strc_ui() {
  u1 t, x,y,sx,sy,ex,ey,rx,ry, bar_y, bar_e;
  ppu_set_adr(PPU_NT);

  t = get_build_state(sel);
  if(t == NONE) bar_y = 0xff;
  else {
    u2 cost = get_cost(get_type(t));
    u2 prog = cost - get_progress(sel);
    bar_e = (prog*20)/cost;
    bar_y = 8;
    bar_e += bar_e ? 6 : 5;
  }

  t = bl[blsel];
  if(t < STRCS_START || !blsz) {
    sy = 0xff;
  } else {
    sx = 10; ex = 22;
    sy = 10; ey = 22;
    switch(get_size(t)) {
    case TILE_1x1: ex -= 8; ey -= 8; break;
    case TILE_2x2: ex -= 4;
    case TILE_3x2: ey -= 4;
    }
  }

  for(y = 0; y < 30; y++) {
    for(x = 0; x < 32; x++) {
      if(y == bar_y && 5 <= x && x < 26) {
        if(x < bar_e) {
          if(x == 5) ppu_write(XYT(104,72));
          else if(x == 25) ppu_write(XYT(120,72));
          else ppu_write(XYT(112,72));
        } else {
          if(x == 5) ppu_write(XYT(104,64));
          else if(x == 25) ppu_write(XYT(120,64));
          else ppu_write(XYT(112,64));
        }
      } else if(sy <= y && y < ey && sx <= x && x < ex) {
        rx = x-sx;
        ry = y-sy;
        set_cell_palette(x/2, y/2, PAL_STRC);
        switch(get_size(t)) {
        case TILE_3x3:
          ppu_write(tiles1x1[get_tile(t)][ry*12 + rx]);
          break;

        case TILE_3x2:
          ppu_write(tiles1x1[get_tile(t)][ry*12 + rx]);
          break;

        case TILE_2x2:
          ppu_write(tiles2x2[get_tile(t)][ry*8 + rx]);
          break;

        case TILE_1x1:
          ppu_write(tiles1x1[get_tile(t)][ry*4 + rx]);
          break;
        }
      } else {
        ppu_write(XYT(8,56));
      }
    }
  }

  for(y = 0; y < 15; y++)
    for(x = 0; x < 16; x++)
      set_cell_palette(x, y, PAL_STRC);

  if(t < STRCS_START) {
    if(blsz) {
      sprite_draw(get_sprite(bl[blsel]),
                  PAL_BULLET, 128, 120, (anim_time>>2)&0x7, NONE);
    }
    return;
  }
}


u1 quot, rem;
void div10(u1 i) {
  // approximate the quotient as q = i*0.000110011 (binary)
  quot = ((i>>1) + i) >> 1; // times 0.11 
  quot = ((quot>>4) + quot) >> 3; // times 0.000110011

  // compute the remainder as r = i - 10*q
  rem = ((quot<<2) + quot) << 1; // times 1010.
  rem = i - rem;

  // fixup if approximate remainder out of bounds
  if(rem >= 10) {
    rem -= 10;
    quot++;
  }
}

#define SPR_DIGITS XYT(0,96)
void draw_credits() {
  // NOTE: only values up to around 48000 are handled correctly
  u2 n = players[0].credits;
  u1 t, d4, d3, d2, d1, d0;

  d0 = n       & 0xF;
  d1 = (n>>4)  & 0xF;
  d2 = (n>>8)  & 0xF;
  d3 = (n>>12) & 0xF;

  //div10(6*(u2)(d3+d2+d1) + d0);
  t = d3+d2+d1;
  div10(((t+t+t)<<1) + d0), d0 = rem;

  //div10(quot + 9*d3 + 5*d2 + d1);
  t = d3+d3+d3+d2;
  div10(quot + t+t+t + (d2<<1) + d1), d1 = rem;
  div10(quot + (d2<<1)), d2 = rem;
  div10(quot + (d3<<2)), d3 = rem;
  d4 = quot;

  t = 255 - 8*5 - 3;
  draw_spr(t   , 8, SPR_DIGITS + (d4<<1), PAL_ALLIED, NONE);
  draw_spr(t+ 8, 8, SPR_DIGITS + (d3<<1), PAL_ALLIED, NONE);
  draw_spr(t+16, 8, SPR_DIGITS + (d2<<1), PAL_ALLIED, NONE);
  draw_spr(t+24, 8, SPR_DIGITS + (d1<<1), PAL_ALLIED, NONE);
  draw_spr(t+32, 8, SPR_DIGITS + (d0<<1), PAL_ALLIED, NONE);
}


void clear_sprites() {
  u1 i;
  spr_cnt = prng()&0x3f;
  cursor_sprite = 64;
  for(i = 0; i < 64; i++) spr[i<<2] = 240;
}

void draw_view() {
  clear_sprites();

  switch(ui_state) {
  case UI_NORMAL:
    draw_credits();
    draw_bg();
    draw_bullets();
    draw_cursor();
    break;

  case UI_PAUSE:
    break;

  case UI_BUILDING:
    draw_strc_ui();
    draw_credits();
    break;

  case UI_PLACEMENT:
    draw_bg();
    draw_credits();
    draw_bullets();
    hide_placement();
    break;
  }

  ppu_write_spr_ram(spr);
}



/*****************************************************************
*
*                      INITIALIZATION ROUTINES
*
*****************************************************************/
u4 bg_file_pal[] = {
  0xAAAAAA, // default
  0x000000, // outline
  0x555555, // gray
  0xFFFFFF  // white
};

u4 fg_file_pal[] = {
  0x8080CC, // transparent
  0x000000, // outline
  0x888888, // gray
  0xFFFFFF  // white
};

u1 bg_pal[16] = {
  //0x18, 0x0D, 0x08, 0x38, // dune
  0x08, 0x0D, 0x18, 0x38, // dune
  0x18, 0x16, 0x28, 0x38, // spice
  0x0C, 0x1C, 0x2C, 0x3C, // unused (maybe for cliffs?)
  0x00, 0x0D, 0x00, 0x20, // buildings
};

u1 fg_pal[16]  = {
  //0x00, 0x0D, 0x00, 0x20,
  0x00, 0x0D, 0x03, 0x30, // color for friendly units
  0x00, 0x0B, 0x1B, 0x20, // color for enemy units
  0x00, 0x1C, 0x2C, 0x3C, // color for selected unit
  0x00, 0x06, 0x27, 0x30, // color for projectiles
};

void init_pals() {
  int i;
  ppu_set_adr(PPU_CLUT);
  for(i = 0; i < 16; i++) ppu_write(bg_pal[i]);
  for(i = 0; i < 16; i++) ppu_write(fg_pal[i]);
}

void set_corner_color(u1 t, u1 c) {
  int x,y;
  for(y = 0; y < 8; y++)
    for(x = 0; x < 8; x++)
      ppu_set_color(PPU_PT0,t,x,y,c);
}

void set_tile_color(u1 t, u1 c) {
  set_corner_color(t+0, c);
  set_corner_color(t+1, c);
  set_corner_color(t+2, c);
  set_corner_color(t+3, c);
}

void load_tile8x8(u2 pt, int dst, SDL_Surface *src, u4 *pal, int x, int y) {
  int i, tx, ty;
  for(ty = 0; ty < 8; ty++) {
    u1 *p = src->pixels + (y+ty)*src->pitch + x*src->format->BytesPerPixel;
    for(tx = 0; tx < 8; tx++) {
      u1 c = 0;
      for(i = 0; i < 4; i++)
        if((((u4)p[0]<<16)|((u4)p[1]<<8)|(u4)p[2]) == pal[i]) {
          c = i;
          break;
        }
      ppu_set_color(pt,dst,tx,ty, c);
      p += src->format->BytesPerPixel;
    }
  }
}

void load_tile16x16(u2 pt, int dst, SDL_Surface *src, u4 *pal, int x, int y) {
  dst *= 4;
  x *= 16;
  y *= 16;
  load_tile8x8(pt, dst++, src, pal, x  , y  );
  load_tile8x8(pt, dst++, src, pal, x  , y+8);
  load_tile8x8(pt, dst++, src, pal, x+8, y  );
  load_tile8x8(pt, dst++, src, pal, x+8, y+8);
}


// Tileset is 128x128 picture of 64 16x16 tiles total.
// Size 16x16 used because:
//   1. NES works best with 16x8 sprites
//   2. We tile map with 16x16 corners
bool load_tileset(u2 pt, char *name, u4 *pal) {
  int x,y,dst;
  SDL_Surface *img = IMG_Load(name);
  if(!img) {
    printf("Couldn't load tileset %s\n", name);
    return false;
  }

  dst = 0;
  for(y = 0; y < 8; y++)
    for(x = 0; x < 8; x++)
      load_tile16x16(pt, dst++, img, pal, x, y);

  SDL_FreeSurface(img);
  return true;
}

void load_empty_map() {
  u1 x, y;
  for(y = 0; y < MAP_SIZE; y++)
    for(x = 0; x < MAP_SIZE; x++)
      map_set(x, y, TERRA_SAND);

  map_init();
}

bool load_map(char *p) {
  FILE *f;
  u1 x,y;
  if(!(f = fopen(p,"r"))) {
    printf("Couldn't load %s\n", p);
    return false;
  }
  for(y = 0; y < 64; y++)
    for(x = 0; x < 64; x++) {
      u1 c;
      do { c = fgetc(f); } while(c == 0x0d || c == 0x0a);
           if(c == '^') c = TERRA_ROCK;
      else if(c == ')') c = TERRA_SAND;
      else if(c == '+') c = TERRA_SPICE;
      else              c = TERRA_SAND;
      map_set(x, y, c);
    }

  fclose(f);

  map_init();

  return true;
}

void print_map() {
  u1 x,y;
  for(y = 0; y < MAP_SIZE; y++) {
    for(x = 0; x < MAP_SIZE; x++)
      switch(get_terra(map_get(x, y))) {
      case TERRA_ROCK:  printf("^"); break;
      case TERRA_SAND:  printf(")"); break;
      case TERRA_SPICE: printf("+"); break;
      default: printf("U"); break;
      }
    printf("\n");
  }
}

void init_game() {
  ppu_init();
  load_tileset(PPU_PT0, "bg.png", bg_file_pal);
  load_tileset(PPU_PT1, "fg.png", fg_file_pal);
  init_pals();

  
  //load_empty_map();
  //load_map("test.map");
  load_map("175.map");

  players[0].credits = 8000;
  players[1].credits = 8000;

  players[0].nunits = 0;
  players[0].nstrcs = 0;
  players[0].nharvs = 0;
  players[1].nunits = 0;
  players[1].nstrcs = 0;
  players[1].nharvs = 0;

  sel = NONE;

  //view_y = 5 << 2;
  //map_place_strc(6,1, create_strc(STRC_TEST1x1, 1));
  //map_place_strc(1,1,create_strc(STRC_REFINERY, 0));
  //map_place_strc(0,0,create_strc(STRC_FACTORY, 0));
  map_place_strc(0,0,create_strc(STRC_CYARD, 0));

  map_place_strc(15,15,create_strc(STRC_CYARD, 1));
  map_place_strc(17,15,create_strc(STRC_FACTORY, 1));


  map_create_unit(2, 4, UNIT_SCOUT, 1);
  //map_create_unit(4, 4, UNIT_HEAVY_TANK, 0);
  //map_create_unit(0, 0, UNIT_TRANSPORTER, 0));


  collect_preqs_for(0);
  collect_preqs_for(1);
}


/*****************************************************************
*
*                          SDL
*
*****************************************************************/
char keys[(1<<16)-1];
SDL_Surface *screen;

bool double_screen = true;
bool interlace = false;
int input_timer = 0;

#define INTERLACE_BRIGHTNESS 0x30

void keydown(int key) {
  switch(key) {
  case 'w': pad_state |= PAD_UP; break;
  case 'a': pad_state |= PAD_LEFT; break;
  case 's': pad_state |= PAD_DOWN; break;
  case 'd': pad_state |= PAD_RIGHT; break;
  case 'k': pad_state |= PAD_B; break;
  case 'l': pad_state |= PAD_A; break;
  case 'i': pad_state |= PAD_START; break;
  case 'o': pad_state |= PAD_SELECT; break;
  default: break;
  }
}

void keyup(int key) {
  switch(key) {
  case 'w': pad_state &= ~PAD_UP; break;
  case 'a': pad_state &= ~PAD_LEFT; break;
  case 's': pad_state &= ~PAD_DOWN; break;
  case 'd': pad_state &= ~PAD_RIGHT; break;
  case 'k': pad_state &= ~PAD_B; break;
  case 'l': pad_state &= ~PAD_A; break;
  case 'i': pad_state &= ~PAD_START; break;
  case 'o': pad_state &= ~PAD_SELECT; break;
  default: break;
  }
}

void upload_fb() {
  int fb_x, fb_y;
  if(!double_screen) {
    for(fb_y = 0; fb_y < PPU_FB_HEIGHT; fb_y++) {
      u1 *s = ppu.fb + fb_y*PPU_FB_WIDTH*4;
      u1 *d = screen->pixels + fb_y*screen->pitch;
      for(fb_x = 0; fb_x < PPU_FB_WIDTH; fb_x++) {
        *d++ = s[2];
        *d++ = s[1];
        *d++ = s[0];
        *d++ = s[3];
        s+=4;      
      }
    }
    return;
  }

  for(fb_y = 0; fb_y < PPU_FB_HEIGHT; fb_y++) {
    u1 *s = ppu.fb + fb_y*PPU_FB_WIDTH*4;
    u1 *d = screen->pixels + fb_y*2*screen->pitch;
    for(fb_x = 0; fb_x < PPU_FB_WIDTH; fb_x++) {
      *d++ = s[2];
      *d++ = s[1];
      *d++ = s[0];
      *d++ = s[3];
      *d++ = s[2];
      *d++ = s[1];
      *d++ = s[0];
      *d++ = s[3];
      s+=4;
    }
  }

  if(interlace) {
    for(fb_y = 0; fb_y < PPU_FB_HEIGHT; fb_y++) {
      u1 *s = ppu.fb + fb_y*PPU_FB_WIDTH*4;
      u1 *d = screen->pixels + (fb_y*2+1)*screen->pitch;
      for(fb_x = 0; fb_x < PPU_FB_WIDTH; fb_x++) {
        *d++ = INTERLACE_BRIGHTNESS;
        *d++ = INTERLACE_BRIGHTNESS;
        *d++ = INTERLACE_BRIGHTNESS;
        *d++ = 0;
        *d++ = INTERLACE_BRIGHTNESS;
        *d++ = INTERLACE_BRIGHTNESS;
        *d++ = INTERLACE_BRIGHTNESS;
        *d++ = 0;
        s+=4;
      }
    }
    return;
  }

  for(fb_y = 0; fb_y < PPU_FB_HEIGHT; fb_y++) {
    u1 *s = ppu.fb + fb_y*PPU_FB_WIDTH*4;
    u1 *d = screen->pixels + (fb_y*2+1)*screen->pitch;
    for(fb_x = 0; fb_x < PPU_FB_WIDTH; fb_x++) {
      *d++ = s[2];
      *d++ = s[1];
      *d++ = s[0];
      *d++ = s[3];
      *d++ = s[2];
      *d++ = s[1];
      *d++ = s[0];
      *d++ = s[3];
      s+=4;
    }
  }
}


#define NES_FREQ    60
#define NES_WIDTH   256
#define NES_HEIGHT  240

#define FPS 30

void lock_surface(SDL_Surface *surface) {
  if(SDL_MUSTLOCK(surface))
    assert(SDL_LockSurface(surface) >= 0);
}

void unlock_surface(SDL_Surface *surface) {
  if(SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
}

int sdl_flags = SDL_ANYFORMAT
               |SDL_HWSURFACE
               //|SDL_ASYNCBLIT
               //|SDL_DOUBLEBUF
               //|SDL_FULLSCREEN
               //|SDL_RESIZABLE
               ;



int main(int argc, char **argv) {
  float tb, ta;
  int T0 = 0, done = 0, frames = 0;
  float fps = 0;

  assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) >= 0);

  init_game();

  if(double_screen) {
    assert(screen = SDL_SetVideoMode(NES_WIDTH*2, NES_HEIGHT*2, 32, sdl_flags));
  } else {
    assert(screen = SDL_SetVideoMode(NES_WIDTH, NES_HEIGHT, 32, sdl_flags));
  }

  //assert(screen = SDL_SetVideoMode(1280, 1024, 32, sdl_flags|SDL_FULLSCREEN));

  while(!done) { // main loop
    SDL_Event event;

    while(SDL_PollEvent(&event)) { // read system events
      switch(event.type) {
        //case SDL_VIDEORESIZE:
        //  resize(event.resize.w, event.resize.h);
        //  break;

        case SDL_QUIT:
          done = 1;
          break;

        case SDL_KEYDOWN:
          if(event.key.keysym.sym == SDLK_ESCAPE) done = 1;
          keydown(event.key.keysym.sym);
          break;

        case SDL_KEYUP:
          keyup(event.key.keysym.sym);
          break;
      }
    }

    tb = SDL_GetTicks()/1000.0;
    lock_surface(screen);
    update_world();
    draw_view();
    ppu_render();
    upload_fb();
    //print("FPS:%.0f\n", fps);
    unlock_surface(screen);
    ta = SDL_GetTicks()/1000.0;

    if(ta-tb < 1.0/FPS) {
      usleep((useconds_t)((1.0/FPS - (ta-tb))*1000000));
    }
    SDL_Flip(screen);

    frames++;
    int t = SDL_GetTicks();
    if(t - T0 >= 1000) {
        float seconds = (t - T0) / 1000.0;
        fps = frames / seconds;;
        T0 = t;
        frames = 0;
    }
  }

  SDL_Quit();
  return 0;
}
