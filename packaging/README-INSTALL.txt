RADICE / INSTALLAZIONE — INSTALLATION

ITALIANO
Radice si chiamava Lowline. Se avevi Lowline, elimina i vecchi Lowline.app,
Lowline.component e Lowline.vst3 (su Mac Install-Radice.command lo fa per te
nelle cartelle del tuo utente): due plugin con gli stessi codici confondono
le DAW. Le sessioni e i preset .lowline si aprono con Radice.

Mac: apri il DMG o estrai lo ZIP. Chiudi Radice e la tua DAW.
Puoi copiare manualmente Standalone/Radice.app in ~/Applications,
AU/Radice.component in ~/Library/Audio/Plug-Ins/Components e
VST3/Radice.vst3 in ~/Library/Audio/Plug-Ins/VST3.
In alternativa, Install-Radice.command esegue queste copie per il tuo utente,
senza sudo, conservando una copia di eventuali versioni precedenti.

Le note della Release dicono se l'app e firmata con Developer ID e notarizzata da
Apple: in quel caso si apre senza avvisi. Se non lo e e macOS blocca l'apertura, verifica che il pacchetto provenga dalla Release
ufficiale del progetto; consulta Privacy e sicurezza nelle Impostazioni di
Sistema per l'opzione di apertura consentita da macOS. Non disattivare Gatekeeper.
Se l'installer viene bloccato, usa la copia manuale descritta sopra.

Windows: estrai lo ZIP. L'app e in Standalone; copia il plugin VST3 nella
cartella VST3 riconosciuta dalla tua DAW. Il pacchetto non e firmato.
Linux: estrai il tar.gz. Esegui Standalone/Radice e copia il bundle VST3
in ~/.vst3. Sono richieste librerie audio/grafiche compatibili con Ubuntu 24.04.

ENGLISH
Radice was called Lowline. If you had Lowline, delete the old Lowline.app,
Lowline.component and Lowline.vst3 (on Mac Install-Radice.command does it
for you in your user folders): two plugins with the same codes confuse
hosts. Sessions and .lowline presets open in Radice.

Mac: open the DMG or extract the ZIP. Close Radice and your DAW.
Copy Standalone/Radice.app to ~/Applications,
AU/Radice.component to ~/Library/Audio/Plug-Ins/Components and
VST3/Radice.vst3 to ~/Library/Audio/Plug-Ins/VST3.
Alternatively, Install-Radice.command makes these copies for your user,
without sudo, retaining backups of any existing versions.

The Release notes state whether the app is Developer-ID signed and notarised by
Apple: then it opens without warnings. If not, and macOS blocks opening, verify that the package is from the project's official
Release, then consult Privacy & Security in System Settings for the opening
option allowed by macOS. Do not disable Gatekeeper. Use manual copying if the
installer is blocked.

Windows: extract the ZIP. Run the app in Standalone; copy the VST3 plugin to
your DAW's VST3 location. The package is unsigned.
Linux: extract the tar.gz, run Standalone/Radice and copy the VST3 bundle
to ~/.vst3. Compatible Ubuntu 24.04 audio/graphics libraries are required.

Downloads / aggiornamenti / updates:
https://github.com/MassimoDanieli/radice/releases/latest
Guides / guide:
https://github.com/MassimoDanieli/radice/blob/main/docs/HELP_IT.md
https://github.com/MassimoDanieli/radice/blob/main/docs/HELP_EN.md

NAM captures and IRs are not bundled. / Catture NAM e IR non inclusi.
