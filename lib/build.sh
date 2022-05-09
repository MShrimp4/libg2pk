#!/bin/bash

g++ -D LIBEXECDIR=. -fPIC -c libg2pk.cpp `pkg-config --cflags libhangul`

gcc -shared -o libg2pk.so libg2pk.o -L../dependencies/multifast-v2.0.0/ahocorasick/build -lmecab -lahocorasick `pkg-config --libs libhangul`

