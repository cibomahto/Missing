#!/bin/bash

cd /Users/sonos/Documents/openFrameworks/apps/sonos_project/MissingApp
git pull origin master
xcodebuild -configuration Release -target "MissingApp" -project "MissingApp.xcodeproj"
