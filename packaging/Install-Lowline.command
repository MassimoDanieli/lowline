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
echo 'Lowline: installazione per il tuo utente / Installation for your user.'
echo 'Chiudi Lowline e la DAW prima di continuare. / Close Lowline and your DAW first.'
read -r -p 'Premi Invio per installare, Ctrl-C per annullare / Enter to install, Ctrl-C to cancel: '
install_bundle 'Standalone/Lowline.app' "$HOME/Applications/Lowline.app"
install_bundle 'AU/Lowline.component' "$HOME/Library/Audio/Plug-Ins/Components/Lowline.component"
install_bundle 'VST3/Lowline.vst3' "$HOME/Library/Audio/Plug-Ins/VST3/Lowline.vst3"
echo 'Installato! / Installed!'
echo 'App: ~/Applications/Lowline.app'
echo 'Le versioni precedenti sono conservate con suffisso .backup-data. / Old versions are backed up.'
echo 'App non notarizzata: consulta README-INSTALL.txt. / Not notarised: see README-INSTALL.txt.'
read -r -p 'Premi Invio per chiudere / Press Enter to close: '
