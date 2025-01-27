#!/usr/bin/env bash
set -euo pipefail

# Script to update external repositories from their remotes.

SUBTREES=(
  OpenGLRendering "git@github.com:RichDavisonNCL/OpenGLRendering.git" main
  NCLCoreClasses "git@github.com:RichDavisonNCL/NCLCoreClasses.git" main
)

for ((i = 0; i < ${#SUBTREES[@]}; i += 3)); do
  prefix=${SUBTREES[i]}
  source=${SUBTREES[i + 1]}
  branch=${SUBTREES[i + 2]}
  git subtree pull --prefix=${prefix} ${source} ${branch} --squash
done
