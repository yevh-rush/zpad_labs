#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PROJECT_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
MODELS_DIR="$PROJECT_DIR/models"

sudo apt update
sudo apt install -y build-essential cmake gcc g++ make libopencv-dev pkg-config wget

mkdir -p "$MODELS_DIR"

wget -nc -O "$MODELS_DIR/deploy.prototxt"       "https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt"

wget -nc -O "$MODELS_DIR/res10_300x300_ssd_iter_140000.caffemodel"       "https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel"

echo "Dependencies installed and face detector model files downloaded."
