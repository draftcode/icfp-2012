#!/usr/bin/env ruby

require_relative 'lib/field.rb'
require 'priority_queue'

CMDLIST = [Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT, Direction::WAIT].freeze
REV = {:U => :D, :D => :U, :R => :L, :L => :R}.freeze

def heuristic(field)
  base = field.width + field.height
  cost = 0
  f = field.field
  visited = Array.new(field.height){Array.new(field.width, false)}
  q = [[field.robot_x, field.robot_y, 0]]
  visited[field.robot_y][field.robot_x] = true
  dist_sum = 0
  until q.empty?
    x, y, dist = q.shift
    dist += 1
    CMDLIST.each do |cmd|
      nx = x + cmd.dx
      ny = y + cmd.dy
      if !field.wall?(nx, ny) && !field.rock?(nx, ny) && !visited[ny][nx]
        if field.lambda?(nx, ny)
          dist_sum += dist
        elsif field.opened_lift?(nx, ny)
          cost += (field.lambda_count+base-dist) * 50
        end
        visited[ny][nx] = true
        q << [nx, ny, dist]
      end
    end
  end
  cost + field.score*25 + 25*(field.lambda_max_count-dist_sum)
end

seen = {}
max_score = 0
best_seq = [:A]

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

q = CPriorityQueue.new
q.push([field, []], -field.score-heuristic(field))
until q.empty?
  cur,seq = q.delete_min_return_key
  next if seen.fetch(cur, -100000) > cur.score
  #puts cur
  #puts cur.score
  candidates = []
  CMDLIST.each do |cmd|
    next_field = cur.move(cmd).update!
    next_seq = seq + [cmd.cmd]
    if seen.fetch(next_field, -100000) < next_field.score
      seen[next_field] = next_field.score
      if !next_field.win && !next_field.lose
        h = heuristic(next_field)
        candidates << [[next_field, next_seq], next_field.score + h]
      end
      score = next_field.score
      score = [score, next_field.aborted_score].max if !next_field.win && !next_field.lose
      if score > max_score
        max_score = score
        best_seq = next_seq.dup + [:A]
        puts next_field
        puts max_score
        puts best_seq.join
      end
    end
  end
  sel = candidates.sort_by{|elem| elem[1]}.reverse[0,2]
  sel.each do |elem|
    q.push(elem[0], -elem[1])
  end
end

puts max_score
puts best_seq.join
