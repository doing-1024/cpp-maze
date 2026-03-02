#!/bin/bash

git add .
git commit -m "$1 $(date '+%-m/%-d %H:%M') $2"
git push origin master
