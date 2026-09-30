#!/bin/sh
set -eu
site=$(cd "${1:?site directory required}" && pwd)
root=$(git rev-parse --show-toplevel)
cd "$root"
for name in index.html .nojekyll LICENSE NOTICE; do
  test -f "$site/$name"
  test ! -L "$site/$name"
done
for path in "$site"/* "$site"/.[!.]* "$site"/..?*; do
  if test ! -e "$path" && test ! -L "$path"; then continue; fi
  case "${path##*/}" in
    index.html|.nojekyll|LICENSE|NOTICE) ;;
    *) echo "Unexpected site file: ${path##*/}" >&2; exit 1 ;;
  esac
done
previous=
if git ls-remote --exit-code origin refs/heads/gh-pages >/dev/null; then
  git fetch --no-tags origin refs/heads/gh-pages
  previous=$(git rev-parse FETCH_HEAD)
else
  status=$?
  if test "$status" -ne 2; then exit "$status"; fi
fi
index_dir=$(mktemp -d)
export GIT_INDEX_FILE="$index_dir/index"
git read-tree --empty
git --work-tree="$site" add -- index.html .nojekyll LICENSE NOTICE
tree=$(git write-tree)
if test -n "$previous" && test "$tree" = "$(git rev-parse "$previous^{tree}")"; then
  echo 'gh-pages already up to date'
  exit 0
fi
revision=$(git rev-parse HEAD)
if test -n "$previous"; then
  commit=$(git commit-tree "$tree" -p "$previous" -m '[DOCS] PUBLICATION DU SITE' -m "sources : $revision")
else
  commit=$(git commit-tree "$tree" -m '[DOCS] PUBLICATION DU SITE' -m "sources : $revision")
fi
git push origin "$commit:refs/heads/gh-pages"
git fetch --no-tags origin refs/heads/gh-pages
