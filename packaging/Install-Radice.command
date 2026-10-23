#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ "$(uname -s)" != Darwin ]]; then echo 'This installer is for macOS.'; exit 1; fi
stamp=$(date +%Y%m%d-%H%M%S)
install_bundle() {
  local src="$1" dest="$2"
  if [[ ! -d "$src" ]]; then echo "Missing bundle: $src"; exit 1; fi
  mkdir -p "$(dirname "$dest")"
  if [[ -e "$dest" ]]; then mv "$dest" "$dest.backup-$stamp"; fi
  ditto "$src" "$dest"
}
echo 'Radice: installazione per il tuo utente / Installation for your user.'
echo 'Chiudi Radice (o Lowline) e la DAW prima di continuare. / Close Radice (or Lowline) and your DAW first.'
read -r -p 'Premi Invio per installare, Ctrl-C per annullare / Enter to install, Ctrl-C to cancel: '
# Radice was called Lowline until 0.4 and keeps its plugin codes, so DAW sessions still load it.
# Two bundles with the same codes confuse hosts: set the old Lowline bundles aside first.
for old in "$HOME/Applications/Lowline.app" \
           "$HOME/Library/Audio/Plug-Ins/Components/Lowline.component" \
           "$HOME/Library/Audio/Plug-Ins/VST3/Lowline.vst3"; do
  if [[ -e "$old" ]]; then
    mv "$old" "$old.backup-$stamp"
    echo "Lowline -> Radice: $old spostato in / moved to $old.backup-$stamp"
  fi
done
install_bundle 'Standalone/Radice.app' "$HOME/Applications/Radice.app"
install_bundle 'AU/Radice.component' "$HOME/Library/Audio/Plug-Ins/Components/Radice.component"
install_bundle 'VST3/Radice.vst3' "$HOME/Library/Audio/Plug-Ins/VST3/Radice.vst3"
echo 'Installato! / Installed!'
echo 'App: ~/Applications/Radice.app'
echo 'Le versioni precedenti sono conservate con suffisso .backup-data. / Old versions are backed up.'
if ! spctl --assess --type execute "$HOME/Applications/Radice.app" >/dev/null 2>&1; then
  echo 'App non notarizzata: consulta README-INSTALL.txt. / Not notarised: see README-INSTALL.txt.'
fi
read -r -p 'Premi Invio per chiudere / Press Enter to close: '
