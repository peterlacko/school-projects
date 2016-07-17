#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Simple script for plotting output of jobshop executable to file output
# Autor: Peter Lacko
import numpy as np
import matplotlib.pyplot as plt

colormap = {
    0:"r",
    1:"g",
    2:"b",
    3:"y",
    4:"m",
    5:"k",
    6:"chartreuse",
    7:"deeppink",
    8:"lightcoral",
    9:"sienna",
    10:"wheat",
    11:"blueviolet",
    12:"cadetblue",
    13:"coral",
    14:"cornsilk",
    15:"fuchsia",
    16:"gold",
    17:"levander",
    18:"lightgreen",
    19:"navajowhite",
}

schedule = np.loadtxt('schedule.txt')
plt.figure(1, figsize=(15,10))
machines = set()
maxSpan = 0
for task in reversed(schedule):
    machines.add(task[0])
    if task[3] > maxSpan:
        maxSpan = task[3]
    plt.hlines(task[0], task[2], task[3], colors=colormap[task[1]], lw=50)
plt.xlabel(u"Uplynutý čas", fontsize='x-large')
plt.ylabel("Stroje", fontsize='x-large')
plt.title(u"Výrobný plán pre čas " + str(maxSpan))
plt.yticks(range(0, len(machines)+2))
plt.savefig('schedule.png', dpi=200, bbox_inches='tight')
plt.show()
