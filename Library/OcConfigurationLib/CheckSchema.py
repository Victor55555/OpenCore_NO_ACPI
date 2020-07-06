#!/usr/bin/env python

"""
Validates schema files (e.g. OcConfigurationLib.c) for being sorted.
"""

import re
import sys

if len(sys.argv) < 2:
  print('传递文件进行检查')
  sys.exit(-1)

with open(sys.argv[1], 'r') as f:
  prev = ''
  content = [l.strip() for l in f.readlines()]
  for i, l in enumerate(content):
    if l == 'OC_SCHEMA':
      print('检查架构 {}'.format(re.match('^\w+', content[i+1]).group(0)))
      prev = ''
      continue
    x = re.search('"([^"]+)"', l)
    if x:
      if x.group(1) < prev:
        print('错误: {} 成功 {}'.format(prev, x.group(1)))
        sys.exit(1)
      prev = x.group(1)
