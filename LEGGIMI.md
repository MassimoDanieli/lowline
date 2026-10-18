# Lowline — versione 0.1.1 per Massimo

Una pedaliera virtuale per basso, con compressore e clean blend, motore NAM e casse/riverberi IR. Applicazione autonoma e progetto per plugin AU/VST3, predisposto per macOS, Windows e Linux.

## Cosa trovi qui

Questo è il **progetto sorgente completo**, con dipendenze incluse e script per compilarlo. Non contiene ancora un'app Mac pronta da trascinare in Applicazioni. I risultati delle verifiche effettivamente eseguite sono in `VALIDATION.md`.

Le correzioni della versione 0.1.1 sono elencate in `CHANGELOG.md`. Ora l'uscita ha un limiter stereo con 1 ms di anticipo, compensato nella DAW; i modelli rimossi non si riattivano al cambio delle impostazioni audio e il richiamo dei preset è più coerente.

La grafica è originale: pannello scuro, accenti verde salvia e oro, controlli grandi e tutta la catena visibile. L'interfaccia permette di cambiare IT/EN; le etichette audio più comuni restano in inglese.

## Compilare sul tuo Mac

Apri Terminale:

1. Se non hai gli strumenti Apple: `xcode-select --install`, quindi completa l'installazione.
2. Installa CMake. Se usi già Homebrew: `brew install cmake`.
3. Entra nella cartella `lowline` estratta dallo ZIP.
4. Esegui `bash scripts/build-mac.command`.

Alla fine si apre la cartella dei risultati. In `Standalone` trovi `Lowline.app`; in `AU` il componente per GarageBand; in `VST3` il relativo plugin. Non serve una licenza Logic per l'app autonoma.

Il progetto include anche un workflow GitHub per compilare sulle tre piattaforme. Non è stato pubblicato né eseguito nel tuo account.

## Primo utilizzo con la Scarlett

- Basso nell'ingresso della Scarlett, modalità INST attiva.
- Dalle impostazioni audio dell'app scegli Scarlett come ingresso e uscita; abilita l'ingresso audio se risulta silenziato.
- Se il basso è nell'ingresso 1, seleziona `Input 1 / Left` in alto.
- Cuffie sulla Scarlett e direct monitor disattivato: il segnale pulito lo aggiunge già Lowline.
- Parti con 48 kHz e buffer 128 o 256 campioni, volume basso.
- Il caricamento NAM segnala se il modello richiede una frequenza diversa.

## I due controlli principali per basso

**Compressore:** controlli completi di soglia, rapporto, attacco, rilascio e recupero manuale del volume. La sigla GR indica la riduzione di guadagno.

**AMP / CLEAN:** a 0% ascolti il pulito, a 100% il ramo ampli/cassa. A metà li mescoli. Il pulito è preso dopo il compressore e viene unito prima di delay e riverbero. CLEAN PHASE inverte la polarità; CLEAN ALIGN ritarda il pulito se il modello o la cassa causano cancellazioni.

Il controllo del boost attiva una saturazione semplice. Per il suono di un vero ampli/pedale carica un file `.nam` negli slot dedicati. Per la cassa carica un IR WAV/AIFF; lo stesso vale per il riverbero. Non sono inclusi modelli commerciali o catture di terzi.

## Cosa manca ancora

- Build Mac verificata su hardware e Scarlett, firma/notarizzazione Apple.
- Ricerca TONE3000 interna: il pulsante apre il sito nel browser, poi carichi i file scaricati.
- MIDI learn e tap tempo: la prima versione ha associazioni CC fisse, descritte nel README.
- Cambio preset/catture senza discontinuità; è meglio caricarle tra una frase e l'altra.
- Oversampling del boost, conversione automatica della frequenza dei modelli, allineamento automatico delle catture.

I preset salvano i percorsi dei file: tieni NAM e IR in una cartella stabile. I preset iniziali modificano alcuni controlli e non caricano suoni. Il pulsante BYPASS mantiene selezione ingresso, guadagni e protezione finale del livello.

Per dettagli tecnici, licenze e compilazione su Windows/Linux, leggi `README.md`.
