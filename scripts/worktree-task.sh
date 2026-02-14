#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  scripts/worktree-task.sh create <task-or-branch>
  scripts/worktree-task.sh merge <task-or-branch>
  scripts/worktree-task.sh remove <task-or-branch>
  scripts/worktree-task.sh finish <task-or-branch>
  scripts/worktree-task.sh list
  scripts/worktree-task.sh path <task-or-branch>

Notes:
  - Task names become branches under codex/, e.g. "fix-menu" -> "codex/fix-menu".
  - Worktree folders are created next to the repo as "<repo-name>-<branch-with-dashes>".
EOF
}

repo_root="$(git rev-parse --show-toplevel)"
repo_parent="$(dirname "$repo_root")"
repo_name="$(basename "$repo_root")"

normalize_branch() {
  local input="${1:-}"
  if [[ -z "$input" ]]; then
    echo ""
    return
  fi
  if [[ "$input" == */* ]]; then
    echo "$input"
  else
    echo "codex/$input"
  fi
}

worktree_path_for_branch() {
  local branch="$1"
  local safe_branch="${branch//\//-}"
  echo "$repo_parent/$repo_name-$safe_branch"
}

command="${1:-}"
target="${2:-}"
branch="$(normalize_branch "$target")"
worktree_path=""
if [[ -n "$branch" ]]; then
  worktree_path="$(worktree_path_for_branch "$branch")"
fi

case "$command" in
  create)
    [[ -n "$branch" ]] || { usage; exit 1; }
    if [[ -d "$worktree_path" ]]; then
      echo "Worktree already exists: $worktree_path"
      exit 0
    fi
    if git show-ref --verify --quiet "refs/heads/$branch"; then
      git worktree add "$worktree_path" "$branch"
    else
      git worktree add "$worktree_path" -b "$branch"
    fi
    echo "Created: $worktree_path ($branch)"
    ;;
  merge)
    [[ -n "$branch" ]] || { usage; exit 1; }
    git -C "$repo_root" checkout main
    git -C "$repo_root" merge --no-ff "$branch"
    echo "Merged $branch into main"
    ;;
  remove)
    [[ -n "$branch" ]] || { usage; exit 1; }
    if [[ -d "$worktree_path" ]]; then
      git worktree remove "$worktree_path"
      echo "Removed worktree: $worktree_path"
    fi
    if git show-ref --verify --quiet "refs/heads/$branch"; then
      git branch -D "$branch"
      echo "Deleted branch: $branch"
    fi
    ;;
  finish)
    [[ -n "$branch" ]] || { usage; exit 1; }
    "$0" merge "$branch"
    "$0" remove "$branch"
    ;;
  list)
    git worktree list
    ;;
  path)
    [[ -n "$branch" ]] || { usage; exit 1; }
    echo "$worktree_path"
    ;;
  *)
    usage
    exit 1
    ;;
esac
