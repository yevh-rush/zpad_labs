#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y build-essential cmake gcc g++ make libopencv-dev pkg-config
