#!/usr/bin/env ruby

require_relative 'lib/field.rb'
require 'priority_queue'
require 'optparse'

CMDLIST = [Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT, Direction::WAIT].freeze
REV = {:U => :D, :D => :U, :R => :L, :L => :R}.freeze

def heuristic(field)
  return 0
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

# 岩の下3マスに入らないようにしてBFSし，λを回収していく．
def search_around(field)
  around_rock_count = Array.new(field.height){Array.new(field.width, 0)}
  field.height.times do |y|
    field.width.times do |x|
      if field.rock?(x, y)
        check_y = y+1
        (-1..1).each do |dx|
          check_x = x+dx
          if field.in_grid?(check_x, check_y)
            around_rock_count[check_y][check_x] += 1
          end
        end
      end
    end
  end
  return nil if around_rock_count[field.robot_y][field.robot_x] > 0

  distance = Array.new(field.height){Array.new(field.width, nil)}
  lambda_pos = catch(:found) {
    q = [[[field.robot_x, field.robot_y], 0]]
    until q.empty?
      pos, dist = q.shift
      x, y = pos
      dist += 1
      CMDLIST.each do |cmd|
        nx = x + cmd.dx
        ny = y + cmd.dy
        if field.enterable?(nx, ny) && around_rock_count[ny][nx] == 0 && distance[ny][nx].nil?
          distance[ny][nx] = [dist, cmd]
          throw :found, [nx, ny] if field.lambda?(nx, ny)
          q << [[nx, ny], dist]
        end
      end
    end
  }
  if lambda_pos
    # 経路復元
    x, y = lambda_pos
    buf = []
    while x != field.robot_x || y != field.robot_y
      dist, cmd = distance[y][x]
      buf << cmd
      x -= cmd.dx
      y -= cmd.dy
    end
    buf.reverse
  else
    nil
  end
end

$silent = false
def message(*args)
  puts(*args) unless $silent
end

opts = OptionParser.new
opts.on("--silent", "do not show any messages"){|v| $silent = true}
opts.parse!

seen = {}
max_score = 0
best_seq = [:A]

trap(:INT) {
  exit
}
END {
  puts best_seq.join
}

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
      message "Unknown metadata: #{line}"
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
  if res = search_around(cur)
    next_field = cur.dup
    cmd_len = res.size
    res.each_with_index do |cmd, idx|
      prev_field = next_field.dup
      next_field.move!(cmd)
      next_field.update!
      if !prev_field.valid_move?(cmd) || next_field.lose
        next_field = prev_field
        cmd_len = idx
        break
      end
    end
    if cmd_len > 0
      next_seq = seq + res[0,cmd_len].map{|cmd| cmd.cmd}
      score = next_field.score
      score = [score, next_field.aborted_score].max if !next_field.win && !next_field.lose
      if score > max_score
        max_score = score
        best_seq = next_seq.dup + [:A]
        message next_field
        message max_score
        message best_seq.join
      end
      q.push([next_field, next_seq], -(next_field.score + heuristic(next_field))) if !next_field.win && !next_field.lose
    end
  end

  CMDLIST.each do |cmd|
    next unless cur.valid_move?(cmd)
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
        message next_field
        message max_score
        message best_seq.join
      end
    end
  end
  sel = candidates.sort_by{|elem| elem[1]}.reverse[0,2]
  sel.each do |elem|
    q.push(elem[0], -elem[1])
  end
end
