#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import argparse
import subprocess
import signal
import re
import os
import sys
import glob
import time
import cPickle as pickle

base_path = os.path.dirname(os.path.dirname(os.path.abspath(sys.argv[0])))

def get_point(commands, map_path):
    p = subprocess.Popen(['ruby',
                          os.path.join(base_path, "simulator", "Ruby", "main.rb"),
                          map_path,
                          '--evaluate'],
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    p.stdin.write(commands)
    p.stdin.close()
    p.wait()

    result = p.stdout.read()
    match = re.search("Score: (\d+)", result)
    if not match:
        return 0
    return int(match.group(1))

def get_command(lifter, map_path):
    lifter = [map_path if v == "@" else v for v in lifter]
    start_time = time.time()
    p = subprocess.Popen(lifter,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    p.stdin.write(open(map_path).read())
    p.stdin.close()
    out = ""
    try:
        while p.poll() is None:
            out += p.stdout.read()
            out = out.split()[-1]
    except KeyboardInterrupt as e:
        p.send_signal(signal.SIGINT)
        p.wait()
    elapsed_time = time.time() - start_time
    out += p.stdout.read()
    out = out.split()[-1]

    return (out, elapsed_time)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('lifter')
    parser.add_argument('--pattern', type=str)
    parser.add_argument('--no-beard', action='store_true', dest='nobeard', default=False)
    parser.add_argument('--no-trampoline', action='store_true', dest='notrampoline', default=False)
    parser.add_argument('--no-flood', action='store_true', dest='noflood', default=False)
    parser.add_argument('--no-default', action='store_true', dest='nodefault', default=False)
    parser.add_argument('--no-10', action='store_true', dest='no10', default=False)
    args = parser.parse_args()

    lifter = args.lifter.split()
    results_path = os.path.join(os.path.dirname(os.path.abspath(lifter[0])),
                                "batch-check-result.pickle")
    files = sorted(glob.glob(os.path.join(base_path, 'map', '*.map')))


    results = dict()
    if os.path.exists(results_path):
        results = pickle.load(open(results_path, 'rb'))

    for map_path in files:
        if args.nobeard and os.path.basename(map_path).startswith("beard"):
            continue
        if args.notrampoline and os.path.basename(map_path).startswith("trampoline"):
            continue
        if args.noflood and os.path.basename(map_path).startswith("flood"):
            continue
        if args.nodefault and os.path.basename(map_path).startswith("contest"):
            continue
        if args.no10 and os.path.basename(map_path) == ("contest10.map"):
            continue
        if args.pattern and not re.search(args.pattern, map_path):
            continue
        try:
            key = os.path.basename(map_path)
            print "%s\t" % key,
            sys.stdout.flush()
            command, elapsed_time = get_command(lifter, map_path)
            point = get_point(command, map_path)
            if key in results:
                prev_point, prev_time = results[key]
                print "%d\t(%fsec)\t(%+dpoint, %+fsec)" % (
                        point, elapsed_time, point-prev_point, elapsed_time-prev_time)
            else:
                print "%d\t(%fsec)" % (point, elapsed_time)
            results[key] = (point, elapsed_time)
            pickle.dump(results, open(results_path, 'wb'))
        except KeyboardInterrupt:
            print ""
            pass


if __name__ == '__main__':
    main()
