class Random
  int : (n) -> Math.floor(Math.random() * n)

rand = new Random()

class Map

  constructor:(@w,@h,fill = '.')->
    @map = []
    for y in [0...@h]
      @map[y] = []
      for x in [0...@w]
        if (x is 0) or (x is @w - 1) or (y is 0) or (y is @h - 1)
          @map[y][x] = '#'
        else
          @map[y][x] = fill

  setRobot: (x,y)->
    @map[y][x] = 'R'

  setLift: (x,y)->
    @map[y][x] = 'L'

  setLambda: (lambda)->
    for i in [0...@lambda]
      while(true)
        x = rand.int(@w-2) + 1
        y = rand.int(@h-2) + 1
        if @map[y][x] is '.' or @map[y][x] is ' '
          @map[y][x] = '\\'
          break

  toString:() ->
    for y in [0...@h]
      line = ""
      for x in [0...@w]
        line += @map[y][x]
      console.log line

class RandomMap extends Map
  constructor : (@w,@h,@lambda) ->
    super(@w,@h,)
    x = rand.int(@w - 2) + 1
    y = rand.int(@h - 2) + 1
    @setRobot(x,y)
    
    while @map[y][x] is 'R'
      x = rand.int(@w - 2) + 1
      y = rand.int(@h - 2) + 1
    @setLift(x,y)
    @setLambda(@lambda)

class ClusterMap extends Map
  constructor : (@w,@h,@lambda) ->
    super(@w, @h)

class MazeMap extends Map
  constructor : (@w, @h, @lambda) ->
    super(@w, @h)
    for y in [1 ... (@w)/2 - 1]
      for x in [1... (@h)/2 - 1]
        @map[2 * y][2 * x] = '#'
        if rand.int(3) < 2
          @map[2 * y][2 * x - 1] = '#'
        else
          @map[2 * y - 1][2 * x] = '#'

    @setRobot(1, @h - 2)
    @setLift(@w - 2,@h - 1)
    @setLambda(@lambda)

class FloorMap extends Map
  constructor : (@w, @h, @ww, @hh, @lambda) ->
    super(@w, @h, '#')

    sxx = rand.int(@ww)
    syy = rand.int(@hh)

    #通路生成
    wayW = []
    wayH = []

    dx = 1

    wayW[0] = 0
    wayH[0] = 0

    for x in [0 ... @ww - 1]
      wayW[x+1] = rand.int((@w-dx)/(@ww-x)) + Math.floor((@w-dx)/((@ww-x)*2)) + dx
      dx = wayW[x+1]

    dy = 1
    for y in [0 ... @hh - 1]
      wayH[y+1] = rand.int((@h-dy)/(@hh-y)) + Math.floor((@h-dy)/((@hh-y)*2)) + dy
      dy = wayH[y+1]

    wayW[@ww] = (@w-1)
    wayH[@hh] = (@h-1)

    #通路配置
    for i in [1 ... @hh]
      for x in [1 ... @w - 1]
        @map[wayH[i]][x] = '.'

    for i in [1 ... @ww]
      for y in [1... @h - 1]
        @map[y][wayW[i]] = '.'

    #cell fill
    for yy in [0 ... @hh]
      for xx in [0 ... @ww]
        cell = ''
        switch rand.int(1)
          when 0
            cell = '#'
          when 1
            cell = '.'
          when 2
            cell = ' '
        @fill(wayW[xx] + 1,wayH[yy] + 1,wayW[xx+1] - wayW[xx] - 1,wayH[yy+1] - wayH[yy] - 1, cell)

    #room Make
    for y in [0 ... @hh]
      for x in [0 ... @ww]

        #cell size
        w = wayW[x+1] - wayW[x] - 1
        h = wayH[y+1] - wayH[y] - 1

        #room pos and size
        rx = rand.int(w/4) + 1
        ry = rand.int(h/4) + 1
        rw = w - rand.int(w/4) - rx - 1
        rh = h - rand.int(h/4) - ry - 1
        
        @fill(rx + wayW[x] ,ry + wayH[y] ,rw,rh,'.')

        wx = rx + wayW[x] + rand.int(rw - 2) + 1
        wy = ry + wayH[y] + rand.int(rh - 2) + 1
      
        while(true)
          dir = ['l','r','u','d'][rand.int(4)]
          console.log dir
          if x is 0 and dir is 'l' then continue
          if y is 0 and dir is 'u' then continue
          if x is @ww-1 and dir is 'r' then continue
          if y is @hh-1 and dir is 'd' then continue
          break

        flag = true
        while(flag)
          switch dir
            when 'l'
              wx--
              if wx <= wayW[x] then flag = false
            when 'u'
              wy--
              if wy <= wayH[y] then flag = false
            when 'r'
              wx++
              if wx >= wayW[x+1] then flag = false
            when 'd'
              wy++
              if wy >= wayH[y+1] then flag = false
          @map[wy][wx] = '.'

    @setLambda(@lambda)

  fill : (sx,sy,w,h,cell) ->
    for y in [sy ... sy+h]
      for x in [sx ... sx+w]
        @map[y][x] = cell


class PuzzleMap extends Map
  constructor : (@w,@h,@lambda) ->
    super(@w,@h)

class LatticeMap extends Map
  construtor : (@w,@h) ->
    super(@w,@h)

class RogueMap extends Map

class RouteMap extends Map


#rmap = new RandomMap(50,50,200)
#maze = new MazeMap(49,49,400)
floor = new FloorMap(35, 35, 2, 2, 40)

#console.log rmap.toString()
#console.log maze.toString()
console.log floor.toString()

