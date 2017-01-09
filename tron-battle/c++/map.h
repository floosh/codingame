#ifndef MAP_H
#define MAP_H

class Map
{

private:


public:
    Map(int w, int h);
    int get(int x, int y);
    void set(int x, int y, int v);
};

#endif // MAP_H
