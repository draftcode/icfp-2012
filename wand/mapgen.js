// Generated by CoffeeScript 1.3.3
var LatticeMap, Map, PuzzleMap, Random, RandomMap, map, rand,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Random = (function() {

  function Random() {}

  Random.prototype.int = function(n) {
    return Math.floor(Math.random() * n);
  };

  return Random;

})();

rand = new Random();

Map = (function() {

  function Map(w, h) {
    var x, y, _i, _j, _ref, _ref1;
    this.w = w;
    this.h = h;
    this.map = [];
    for (y = _i = 0, _ref = this.h; 0 <= _ref ? _i < _ref : _i > _ref; y = 0 <= _ref ? ++_i : --_i) {
      this.map[y] = [];
      for (x = _j = 0, _ref1 = this.w; 0 <= _ref1 ? _j < _ref1 : _j > _ref1; x = 0 <= _ref1 ? ++_j : --_j) {
        if ((x === 0) || (x === this.w - 1) || (y === 0) || (y === this.h - 1)) {
          this.map[y][x] = '#';
        } else {
          this.map[y][x] = '.';
        }
      }
    }
  }

  Map.prototype.setRobot = function() {
    var x, y;
    x = rand.int(this.w - 2) + 1;
    y = rand.int(this.h - 2) + 1;
    return this.map[y][x] = 'R';
  };

  Map.prototype.setLift = function() {
    var x, y, _results;
    _results = [];
    while (true) {
      x = rand.int(this.w - 2) + 1;
      y = rand.int(this.h - 2) + 1;
      if (this.map[y][x] !== 'R') {
        this.map[y][x] = 'L';
        break;
      } else {
        _results.push(void 0);
      }
    }
    return _results;
  };

  Map.prototype.setLambda = function() {
    var i, x, y, _i, _ref, _results;
    _results = [];
    for (i = _i = 0, _ref = this.lambda; 0 <= _ref ? _i < _ref : _i > _ref; i = 0 <= _ref ? ++_i : --_i) {
      _results.push((function() {
        var _results1;
        _results1 = [];
        while (true) {
          x = rand.int(this.w - 2) + 1;
          y = rand.int(this.h - 2) + 1;
          if (this.map[y][x] === '.') {
            this.map[y][x] = '\\';
            break;
          } else {
            _results1.push(void 0);
          }
        }
        return _results1;
      }).call(this));
    }
    return _results;
  };

  Map.prototype.toString = function() {
    var line, x, y, _i, _j, _ref, _ref1, _results;
    _results = [];
    for (y = _i = 0, _ref = this.h; 0 <= _ref ? _i < _ref : _i > _ref; y = 0 <= _ref ? ++_i : --_i) {
      line = "";
      for (x = _j = 0, _ref1 = this.w; 0 <= _ref1 ? _j < _ref1 : _j > _ref1; x = 0 <= _ref1 ? ++_j : --_j) {
        line += this.map[y][x];
      }
      _results.push(console.log(line));
    }
    return _results;
  };

  return Map;

})();

RandomMap = (function(_super) {

  __extends(RandomMap, _super);

  function RandomMap(w, h, lambda) {
    this.w = w;
    this.h = h;
    this.lambda = lambda;
    RandomMap.__super__.constructor.call(this, this.w, this.h);
    RandomMap.__super__.constructor.apply(this, arguments).setRobot();
    RandomMap.__super__.constructor.apply(this, arguments).setLift();
    RandomMap.__super__.constructor.apply(this, arguments).setLambda();
  }

  return RandomMap;

})(Map);

PuzzleMap = (function(_super) {

  __extends(PuzzleMap, _super);

  function PuzzleMap(w, h, lambda) {
    this.w = w;
    this.h = h;
    this.lambda = lambda;
    PuzzleMap.__super__.constructor.call(this, this.w, this.h);
  }

  return PuzzleMap;

})(Map);

LatticeMap = (function(_super) {

  __extends(LatticeMap, _super);

  function LatticeMap() {
    return LatticeMap.__super__.constructor.apply(this, arguments);
  }

  LatticeMap.prototype.construtor = function(w, h) {
    this.w = w;
    this.h = h;
    return LatticeMap.__super__.construtor.call(this, this.w, this.h);
  };

  return LatticeMap;

})(Map);

map = new RandomMap(50, 50, 200);

console.log(map.toString());

/*
#seed
#
#maze
#koushi
#nethack
#cluster
#puzzle
#route
#
*/

