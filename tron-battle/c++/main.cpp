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

class Move {
public:
    string action;
    int score, player, alpha, beta;
    bool visited;
    Move* parent;
    Map* map;
    vector<Move*> children;

    Move(Move* parent, Map* map, int player) {
        this->parent = parent;
        this->map = map;
        this->player = player;
        if(parent != nullptr) {
            parent->children.push_back(this);
        }
        this->alpha = -1000000;
        this->beta = 1000000;
    }

    ~Move() {
        delete map;
        children.clear();
    }

    void print(string header) {
        cerr << header << player << " " << action << " a " << alpha << " b " << beta << endl;
        for(Move* m : children) {
            m->print(header + "  ");
        }
    }

};

bool compareByScore(Move* a, Move* b) {
    if(a->score == 0 && b->score == 0) {
        return b->alpha < a->alpha;
    }
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
        m->getPlayers()->clear();
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

    int fitness(Move* m) {
        Map* map = new Map(m->map);
        voronoi(map);
        int score =  map->getPlayers()->at(m->player).score;
        delete map;
        return score;
    }

    vector<Move*> play(Move* parent, int player) {
        vector<Move*> actions;
        vector<Coord> neighboors =  getNeighboors(parent->map->getPlayers()->at(player).c);
        for (Coord c : neighboors) {
            if(parent->map->get(c) != -1) continue;

            Map* newMap = new Map(parent->map);
            Move* move = new Move(parent, newMap, player);

            newMap->getPlayers()->at(player).c = c;
            newMap->set(c, player);

            move->action = getAction(parent->map->getPlayers()->at(player).c, c);
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

        vector<Move*> moves = play(new Move(nullptr,m,-1), P);
        for(Move* move : moves) {
            move->score = fitness(move);
        }
        if(moves.size() == 0) {
            out << "LEFT" << endl; // Yolo
        } else {
            sort(moves.begin(), moves.end(), compareByScore);
            out <<  moves[0]->action << endl;
        }
    }
};

class CleverAgent : public Agent {
public:
    CleverAgent():Agent() {}

    // NegaMax Implementation with calculations at every level (poor final results and time-eating)
    void nextAction(stringstream &in, stringstream &out) {
        Agent::nextAction(in, out);

        Move* root = new Move(nullptr,new Map(m),-1);
        vector<Move*> children = play(root, P);


        for(int iteration = 0; iteration<2; iteration++) {
            vector<Move*> newChildren;
            for(Move* move : children) {

                //Opponents
                Move* newMove = move;
                for(int i=(P+1)%N;i!=P;i=(i+1)%N) {
                    vector<Move*> opponentMoves = play(newMove,i);
                    if(opponentMoves.size()>0) {
                        for(Move* opponentMove : opponentMoves) {
                            opponentMove->score = fitness(opponentMove);
                        }
                        sort(opponentMoves.begin(), opponentMoves.end(), compareByScore);
                        newMove = opponentMoves[0];
                    }

                }

                //Me
                vector<Move*> playerMoves = play(newMove,P);
                for(Move* playerMove : playerMoves) {
                    playerMove->score = fitness(playerMove);
                }
                newChildren.insert(newChildren.end(), playerMoves.begin(), playerMoves.end());
            }
            if(newChildren.size()>0) {
                children = newChildren;
            } else {
                break;
            }
        }
        if(children.size() > 0) {
            sort(children.begin(), children.end(), compareByScore);
            Move* bestEnding = children[0];
            while(bestEnding->parent != nullptr && bestEnding->parent->player != -1) {
                bestEnding = bestEnding->parent;
            }
            out << bestEnding->action << endl;
        } else {
            out << "LEFT" << endl;
        }

        delete root;
    }
};

class CleverAgent2 : public Agent {
public:
    CleverAgent2():Agent() {}
    // Little more clever negamax (alpha beta yolo)
    void nextAction(stringstream &in, stringstream &out) {
        Agent::nextAction(in, out);

        Move* root = new Move(nullptr,new Map(m),-1);
        vector<Move*> children;
        children.push_back(root);

        // build the tree
        int player = P-1, iterations = 0;
        do {
            player = (player+1)%N;
            vector<Move*> newChildren;
            for(Move* move : children) {
                vector<Move*> playerMoves = play(move, player);
                newChildren.insert(newChildren.end(), playerMoves.begin(), playerMoves.end());
            }
            children = newChildren;
            if(player == P) {
                iterations++;
            }
        } while(iterations < 2);

        //AlphaBeta MinMax
        if(root->children.size() > 0) {
            alphaBeta(root);
            root->print("");
            sort(root->children.begin(), root->children.end(), compareByScore);
            out << root->children[0]->action << endl;
        } else {
            out << "LEFT" << endl;
        }



        delete root;
    }

    int alphaBeta(Move* move) {
        if(move->children.size() == 0) {
            return fitness(move);
        } else {
            if(move->player == P) {
                int v = -1000000;
                for(Move* m : move->children) {
                    v = max(v, alphaBeta(m));
                    move->alpha = max(move->alpha, v);
                    if(move->alpha >= move->beta) {
                        break;  /* beta cut-off */
                    }
                }
                return v;
            } else {
                int v = 1000000;
                for(Move* m : move->children) {
                    v = min(v, alphaBeta(m));
                    move->beta = min(move->beta, v);
                    if(move->beta <= move->alpha) {
                        break;  /* alpha cut-off */
                    }
                }
                return v;
            }
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
                agents.insert(pair<int, Agent*>(i, new CleverAgent2()));
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
                cout << *m << endl;
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






