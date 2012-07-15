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
trampoline_spec = {}
File.open(ARGV[0]) do |f|
  while line = f.gets
    line.chomp!
    break if line == ""
    str_map << line
  end
  # load specs
  while line = f.gets
    line.chomp!
    case line
    when /Flooding\s+(\d+)/
      metadata[:flooding] = $1.to_i
    when /Water\s+(\d+)/
      metadata[:water] = $1.to_i
    when /Waterproof\s+(\d+)/
      metadata[:waterproof] = $1.to_i
    when /Trampoline (.) targets (.)/
      metadata[:trampoline] ||= {}
      metadata[:trampoline][$1] = $2.to_i
    else
      puts "Unknown metadata: #{line}"
    end
  end
end

Field.metadata = trampoline_spec
field = Field.new(str_map)
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
        puts field
        next
      else
        dir = KEYBIND[mode][ch]
        if dir
          if game_mode && !next_field.valid_move?(dir)
            puts "Invalid move. Ignored."
            next
          end
          next_field.move!(dir)
          history << [field,dir.cmd]
        else
          puts "Unknown command: #{ch}"
          next
        end
      end
      next_field.update!

      field = next_field
      puts "Score: #{field.score} / aborted #{field.aborted_score}"
      puts "HP:#{field.hp}"
      if metadata.fetch(:flooding, nil)
        puts "Next water rising: #{field.turn%field.flooding}/#{field.flooding}"
      else
        puts "Water never rise"
      end
      puts field
      puts ""

      if field.win
        puts "Win!"
        puts "Score: #{field.score}"
        puts history.map{|arr| arr[1]}.join
        #throw :end
      elsif field.lose
        puts "Lose..."
        puts "Score: #{field.score}"
        puts history.map{|arr| arr[1]}.join
        #throw :end
      end
    end
  end
}

score = field.score
