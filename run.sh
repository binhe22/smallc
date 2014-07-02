#!/bin/bash
cc -std=c99 -Wall $1 mpc.c -ledit -lm -o parsing
