#!/usr/bin/env ruby

require_relative 'lib/field.rb'

CMDLIST = [Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT, Direction::WAIT].freeze
REV = {:U => :D, :D => :U, :R => :L, :L => :R}.freeze
$max_score = -1
$best_seq = [:A]
$seen = {}
def dfs(field, seq, depth)
  return if $seen.fetch(field, -1000000) > field.score
  $seen[field] = field.score
  if depth == $max_depth || field.win || field.lose
    if !field.win
      field.abort! 
      seq = seq + [:A]
    end
    score = field.score
    if field.score > $max_score
      $max_score = field.score
      $best_seq = seq
      puts $max_score
      puts $best_seq.join
    end
  else
    rev = REV[seq.last]
    CMDLIST.each do |cmd|
      #next if cmd.cmd == rev
      dfs(field.move(cmd).update!, seq+[cmd.cmd], depth+1)
    end
  end
end

maparr = File.open(ARGV[0]) do |f|
  f.each_line.map(&:chomp)
end
field = Field.new(maparr)

$max_depth = 6
100.times do
  puts "Maxdepth: #{$max_depth}"
  $seen.clear
  dfs(field, [], 0)
  $max_depth += 2
end

puts $max_score
puts $best_seq.join
