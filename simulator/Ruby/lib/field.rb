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

  CHAR_TO_SYM = {'#' => :wall, '*' => :rock, ' ' => :space, '\\' => :lambda, 'L' => :lift, 'R' => :robot, '.' => :earth}.freeze
  SYM_TO_CHAR = CHAR_TO_SYM.invert.freeze
  SYM_TO_NUM = {}
  CHAR_TO_SYM.values.each_with_index do |sym,idx|
    SYM_TO_NUM[sym] = idx
  end
  def initialize(*args)
    if args.size == 1
      new_one(args.first)
    else
      clone_from(*args)
    end
  end

  def new_one(str_map)
    @lambda_count = 0
    @field = str_map.map do |row|
      row.each_char.map{|ch| CHAR_TO_SYM[ch]}
    end
    @width = @field.map{|row| row.size}.max
    @field.map! do |row|
      if row.size < @width
        rem = [:space] * (@width-row.size)
        row.push(*rem)
      end
      row.unshift(:wall)
      row.push(:wall)
    end
    @width += 2
    @field.unshift([:wall]*@width)
    @field.push([:wall]*@width)
    @height = @field.size

    @lambda_max_count = @field.inject(0){|acc,row| acc+row.count(:lambda)}
    @lambda_count = 0
    @field.each_with_index do |row,y|
      idx = row.index(:robot)
      if idx
        @robot_x = idx
        @robot_y = y
      end
    end
    @win = @lose = false
    @score = 0
  end
  private :new_one

  def clone_from(field, obj)
    @field = field
    @robot_x = obj.robot_x
    @robot_y = obj.robot_y
    @win = obj.win
    @lose = obj.lose
    @score = obj.score
    @lambda_count = obj.lambda_count
    @lambda_max_count = obj.lambda_max_count
    @width = obj.width
    @height = obj.height
  end

  def lift_opened?
    @lambda_count == @lambda_max_count
  end

  def in_grid?(x, y)
    true
    #(0...@width).include?(x) && (0...@height).include?(y)
  end

  def empty?(x, y)
    in_grid?(x, y) && @field[y][x] == :space
  end

  def rock?(x, y, field=@field)
    in_grid?(x, y) && field[y][x] == :rock
  end

  def lambda?(x, y)
    in_grid?(x, y) && @field[y][x] == :lambda
  end

  def wall?(x, y)
    in_grid?(x, y) && @field[y][x] == :wall
  end

  def closed_lift?(x, y)
    in_grid?(x, y) && !lift_opened? && @field[y][x] == :lift
  end

  def opened_lift?(x, y)
    in_grid?(x, y) && @field[y][x] == :lift && lift_opened?
  end

  def valid_move?(dir)
    nx = @robot_x + dir.dx
    ny = @robot_y + dir.dy
    if rock?(nx, ny)
      return false if dir == Direction::UP || dir == Direction::DOWN
      empty?(nx+dir.dx, ny)
    else
      in_grid?(nx, ny) && !wall?(nx, ny) && !closed_lift?(nx, ny)
    end
  end

  def update!
    new_field = Array.new(@height){Array.new(@width, :space)}
    (0...@height).reverse_each do |y|
      (0...@width).each do |x|
        new_field[y][x] = @field[y][x]
        case @field[y][x]
        when :rock
          if empty?(x, y+1)
            new_field[y][x] = :space
            new_field[y+1][x] = :rock
          elsif rock?(x, y+1) || lambda?(x, y+1)
            if empty?(x+1, y) && empty?(x+1, y+1)
              new_field[y][x] = :space
              new_field[y+1][x+1] = :rock
            elsif !lambda?(x, y+1) && empty?(x-1, y) && empty?(x-1, y+1)
              new_field[y][x] = :space
              new_field[y+1][x-1] = :rock
            end
          end
        end
      end
    end
    if !rock?(@robot_x, @robot_y-1) && rock?(@robot_x, @robot_y-1, new_field)
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
      if rock?(nx, ny)
        @field[ny][nx+dir.dx] = :rock
      elsif lambda?(nx, ny)
        @lambda_count += 1
        @score += 25
      elsif opened_lift?(nx, ny)
        @win = true
        @score += 50*@lambda_count
      end
      @field[@robot_y][@robot_x] = :space
      @field[ny][nx] = :robot
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
    @field.map{|row| row.map{|sym| SYM_TO_CHAR[sym]}.join}.join("\n")
  end

  def dup
    new_field = @field.map{|row| row.dup}
    Field.new(new_field, self)
  end

  def hash
    #@field.map{|row| row.map{|sym| SYM_TO_NUM[sym]}}.hash
    @field.hash
  end

  def eql?(other)
    @field == other.field
  end
end
