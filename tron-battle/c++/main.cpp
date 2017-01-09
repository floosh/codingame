#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <time.h>
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
        str << "|";
        for(int j=0;j<m.getW();j++) {
            if(m.get(Coord(j,i)) == -1) {
                str << " ";
            } else {
                str << m.get(Coord(j,i));
            }

        }
        str << "|" << endl;
    }
    return str;
}

struct Move {
    string action;
    int score;
    Move* parent;
    Map* map;
    vector<Move*> children;
};

bool compareByScore(Move* a, Move* b) {
    return b->score < a->score;
}

// Agent class
class Agent{
protected:
    Map *m;
    int N, P;
public:

    Agent() {
        this->m = new Map(30,20);
    }

    ~Agent() {
        delete this->m;
    }

    virtual void nextAction(stringstream &in, stringstream &out) {
        in >> N >> P; //cin.ignore();
        for (int i = 0; i < N; i++) {
            int X0; // starting X coordinate of lightcycle (or -1)
            int Y0; // starting Y coordinate of lightcycle (or -1)
            int X1; // starting X coordinate of lightcycle (can be the same as X0 if you play before this player)
            int Y1; // starting Y coordinate of lightcycle (can be the same as Y0 if you play before this player)
            in >> X0 >> Y0 >> X1 >> Y1; //cin.ignore();
            //cerr << X0 << " " << Y0 << " " << X1 << " " << Y1 << endl;

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
    }

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

    vector<Move*> play(Map* m, int playerId) {
       vector<Move*> actions;

        for (Coord c : getNeighboors(m->getPlayers()->at(playerId).c)) {
            if(m->get(c) != -1) continue;

            Move* move = new Move;
            Map* newMap = new Map(m);

            newMap->getPlayers()->at(playerId).c = c;

            move->map = newMap;
            move->action = getAction(m->getPlayers()->at(playerId).c, c);
            move->score = fitness(newMap, playerId);
            actions.push_back(move);
        }
        sort(actions.begin(), actions.end(), compareByScore);
        return actions;
    }

    string getAction(Coord f, Coord t) {
        if(f.x > t.x) {
            return "LEFT";
        } else if(f.x < t.x) {
            return "RIGHT";
        } else if(f.y > t.y) {
            return "UP";
        } else {
            return "DOWN";
        }
    }

};

class DumbAgent : public Agent {
public:
    DumbAgent():Agent() {}

    void nextAction(stringstream &in, stringstream &out) {
        Agent::nextAction(in, out);

        vector<Move*> plays = play(m, P);
        for(Move* move : plays) {
            //cerr << move->action << " " << move->score << endl;
        }
        if(plays.size() == 0) {
            out << "LEFT" << endl; // Yolo
        } else {
            Move* move = plays[0];
            out << move->action << endl; // A single line with UP, DOWN, LEFT or RIGHT
        }
    }
};

class CleverAgent : public Agent {
public:
    DumbAgent():Agent() {}

    void nextAction(stringstream &in, stringstream &out) {
        Agent::nextAction(in, out);

       vector<Move*> plays = play(m, P);
        for(Move* move : plays) {
            //cerr << move->action << " " << move->score << endl;
        }
        if(plays.size() == 0) {
            out << "LEFT" << endl; // Yolo
        } else {
            Move* move = plays[0];
            out << move->action << endl; // A single line with UP, DOWN, LEFT or RIGHT
        }
    }
};

int main()
{
    std::srand(time(0));

    map<int,int> scores;
    for(int i=0;i<4;i++) {
        scores[i] = 0;
    }

    for(int games=0;games<100;games++) {
        cout << games << endl;
        Map* m = new Map(30,20);
        int N = 2;
        int A = 2; // alive players
        map<int, Agent*> agents;
        map<int, Coord> heads;
        int offset = rand()%N;
        int iterations = 0;


        for(int i=0;i<N;i++) {
            if(i==0) {
                agents.insert(pair<int, Agent*>(i, new DumbAgent()));
            } else {
                agents.insert(pair<int, Agent*>(i, new DumbAgent()));
            }
            heads.insert(pair<int, Coord>(i, Coord(rand()%30, rand()%20)));
            m->set(heads.at(i), i);
        }

        map<int, Coord> queues(heads);

        // game loop
        while (A > 1) {
            // Each player
            for(int i=0;i<N;i++) {
                int current = (i+offset)%N;
                Agent* a = agents.at(current);
                if(a == nullptr) continue;
                stringstream in, out;

                in << N << " " << current << endl;
                for(int j=0;j<N;j++) {
                    in << queues.at(j).x << " " << queues.at(j).y << " " << heads.at(j).x << " " << heads.at(j).y << endl;
                }

                a->nextAction(in, out);

                string action;
                out >> action;
                // cerr << "Player " << i << " play " << action << endl;

                if(action == "RIGHT") {
                    heads.at(current).x++;
                } else if(action == "LEFT") {
                    heads.at(current).x--;
                } else if(action == "UP") {
                    heads.at(current).y--;
                } else if(action == "DOWN") {
                    heads.at(current).y++;
                }

                if(m->get(heads.at(current)) != -1) {
                    // player i is dead
                    cout << "Player " << current << " is dead in " << iterations << " iterations" << endl;
                    A--;
                    scores.at(current)+=A;
                } else {
                    m->set(heads.at(current), current);
                }
                //cout << *m << endl;
            }
            iterations++;
        }

        // Delete things
        agents.clear();
        delete m;
    }

    // print scores
    cout << "----- Scores -----" << endl;
    for(int i=0;i<4;i++) {
        cout << i << " : " << scores.at(i) << endl;
    }

}






