#!/usr/bin/env ruby

require_relative 'lib/field.rb'
require 'optparse'

KEYBIND = {
  :normal => {'U' => Direction::UP, 'D' => Direction::DOWN, 'L' => Direction::LEFT, 'R' => Direction::RIGHT, 'W' => Direction::WAIT},
  :vim => {'H' => Direction::LEFT, 'J' => Direction::DOWN, 'K' => Direction::UP, 'L' => Direction::RIGHT, 'W' => Direction::WAIT}
}.freeze
mode = :normal
game_mode = false
opts = OptionParser.new
opts.on("--vim", "vim key bind(HJKL and W,A)"){|v| mode = :vim}
opts.on("--game", "game mode(key input will be processed immediately)"){|v| game_mode = true}
opts.parse!
if game_mode
  puts "Game mode"
  require 'io/console'
end

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
puts "Press A to terminate"
catch(:end) {
  while true
    if game_mode
      cmd = STDIN.getch
    else
      cmd = STDIN.gets.chomp
    end
    cmd.each_char do |ch|
      ch.upcase!
      next_field = field.dup
      case ch
      when 'A'
        next_field.abort!
        history << [field,:A]
        throw :end
      when '<'
        puts "Undo!"
        field = history.pop.first
        next
      else
        dir = KEYBIND[mode].fetch(ch)
        if dir
          next_field.move!(dir)
          history << [field,dir.cmd]
        else
          puts "Unknown command: #{ch}"
          next
        end
      end
      next_field.update!

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
