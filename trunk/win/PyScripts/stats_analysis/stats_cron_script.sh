#!/bin/bash
export HOME=/home/47095/users/.home
export PATH=.:$HOME/bin:$PATH

cd $HOME/domains/s47095.gridserver.com/scripts
export PYTHON_EGG_CACHE=$HOME/.python-eggs
# need to run my own version of Python (2.5+the necessary extensions)
$HOME/bin/python create_stats_graphs.py
