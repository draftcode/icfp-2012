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
history = []
puts field
catch(:end) {
  while true
    cmd = STDIN.gets.chomp
    cmd.each_char do |ch|
      next_field = field.dup
      case ch
      when 'U'
        next_field.move!(Direction::UP)
      when 'D'
        next_field.move!(Direction::DOWN)
      when 'L'
        next_field.move!(Direction::LEFT)
      when 'R'
        next_field.move!(Direction::RIGHT)
      when 'W'
        next_field.move!(Direction::WAIT)
      when 'A'
        next_field.abort!
        throw :end
      when '<'
        puts "Undo!"
        field = history.pop.first
        next
      else
        puts "Unknown command: #{ch}"
        next
      end
      next_field.update!
      history << [field,cmd]

      field = next_field
      if field.win
        puts "Win!"
        throw :end
      elsif field.lose
        puts "Lose..."
        throw :end
      end
    end
    puts "HP:#{field.hp}"
    if field.flooding > 0
      puts "Next water rising: #{field.turn%field.flooding}/#{field.flooding}"
    else
      puts "Water never rise"
    end
    puts field
  end
}

score = field.score
puts "Score: #{score}"
puts history.map{|arr| arr[1]}.join
