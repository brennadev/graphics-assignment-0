#!/bin/sh

#  run.sh
#  Assignment0
#
#  Created by Brenna Olson on 9/11/18.
#  Copyright © 2018 Brenna Olson. All rights reserved.

g++ SquareStarterCode.cpp glad/glad.c -F/Library/Frameworks -framework SDL2 -framework OpenGL -o square
