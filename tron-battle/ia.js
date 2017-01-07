var Map = (function() {
    function Map(s) {
        this._m = Array.apply(null, Array(s[0])).map(function(m) {return Array.apply(null, Array(s[1]))});
        this._w = s[0];
        this._h = s[1];
    }
    Map.prototype.get = function(c) {
        if(c[0]<0 || c[0]>=this._w || c[1]<0 || c[1]>=this._h) return -1;
        return this._m[c[0]][c[1]];
    }
    Map.prototype.set = function(c, v)  {
        if(c[0]<0 || c[0]>=this._w || c[1]<0 || c[1]>=this._h) return -1;
        this._m[c[0]][c[1]] = v;
    }
    Map.prototype.size = function() {
        return [this._w, this._h];
    }
    Map.prototype.print = function() {
        for(var i=0;i<this._h;i++) {
            var line = '';
            for(var j=0;j<this._w;j++) {
                line += (this._m[j][i] == null) ? ' ' : this._m[j][i];
            }
            printErr(line);
        }
    }
    Map.prototype.each = function(callback) {
        for(var i=0;i<this._w;i++) {
            for(var j=0;j<this._h;j++) {
                callback([i,j], this._m[i][j]);
            }
        }
    }
    Map.prototype.clone = function() {
        var m = new Map(this.size());
        this.each(function(c,v) {
            m.set(c,v);  
        })
        return m;
    }
    return Map;
})();

var globalMap = new Map([30,20]);
var globalHeads = [];

// game loop
/*while (true) {
    play()
}*/

function play() {
    var inputs = readline().split(' ');
    var N = parseInt(inputs[0]); // total number of players (2 to 4).
    var P = parseInt(inputs[1]); // your player number (0 to 3).
    
    var start = new Date().getTime()
    
    for (var i = 0; i < N; i++) {
        var inputs = readline().split(' ');
        var X0 = parseInt(inputs[0]); // starting X coordinate of lightcycle (or -1)
        var Y0 = parseInt(inputs[1]); // starting Y coordinate of lightcycle (or -1)
        var X1 = parseInt(inputs[2]); // starting X coordinate of lightcycle (can be the same as X0 if you play before this player)
        var Y1 = parseInt(inputs[3]); // starting Y coordinate of lightcycle (can be the same as Y0 if you play before this player)
        
        globalMap.set([X0,Y0], i);
        globalHeads[i] = [X1, Y1];
        if(X1 >= 0) {
            globalMap.set(globalHeads[i], i);
        } else {
            globalMap.each(function(c,v) {
                if(v==i) globalMap.set(c, null);   
            })
        }
    }

    // I play
    var rootMoves = getMoves(globalMap, globalHeads, P)
    var childMoves = []
    moves = rootMoves
    
    iterations:
    for(var i=0 ; i<Math.min(globalHeads.filter(h=>h[0]==-1)+1,2) ; i++) {
        printErr('Iteration '+i)
    
       // Opponents plays
        //moves = moves.map(function(move) {
        for(var j=0;j<moves.length;j++) {
            var move = moves[j]
            for(var opponent=(P+1)%N ; opponent!=P ; opponent=(opponent+1)%N) {
                // Each opponent   
                
                var current = new Date().getTime()
                if(current - start > 60) {
                    printErr('Times out ! '+ (current - start))
                    break iterations;   
                } else {
                   printErr('Time : '+ (current - start))
                }
                
                if(move.heads[opponent][0]>=0) {
                    // Opponent still alive, bastard
                    var opponentMoves = getMoves(move.map, move.heads, opponent).sort(function(a,b) {
                        return b.score - a.score
                    })
                    if(opponentMoves.length) {
                        var opponentMove = opponentMoves[0]
                        // Apply move
                        move.map = opponentMove.map
                        move.heads = opponentMove.heads
                    }
                }
            }
        }
        
        var current = new Date().getTime()
        if(current - start + 3*moves.length > 90) {
            printErr('Times out ! '+ (current - start))
            break iterations;   
        } else {
           printErr('Time : '+ (current - start))
        }
        
        
        // I play
        for(var move of moves) {
            move.children = getMoves(move.map, move.heads,P)
            for(var child of move.children) {
                child.parent = move   
            }
            childMoves = childMoves.concat(move.children)
        }
        
        moves = childMoves
        
        var current = new Date().getTime()
        if(current - start > 70) {
            printErr('Times out ! '+ (current - start))
            break iterations;   
        } else {
           printErr('Time : '+ (current - start))
        }
        
    }
    
    // Choose the best one
    childMoves = childMoves.length ? childMoves : rootMoves
    printErr(childMoves.length + ' child moves')
    bestMove = childMoves.sort(function(a, b) {
        return b.score - a.score
    })[0]
    printErr('Best child move: ' +bestMove.action + ' ' + bestMove.score)
    while(bestMove.parent) {
        bestMove = bestMove.parent
        printErr('Parent move: ' +bestMove.action + ' ' + bestMove.score)
    }
        
    return (bestMove?bestMove.action:'LEFT')
}

function getMoves(m,h,p) {
    return getNeighbors(h[p]).filter(n=>m.get(n)==null).map(function(n) {
        var heads = h.slice(0)
        var map = m.clone()
        heads[p] = n
        map.set(n,p)
        var move = {'action':getAction(h[p], n), 'player':p, 'destination':n, 'map':map, 'heads':heads, 'score':lightFitness(map, heads, p)}
        printErr('Player ' + p + ' : ' + move.action + ' ' + move.score)
        return move
    })
}


//Map and players configuration
function fitness(m,h,p) {
    var score = 0
    
    
    //score about surfaces
    var [am, areas] = getAreas(m,h)
    var areaPerPlayer = []
    for(var i=0;i<h.length;i++) {
        areaPerPlayer[i] = areas.filter(a=>a.players.indexOf(i)>=0).sort(function(a,b) {
            return b.surface - a.surface  
        })
    }
    if(areaPerPlayer[p].length == 0) {
        // Arrrrgggg
        score += -10000000
    } else if(areaPerPlayer[p].length == 1 && areaPerPlayer[p][0].players.length == 1) {
        // Ohoo, I'm solo !
        
        var playerWithBiggestArea = areaPerPlayer.map(function(a, i) {
            return [i, (a[0])?a[0].surface:0] 
        }).sort(function(a,b) {
            return b[1]-a[1]
        })[0][0]
        //score += (areaPerPlayer[P][0].surface - areaPerPlayer[playerWithBiggestArea][0].surface)*1000
        
        // Better to fill in this case
        
    } 
    var v = voronoi(m,h)
    score += 3*(2*v[p] - v.reduce((a,b)=>a+b,0))
    score -= getNeighbors(h[p]).filter(c=>m.get(c) == null).length
    //score -= getNeighbors(h[p]).filter(c=>m.get(c) == null).length
    //score about voronoi
    
    
    return score;
}

function lightFitness(m,h,p) {
    var v = voronoi(m,h)
    return (3*v[p] - v.reduce((a,b)=>a+b,0))
}

function getAreas(map, heads) {
    var a = 0;
    var areas = [];
    var m = new Map(map.size());
    m.each(function(c,v) {
        if(map.get(c) != null) {
            m.set(c,'#')
        } else if(m.get(c) == null) {
            var stack = [c];
            areas.push({'surface':0})
            do {
                var s = stack.pop();
                if(m.get(s) != null) continue;
                areas[a].surface++;
                m.set(s, a);
                for(var n of getNeighbors(s)) {
                    if(m.get(n) == null && map.get(n) == null) {
                        stack.push(n);
                    }
                }
            } while(stack.length);
            a++;
        }
    });

    areas = areas.map(function(a,i) {
        a.players = []
        for(var j=0;j<heads.length;j++) {
            if(getNeighbors(heads[j]).map(h=>m.get(h)).indexOf(i) >= 0) {
                a.players.push(j)
            }
        }
        return a
    });
     
    return [m,areas];
}

function voronoi(map, heads) {
    var stack = heads.map(x=>[x])
    var scores = [0,0,0,0]
    var m = new Map(map.size());
    do {
       for(var i=0;i<stack.length;i++) {
            var neighboors = [].concat.apply([],stack[i].map(getNeighbors))
            stack[i] = []
            for(var n of neighboors) {
                if(m.get(n) == null && map.get(n) == null) {
                    m.set(n, i)
                    scores[i]++
                    stack[i].push(n)
                }
            } 
       }
    } while(stack.reduce((a,b)=>a+b.length, 0) > 0);
    return scores
}

function getAction(from, to) {
    if(from[0] < to[0]) return 'RIGHT';
    if(from[0] > to[0]) return 'LEFT';
    if(from[1] < to[1]) return 'DOWN';
    if(from[1] > to[1]) return 'UP';
    return 'UP';
}

function getNeighbors(c) {
    return [[c[0]+1,c[1]],[c[0],c[1]-1],[c[0]-1,c[1]],[c[0],c[1]+1]];
}

function getValidNeighbors(map, c) {
    return getNeighbors(c).filter(n=>map.get(n)==null)
}

var readlineStack = [];

function readline() {
    var s = readlineStack.shift()
    return s
}

function printErr(s) {
    //console.log(s)
}

module.exports = {
    play: function(s) {
        readlineStack = readlineStack.concat(s);
        return play()
    }
}