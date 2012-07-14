#!/usr/bin/env ruby

require_relative 'lib/field.rb'

str_map = []
metadata = {}
File.open(ARGV[0]) do |f|
  while line = f.gets
    line.chomp!
    break if line == ""
    str_map << line
  end
  while line = f.gets
    line.chomp!
    name, num = line.split
    metadata[name.downcase.to_sym] = num.to_i
  end
end
field = Field.new(str_map, metadata)
puts field
catch(:end) {
  while true
    cmd = STDIN.gets.chomp
    cmd.each_char do |ch|
      case ch
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
        field.abort!
        throw :end
      else
        puts "Unknown command: #{ch}"
        field.abort!
        throw :end
      end
      field.update!
      if field.win
        puts "Win!"
        throw :end
      elsif field.lose
        puts "Lose..."
        throw :end
      end
    end
    puts field
  end
}

score = field.score
puts "Score: #{score}"
