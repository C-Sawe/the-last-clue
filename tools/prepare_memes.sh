#!/usr/bin/env bash
# Converts the meme files in a folder into what the game reads (needs ffmpeg).
#   bash tools/prepare_memes.sh <folder with the originals>
# Images  mem1..mem4, gta6, correct1  (.jpg/.jpeg/.png)  -> assets/memes/<name>.jpg (max 800 px)
# Videos  jumpscare, lost1, dofido1, songmem1, correct1 (.mp4/.mov) -> assets/memes/<name>/
#         0001.jpg 0002.jpg ... (12 fps, 480 px tall) + audio.mp3 + fps.txt
# Sounds  yaah (.mp3/.wav/.m4a/.ogg)                     -> assets/memes/<name>.mp3
set -euo pipefail
SRC="${1:?usage: prepare_memes.sh <folder>}"
OUT="$(cd "$(dirname "$0")/.." && pwd)/assets/memes"
FPS=12
mkdir -p "$OUT"
for f in "$SRC"/*; do
  [[ -f "$f" ]] || continue
  base="$(basename "$f")"; name="${base%.*}"; ext="$(echo "${base##*.}" | tr 'A-Z' 'a-z')"
  case "$ext" in
    jpg|jpeg|png|webp)
      ffmpeg -loglevel error -y -i "$f" -vf "scale='min(800,iw)':-2" -q:v 3 "$OUT/$name.jpg"
      echo "image  $name";;
    mp4|mov|m4v|webm)
      rm -rf "$OUT/$name"; mkdir -p "$OUT/$name"
      ffmpeg -loglevel error -y -i "$f" -vf "fps=$FPS,scale=-2:480" -q:v 6 "$OUT/$name/%04d.jpg"
      ffmpeg -loglevel error -y -i "$f" -vn -ac 1 -ar 44100 -b:a 96k "$OUT/$name/audio.mp3" || true
      echo "$FPS" > "$OUT/$name/fps.txt"
      echo "video  $name ($(ls "$OUT/$name"/*.jpg | wc -l) frames)";;
    mp3|wav|m4a|ogg|aac)
      ffmpeg -loglevel error -y -i "$f" -ac 1 -ar 44100 -b:a 96k "$OUT/$name.mp3"
      echo "sound  $name";;
  esac
done
