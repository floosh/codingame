#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>

using namespace std;


// Coord class
class Coord{
public:

    int x,y;

    Coord(int x, int y) {
        this->x = x;
        this->y = y;
    }

    Coord() {
        this->x = 0;
        this->y = 0;
    }

};
bool operator==(const Coord& c1, const Coord& c2){
    return (c1.x == c2.x && c1.y == c2.y);
}

// Player class
class Player {
public:
    Player(Coord c, int score) {
        this->c = c;
        this->score = score;
    }

    Player() {
        this->c = Coord();
        this->score = 0;
    }

    Coord c;
    int score;
};

// Map class
class Map{
public:
    Map(int w, int h)
    {
        this->w = w;
        this->h = h;
        this->m = new vector<int>(w*h);
        this->players = new map<int, Player>();
        std::fill(m->begin(), m->end(), -1);
    }

    Map(const Map* m) {
        this->w = m->w;
        this->h = m->h;
        this->m = new vector<int>(*(m->m));
        this->players = new map<int, Player>(*(m->players));
    }

    ~Map() {
        delete this->players;
        delete this->m;
    }

    int get(Coord c)
    {
        if(c.x >= w || c.x < 0 || c.y < 0 || c.y >= h) {
            return -2;
        }
        return (*m)[c.x + c.y*w];
    }

    void set(Coord c, int v)
    {
        if(c.x < w && c.x >= 0 && c.y >= 0 && c.y < h) {
            (*m)[c.x + c.y*w] = v;
        }
    }

    int getH() {
        return this->h;
    }

    int getW() {
        return this->w;
    }

    map<int, Player>* getPlayers() {
        return players;
    }

private:
    int h, w;
    vector<int> *m;
    map<int, Player> *players;
};

ostream& operator<< ( ostream& str, Map& m ) {
    for(int i=0;i<m.getH();i++) {
        for(int j=0;j<m.getW();j++) {
            if(m.get(Coord(j,i)) == -1) {
                str << " ";
            } else {
                str << m.get(Coord(j,i));
            }

        }
        str << endl;
    }
    return str;
}

enum Action {RIGHT, LEFT, DOWN, UP, LAST};

struct Move {
    Action action;
    Move* parent;
    Map* map;
    vector<Move*> children;
};

vector<Coord> getNeighboors(Coord c) {
    vector<Coord> coords;
    coords.push_back(Coord(c.x+1, c.y));
    coords.push_back(Coord(c.x-1, c.y));
    coords.push_back(Coord(c.x, c.y+1));
    coords.push_back(Coord(c.x, c.y-1));
    return coords;
}


void voronoi(Map* m) {
    queue<pair<int, Coord>> s;

    for(auto &player : *(m->getPlayers())) {
        s.push(pair<int, Coord>(player.first, player.second.c));
    }

    do {
           pair<int, Coord> p = s.front();
           s.pop();
           for(auto &nc : getNeighboors(p.second)) {
               if(m->get(nc) == -1) {
                   m->set(nc, p.first);
                   s.push(pair<int, Coord>(p.first, nc));
                   m->getPlayers()->at(p.first).score ++;
               }
           }
    } while(s.size() > 0);

    //cerr << *m << endl;
}

int fitness(Map* m, int playerId) {
    voronoi(m);
    return m->getPlayers()->at(playerId).score;
}

map<int, Move*> play(Map* m, int playerId) {
    map<int, Move*> actions;

    for ( int action = 0; action != LAST; action++ ) {
        Move* move = new Move;
        Map* newMap = new Map(m);
        switch((Action)action) {
            case LEFT:
                newMap->getPlayers()->at(playerId).c.x--;
                break;
            case RIGHT:
                newMap->getPlayers()->at(playerId).c.x++;
                break;
            case UP:
                newMap->getPlayers()->at(playerId).c.y--;
                break;
            case DOWN:
                newMap->getPlayers()->at(playerId).c.y++;
                break;
        }
        if(m->get(newMap->getPlayers()->at(playerId).c) == -1) {
            voronoi(newMap);
            move->map = newMap;
            move->action = (Action)action;
            actions.insert(pair<int, Move*>(newMap->getPlayers()->at(playerId).score, move));
        }
    }

    return actions;
}


int main()
{
    Map* m = new Map(30,20);
    map<Action, string> getAction;
    getAction.insert(pair<Action, string>(RIGHT, "RIGHT"));
    getAction.insert(pair<Action, string>(LEFT, "LEFT"));
    getAction.insert(pair<Action, string>(DOWN, "DOWN"));
    getAction.insert(pair<Action, string>(UP, "UP"));

    // game loop
    while (1) {
        int N; // total number of players (2 to 4).
        int P; // your player number (0 to 3).
        cin >> N >> P; cin.ignore();
        for (int i = 0; i < N; i++) {
            int X0; // starting X coordinate of lightcycle (or -1)
            int Y0; // starting Y coordinate of lightcycle (or -1)
            int X1; // starting X coordinate of lightcycle (can be the same as X0 if you play before this player)
            int Y1; // starting Y coordinate of lightcycle (can be the same as Y0 if you play before this player)
            cin >> X0 >> Y0 >> X1 >> Y1; cin.ignore();
            cerr << X0 << " " << Y0 << " " << X1 << " " << Y1 << endl;

            m->set(Coord(X0,Y0), i);
            (*(m->getPlayers()))[i] = Player(Coord(X1,Y1), 0);
            if(X1 >= 0) {
                 m->set(Coord(X1,Y1), i);
            } else {
                // Oh noo... he's dead
//                globalMap.each(function(c,v) {
//                    if(v==i) globalMap.set(c, null);
//                })
            }
        }

        map<int, Move*> plays = play(m, P);

        for(auto &move : plays) {
            cout << move.first << " " << move.second << endl;
        }

        Move* move = plays.rbegin()->second;

        cout << getAction[move->action] << endl; // A single line with UP, DOWN, LEFT or RIGHT
    }


//    //MinMax graph
//    Move root = {nullptr, m, vector<Move*>()};
//    vector<Move*> children;
//    children.push_back(root);

//    for(int i=0;i<iterations;i++) {
//        cout << "iteration " << i << endl;

//        // For all children moves
//        for(auto &move : children) {
//            // Opponents plays
//            for(int p=(P+1)%N;p!=P;p=(p+1)%N) {
//                // Each opponent
//                map<Action, Map*> plays = play(move->map, p);
//                // Get best score
//                Move* newMove = new Move;
//                newMove->map = plays.rbegin()->second;
//                newMove->parent = move;
//            }


//            // I play


//        }

//    }


}






