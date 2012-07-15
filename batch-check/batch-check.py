#!/usr/bin/env python2
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import argparse
import subprocess
import signal
import re
import os
import sys
import glob
import fcntl
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

def get_command(lifter, map_path, timeout=150):
    lifter = [map_path if v == "@" else v for v in lifter]
    start_time = time.time()
    p = subprocess.Popen(lifter,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    p.stdin.write(open(map_path).read())
    p.stdin.close()
    out = ""
    fl = fcntl.fcntl(p.stdout, fcntl.F_GETFL)
    fcntl.fcntl(p.stdout, fcntl.F_SETFL, fl | os.O_NONBLOCK)
    try:
        while p.poll() is None:
            try:
                out += p.stdout.read()
                sp = out.split()
                if len(sp) != 0:
                    out = sp[-1]
            except:
                pass
            if time.time() - start_time >= timeout:
                p.send_signal(signal.SIGINT)
                break
    except KeyboardInterrupt as e:
        p.send_signal(signal.SIGINT)
    p.wait()
    elapsed_time = time.time() - start_time
    out += p.stdout.read()
    sp = out.split()
    if len(sp) != 0:
        out = sp[-1]

    return (out, elapsed_time)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('lifter')
    parser.add_argument('--pattern', type=str)
    parser.add_argument('--timeout', type=int, default=150)
    args = parser.parse_args()

    lifter = args.lifter.split()
    results_path = os.path.join(os.path.dirname(os.path.abspath(lifter[0])),
                                "batch-check-result.pickle")
    files = sorted(glob.glob(os.path.join(base_path, 'map', '*.map')))

    results = dict()
    if os.path.exists(results_path):
        results = pickle.load(open(results_path, 'rb'))

    for map_path in files:
        if args.pattern and not re.search(args.pattern, map_path):
            continue
        try:
            key = os.path.basename(map_path)
            print "%s\t" % key,
            sys.stdout.flush()
            command, elapsed_time = get_command(lifter, map_path, args.timeout)
            point = get_point(command, map_path)
            if key in results:
                prev_point, prev_time = results[key]
                print "%d\t(%fsec)\t(%+dpoint, %+fsec)" % (
                        point, elapsed_time, point-prev_point, elapsed_time-prev_time)
            else:
                print "%d\t(%fsec)" % (point, elapsed_time)
            print command
            results[key] = (point, elapsed_time)
            pickle.dump(results, open(results_path, 'wb'))
        except KeyboardInterrupt:
            print ""
            pass

if __name__ == '__main__':
    main()
