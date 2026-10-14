#!/usr/bin/env bash
# Adds the CP2 / CP3 / Final milestones and issues (lecturer's roadmap, 14 Oct 2026).
#   REPO=C-Sawe/the-last-clue GH_CALEB=C-Sawe GH_ALVIN=alvviinn GH_SUDHEYSI=Satasdin GH_LATIFFA=latifahjaafar \
#     bash scripts/roadmap_cp2_cp3_final.sh
# Safe to re-run: existing milestones/issues (same title) are skipped.
set -euo pipefail
: "${REPO:?Set REPO=owner/name}"

label() { gh api "repos/$REPO/labels" -f name="$1" -f color="$2" -f description="$3" >/dev/null 2>&1 || true; }
label "animation"   "e99695" "Transformations & animation (CP2)"
label "opengl"      "5319e7" "OpenGL integration (CP3)"
label "interaction" "006b75" "Mouse input, picking, dragging"
label "gameplay"    "b60205" "Puzzles, game state, deduction"

milestone() {
  if gh api "repos/$REPO/milestones?state=all&per_page=100" --jq '.[].title' | grep -Fxq "$1"; then
    echo "  = milestone '$1' exists"
  else
    gh api "repos/$REPO/milestones" -f title="$1" -f description="$2" >/dev/null && echo "  + milestone $1"
  fi
}
CP2="CP2 – Transformation & Animation (Week 10)"
CP3="CP3 – OpenGL Integration + Prototype (Week 12)"
FIN="Final Project (Week 13)"
milestone "$CP2" "Build on CP1 components with transformations and animation in Pure C++."
milestone "$CP3" "Port components to OpenGL and combine them into a functional, playable prototype."
milestone "$FIN" "Complete functionality, graphics, interaction, animation; demo-ready."

issue() {
  local who="$1" ms="$2" labels="$3" title="$4" body="$5" user=""
  case "$who" in
    caleb) user="${GH_CALEB:-}";; alvin) user="${GH_ALVIN:-}";;
    sudheysi) user="${GH_SUDHEYSI:-}";; latiffa) user="${GH_LATIFFA:-}";;
  esac
  if gh api "repos/$REPO/issues?state=all&per_page=100" --paginate --jq '.[].title' | grep -Fxq "$title"; then
    echo "  = [$who] $title (exists)"; return
  fi
  local ms_num; ms_num=$(gh api "repos/$REPO/milestones?state=all&per_page=100" --jq ".[] | select(.title==\"$ms\") | .number")
  local args=(-f title="$title" -f body="$body" -F milestone="$ms_num" -f "labels[]=owner:$who")
  IFS=',' read -ra extra <<< "$labels"; for l in "${extra[@]}"; do args+=(-f "labels[]=$l"); done
  # Only assign if GitHub accepts the user (invite accepted); otherwise label-only.
  if [[ -n "$user" ]] && gh api "repos/$REPO/assignees/$user" >/dev/null 2>&1; then args+=(-f "assignees[]=$user"); fi
  gh api "repos/$REPO/issues" "${args[@]}" >/dev/null && echo "  + [$who] $title"
}

PURE="**Pure C++ only** — builds on our own Canvas + algorithms, no graphics library."
DONE="**Done when:** merged into \`main\` via a reviewed PR, with output (frames/GIF or screenshot) committed to \`output/\`."

# ======================= CP2 — Transformation & Animation =======================
issue caleb "$CP2" "algorithm,animation" "Extend transform2d: reflection, shear, composite transforms" \
"Add \`reflectX/reflectY/reflectAbout(line)\`, \`shear(shx, shy)\` and a demo showing a composite (e.g. rotate→scale→translate) applied to a CP1 component. Document the matrix for each in comments. $PURE
$DONE"
issue caleb "$CP2" "animation" "Frame output + pure C++ animated GIF writer" \
"Render N frames from a \`renderFrame(Canvas&, double t)\` callback and save them as \`output/frames/<name>_###.bmp\` **and** an animated GIF written by our own LZW encoder (no library). This is what everyone uses to show animation evidence. $PURE
$DONE"
issue latiffa "$CP2" "animation" "Animation core: timeline, keyframes, easing (lerp, ease-in/out)" \
"\`src/animation/\`: \`lerp\`, easing curves, a \`Keyframe<T>\` track sampled by time, and tweening of position/angle/scale that outputs a \`Mat3\` from Caleb's transforms. Shared by every CP2 scene. $PURE
$DONE"
issue latiffa "$CP2" "animation,scene" "Evidence board animation: strings 'draw' along Bézier, cards slide in" \
"Animate t: 0→1 along each Bézier string so connections draw themselves; cards translate in from off-screen with easing; 'clue found' pulse (scale + colour flash). Output GIF. $PURE
$DONE"
issue latiffa "$CP2" "animation" "Scene transitions: fade and wipe between crime scene and puzzles" \
"Per-pixel colour interpolation for fade-to-black / cross-fade, and a clipped wipe (reuse Sutherland–Hodgman / clip rect). Output GIF. $PURE
$DONE"
issue alvin "$CP2" "animation,scene" "Detective walk cycle across the crime scene" \
"Hierarchical transforms: body translates across the room while arms/legs rotate about their joints (pivot rotation). Loop 2–3 s. Output GIF. $PURE
$DONE"
issue alvin "$CP2" "animation,scene" "Door / drawer / cabinet opening animations" \
"Door swings (scale-x about hinge or shear to fake perspective), drawer slides out (translate), highlighted object glows on discovery. Output GIF. $PURE
$DONE"
issue sudheysi "$CP2" "animation,gameplay" "Safe puzzle animation: dial turns through combination, door swings open" \
"Rotate the CP1 dial through a 3-number combination (with easing + tick 'click' pauses), then animate the safe door opening. Output GIF. $PURE
$DONE"
issue sudheysi "$CP2" "animation,gameplay" "Torn document reconstruction animation (pieces fly into place)" \
"Each piece interpolates translation + rotation from its scattered pose to the solved pose; add a snap + highlight at the end. Output GIF. $PURE
$DONE"
issue sudheysi "$CP2" "animation" "Magnifier zoom: scale about the cursor point with clipping" \
"Animate a zoom (scaleAbout pivot) of a region of the crime scene inside a circular/rect magnifier window, clipped with our clipping algorithms. Output GIF. $PURE
$DONE"
issue caleb "$CP2" "docs" "CP2 integration, README update, screenshots/GIFs, tag v0.2-checkpoint2" \
"All CP2 demos build from a clean clone, README 'techniques' table updated with transforms + animation, evidence PDF/GIFs collected, release tagged."

# ======================= CP3 — OpenGL + Prototype =======================
issue caleb "$CP3" "opengl,setup" "OpenGL setup: GLFW + GLAD via CMake FetchContent, window + render loop" \
"Cross-platform window, 60 fps loop, fixed-timestep update. Must build on every member's laptop (Windows/macOS) — test on all four."
issue caleb "$CP3" "opengl" "Bridge: display our Canvas in OpenGL (texture upload) + GL primitive backend" \
"Upload the CP1/CP2 pixel buffer as a texture on a fullscreen quad so our own algorithms still do the drawing; add an optional GL backend (VBO lines/triangles) for performance. Keeps the 'we implemented it' story intact."
issue sudheysi "$CP3" "interaction" "Mouse input: picking (point-in-polygon), drag-and-drop, boundary constraints" \
"Hover/click detection on scene objects using our pointInPolygon, drag puzzle pieces with the mouse, constrain them to the play area."
issue sudheysi "$CP3" "gameplay" "Game state machine: Explore → Puzzle → Evidence Board → Deduction" \
"States, transitions (use Latiffa's fade/wipe), and an evidence inventory data model."
issue sudheysi "$CP3" "gameplay" "Interactive safe + document puzzles with win validation" \
"Safe: drag to rotate dial, detect correct combination. Document: drag/rotate pieces, snap when within tolerance, detect solved state."
issue alvin "$CP3" "opengl,scene" "Port crime scene + characters into the OpenGL prototype" \
"Crime scene and suspects rendered in the window; interactive objects highlight on hover; character animations from CP2 play in-engine."
issue latiffa "$CP3" "opengl,interaction" "Case File panel + evidence board interaction (drag cards, draw connections)" \
"Live left panel listing suspects and collected evidence; on the board, drag cards and click two items to create a Bézier connection."
issue latiffa "$CP3" "opengl" "Bitmap font text rendering" \
"Simple glyph atlas or hand-built 5×7 bitmap font for labels, clue text and menus."
issue caleb "$CP3" "gameplay" "Playable prototype: one full investigation loop end-to-end" \
"Start at crime scene → find clue → solve one puzzle → evidence added to board → reach deduction screen. Tag v0.3-prototype."

# ======================= Final Project =======================
issue sudheysi "$FIN" "gameplay" "Final deduction (WHO / HOW / WHY) + Case Solved / Case Failed outcomes" \
"Deduction screen with three choices driven by connected evidence; win/lose screens with animation."
issue alvin "$FIN" "scene,gameplay" "Additional puzzle art: fingerprint matching (concentric ellipses)" \
"Fingerprint cards built from ellipse ridges for the suspect line-up; Sudheysi wires the matching logic. Brings us to the required 4–6 puzzles."
issue sudheysi "$FIN" "gameplay" "Additional puzzle logic: fingerprint matching + hidden object / cipher" \
"Logic for the fingerprint puzzle and one more (hidden-object or cipher) so the case has 4–6 puzzles total."
issue alvin "$FIN" "scene" "Visual polish: colour coding, highlights, consistent palette" \
"Suspect colours, correct/incorrect feedback colours, evidence category colours applied consistently."
issue latiffa "$FIN" "animation" "Final animations: transitions, feedback, win/lose sequences" \
"Polish all transitions and feedback animations; make sure nothing pops without easing."
issue latiffa "$FIN" "docs" "Final README, report and demo video" \
"Full documentation of every algorithm/technique and where it is used; 2–3 min gameplay video."
issue caleb "$FIN" "setup" "Bug bash, performance, clean build on all machines, tag v1.0" \
"Everyone plays through; fix issues; final release tag and demo preparation."

echo "✔ roadmap done"
