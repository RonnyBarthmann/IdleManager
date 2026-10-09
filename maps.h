#define MAX_MAPS 1
#define MAX_MONSTERS 10
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
    MapTiles[0].exist = 1;
    strncpy(MapTiles[0].mapFilename, "assets/map/dev/bigmap.png", 256);
    strncpy(MapTiles[0].overlayFilename, "assets/map/dev/bigmapTop.png", 256);
    strncpy(MapTiles[0].collisionFilename, "assets/map/dev/bigmapAlpha.png", 256);
    MapTiles[0].x = 0;
    MapTiles[0].y = 0;
    MapTiles[0].width = 8192;
    MapTiles[0].height = 8192;
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
    int monsterHP[MAX_MONSTERS];
    int monsterAlive[MAX_MONSTERS];
};

struct map mapdata[MAX_MAPS];

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
            mapdata[y].monsterHP[x] = 0;
            mapdata[y].monsterAlive[x] = 0;
        }
    }
}

void InitMapInfo(int hero,int map) {
    
    // map 1-1
    if (map==0) {
        mapdata[0].startx = 1020;
        mapdata[0].starty = 6900;
        mapdata[0].hero = hero;
        mapdata[0].angle = 90;
        mapdata[0].monsterExist[0] = 1;
        mapdata[0].monsterType[0] = MONSTER_HUMAN;
        mapdata[0].monsterX[0] = 6800;
        mapdata[0].monsterY[0] = 4500;
        mapdata[0].monsterAngle[0] = 0;
        mapdata[0].monsterHP[0] = 1;
        mapdata[0].monsterAlive[0] = 1;
    }
}

void InitMapWaypoints(int hero,int map) {
    
    // map 1-1
    if (map==0) {
        heroes[hero].waypoints = 3;
        heroes[hero].waypoint = 0;
        heroWaypoints[hero][0].x = 4600;
        heroWaypoints[hero][0].y = 6800;
        heroWaypoints[hero][1].x = 4600;
        heroWaypoints[hero][1].y = 4500;
        heroWaypoints[hero][2].x = 6800;
        heroWaypoints[hero][2].y = 4500;
    }
    
}





































