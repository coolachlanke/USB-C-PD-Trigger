# from repo root

# 1) Untrack the already-committed junk (kept on disk, removed from Git)
git rm --cached -r \
  '#auto_saved_files#' \
  '_autosave-pdtrigger.kicad_pcb' \
  'fp-info-cache' \
  'pdtrigger-backups' \
  'pdtrigger.kicad_prl' \
  'libs.bak' \

# 2) Commit the removal + your .gitignore
git add .gitignore
git commit -m "chore: untrack KiCad autosave/lock/backup files"
