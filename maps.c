#define MAX_MAPS 5
#define MAX_MONSTERS 40
#define MAX_TILES 32

#define MONSTER_NULL 0
#define MONSTER_HUMAN 1



struct MapTile {
    int exist;
    int loaded;
    char mapFilename[256];
    char overlayFilename[256];
    char collisionFilename[256];
    int map;
    int overlay;
    int collision;
    char *collisionShadow;
    int x;
    int y;
    int width;
    int height;
};

struct MapTile MapTiles[MAX_TILES];

void InitMapTiles() {
    for (int i=0; i<MAX_TILES; i++) {
        MapTiles[i].exist = 0;
        MapTiles[i].loaded = 0;
        strncpy(MapTiles[i].mapFilename, "", 256);
        strncpy(MapTiles[i].overlayFilename, "", 256);
        strncpy(MapTiles[i].collisionFilename, "", 256);
        MapTiles[i].map = -1;
        MapTiles[i].overlay = -1;
        MapTiles[i].collision = -1;
        MapTiles[i].x = 0;
        MapTiles[i].y = 0;
        MapTiles[i].width = 0;
        MapTiles[i].height = 0;
    }
    
    // map 1-1
    // map 1-2
    // map 1-3
    // map 1-4
    MapTiles[0].exist = 1;
    strncpy(MapTiles[0].mapFilename, "assets/map/dev/bigmap.png", 256);
    strncpy(MapTiles[0].overlayFilename, "assets/map/dev/bigmapTop.png", 256);
    strncpy(MapTiles[0].collisionFilename, "assets/map/dev/bigmapAlpha.png", 256);
    MapTiles[0].x = 0;
    MapTiles[0].y = 0;
    MapTiles[0].width = 8192;
    MapTiles[0].height = 8192;
    
    // map 2-1
    MapTiles[1].exist = 1;
    strncpy(MapTiles[1].mapFilename, "assets/map/dev/bigmap2.png", 256);
    strncpy(MapTiles[1].overlayFilename, "assets/map/dev/bigmap2Top.png", 256);
    strncpy(MapTiles[1].collisionFilename, "assets/map/dev/bigmap2Alpha.png", 256);
    MapTiles[1].x = 10000;
    MapTiles[1].y = 10000;
    MapTiles[1].width = 8192;
    MapTiles[1].height = 8192;
}

struct map {
    int startx;
    int starty;
    int hero;
    int angle;
    int monsterExist[MAX_MONSTERS];
    int monsterType[MAX_MONSTERS];
    int monsterX[MAX_MONSTERS];
    int monsterY[MAX_MONSTERS];
    int monsterAngle[MAX_MONSTERS];
    int monsterAttack[MAX_MONSTERS];
    int monsterHealth[MAX_MONSTERS];
    int monsterHealthMax[MAX_MONSTERS];
    int monsterLevel[MAX_MONSTERS];
    int monsterAlive[MAX_MONSTERS];
    int monsterTier[MAX_MONSTERS];
    int monsterBoss[MAX_MONSTERS];
};

struct map mapdata[MAX_MAPS];

double MonsterSpeed(int type) {
    if (type == MONSTER_HUMAN) return 2;
    return 1;
}

void InitMapInfos() {
    
    for (int y=0; y<MAX_MAPS; y++) {
        mapdata[y].startx = 0;
        mapdata[y].starty = 0;
        mapdata[y].hero = -1;
        mapdata[y].angle = 0;
        for (int x=0; x<MAX_MONSTERS; x++) {
            mapdata[y].monsterExist[x] = 0;
            mapdata[y].monsterType[x] = MONSTER_NULL;
            mapdata[y].monsterX[x] = 0;
            mapdata[y].monsterY[x] = 0;
            mapdata[y].monsterAngle[x] = 0;
            mapdata[y].monsterHealth[x] = 0;
            mapdata[y].monsterAttack[x] = 0;
            mapdata[y].monsterHealthMax[x] = 0;
            mapdata[y].monsterLevel[x] = 0;
            mapdata[y].monsterAlive[x] = 0;
            mapdata[y].monsterTier[x] = 0;
            mapdata[y].monsterBoss[x] = 0;
        }
    }
}

void AddMonster(int map, int monster, int type, int x, int y, int tier, int boss, int level, int atk, int hp) {
        mapdata[map].monsterExist[monster] = 1;
        mapdata[map].monsterType[monster] = type;
        mapdata[map].monsterX[monster] = x;
        mapdata[map].monsterY[monster] = y;
        mapdata[map].monsterAngle[monster] = 0;
        mapdata[map].monsterAttack[monster] = atk;
        mapdata[map].monsterHealth[monster] = hp;
        mapdata[map].monsterHealthMax[monster] = hp;
        mapdata[map].monsterLevel[monster] = level;
        mapdata[map].monsterAlive[monster] = 1;
        mapdata[map].monsterTier[monster] = tier;
        mapdata[map].monsterBoss[monster] = boss;
}

void InitMapInfo(int hero,int map) {
    
    // map 1-1
    if (map==0) {
        mapdata[map].startx = 1020;
        mapdata[map].starty = 6900;
        mapdata[map].hero = hero;
        mapdata[map].angle = 90;
        mapdata[map].monsterExist[0] = 1;
        mapdata[map].monsterType[0] = MONSTER_HUMAN;
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 0, MONSTER_HUMAN, 6800, 4500,   0,   1,    1,    10,  1000); // BOSS
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 1, MONSTER_HUMAN, 4500, 6000,   0,   0,    1,     1,   100);
        AddMonster(map, 2, MONSTER_HUMAN, 4700, 6000,   0,   0,    1,     1,   100);
        AddMonster(map, 3, MONSTER_HUMAN, 4500, 5200,   0,   0,    1,     1,   100);
        AddMonster(map, 4, MONSTER_HUMAN, 4700, 5200,   0,   0,    1,     1,   100);
        AddMonster(map, 5, MONSTER_HUMAN, 5200, 4200,   0,   0,    1,     1,   100);
        AddMonster(map, 6, MONSTER_HUMAN, 5200, 4500,   0,   0,    1,     1,   100);
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
    }
    
    // map 1-2
    if (map==1) {
        mapdata[map].startx = 6150;
        mapdata[map].starty = 7300;
        mapdata[map].hero = hero;
        mapdata[map].angle = 90;
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 0, MONSTER_HUMAN, 7500, 5900,   0,   1,    1,    10,  1000); // BOSS
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 1, MONSTER_HUMAN, 6000, 5800,   0,   0,    1,     1,   100);
        AddMonster(map, 2, MONSTER_HUMAN, 6300, 5800,   0,   0,    1,     1,   100);
        AddMonster(map, 3, MONSTER_HUMAN, 6600, 5800,   0,   0,    1,     1,   100);
        AddMonster(map, 4, MONSTER_HUMAN, 6900, 5800,   0,   0,    1,     1,   100);
        AddMonster(map, 5, MONSTER_HUMAN, 7200, 5800,   0,   0,    1,     1,   100);
        AddMonster(map, 6, MONSTER_HUMAN, 7500, 5800,   0,   0,    1,     1,   100);
        AddMonster(map, 7, MONSTER_HUMAN, 6000, 6100,   0,   0,    1,     1,   100);
        AddMonster(map, 8, MONSTER_HUMAN, 6300, 6100,   0,   0,    1,     1,   100);
        AddMonster(map, 9, MONSTER_HUMAN, 6600, 6100,   0,   0,    1,     1,   100);
        AddMonster(map,10, MONSTER_HUMAN, 6900, 6100,   0,   0,    1,     1,   100);
        AddMonster(map,11, MONSTER_HUMAN, 7200, 6100,   0,   0,    1,     1,   100);
        AddMonster(map,12, MONSTER_HUMAN, 7500, 6100,   0,   0,    1,     1,   100);
        AddMonster(map,13, MONSTER_HUMAN, 6000, 6400,   0,   0,    1,     1,   100);
        AddMonster(map,14, MONSTER_HUMAN, 6300, 6400,   0,   0,    1,     1,   100);
        AddMonster(map,15, MONSTER_HUMAN, 6600, 6400,   0,   0,    1,     1,   100);
        AddMonster(map,16, MONSTER_HUMAN, 6900, 6400,   0,   0,    1,     1,   100);
        AddMonster(map,17, MONSTER_HUMAN, 7200, 6400,   0,   0,    1,     1,   100);
        AddMonster(map,18, MONSTER_HUMAN, 7500, 6400,   0,   0,    1,     1,   100);
        AddMonster(map,19, MONSTER_HUMAN, 6000, 6700,   0,   0,    1,     1,   100);
        AddMonster(map,20, MONSTER_HUMAN, 6300, 6700,   0,   0,    1,     1,   100);
        AddMonster(map,21, MONSTER_HUMAN, 6600, 6700,   0,   0,    1,     1,   100);
        AddMonster(map,22, MONSTER_HUMAN, 6900, 6700,   0,   0,    1,     1,   100);
        AddMonster(map,23, MONSTER_HUMAN, 7200, 6700,   0,   0,    1,     1,   100);
        AddMonster(map,24, MONSTER_HUMAN, 7500, 6700,   0,   0,    1,     1,   100);
        AddMonster(map,25, MONSTER_HUMAN, 6000, 7000,   0,   0,    1,     1,   100);
        AddMonster(map,26, MONSTER_HUMAN, 6300, 7000,   0,   0,    1,     1,   100);
        AddMonster(map,27, MONSTER_HUMAN, 6600, 7000,   0,   0,    1,     1,   100);
        AddMonster(map,28, MONSTER_HUMAN, 6900, 7000,   0,   0,    1,     1,   100);
        AddMonster(map,29, MONSTER_HUMAN, 7200, 7000,   0,   0,    1,     1,   100);
        AddMonster(map,30, MONSTER_HUMAN, 7500, 7000,   0,   0,    1,     1,   100);
        AddMonster(map,31, MONSTER_HUMAN, 6000, 7300,   0,   0,    1,     1,   100);
        AddMonster(map,32, MONSTER_HUMAN, 6300, 7300,   0,   0,    1,     1,   100);
        AddMonster(map,33, MONSTER_HUMAN, 6600, 7300,   0,   0,    1,     1,   100);
        AddMonster(map,34, MONSTER_HUMAN, 6900, 7300,   0,   0,    1,     1,   100);
        AddMonster(map,35, MONSTER_HUMAN, 7200, 7300,   0,   0,    1,     1,   100);
        AddMonster(map,36, MONSTER_HUMAN, 7500, 7300,   0,   0,    1,     1,   100);
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
    }
    
    // map 1-3
    if (map==2) {
        mapdata[map].startx = 2200;
        mapdata[map].starty = 3000;
        mapdata[map].hero = hero;
        mapdata[map].angle = 90;
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 0, MONSTER_HUMAN, 7400, 1950,   0,   1,    1,    10,  1000); // BOSS
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 1, MONSTER_HUMAN, 5200, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 2, MONSTER_HUMAN, 5250, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 3, MONSTER_HUMAN, 5300, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 4, MONSTER_HUMAN, 5350, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 5, MONSTER_HUMAN, 5400, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 6, MONSTER_HUMAN, 5450, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 7, MONSTER_HUMAN, 5500, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 8, MONSTER_HUMAN, 5550, 3050,   0,   0,    1,     1,   100);
        AddMonster(map, 9, MONSTER_HUMAN, 5600, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,10, MONSTER_HUMAN, 5650, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,11, MONSTER_HUMAN, 5700, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,12, MONSTER_HUMAN, 5750, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,13, MONSTER_HUMAN, 5800, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,14, MONSTER_HUMAN, 5850, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,15, MONSTER_HUMAN, 5900, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,16, MONSTER_HUMAN, 5950, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,17, MONSTER_HUMAN, 6000, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,18, MONSTER_HUMAN, 6050, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,19, MONSTER_HUMAN, 6100, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,20, MONSTER_HUMAN, 6150, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,21, MONSTER_HUMAN, 6200, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,22, MONSTER_HUMAN, 6250, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,23, MONSTER_HUMAN, 6300, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,24, MONSTER_HUMAN, 6350, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,25, MONSTER_HUMAN, 6400, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,26, MONSTER_HUMAN, 6450, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,27, MONSTER_HUMAN, 6500, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,28, MONSTER_HUMAN, 6550, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,29, MONSTER_HUMAN, 6600, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,30, MONSTER_HUMAN, 6650, 3050,   0,   0,    1,     1,   100);
        AddMonster(map,31, MONSTER_HUMAN, 6700, 3050,   0,   0,    1,     1,   100);
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
    }
    
    // map 1-4
    if (map==3) {
        mapdata[map].startx = 1100;
        mapdata[map].starty = 5500;
        mapdata[map].hero = hero;
        mapdata[map].angle = 90;
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 0, MONSTER_HUMAN, 5900,  900,   0,   1,    1,    10,  1000); // BOSS
        AddMonster(map, 1, MONSTER_HUMAN, 5700, 2000,   0,   0,    1,     5,   500); // BOSS (Mini)
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 2, MONSTER_HUMAN, 2000,  700,   0,   0,    1,     2,   100);
        AddMonster(map, 3, MONSTER_HUMAN, 2000,  850,   0,   0,    1,     2,   100);
        AddMonster(map, 4, MONSTER_HUMAN, 2000, 1000,   0,   0,    1,     2,   100);
        AddMonster(map, 5, MONSTER_HUMAN, 2400,  700,   0,   0,    1,     2,   100);
        AddMonster(map, 6, MONSTER_HUMAN, 2400,  850,   0,   0,    1,     2,   100);
        AddMonster(map, 7, MONSTER_HUMAN, 2400, 1000,   0,   0,    1,     2,   100);
        AddMonster(map, 8, MONSTER_HUMAN, 4000,  700,   0,   0,    1,     3,   100);
        AddMonster(map, 9, MONSTER_HUMAN, 4000,  850,   0,   0,    1,     3,   100);
        AddMonster(map,10, MONSTER_HUMAN, 4000, 1000,   0,   0,    1,     3,   100);
        AddMonster(map,11, MONSTER_HUMAN, 4500,  700,   0,   0,    1,     3,   100);
        AddMonster(map,12, MONSTER_HUMAN, 4500,  850,   0,   0,    1,     3,   100);
        AddMonster(map,13, MONSTER_HUMAN, 4500, 1000,   0,   0,    1,     3,   100);
        AddMonster(map,14, MONSTER_HUMAN, 2000, 1850,   0,   0,    1,     1,   100);
        AddMonster(map,15, MONSTER_HUMAN, 2000, 2000,   0,   0,    1,     1,   100);
        AddMonster(map,16, MONSTER_HUMAN, 2000, 2150,   0,   0,    1,     1,   100);
        AddMonster(map,17, MONSTER_HUMAN, 2400, 1850,   0,   0,    1,     1,   100);
        AddMonster(map,18, MONSTER_HUMAN, 2400, 2000,   0,   0,    1,     1,   100);
        AddMonster(map,19, MONSTER_HUMAN, 2400, 2150,   0,   0,    1,     1,   100);
        AddMonster(map,20, MONSTER_HUMAN, 4000, 1850,   0,   0,    1,     1,   100);
        AddMonster(map,21, MONSTER_HUMAN, 4000, 2000,   0,   0,    1,     1,   100);
        AddMonster(map,22, MONSTER_HUMAN, 4000, 2150,   0,   0,    1,     1,   100);
        AddMonster(map,23, MONSTER_HUMAN, 4500, 1850,   0,   0,    1,     1,   100);
        AddMonster(map,24, MONSTER_HUMAN, 4500, 2000,   0,   0,    1,     1,   100);
        AddMonster(map,25, MONSTER_HUMAN, 4500, 2150,   0,   0,    1,     1,   100);
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
    }
    
    // map 2-1
    if (map==4) {
        mapdata[map].startx = 16000;
        mapdata[map].starty = 17500;
        mapdata[map].hero = hero;
        mapdata[map].angle = 90;
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        AddMonster(map, 0, MONSTER_HUMAN,11600,17400,   0,   1,    1,   100, 10000); // BOSS
        AddMonster(map, 1, MONSTER_HUMAN,13500,13000,   0,   0,    1,    10,  5000); // BOSS (Mini)
        AddMonster(map, 2, MONSTER_HUMAN,15800,13000,   0,   0,    1,     5,  2000); // BOSS (Mini)
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
        //              #, Type         ,    X,    Y,Tier,Boss,Level,Attack,    HP)
    }
}

void InitMapWaypoints(int hero,int map) {
    
    // map 1-1
    if (map==0) {
        heroes[hero].waypoints = 3;
        heroWaypoints[hero][0].x = 4600;
        heroWaypoints[hero][0].y = 6800;
        heroWaypoints[hero][1].x = 4600;
        heroWaypoints[hero][1].y = 4500;
        heroWaypoints[hero][2].x = 6800;
        heroWaypoints[hero][2].y = 4500;
    }
    
    // map 1-2
    if (map==1) {
        heroes[hero].waypoints = 3;
        heroWaypoints[hero][0].x = 7500;
        heroWaypoints[hero][0].y = 7300;
        heroWaypoints[hero][1].x = 6100;
        heroWaypoints[hero][1].y = 6000;
        heroWaypoints[hero][2].x = 7500;
        heroWaypoints[hero][2].y = 5900;
    }
    
    // map 1-3
    if (map==2) {
        heroes[hero].waypoints = 6;
        heroWaypoints[hero][0].x = 2200;
        heroWaypoints[hero][0].y = 5300;
        heroWaypoints[hero][1].x = 3500;
        heroWaypoints[hero][1].y = 5300;
        heroWaypoints[hero][2].x = 3500;
        heroWaypoints[hero][2].y = 3100;
        heroWaypoints[hero][3].x = 6700;
        heroWaypoints[hero][3].y = 3000;
        heroWaypoints[hero][4].x = 6700;
        heroWaypoints[hero][4].y = 2000;
        heroWaypoints[hero][5].x = 7400;
        heroWaypoints[hero][5].y = 1950;
    }
    
    // map 1-4
    if (map==3) {
        heroes[hero].waypoints = 5;
        heroWaypoints[hero][0].x = 1100;
        heroWaypoints[hero][0].y = 2000;
        heroWaypoints[hero][1].x = 5700;
        heroWaypoints[hero][1].y = 2000;
        heroWaypoints[hero][2].x = 1100;
        heroWaypoints[hero][2].y = 2000;
        heroWaypoints[hero][3].x = 1100;
        heroWaypoints[hero][3].y =  900;
        heroWaypoints[hero][4].x = 5900;
        heroWaypoints[hero][4].y =  900;
    }
    
    // map 1-4
    if (map==4) {
        heroes[hero].waypoints = 20;
        heroWaypoints[hero][ 0].x = 16000;
        heroWaypoints[hero][ 0].y = 15500;
        heroWaypoints[hero][ 1].x = 15836;
        heroWaypoints[hero][ 1].y = 15236;
        heroWaypoints[hero][ 2].x = 15844;
        heroWaypoints[hero][ 2].y = 15272;
        heroWaypoints[hero][ 3].x = 15804;
        heroWaypoints[hero][ 3].y = 13048;
        heroWaypoints[hero][ 4].x = 15308;
        heroWaypoints[hero][ 4].y = 13016;
        heroWaypoints[hero][ 5].x = 14852;
        heroWaypoints[hero][ 5].y = 13012;
        heroWaypoints[hero][ 6].x = 14464;
        heroWaypoints[hero][ 6].y = 13024;
        heroWaypoints[hero][ 7].x = 14328;
        heroWaypoints[hero][ 7].y = 13124;
        heroWaypoints[hero][ 8].x = 13876;
        heroWaypoints[hero][ 8].y = 13120;
        heroWaypoints[hero][ 9].x = 13452;
        heroWaypoints[hero][ 9].y = 12960;
        heroWaypoints[hero][10].x = 13472;
        heroWaypoints[hero][10].y = 14400;
        heroWaypoints[hero][11].x = 13464;
        heroWaypoints[hero][11].y = 15108;
        heroWaypoints[hero][12].x = 13356;
        heroWaypoints[hero][12].y = 15416;
        heroWaypoints[hero][13].x = 13356;
        heroWaypoints[hero][13].y = 15416;
        heroWaypoints[hero][14].x = 13164;
        heroWaypoints[hero][14].y = 15488;
        heroWaypoints[hero][15].x = 11944;
        heroWaypoints[hero][15].y = 15476;
        heroWaypoints[hero][16].x = 11740;
        heroWaypoints[hero][16].y = 15532;
        heroWaypoints[hero][17].x = 11608;
        heroWaypoints[hero][17].y = 15744;
        heroWaypoints[hero][18].x = 11596;
        heroWaypoints[hero][18].y = 16824;
        heroWaypoints[hero][19].x = 11600;
        heroWaypoints[hero][19].y = 17400;
    }
    
}





































