#!/bin/bash

scriptPath=$(dirname "$0")
cp "$scriptPath/commit-msg" "$scriptPath/../.git/hooks/"
chmod +x "$scriptPath/../.git/hooks/commit-msg"
