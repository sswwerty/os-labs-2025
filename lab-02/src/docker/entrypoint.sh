#!/usr/bin/env bash
set -e
# Usage inside container: ./entrypoint.sh
cd /work/src
make
exec "$@"