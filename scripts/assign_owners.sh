#!/usr/bin/env bash
# Assigns every open issue to its owner based on the owner:<name> label.
# Run after teammates have ACCEPTED their collaborator invites:
#   REPO=C-Sawe/the-last-clue GH_ALVIN=alvviinn GH_LATIFFA=latifahjaafar GH_SUDHEYSI=<username> \
#     bash scripts/assign_owners.sh
set -euo pipefail
: "${REPO:?Set REPO=owner/name}"

declare -A USERS=(
  [caleb]="${GH_CALEB:-C-Sawe}"
  [alvin]="${GH_ALVIN:-}"
  [sudheysi]="${GH_SUDHEYSI:-}"
  [latiffa]="${GH_LATIFFA:-}"
)

for who in "${!USERS[@]}"; do
  user="${USERS[$who]}"
  [[ -z "$user" ]] && { echo "skip $who (no username)"; continue; }
  if ! gh api "repos/$REPO/assignees/$user" >/dev/null 2>&1; then
    echo "skip $who: $user can't be assigned yet (invite not accepted?)"; continue
  fi
  for n in $(gh api "repos/$REPO/issues?state=open&labels=owner:$who&per_page=100" --jq '.[].number'); do
    gh api "repos/$REPO/issues/$n/assignees" -f "assignees[]=$user" >/dev/null && echo "  #$n → $user"
  done
done
