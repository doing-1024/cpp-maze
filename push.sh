#!/bin/bash

git add .
git commit -m "$0 $(date '+%-m/%-d %H:%M') $1"
git push origin master
