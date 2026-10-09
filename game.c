#include "game.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_HEROES 8
#define MAX_WAYPOINTS 8
#define DEBUG 1
#define BIGFLOAT 9999999999999999.9;

struct waypoint {
    int x;
    int y;
};

struct waypoint heroWaypoints[MAX_HEROES][MAX_WAYPOINTS];

void InitWaypointsHero(int hero) {
    for (int i=0; i<MAX_WAYPOINTS; i++) {
        heroWaypoints[hero][i].x = 0;
        heroWaypoints[hero][i].y = 0;
    }
}

void InitWaypoints() {
    for (int i=0; i<MAX_HEROES; i++) {
        InitWaypointsHero(i);
    }
}

int renderWidth;
int renderHeight;
double scale;

struct heroClass {
    int isHero;
    int map;
    int x;
    double xf;
    int xTarget;
    int y;
    double yf;
    int yTarget;
    int angle;
    int mana;
    int manaMax;
    int health;
    int healthMax;
    int walking;
    int speed;
    int autopilot;
    int autopilotEnabled;
    int waypoint;
    int waypoints;
    double walkCycle;
    int attack;
    double attacCycle;
};

struct heroClass heroes[MAX_HEROES];

void ClearHero(int hero) {
    heroes[hero].isHero = 0;
    heroes[hero].map = 0;
    heroes[hero].x = 0;
    heroes[hero].y = 0;
    heroes[hero].angle = 0;
    heroes[hero].mana = 1;
    heroes[hero].manaMax = 1;
    heroes[hero].health = 1;
    heroes[hero].healthMax = 1;
    heroes[hero].walking = 0;
    heroes[hero].speed = 1;
    heroes[hero].autopilot = 0;
    heroes[hero].autopilotEnabled = 0;
    heroes[hero].waypoint = 0;
    heroes[hero].waypoints = 0;
    heroes[hero].walkCycle = 0;
    heroes[hero].attack = 0;
    heroes[hero].attacCycle = 0;
    InitWaypointsHero(hero);
}

#include "maps.c"

int MapRenderDistance;

void DrawImageScaled(int ImageID, double x, double y, double w, double h) {
    DrawImage(ImageID,x*scale,y*scale,w*scale,h*scale);
}
void DrawImageRotatedScaled(int ImageID, double x, double y, double a, double s) {
    DrawImageRotated(ImageID,x*scale,y*scale,a,s*scale);
}

void InitMapCollision(int tile) {
    if (MapTiles[tile].loaded != 0) {
        int size = MapTiles[tile].width*MapTiles[tile].height;
        
        unsigned char *green = GetImageGreenChannel(MapTiles[tile].collision, &MapTiles[tile].width, &MapTiles[tile].height);
        if (green == NULL) return;
        
        MapTiles[tile].collisionShadow = (char*)malloc(size);
        if (MapTiles[tile].collisionShadow == NULL) return;
        
        
        for (int x=0; x<MapTiles[tile].width; x++) {
            for (int y=0; y<MapTiles[tile].height; y++) {
                int collision = green[x + y * MapTiles[tile].width];
                if (collision > 127) {
                    MapTiles[tile].collisionShadow[x+y*MapTiles[tile].width] = 0;
                } else {
                    MapTiles[tile].collisionShadow[x+y*MapTiles[tile].width] = 1;
                }
            }
        }
        
        free(green);
    }
}

void LoadMapTile(int tile) {
    MapTiles[tile].loaded = 1;
    MapTiles[tile].map = LoadImage(MapTiles[tile].mapFilename);
    MapTiles[tile].overlay = LoadImage(MapTiles[tile].overlayFilename);
    MapTiles[tile].collision = LoadImage(MapTiles[tile].collisionFilename);
    if (MapTiles[tile].collision != -1) InitMapCollision(tile);
}

void DrawOverlay(int xPos,int yPos) {
    for (int i=0; i<MAX_TILES; i++) {
        if (MapTiles[i].exist != 0) {
            int tileNeeded = 1;
            if (xPos < MapTiles[i].x-MapRenderDistance) tileNeeded = 0;
            if (xPos > MapTiles[i].x+MapTiles[i].width+MapRenderDistance) tileNeeded = 0;
            if (yPos < MapTiles[i].y-MapRenderDistance) tileNeeded = 0;
            if (yPos > MapTiles[i].y+MapTiles[i].height+MapRenderDistance) tileNeeded = 0;
            
            if (tileNeeded!=0) {
                if (MapTiles[i].loaded == 0) {
                    LoadMapTile(i);
                }
                if (MapTiles[i].overlay > -1) {
                    DrawImageScaled(MapTiles[i].overlay,MapTiles[i].x-xPos+960,MapTiles[i].y-yPos+540,MapTiles[i].width,MapTiles[i].height);
                }
            }
        }
    }
}

void DrawMap(int xPos,int yPos) {
    for (int i=0; i<MAX_TILES; i++) {
        if (MapTiles[i].exist != 0) {
            int tileNeeded = 1;
            if (xPos < MapTiles[i].x-MapRenderDistance) tileNeeded = 0;
            if (xPos > MapTiles[i].x+MapTiles[i].width+MapRenderDistance) tileNeeded = 0;
            if (yPos < MapTiles[i].y-MapRenderDistance) tileNeeded = 0;
            if (yPos > MapTiles[i].y+MapTiles[i].height+MapRenderDistance) tileNeeded = 0;
            
            if (tileNeeded!=0) {
                if (MapTiles[i].loaded == 0) {
                    LoadMapTile(i);
                }
                if (MapTiles[i].map > -1) {
                    DrawImageScaled(MapTiles[i].map,MapTiles[i].x-xPos+960,MapTiles[i].y-yPos+540,MapTiles[i].width,MapTiles[i].height);
                }
            }
        }
    }
}

int HeroCanvas[MAX_HEROES];
int manageMode;
int ActiveHeroSlots[4];
int ActiveHero;

int head;
int leftHand;
int rightHand;
int leftWing1;
int rightWing1;
int CanvasBG;
int HeroBG;
int HeroBG;
int VoidBG;
int Sword;
int TargetIcon;
int monsterHuman;
int AutopilotOnIcon;
int AutopilotOffIcon;
int BarTexture;
int LifeTexture;
int ManaTexture;
int ManagedModeIcon;
int FullscreenIcon;
int Font[256];

#define M_PIMALDAUMEN 3.14159265358979323846

double LookAt(double x, double y, double target_x, double target_y)
{
    double dx = target_x - x;
    double dy = target_y - y;

    double angle = atan2(dy, dx) * (180.0 / M_PIMALDAUMEN);
    
    angle += 90;

    if (angle < 0.0) angle += 360.0;

    return angle;
}

void rotate_point(double *x, double *y, double a) {
    a = fmod(a, 360.0);
    if (a < 0.0) a += 360.0;

    double rad = a * (M_PIMALDAUMEN / 180.0);

    double new_x = (*x) * cos(rad) - (*y) * sin(rad);
    double new_y = (*x) * sin(rad) + (*y) * cos(rad);

    *x = new_x;
    *y = new_y;
}

int IsInside(int xPos, int yPos, int x, int y, int w, int h) {
    if ( xPos > x ) {
        if ( xPos < x+w ) {
            if ( yPos > y ) {
                if ( yPos < y+h ) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

double ToPointDistance(double x1,double y1, double x2, double y2) {
    double x = x2-x1;
    double y = y2-y1;
    double d = sqrt(x*x + y*y);
    return d;
}

int CheckToPointDistance(double x1,double y1, double x2, double y2, double distance) {
    double d = ToPointDistance(x1,y1,x2,y2);
    return (d < distance);
}

int NearestWaypoint(int hero) {
    double distance = (double)BIGFLOAT;
    int waypoint = -1;
    for (int i=0; i<heroes[hero].waypoints; i++) {
        double nextDistance = ToPointDistance(heroes[hero].x,heroes[hero].y,heroWaypoints[hero][i].x,heroWaypoints[hero][i].y);
        if (nextDistance < distance) {
            distance = nextDistance;
            waypoint = i;
        }
    }
    return waypoint;
}

int HeroHasTarget(int hero, int range) {
    return ( CheckToPointDistance(heroes[hero].x,heroes[hero].y,heroes[hero].xTarget,heroes[hero].yTarget,range) == 0 );
}

int getMapCollision(int xPos,int yPos) {
    for (int i=0; i<MAX_TILES; i++) {
        if (MapTiles[i].exist != 0) {
            int tileNeeded = 1;
            if (xPos < MapTiles[i].x) tileNeeded = 0;
            if (xPos > MapTiles[i].x+MapTiles[i].width) tileNeeded = 0;
            if (yPos < MapTiles[i].y) tileNeeded = 0;
            if (yPos > MapTiles[i].y+MapTiles[i].height) tileNeeded = 0;
            
            if (tileNeeded!=0) {
                if (MapTiles[i].loaded == 0) {
                    LoadMapTile(i);
                }
                if (MapTiles[i].collisionShadow != NULL) {
                    int x = xPos - MapTiles[i].x;
                    int y = yPos - MapTiles[i].y;
                    return MapTiles[i].collisionShadow[x+y*MapTiles[i].width];
                }
                return 0;
            }
        }
    }
    return 0;
}

int getMapCollisionBox(int x1,int y1,int x2, int y2) {
    int collisions=0;
    for (int y=y1; y<=y2; y++) {
        for (int x=x1; x<=x2; x++) {
            collisions+=getMapCollision(x,y);
        }
    }
    return collisions;
}

int getMapCollisionBoxCenter(int x,int y,int r) {
    return getMapCollisionBox(x-r,y-r,x+r,y-r);
}

void StartMap(int hero,int map);

void HeroDies(int hero) {
    // Hero lost Map
    StartMap(hero,heroes[hero].map);
}

void HeroGetsFlatDamage(int hero,int monster, double damage) {
    int map = heroes[hero].map;
    if (map > -1) {
        if (monster > -1) {
            if (monster < MAX_MONSTERS) {
                heroes[hero].health -= damage;
                if (heroes[hero].health < 1) {
                    HeroDies(hero);
                }
            }
        }
    }
}
    
void MonsterAttacksHero(int hero,int monster) {
    int map = heroes[hero].map;
    if (map > -1) {
        if (monster > -1) {
            if (monster < MAX_MONSTERS) {
                if (mapdata[map].monsterExist[monster] != 0) {
                    if (mapdata[map].monsterAlive[monster] != 0) {
                        HeroGetsFlatDamage(hero,monster,mapdata[map].monsterAttack[monster]);
                    }
                }
            }
        }
    }
}

void MonsterDeamon(int map, int monster) {
    int hero = mapdata[map].hero;
    mapdata[map].monsterAngle[monster] = LookAt(mapdata[map].monsterX[monster],mapdata[map].monsterY[monster],heroes[hero].x,heroes[hero].y);
    if (CheckToPointDistance(mapdata[map].monsterX[monster],mapdata[map].monsterY[monster],heroes[hero].x,heroes[hero].y,1000) != 0) {
        if (CheckToPointDistance(mapdata[map].monsterX[monster],mapdata[map].monsterY[monster],heroes[hero].x,heroes[hero].y,20) != 0) {
            MonsterAttacksHero(hero,monster);
        } else {
            double nextX = mapdata[map].monsterX[monster];
            double nextY = mapdata[map].monsterY[monster];
            double speed = ToPointDistance(mapdata[map].monsterX[monster],mapdata[map].monsterY[monster],heroes[hero].x,heroes[hero].y);
            if (speed > MonsterSpeed(mapdata[map].monsterType[monster])) speed = MonsterSpeed(mapdata[map].monsterType[monster]);
            nextX += sin(mapdata[map].monsterAngle[monster] / (180.0 / M_PIMALDAUMEN)) * speed;
            nextY -= cos(mapdata[map].monsterAngle[monster] / (180.0 / M_PIMALDAUMEN)) * speed;
            if (getMapCollisionBoxCenter(nextX,nextY,0) == 0) {
                mapdata[map].monsterX[monster] = nextX;
                mapdata[map].monsterY[monster] = nextY;
            } else if (getMapCollisionBoxCenter(mapdata[map].monsterX[monster],nextY,0) == 0) {
                mapdata[map].monsterY[monster] = nextY;
            } else if (getMapCollisionBoxCenter(nextX,mapdata[map].monsterY[monster],0) == 0) {
                mapdata[map].monsterX[monster] = nextX;
            }
        }
    }
}

void MonstersDeamon(int map) {
    for (int i=0; i<MAX_MONSTERS; i++) {
        if (mapdata[map].monsterExist[i] != 0) {
            MonsterDeamon(map,i);
        }
    }
}

void MapDeamon(int map) {
    if (mapdata[map].hero != -1) {
        MonstersDeamon(map);
    }
}

int CheckHeroAtWaypoint(int hero,int waypoint) {
    return CheckToPointDistance(heroes[hero].x,heroes[hero].y,heroWaypoints[hero][waypoint].x,heroWaypoints[hero][waypoint].y,(double)heroes[hero].speed*1.1);
}

double HeroMonsterDistance(int hero, int monster) {
    int map = heroes[hero].map;
    if (map > -1) {
        if (monster > -1) {
            if (monster < MAX_MONSTERS) {
                return ToPointDistance(heroes[hero].x,heroes[hero].y,mapdata[map].monsterX[monster],mapdata[map].monsterY[monster]);
            }
        }
    }
    return BIGFLOAT;
}

int HeroNearestMonster(int hero) {
    double distance = (double)BIGFLOAT;
    int monster = -1;
    int map = heroes[hero].map;
    if (map > -1) {
        for (int i=0; i<MAX_MONSTERS; i++) {
            if (mapdata[map].monsterExist[i] != 0) {
                if (mapdata[map].monsterAlive[i] != 0) {
                    double nextDistance = HeroMonsterDistance(hero,i);
                    if (nextDistance < distance) {
                        distance = nextDistance;
                        monster = i;
                    }
                }
            }
        }
    }
    return monster;
}

int HeroNearestMonsterDistance(int hero) {
    int monster = HeroNearestMonster(hero);
    return HeroMonsterDistance(hero,monster);
}

int IsMonsterNearHero(int hero, double distance) {
    return (HeroNearestMonsterDistance(hero) < distance);
}

void MonsterDies(int hero,int monster) {
    int map = heroes[hero].map;
    if (map > -1) {
        if (monster > -1) {
            if (monster < MAX_MONSTERS) {
                mapdata[map].monsterHealth[monster] = 0;
                mapdata[map].monsterAlive[monster] = 0;
                if (mapdata[map].monsterBoss[monster] != 0) {
                    // Hero wins Map
                    StartMap(hero,heroes[hero].map);
                }
            }
        }
    }
}

void MonsterGetsFlatDamage(int hero,int monster, double damage) {
    int map = heroes[hero].map;
    if (map > -1) {
        if (monster > -1) {
            if (monster < MAX_MONSTERS) {
                mapdata[map].monsterHealth[monster] -= damage;
                if (mapdata[map].monsterHealth[monster] < 1) {
                    MonsterDies(hero,monster);
                }
            }
        }
    }
}
    
void HeroAttacksMonster(int hero,int monster) {
    int map = heroes[hero].map;
    if (map > -1) {
        if (monster > -1) {
            if (monster < MAX_MONSTERS) {
                if (mapdata[map].monsterExist[monster] != 0) {
                    if (mapdata[map].monsterAlive[monster] != 0) {
                        MonsterGetsFlatDamage(hero,monster,10);
                    }
                }
            }
        }
    }
}

void HeroDeamon(int hero) {
    if (heroes[hero].isHero!=0) {
        
        if ( heroes[hero].attack == 0) {
            
            if ( heroes[hero].autopilot != 0) {
                if ( heroes[hero].waypoint < heroes[hero].waypoints) {
                    if (CheckHeroAtWaypoint(hero,heroes[hero].waypoint) == 1) {
                        if ( heroes[hero].waypoint < heroes[hero].waypoints-1) {
                            heroes[hero].waypoint++;
                        } else {
                            heroes[hero].autopilot = 0;
                        }
                    }
                }
            }
            if ( heroes[hero].autopilot != 0) {
                heroes[hero].xTarget = heroWaypoints[hero][heroes[hero].waypoint].x;
                heroes[hero].yTarget = heroWaypoints[hero][heroes[hero].waypoint].y;
            }
            
            heroes[hero].walking = HeroHasTarget(hero,10);
            
            if ( heroes[hero].autopilotEnabled != 0) {
                if (IsMonsterNearHero(hero,500) != 0) {
                    int monster = HeroNearestMonster(hero);
                    if (monster > -1) {
                        heroes[hero].xTarget = mapdata[heroes[hero].map].monsterX[monster];
                        heroes[hero].yTarget = mapdata[heroes[hero].map].monsterY[monster];
                        heroes[hero].walking = 1;
                    }
                }
            }
            
            if ( heroes[hero].autopilotEnabled != 0) {
                if (IsMonsterNearHero(hero,20) != 0) {
                    int monster = HeroNearestMonster(hero);
                    if (monster > -1) {
                        HeroAttacksMonster(hero,monster);
                        heroes[hero].walking = 0;
                    }
                }
            }
            
            if (heroes[hero].walking!=0) {
                heroes[hero].angle = LookAt(heroes[hero].x,heroes[hero].y,heroes[hero].xTarget,heroes[hero].yTarget);
                double nextX = heroes[hero].x;
                double nextY = heroes[hero].y;
                double speed = ToPointDistance(heroes[hero].x,heroes[hero].y,heroes[hero].xTarget,heroes[hero].yTarget);
                if (speed > heroes[hero].speed) speed = heroes[hero].speed;
                nextX += sin(heroes[hero].angle / (180.0 / M_PIMALDAUMEN)) * speed;
                nextY -= cos(heroes[hero].angle / (180.0 / M_PIMALDAUMEN)) * speed;
                if (getMapCollisionBoxCenter(nextX,nextY,32) == 0) {
                    heroes[hero].x = nextX;
                    heroes[hero].y = nextY;
                } else if (getMapCollisionBoxCenter(heroes[hero].x,nextY,32) == 0) {
                    heroes[hero].y = nextY;
                } else if (getMapCollisionBoxCenter(nextX,heroes[hero].y,32) == 0) {
                    heroes[hero].x = nextX;
                }
            }
            
            
            if (heroes[hero].walking!=0) {
                heroes[hero].walkCycle++;
            } else {
                heroes[hero].walkCycle=0;
            }
            
        }
    }
}

void DisableAutopilot(int hero) {
    heroes[hero].autopilot = 0;
    heroes[hero].autopilotEnabled = 0;
}

void EnableAutopilot(int hero) {
    heroes[hero].autopilot = 1;
    heroes[hero].autopilotEnabled = 1;
    heroes[hero].waypoint = NearestWaypoint(hero);
}

void DrawMonster(int monster, int map) {
    if (mapdata[map].monsterExist[monster] != 0) {
        if (mapdata[map].monsterAlive[monster] != 0) {
            if (mapdata[map].monsterType[monster] == MONSTER_HUMAN) {
                DrawImageRotatedScaled(monsterHuman,960+mapdata[map].monsterX[monster]-heroes[mapdata[map].hero].x,540+mapdata[map].monsterY[monster]-heroes[mapdata[map].hero].y,0,0.5f);
            }
            double life = mapdata[map].monsterHealth[monster];
            life /= (double)mapdata[map].monsterHealthMax[monster];
            if (mapdata[map].monsterBoss[monster] != 0) {
                DrawImageScaled(BarTexture,810+mapdata[map].monsterX[monster]-heroes[mapdata[map].hero].x,400+mapdata[map].monsterY[monster]-heroes[mapdata[map].hero].y,300,50);
                DrawImageScaled(LifeTexture,811+mapdata[map].monsterX[monster]-heroes[mapdata[map].hero].x,401+mapdata[map].monsterY[monster]-heroes[mapdata[map].hero].y,(double)298*life,48);
            } else {
                DrawImageScaled(BarTexture,920+mapdata[map].monsterX[monster]-heroes[mapdata[map].hero].x,480+mapdata[map].monsterY[monster]-heroes[mapdata[map].hero].y,80,15);
                DrawImageScaled(LifeTexture,921+mapdata[map].monsterX[monster]-heroes[mapdata[map].hero].x,481+mapdata[map].monsterY[monster]-heroes[mapdata[map].hero].y,(double)78*life,13);
            }
    
        }
    }
}

void DrawMonsters(int map, int hero) {
    for (int i=0; i<MAX_MONSTERS; i++) {
        if (mapdata[map].monsterExist[i] != 0) {
            DrawMonster(i,map);
        }
    }
}

void DrawValueFromValue(double x,double y,double scale,int value, int maxValue) {
    DrawImageScaled(Font[47],x,y,scale/2,scale);
    double xpos = 0;
    int v;
    while (value != 0) {
        v = value%10;
        value /= 10;
        v+=48;
        xpos -= scale/2;
        DrawImageScaled(Font[v],x+xpos,y,scale/2,scale);
    }
    xpos = 0;
    value = 1000000000;
    int digits = 0;
    while (value >= 10) {
        value /= 10;
        v = maxValue / value;
        maxValue %= value;
        if (v > 0) digits = 1;
        if (digits != 0) {
            v+=48;
            xpos += scale/2;
            DrawImageScaled(Font[v],x+xpos,y,scale/2,scale);
        }
    }
}

void DrawHeroGUI(int hero) {
    if (heroes[hero].autopilotEnabled != 0) {
        DrawImageScaled(AutopilotOnIcon,1820,20,80,80);
    } else {
        DrawImageScaled(AutopilotOffIcon,1820,20,80,80);
    }
    if (manageMode == 0) {
        DrawImageScaled(ManagedModeIcon,1720,20,80,80);
    }
    DrawImageScaled(BarTexture,20,20,320,150);
    
    double life = heroes[hero].health;
    life /= (double)heroes[hero].healthMax;
    double mana = heroes[hero].mana;
    mana /= (double)heroes[hero].manaMax;
    DrawImageScaled(LifeTexture,30,30,(double)300*life,60);
    DrawImageScaled(ManaTexture,30,100,(double)300*mana,60);
    DrawValueFromValue(170,40,40,heroes[hero].health,heroes[hero].healthMax);
    DrawValueFromValue(170,110,40,heroes[hero].mana,heroes[hero].manaMax);
}

void DrawHeroWings(int hero) {
    if (heroes[hero].attack==0) {
        
        // Left Wing
        double x = -20;
        double y = 40;
        
        double a = heroes[hero].angle-sin(heroes[hero].walkCycle/10)*10;
        
        rotate_point(&x,&y,heroes[hero].angle);
        DrawImageRotatedScaled(leftWing1,960+x,540+y,a,0.5f);
        
        // Right Wing
        x = 20;
        y = 40;
        
        a = heroes[hero].angle+sin(heroes[hero].walkCycle/10)*10;
        
        rotate_point(&x,&y,heroes[hero].angle);
        DrawImageRotatedScaled(rightWing1,960+x,540+y,a,0.5f);
    }
}

void DrawHeroHands(int hero) {
    if (heroes[hero].attack==0) {
        
        // Left Hand
        double x = -40;
        double y = 0;
        
        x += sin(heroes[hero].walkCycle/10)*8;
        y -= sin(heroes[hero].walkCycle/10)*8;
        double a = heroes[hero].angle-sin(heroes[hero].walkCycle/10)*10;
        
        rotate_point(&x,&y,heroes[hero].angle);
        DrawImageRotatedScaled(leftHand,960+x,540+y,a,0.5f);
        
        // Right Hand
        x = 40;
        y = 0;
        
        x += sin(heroes[hero].walkCycle/10)*8;
        y += sin(heroes[hero].walkCycle/10)*8;
        a = heroes[hero].angle-sin(heroes[hero].walkCycle/10)*10;
        
        rotate_point(&x,&y,heroes[hero].angle);
        DrawImageRotatedScaled(Sword,960+x,540+y,a,0.5f);
        DrawImageRotatedScaled(rightHand,960+x,540+y,a,0.5f);
    }
}

void DrawHero(int canvas, int hero) {
    DrawOutput(canvas);
    DrawMap(heroes[hero].x,heroes[hero].y);
    if (heroes[hero].walking!=0) {
        if (heroes[hero].autopilotEnabled==0) {
            DrawImageScaled(TargetIcon,960+heroes[hero].xTarget-heroes[hero].x-32,540+heroes[hero].yTarget-heroes[hero].y-32,64,64);
        }
    }
    DrawImageRotatedScaled(head,960,540,heroes[hero].angle,0.5f);
    DrawHeroHands(hero);
    DrawHeroWings(hero);
    DrawMonsters(heroes[hero].map,hero);
    DrawOverlay(heroes[hero].x,heroes[hero].y);
    
    
    DrawImageScaled(BarTexture,900,440,120,30);
    double life = heroes[hero].health;
    life /= (double)heroes[hero].healthMax;
    double mana = heroes[hero].mana;
    mana /= (double)heroes[hero].manaMax;
    DrawImageScaled(LifeTexture,901,441,(double)118*life,18);
    DrawImageScaled(ManaTexture,901,461,(double)118*mana,8);
    
    DrawHeroGUI(hero);
}

void InitHeroes() {
    for (int i = 0; i < MAX_HEROES; i++) 
        ClearHero(i);
}

void HeroClickCallback(int hero, int x, int y) {
    if ( IsInside(x,y,1720,20,80,80) != 0) {
        if ( GetMousePressed(1)!=0 ) {
            manageMode = 1;
        }
        return;
    }
    if ( IsInside(x,y,1820,20,80,80) != 0) {
        if ( GetMousePressed(1)!=0 ) {
            if (heroes[hero].autopilotEnabled != 0) {
                DisableAutopilot(hero);
            } else {
                EnableAutopilot(hero);
            }
        }
        return;
    }
    heroes[hero].xTarget = heroes[hero].x + (x-960);
    heroes[hero].yTarget = heroes[hero].y + (y-540);
}

void ClickCallback(int x, int y) {
    if (manageMode!=0) {
        if ( GetMousePressed(1)!=0 ) {
            if ( IsInside(x,y,238,130,712,400) != 0) {
                ActiveHero = ActiveHeroSlots[0];
                manageMode = 0;
            }
            if ( IsInside(x,y,970,130,712,400) != 0) {
                ActiveHero = ActiveHeroSlots[1];
                manageMode = 0;
            }
            if ( IsInside(x,y,238,550,712,400) != 0) {
                ActiveHero = ActiveHeroSlots[2];
                manageMode = 0;
            }
            if ( IsInside(x,y,970,550,712,400) != 0) {
                ActiveHero = ActiveHeroSlots[3];
                manageMode = 0;
            }
        }
    } else {
        HeroClickCallback(ActiveHero,x,y);
    }
}

void StartMap(int hero,int map) {
    InitMapInfo(hero,map);
    heroes[hero].x = mapdata[map].startx;
    heroes[hero].xTarget = mapdata[map].startx;
    heroes[hero].y = mapdata[map].starty;
    heroes[hero].yTarget = mapdata[map].starty;
    heroes[hero].angle = 0;
    heroes[hero].map = map;
    heroes[hero].mana = heroes[hero].manaMax;
    heroes[hero].health = heroes[hero].healthMax;
    mapdata[map].hero = hero;
    heroes[hero].angle = mapdata[map].angle;
    heroes[hero].autopilot = heroes[hero].autopilotEnabled;
    InitMapWaypoints(hero,map);
    heroes[hero].waypoint = 0;
}

int LoadFont() {
    for (int i=0; i<256; i++) {
        Font[i] = LoadImage("assets/font/255.png");
    }
    Font[0] = LoadImage("assets/font/0.png");
    Font[47] = LoadImage("assets/font/47.png"); // /
    Font[48] = LoadImage("assets/font/48.png"); // 0
    Font[49] = LoadImage("assets/font/49.png"); // 1
    Font[50] = LoadImage("assets/font/50.png"); // 2
    Font[51] = LoadImage("assets/font/51.png"); // 3
    Font[52] = LoadImage("assets/font/52.png"); // 4
    Font[53] = LoadImage("assets/font/53.png"); // 5
    Font[54] = LoadImage("assets/font/54.png"); // 6
    Font[55] = LoadImage("assets/font/55.png"); // 7
    Font[56] = LoadImage("assets/font/56.png"); // 8
    Font[57] = LoadImage("assets/font/57.png"); // 9
    for (int i=0; i<256; i++) {
        if (Font[i] == -1) return 0;
    }
    return 1;
}
    

void GameMain(void) {
    InitMapTiles();
    InitMapInfos();
    InitHeroes();
    InitWaypoints();
    renderHeight = 2160;
    renderWidth = renderHeight;
    renderWidth *= 16;
    renderWidth /= 9;
    scale = renderHeight;
    scale /= 1080;
    MapRenderDistance = 1024;
    
    int Canvas = CreateImage(renderWidth,renderHeight);
    if (Canvas==0) return;
    for (int i=0; i<MAX_HEROES; i++) {
        HeroCanvas[i] = CreateImage(renderWidth,renderHeight);
        if (HeroCanvas[i]==0) return;
    }
    
    // heros
    
    head = LoadImage("assets/head.png");
    if (head==0) return;
    
    leftHand = LoadImage("assets/handL.png");
    if (leftHand==0) return;
    
    rightHand = LoadImage("assets/handR.png");
    if (rightHand==0) return;
    
    Sword = LoadImage("assets/sword.png");
    if (Sword==0) return;
    
    leftWing1 = LoadImage("assets/LeftWing1.png");
    if (leftWing1==0) return;
    
    rightWing1 = LoadImage("assets/RightWing1.png");
    if (rightWing1==0) return;
    
    // monsters
    
    monsterHuman = LoadImage("assets/monsters/human.png");
    if (monsterHuman==0) return;
    
    // GUI
    
    VoidBG = LoadImage("assets/canvas.png");
    if (VoidBG==0) return;
    
    CanvasBG = LoadImage("assets/bg.png");
    if (CanvasBG==0) return;
    
    HeroBG = LoadImage("assets/bgHero.png");
    if (HeroBG==0) return;
    
    TargetIcon = LoadImage("assets/target.png");
    if (TargetIcon==0) return;
    
    AutopilotOnIcon = LoadImage("assets/autopilot_on.png");
    if (AutopilotOnIcon==0) return;
    
    AutopilotOffIcon = LoadImage("assets/autopilot_off.png");
    if (AutopilotOffIcon==0) return;
    
    BarTexture = LoadImage("assets/bar.png");
    if (BarTexture==0) return;
    
    LifeTexture = LoadImage("assets/life.png");
    if (LifeTexture==0) return;
    
    ManaTexture = LoadImage("assets/mana.png");
    if (ManaTexture==0) return;
    
    ManagedModeIcon = LoadImage("assets/tiled.png");
    if (ManagedModeIcon==0) return;
    
    FullscreenIcon = LoadImage("assets/fullscreen.png");
    if (FullscreenIcon==0) return;
    
    
    
    if (LoadFont() == 0) return;
    
    manageMode = 1;
    ActiveHeroSlots[0] = 0;
    ActiveHeroSlots[1] = 1;
    ActiveHeroSlots[2] = 2;
    ActiveHeroSlots[3] = 3;
    ActiveHero = 0;
    heroes[0].isHero = 1;
    heroes[0].speed = 10;
    heroes[0].autopilotEnabled = 1;
    heroes[0].healthMax = 10000;
    heroes[0].manaMax = 1000;
    heroes[1].isHero = 1;
    heroes[1].speed = 10;
    heroes[1].autopilotEnabled = 1;
    heroes[1].healthMax = 10000;
    heroes[1].manaMax = 1000;
    heroes[2].isHero = 1;
    heroes[2].speed = 10;
    heroes[2].autopilotEnabled = 1;
    heroes[2].healthMax = 10000;
    heroes[2].manaMax = 1000;
    heroes[3].isHero = 1;
    heroes[3].speed = 10;
    heroes[3].autopilotEnabled = 1;
    heroes[3].healthMax = 10000;
    heroes[3].manaMax = 1000;
    
    heroes[4].isHero = 1;
    heroes[4].speed = 50;
    heroes[4].autopilotEnabled = 1;
    heroes[4].healthMax = 10000;
    heroes[4].manaMax = 1000;
    
    StartMap(0,0);
    StartMap(1,1);
    StartMap(2,2);
    StartMap(3,3);
    StartMap(4,4);
    
    while (!ShouldQuit()) {
        
        for (int i=0; i<MAX_HEROES; i++) HeroDeamon(i);
        for (int i=0; i<MAX_MAPS; i++) MapDeamon(i);
        
        if (manageMode==0) {
            if (ActiveHero>=0) {
                DrawHero(HeroCanvas[ActiveHero],ActiveHero);
                DrawOutput(Canvas);
                DrawImage(HeroCanvas[ActiveHero],0,0,renderWidth,renderHeight);
            } else {
                DrawOutput(Canvas);
                DrawImage(HeroBG,0,0,renderWidth,renderHeight);
            }
        } else {
            DrawOutput(Canvas);
            DrawImage(CanvasBG,0,0,renderWidth,renderHeight);
            for (int slot=0; slot<4; slot++) {
                int ActiveHero = ActiveHeroSlots[slot];
                if (ActiveHero>=0) {
                    DrawHero(HeroCanvas[ActiveHero],ActiveHero);
                };
                DrawOutput(Canvas);
                if (ActiveHero<0) {
                    if (slot==0) DrawImageScaled(HeroBG,238,130,712,400);
                    if (slot==1) DrawImageScaled(HeroBG,970,130,712,400);
                    if (slot==2) DrawImageScaled(HeroBG,238,550,712,400);
                    if (slot==3) DrawImageScaled(HeroBG,970,550,712,400);
                } else {
                    if (slot==0) DrawImageScaled(HeroCanvas[ActiveHero],238,130,712,400);
                    if (slot==1) DrawImageScaled(HeroCanvas[ActiveHero],970,130,712,400);
                    if (slot==2) DrawImageScaled(HeroCanvas[ActiveHero],238,550,712,400);
                    if (slot==3) DrawImageScaled(HeroCanvas[ActiveHero],970,550,712,400);
                }
            }
        }
        
        int key = GetKey();
        if (key==27) return;
        if (key=='1') {
            ActiveHero = 0;
            manageMode = 0;
        }
        if (key=='2') {
            ActiveHero = 1;
            manageMode = 0;
        }
        if (key=='3') {
            ActiveHero = 2;
            manageMode = 0;
        }
        if (key=='4') {
            ActiveHero = 3;
            manageMode = 0;
        }
        if (key=='5') {
            ActiveHero = 4;
            manageMode = 0;
        }
        if (key=='9') {
            manageMode = 1;
        }
        if (key=='0') {
            StartMap(0,0);
            StartMap(1,1);
            StartMap(2,2);
            StartMap(3,3);
            StartMap(4,4);
        }
        
        DrawOutput(0); /* ab jetzt wird direkt auf das Fenster gezeichnet */
        
        int windowWidth_ = WindowWidth();
        int windowHeight_ = WindowHeight();
        double scaleX = (double)windowWidth_ / ((double)windowHeight_ * (double)renderWidth / (double)renderHeight);
        double scaleY = (double)windowHeight_ / ((double)windowWidth_ * (double)renderHeight / (double)renderWidth);
        
        if (scaleY < scaleX) {
            //scaleX /= scaleY;
            scaleY = 1;
        } else {
            //scaleY /= scaleX;
            scaleX = 1;
        }
        
        int renderWidth_ = (double)windowWidth_ / scaleX;
        int renderHeight_ = (double)windowHeight_ / scaleY;
        int renderX_ = (windowWidth_ - renderWidth_) / 2;
        int renderY_ = (windowHeight_ - renderHeight_) / 2;
        
        DrawImage(VoidBG,0,0,windowWidth_,windowHeight_);
        DrawImage(Canvas,renderX_,renderY_,renderWidth_,renderHeight_);
        
        if (GetMouseIsPressed(1)!=0) {
            int mouseX_ = (double)((double)GetMouseX() - (double)renderX_) / (double)renderWidth_ * (double)1920;
            int mouseY_ = (double)((double)GetMouseY() - (double)renderY_) / (double)renderHeight_ * (double)1080;
            
            ClickCallback(mouseX_,mouseY_);
        }

        FlipBuffers();
    }
}

