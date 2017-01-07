var ia1 = require('./ia')
var ia2 = require('./ia')

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
            console.log('|' + line + '|');
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

// Map
var map = new Map([30,20])

// Number of players
var N = 2

// starting points
var H = []
for(var i=0;i<N;i++) {
    H[i] = [Math.floor(Math.random()*30), Math.floor(Math.random()*20)]
    for(var j=0;j<i;j++) {
        if(H[i][0] == H[j][0] && H[i][1] == H[j][1]) {
            i--; break;
        }
    }
}
// Queue (= starting points at the begining)
var Q = H.map(h=>h.slice())

// players
var P = [ia1, ia2]
var p = 0;

function nextTurn(p) {
    play(p).then(function(data) {
        switch(data[1]) {
            case 'UP':
                H[data[0]][1]--;
                break;
            case 'DOWN':
                H[data[0]][1]++;
                break;
            case 'LEFT':
                H[data[0]][0]--;
                break;
            case 'RIGHT':
                H[data[0]][0]++;
                break;
            default:
                //Loose

        }
        if(H[data[0]][1] < 0 || H[data[0]][1] >= 20 || H[data[0]][0] < 0 || H[data[0]][0] >= 30) {
            //loose
        }
        map.set(H[data[0]], data[0])
        nextTurn((data[0]+1)%N)
    }, function(data) {
        console.log(data + ' loose')
    })
} 

var play = function(p) {
    //console.log('Player ' + p + ' plays')
    return new Promise(function(resolve, reject) {
        var stack = []
        stack.push(N + ' ' + p)
        for(var i=0;i<N;i++) {
           stack.push(Q[i].join(' ') + ' ' + H[i].join(' '))
        }
        //console.log(JSON.stringify(stack))
        try {
            resolve([p, P[p].play(stack)])
        } catch(err) {
            reject(p)
        }
    });
}

nextTurn(0)