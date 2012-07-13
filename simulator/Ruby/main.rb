#!/usr/bin/env ruby

require 'curses'
include Curses

class Direction
  attr_reader :dx, :dy
  def initialize(x, y)
    @dx = x
    @dy = y
  end
  LEFT = Direction.new(-1, 0)
  RIGHT = Direction.new(1, 0)
  UP = Direction.new(0, -1)
  DOWN = Direction.new(0, 1)
  WAIT = Direction.new(0, 0)

  class <<self
    private :new
  end
end

class Field
  attr_reader :robot_x, :robot_y
  attr_reader :win, :lose
  attr_reader :score

  def initialize(str_map)
    @lambda_count = 0
    @field = str_map.map do |row|
      row.each_char.to_a
    end
    @width = @field.map{|row| row.size}.max
    @field.map! do |row|
      if row.size < @width
        rem = [' '] * (@width-row.size)
        row.push(*rem)
      end
      row.unshift("#")
      row.push("#")
    end
    @width += 2
    @field.unshift(['#']*@width)
    @field.push(['#']*@width)
    @height = @field.size

    @lambda_count = @field.inject(0){|acc,row| acc+row.count('\\')}
    @field.each_with_index do |row,y|
      idx = row.index('R')
      if idx
        @robot_x = idx
        @robot_y = y
      end
    end
    @win = @lose = false
    @score = 0
  end

  def lift_opened?
    @lambda_count == 0
  end

  def in_grid?(x, y)
    (0...@width).include?(x) && (0...@height).include?(y)
  end

  def empty?(x, y)
    in_grid?(x, y) && @field[y][x] == ' '
  end

  def rock?(x, y, field=@field)
    in_grid?(x, y) && field[y][x] == '*'
  end

  def lambda?(x, y)
    in_grid?(x, y) && @field[y][x] == '\\'
  end

  def wall?(x, y)
    in_grid?(x, y) && @field[y][x] == '#'
  end

  def closed_lift?(x, y)
    in_grid?(x, y) && !lift_opened? && @field[y][x] == 'L'
  end

  def opened_lift?(x, y)
    in_grid?(x, y) && @field[y][x] == 'L' && lift_opened?
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
    new_field = Array.new(@height){Array.new(@width, ' ')}
    (0...@height).reverse_each do |y|
      (0...@width).each do |x|
        new_field[y][x] = @field[y][x]
        case @field[y][x]
        when '*'
          if empty?(x, y+1)
            new_field[y][x] = ' '
            new_field[y+1][x] = '*'
          elsif rock?(x, y+1) || lambda?(x, y+1)
            if empty?(x+1, y) && empty?(x+1, y+1)
              new_field[y][x] = ' '
              new_field[y+1][x+1] = '*'
            elsif !lambda?(x, y+1) && empty?(x-1, y) && empty?(x-1, y+1)
              new_field[y][x] = ' '
              new_field[y+1][x-1] = '*'
            end
          end
        end
      end
    end
    if !rock?(@robot_x, @robot_y-1) && rock?(@robot_x, @robot_y-1, new_field)
      @lose = true
    end
    @field = new_field
  end

  def move!(dir)
    nx = @robot_x + dir.dx
    ny = @robot_y + dir.dy
    if valid_move?(dir)
      if rock?(nx, ny)
        @field[ny][nx+dir.dx] = '*'
      elsif lambda?(nx, ny)
        @lambda_count -= 1
        @score += 25
      elsif opened_lift?(nx, ny)
        @win = true
        @score += 50
      end
      @field[@robot_y][@robot_x] = ' '
      @field[ny][nx] = 'R'
      @robot_x, @robot_y = nx, ny
    end
    @score -= 1
  end

  def to_s
    @field.map{|row| row.join}.join("\n")
  end
end

str_map = File.open(ARGV[0]) do |f|
  f.each_line.to_a.map(&:chomp)
end
field = Field.new(str_map)
puts field
abort_flag = false
while true
  cmd = STDIN.gets.chomp
  case cmd
  when 'U'
    field.move!(Direction::UP)
  when 'D'
    field.move!(Direction::DOWN)
  when 'L'
    field.move!(Direction::LEFT)
  when 'R'
    field.move!(Direction::RIGHT)
  when 'W'
    field.move!(Direction::WAIT)
  when 'A'
    abort_flag = true
    break
  end
  field.update!
  puts field
  if field.win
    puts "Win!"
    break
  elsif field.lose
    puts "Lose..."
    break
  end
end

score = field.score
score += 25 if abort_flag
puts "Score: #{score}"
