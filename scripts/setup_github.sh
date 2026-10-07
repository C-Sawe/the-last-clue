#!/usr/bin/env bash
# Creates labels, milestones, issues and per-member branches for The Last Clue.
#
# Usage (from repo root, after `gh auth login`):
#   REPO=owner/the-last-clue \
#   GH_CALEB=caleb-username GH_ALVIN=alvin-username \
#   GH_SUDHEYSI=sudheysi-username GH_LATIFFA=latiffa-username \
#   bash scripts/setup_github.sh
#
# Username variables are optional — if one is missing, that person's issues
# are still created with their owner label, just not assigned.
# Assignment only works once that person has accepted the collaborator invite.
set -euo pipefail
: "${REPO:?Set REPO=owner/name}"

# ---------- Labels ----------
label() { gh label create "$1" --repo "$REPO" --color "$2" --description "$3" --force >/dev/null; }
label "owner:caleb"     "1f6feb" "Caleb Kipchirchir"
label "owner:alvin"     "2da44e" "Alvin Kimani"
label "owner:sudheysi"  "bf8700" "Sudheysi Ali"
label "owner:latiffa"   "8250df" "Latiffa"
label "algorithm"       "d73a4a" "Graphics algorithm implementation"
label "scene"           "0e8a16" "Project component built from algorithms"
label "setup"           "c5def5" "Repo, build, tooling"
label "docs"            "fbca04" "README, report, submission"
echo "✔ labels"

# ---------- Milestones ----------
milestone() {
  gh api "repos/$REPO/milestones" -f title="$1" -f due_on="$2" -f description="$3" >/dev/null 2>&1 \
    || echo "  (milestone '$1' already exists)"
}
M0="M0 – Setup"
M1="M1 – Algorithms"
M2="M2 – Project Components"
M3="M3 – Checkpoint 1 Submission"
milestone "$M0" "2026-10-09T20:59:00Z" "Everyone cloned, on their branch, starter code building."
milestone "$M1" "2026-10-13T20:59:00Z" "Every class algorithm implemented with a test-sheet image."
milestone "$M2" "2026-10-16T20:59:00Z" "Game components rendered from our own algorithms."
milestone "$M3" "2026-10-18T20:59:00Z" "README complete, screenshot PDF, single Google Form submission."
echo "✔ milestones"

# ---------- Issues ----------
# issue <owner-key> <milestone> <labels> <title> <body>
issue() {
  local who="$1" ms="$2" labels="$3" title="$4" body="$5" user=""
  case "$who" in
    caleb)    user="${GH_CALEB:-}" ;;
    alvin)    user="${GH_ALVIN:-}" ;;
    sudheysi) user="${GH_SUDHEYSI:-}" ;;
    latiffa)  user="${GH_LATIFFA:-}" ;;
  esac
  # Skip if an issue with this exact title already exists (safe to re-run).
  if gh api "repos/$REPO/issues?state=all&per_page=100" --paginate --jq '.[].title' | grep -Fxq "$title"; then
    echo "  = [$who] $title (exists)"; return
  fi
  local ms_num
  ms_num=$(gh api "repos/$REPO/milestones?state=all&per_page=100" --jq ".[] | select(.title==\"$ms\") | .number")
  local args=(-f title="$title" -f body="$body" -F milestone="$ms_num" -f "labels[]=owner:$who")
  IFS=',' read -ra extra <<< "$labels"
  for l in "${extra[@]}"; do args+=(-f "labels[]=$l"); done
  [[ -n "$user" ]] && args+=(-f "assignees[]=$user")
  gh api "repos/$REPO/issues" "${args[@]}" >/dev/null   # REST only (no GraphQL)
  echo "  + [$who] $title"
}

DONE="**Done when:** code is on your branch, a demo in \`demos/\` writes the image to \`output/\`, and a PR into \`main\` is reviewed and merged."

# --- Everyone ---
for who in caleb alvin sudheysi latiffa; do
  issue "$who" "$M0" "setup" "Onboarding: clone repo, set git identity, switch to your branch ($who)" \
"1. Accept the collaborator invite.
2. \`git clone\` the repo, then \`git config user.name\` / \`user.email\` (use your GitHub email).
3. \`git switch $who\`
4. Build: \`cmake -B build -S . && cmake --build build && ./build/canvas_test\`
5. Make one small commit on your branch (e.g. add yourself to a CONTRIBUTORS line) and push.

**Done when:** your first commit appears on \`$who\` under your GitHub account."
done

# --- Caleb ---
issue caleb "$M0" "setup" "Starter repo: CMake build, Canvas + BMP writer" \
"Already scaffolded — verify it builds on Windows/macOS/Linux, protect \`main\` (require PR), add all members as collaborators."
issue caleb "$M1" "algorithm" "Lines: DDA and Bresenham (all 8 octants) + thick lines" \
"Implement \`drawLineDDA\`, \`drawLineBresenham\`, \`drawThickLine\` in \`src/primitives/line.cpp\` (see \`primitives.h\`).
Demo \`demos/01_lines.cpp\` → \`output/01_lines.bmp\`: star burst of lines in every octant, DDA vs Bresenham side-by-side.
$DONE"
issue caleb "$M1" "algorithm" "Midpoint circle algorithm + filled circle" \
"Implement \`drawCircleMidpoint\` and \`fillCircle\` in \`src/primitives/circle.cpp\` using 8-way symmetry.
Demo \`demos/02_circles.cpp\` → concentric circles + a filled circle (future board pins / dial).
$DONE"
issue caleb "$M1" "algorithm" "Midpoint ellipse algorithm + filled ellipse" \
"Implement \`drawEllipseMidpoint\` (region 1 & 2) and \`fillEllipse\` in \`src/primitives/ellipse.cpp\`.
Demo \`demos/03_ellipses.cpp\` → grid of ellipses with varying rx/ry (future heads, rug, fingerprint ridges).
$DONE"
issue caleb "$M2" "algorithm" "2D transformation module (translate / rotate / scale about pivot)" \
"Implement \`transform2d.cpp\` per \`transform2d.h\` using 3×3 homogeneous matrices.
Needed by Sudheysi (safe dial, torn document). Demo \`demos/09_transforms.cpp\`: a polygon shown original, translated, rotated about its centre, scaled.
$DONE"
issue caleb "$M3" "setup" "Integrate all demos, review PRs, tag v0.1-checkpoint1" \
"Make sure every demo builds from a clean clone, all outputs regenerate, and tag the release used for submission."

# --- Alvin ---
issue alvin "$M1" "algorithm" "Polygon representation + outline drawing + makeRect" \
"Implement \`makeRect\` and \`drawPolygonOutline\` in \`src/polygon/polygon.cpp\` (outline uses Caleb's Bresenham — stub with your own temporary line if his PR isn't merged yet).
$DONE"
issue alvin "$M1" "algorithm" "Scanline polygon fill" \
"Implement \`scanlineFill\` in \`src/polygon/fill.cpp\` (edge table / active edge list, handles concave polygons).
Demo \`demos/04_fill.cpp\`: convex + concave + star polygon filled.
$DONE"
issue alvin "$M1" "algorithm" "Flood fill and boundary fill (4-connected, iterative)" \
"Implement \`floodFill\` and \`boundaryFill\` with an explicit stack (no recursion → no stack overflow on large areas). Add to \`demos/04_fill.cpp\`.
$DONE"
issue alvin "$M2" "scene" "Crime scene render: room, desk, cabinet, door, safe body, rug" \
"Implement \`renderCrimeScene\` in \`src/scenes/crime_scene.cpp\` using only our primitives, polygons and fills. Highlight interactive objects with a distinct outline colour.
Demo → \`output/10_crime_scene.bmp\`.
$DONE"
issue alvin "$M2" "scene" "Character line-up: detective + 3 colour-coded suspects" \
"Implement \`renderCharacterLineup\` in \`src/scenes/characters.cpp\`: ellipses for heads/bodies, polygons for coats/hats, circles for eyes/buttons, a Bézier for the hat brim (use Latiffa's curves once merged). Each suspect gets a signature colour.
Demo → \`output/11_characters.bmp\`.
$DONE"

# --- Sudheysi ---
issue sudheysi "$M1" "algorithm" "Cohen–Sutherland line clipping" \
"Implement \`cohenSutherlandClip\` in \`src/polygon/clip.cpp\` with region outcodes.
Demo \`demos/05_line_clip.cpp\`: random lines before (grey) vs after clipping (red) against a rectangle.
$DONE"
issue sudheysi "$M1" "algorithm" "Sutherland–Hodgman polygon clipping" \
"Implement \`sutherlandHodgmanClip\` in \`src/polygon/clip.cpp\` (clip against each of the 4 edges in turn).
Demo \`demos/06_poly_clip.cpp\`: polygons partly outside a window, before vs after.
$DONE"
issue sudheysi "$M1" "algorithm" "Point-in-polygon test (ray casting)" \
"Implement \`pointInPolygon\` — we'll reuse it in weeks 7–8 for mouse picking and drag-and-drop. Demo: colour sample points green/red by inside/outside.
$DONE"
issue sudheysi "$M2" "scene" "Safe dial component (circle + ticks + rotation)" \
"Implement \`renderSafeDial(c, angle)\` in \`src/scenes/safe.cpp\`: outer ring (midpoint circle), numbered tick marks (lines), handle (polygon) rotated using Caleb's transforms.
Demo writes the dial at two angles → \`output/12_safe_dial_a.bmp\`, \`12_safe_dial_b.bmp\`.
$DONE"
issue sudheysi "$M2" "scene" "Torn document pieces (irregular polygons, scattered & rotated)" \
"Implement \`renderTornDocument\` in \`src/scenes/document.cpp\`: split a paper rectangle into 4–5 jagged polygons, scanline-fill them, then translate/rotate each to look scattered. Add a 'solved' render too.
Demo → \`output/13_document_scattered.bmp\`, \`13_document_solved.bmp\`.
$DONE"
issue sudheysi "$M2" "scene" "Clipping demo: scene clipped to viewport + magnifier window" \
"Implement \`renderClippingDemo\`: draw furniture partly outside the Crime Scene viewport and clip it; show a rectangular 'magnifier' region clipping a zoomed copy.
Demo → \`output/14_clipping_scene.bmp\`.
$DONE"

# --- Latiffa ---
issue latiffa "$M1" "algorithm" "Bézier curves (de Casteljau, any degree)" \
"Implement \`bezierPoint\`, \`drawBezier\`, \`drawControlPolygon\` in \`src/curves/bezier.cpp\`.
Demo \`demos/07_bezier.cpp\`: quadratic and cubic curves with control points visible.
$DONE"
issue latiffa "$M1" "algorithm" "Catmull-Rom spline" \
"Implement \`drawCatmullRom\` in \`src/curves/spline.cpp\` (curve passes through every point).
Demo \`demos/08_spline.cpp\`: a handwriting-style signature through ~10 points.
$DONE"
issue latiffa "$M2" "scene" "Evidence board: cork board, cards, pins, sagging Bézier strings" \
"Implement \`renderEvidenceBoard\` in \`src/scenes/evidence_board.cpp\`: cork background, evidence cards (filled rects), pins (filled circles), connections as quadratic Bézier strings that sag between pins.
Demo → \`output/15_evidence_board.bmp\`.
$DONE"
issue latiffa "$M2" "scene" "Case File panel (left UI) layout" \
"Implement \`renderCaseFilePanel\` in \`src/scenes/case_file.cpp\`: left panel with suspect slots (colour-coded circles) and evidence slots; combine with the crime scene into a full-window mock (\`output/16_full_layout.bmp\`).
$DONE"
issue latiffa "$M3" "docs" "Finalise README (algorithms, progress, limitations)" \
"Tick off the algorithms table, add the screenshot gallery from \`output/\`, confirm build instructions on a clean clone."
issue latiffa "$M3" "docs" "Screenshot PDF + character images + single Google Form submission" \
"Collect every image in \`output/\` into \`docs/checkpoint1_screenshots.pdf\`, one caption per image naming the algorithm. Submit the Google Form **once** for the whole group."
echo "✔ issues"

# ---------- Branches ----------
git fetch origin main --quiet
for b in caleb alvin sudheysi latiffa; do
  if git ls-remote --exit-code --heads origin "$b" >/dev/null 2>&1; then
    echo "  (branch $b exists)"
  else
    git push origin "origin/main:refs/heads/$b" --quiet && echo "  + branch $b"
  fi
done
echo "✔ branches — done."
