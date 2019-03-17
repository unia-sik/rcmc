#!/bin/bash

cat | egrep -o "[[:alpha:]]+\| [[:digit:]]+|Total: [[:digit:]]+"

