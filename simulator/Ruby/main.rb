#!/usr/bin/env ruby

require_relative 'lib/field.rb'
require 'optparse'

$debug_print = true
def message(*args)
  puts(*args) if $debug_print
end

def report(*args)
  puts(*args)
end

KEYBIND = {
  :normal => {'U' => Direction::UP, 'D' => Direction::DOWN, 'L' => Direction::LEFT, 'R' => Direction::RIGHT, 'W' => Direction::WAIT, 'S' => Direction::SHAVE},
  :vim => {'H' => Direction::LEFT, 'J' => Direction::DOWN, 'K' => Direction::UP, 'L' => Direction::RIGHT, 'W' => Direction::WAIT, 'S' => Direction::SHAVE}
}.freeze
key_mode = :normal
system_mode = :normal
opts = OptionParser.new
opts.on("--vim", "vim key bind(HJKL and W,A)"){|v| key_mode = :vim}
opts.on("--game", "game mode(key input will be processed immediately)"){|v| system_mode = :game}
opts.on("--evaluate", "evaluating mode(no debug outputs)"){|v| system_mode = :evaluate; $debug_print = false}
opts.parse!
if system_mode == :game
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
    when /Growth (\d+)/
      metadata[:growth] = $1.to_i
    when /Razors (\d+)/
      metadata[:razors] = $1.to_i
    else
      puts "Unknown metadata: #{line}"
    end
  end
end

Field.metadata = metadata
field = Field.new(str_map)
history = []
message field
message "Press A to terminate"
loop do
  catch(:end) {
    loop do
      if system_mode == :game
        cmd = STDIN.getch
      else
        cmd = STDIN.gets
      end
      cmd ||= 'A'
      cmd.chomp!
      cmd.each_char do |ch|
        ch.upcase!
        next_field = field.dup
        case ch
        when 'A'
          next_field.abort!
          history << [field,:A]
        when '<'
          puts "Undo!"
          field = history.pop.first
          puts field
          next
        else
          dir = KEYBIND[key_mode][ch]
          if dir
            if !next_field.valid_move?(dir)
              report "Invalid move. Ignored."
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
        message field
        message ""

        throw :end if field.win || field.lose || field.aborted
      end
    end
  }
  message(field.aborted ? "Aborted" : field.win ? "Win!" : field.lose ? "Lose" : raise("Unexpected end"))
  report "Score: #{field.score}"
  report history.map{|elem| elem[1]}.join
  break if field.aborted || system_mode == :evaluate
end
