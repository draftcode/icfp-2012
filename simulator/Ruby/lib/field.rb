class Direction
  attr_reader :dx, :dy, :cmd
  def initialize(x, y, cmd)
    @dx = x
    @dy = y
    @cmd = cmd
  end
  LEFT = Direction.new(-1, 0, :L)
  RIGHT = Direction.new(1, 0, :R)
  UP = Direction.new(0, -1, :U)
  DOWN = Direction.new(0, 1, :D)
  WAIT = Direction.new(0, 0, :W)
  SHAVE = Direction.new(0, 0, :S)

  class <<self
    private :new
  end
end

class Field
  attr_reader :robot_x, :robot_y
  attr_reader :win, :lose
  attr_reader :score, :lambda_count, :lambda_max_count
  attr_reader :width, :height
  attr_reader :field
  attr_reader :water_level, :flooding, :waterproof
  attr_reader :hp, :turn
  attr_reader :razors

  WALL = '#'
  ROCK = '*'
  SPACE = ' '
  LAMBDA = '\\'
  LIFT = 'L'
  ROBOT = 'R'
  EARTH = '.'
  TRAMPOLINE = /[A-I]/
  TARGET = /\d/
  BEARD = 'W'
  RAZOR = '!'
  
  def self.metadata=(hash)
    @@metadata = {:water => 0, :flooding => 0, :waterproof => 10, :growth => 25, :razors => 0}.merge(hash).freeze
    puts @@metadata
  end

  def initialize(*args)
    if args.size == 1 && args.first.class == Field
      clone_from(args.first)
    else
      new_one(*args)
    end
  end

  def new_one(str_map)
    @turn = 0
    @lambda_count = 0
    @field = str_map.map do |row|
      row.each_char.to_a
    end

    # 矩形化して番兵を置く．
    @width = @field.map{|row| row.size}.max
    @field.map! do |row|
      if row.size < @width
        rem = [SPACE] * (@width-row.size)
        row.push(*rem)
      end
      row.unshift(WALL)
      row.push(WALL)
    end
    @width += 2
    @field.unshift([WALL]*@width)
    @field.push([WALL]*@width)
    @height = @field.size

    # λの総個数をカウント
    @lambda_max_count = @field.inject(0){|acc,row| acc+row.count(LAMBDA)}
    @lambda_count = 0

    # トランポリンの列挙(トランポリンの削除に使う)とトランポリンの目的地テーブルの作成
    @@trampolines = []
    @@trampoline_targets = {}
    @field.each_with_index do |row,y|
      row.each_with_index do |ch,x|
        if TRAMPOLINE === ch
          @@trampolines << [x,y]
        elsif TARGET === ch
          @@trampoline_targets[ch.to_i] = [x,y]
        end
      end
    end
    @@trampolines.freeze
    @@trampoline_targets.freeze

    # 初期状態
    @field.each_with_index do |row,y|
      idx = row.index(ROBOT)
      if idx
        @robot_x = idx
        @robot_y = y
      end
    end
    @win = @lose = false
    @score = 0

    # Floodingの設定
    @water_level = @height - @@metadata[:water] - 1
    @hp = @@metadata[:waterproof]

    # Beardの設定
    @razors = @@metadata[:razors]
  end
  private :new_one

  def clone_from(obj)
    @field = obj.field.map{|row| row.dup}
    @robot_x = obj.robot_x
    @robot_y = obj.robot_y
    @win = obj.win
    @lose = obj.lose
    @score = obj.score
    @lambda_count = obj.lambda_count
    @lambda_max_count = obj.lambda_max_count
    @width = obj.width
    @height = obj.height
    @water_level = obj.water_level
    @hp = obj.hp
    @turn = obj.turn
    @razors = obj.razors
  end

  def lift_opened?
    @lambda_count == @lambda_max_count
  end

  def in_grid?(x, y)
    true
    #(0...@width).include?(x) && (0...@height).include?(y)
  end

  def empty?(x, y)
    in_grid?(x, y) && @field[y][x] == SPACE
  end

  def rock?(x, y, field=@field)
    in_grid?(x, y) && field[y][x] == ROCK
  end

  def lambda?(x, y)
    in_grid?(x, y) && @field[y][x] == LAMBDA
  end

  def wall?(x, y)
    in_grid?(x, y) && @field[y][x] == WALL
  end

  def lift?(x, y)
    in_grid?(x, y) && @field[y][x] == LIFT
  end

  def trampoline?(x, y)
    in_grid?(x, y) && TRAMPOLINE === @field[y][x]
  end

  def target?(x, y)
    in_grid?(x, y) && TARGET === @field[y][x]
  end

  def closed_lift?(x, y)
    in_grid?(x, y) && !lift_opened? && @field[y][x] == LIFT
  end

  def opened_lift?(x, y)
    in_grid?(x, y) && @field[y][x] == LIFT && lift_opened?
  end

  def beard?(x, y)
    in_grid?(x, y) && @field[y][x] == BEARD
  end

  def razor?(x, y)
    in_grid?(x, y) && @field[y][x] == RAZOR
  end

  def valid_move?(dir)
    nx = @robot_x + dir.dx
    ny = @robot_y + dir.dy
    if dir == Direction::SHAVE
      @razors > 0
    elsif rock?(nx, ny)
      return false if dir == Direction::UP || dir == Direction::DOWN
      empty?(nx+dir.dx, ny)
    else
      # trampoline target は壁(FAQより)
      in_grid?(nx, ny) && !wall?(nx, ny) && !closed_lift?(nx, ny) && !target?(nx, ny) && !beard?(nx, ny)
    end
  end

  def update!
    @turn += 1
    if @robot_y >= @water_level
      @hp -= 1
    else
      @hp = @@metadata[:waterproof]
    end
    if @@metadata[:flooding] > 0 && @turn % @@metadata[:flooding] == 0
      @water_level -= 1
    end

    new_field = Array.new(@height){Array.new(@width, SPACE)}
    (0...@height).reverse_each do |y|
      (0...@width).each do |x|
        new_field[y][x] = @field[y][x]
        case @field[y][x]
        when ROCK
          if empty?(x, y+1)
            new_field[y][x] = SPACE
            new_field[y+1][x] = ROCK
          elsif rock?(x, y+1) || lambda?(x, y+1)
            if empty?(x+1, y) && empty?(x+1, y+1)
              new_field[y][x] = SPACE
              new_field[y+1][x+1] = ROCK
            elsif !lambda?(x, y+1) && empty?(x-1, y) && empty?(x-1, y+1)
              new_field[y][x] = SPACE
              new_field[y+1][x-1] = ROCK
            end
          end
        when BEARD
          if @turn % @@metadata[:growth] == 0
            (-1..1).each do |dy|
              check_y = y+dy
              (-1..1).each do |dx|
                check_x = x+dx
                next if dy == 0 && dx == 0
                new_field[check_y][check_x] = BEARD if empty?(check_x, check_y)
              end
            end
          end
        end
      end
    end
    if !rock?(@robot_x, @robot_y-1) && rock?(@robot_x, @robot_y-1, new_field)
      @lose = true
    end
    if @hp < 0
      @lose = true
    end
    @field = new_field
    self
  end

  def update
    new_field = self.dup
    new_field.update!
    new_field
  end

  def move!(dir)
    nx = @robot_x + dir.dx
    ny = @robot_y + dir.dy
    if valid_move?(dir)
      if dir == Direction::SHAVE
        (-1..1).each do |dy|
          check_y = ny+dy
          (-1..1).each do |dx|
            check_x = nx+dx
            if beard?(check_x, check_y)
              @field[check_y][check_x] = SPACE
            end
          end
        end
      elsif rock?(nx, ny)
        @field[ny][nx+dir.dx] = ROCK
      elsif lambda?(nx, ny)
        @lambda_count += 1
        @score += 25
      elsif trampoline?(nx, ny)
        target = @@metadata[:trampoline][@field[ny][nx]]
        # 同じターゲットを参照しているトランポリンは即座に消える．
        @@trampolines.each do |pos|
          x, y = pos
          if @@metadata[:trampoline][@field[y][x]] == target
            @field[y][x] = SPACE
          end
        end
        x, y = @@trampoline_targets[target]
        nx, ny = x, y if TARGET === @field[y][x]
      elsif opened_lift?(nx, ny)
        @win = true
        @score += 50*@lambda_count
      elsif razor?(nx, ny)
        @razors += 1
      end
      @field[@robot_y][@robot_x] = SPACE
      @field[ny][nx] = ROBOT
      @robot_x, @robot_y = nx, ny
    end
    @score -= 1
    self
  end
  
  def move(dir)
    new_one = self.dup
    new_one.move!(dir)
  end

  def abort!
    @score += 25*@lambda_count
  end

  def aborted_score
    @score + 25*@lambda_count
  end

  def to_s
    flood_str = @@metadata[:flooding] != 0 ?
      "#{@turn % @@metadata[:flooding]}/#{@@metadata[:flooding]}" :
      "Water never rise"
    str = "Score: #{@score} / aborted #{aborted_score}\n"
    str << "Next Growth: #{@turn % @@metadata[:growth]}/#{@@metadata[:growth]} Razors: #{@razors}\n"
    m = @field.map{|row| row.join}
    m[@water_level] << "~~~ (HP #{@hp})~~~ #{flood_str}"
    str << m.join("\n")
  end

  def dup
    Field.new(self)
  end

  def hash
    @field.hash
  end

  def eql?(other)
    @field == other.field
  end
end
