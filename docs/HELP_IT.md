# Lowline — Guida utente

## 01 · Inizia da qui
Lowline è un rig per basso: combina compressore, booster, due modelli NAM, cassa a risposta impulsiva (IR), equalizzatore, suono pulito parallelo, delay stereo e riverbero IR. Funziona come app autonoma oppure come plugin VST3; su Mac è disponibile anche AU.

Per una prima prova:
1. Collega il basso alla Scarlett, attiva INST sull'ingresso usato e abbassa il volume delle cuffie.
2. Nell'app autonoma apri Options / Audio settings e scegli Scarlett come ingresso e uscita. Se compare “Mute audio input”, disattivalo per sentire lo strumento.
3. Scegli Input 1 / Left oppure Input 2 / Right in Lowline, in base alla presa utilizzata.
4. Parti da 48 kHz e buffer di 128 o 256 campioni. Se il modello NAM richiede un'altra frequenza, imposta quella e ricaricalo.
5. Disattiva il direct monitoring della Scarlett per ascoltare soltanto il segnale elaborato.
6. Scegli Clean foundation. Puoi già suonare senza modelli: gli slot vuoti lasciano passare il segnale.
7. Carica un modello amp .nam e, se serve, una cassa IR. Attiva AMP ON e CAB ON, quindi regola AMP / CLEAN e OUTPUT.

Il verde indica un pulsante attivo; un pulsante scuro è spento. Le parole “ON” sono etichette fisse: guarda il colore per conoscere lo stato. Una spia attiva non significa che nello slot sia stato caricato un file.

Apri GUIDA dalla schermata principale. Scegli un argomento dall'indice; IT / EN cambia la lingua di questa guida. La guida è inclusa nell'app e funziona senza Internet.

## 02 · Percorso del segnale e interfaccia
Il percorso è:
Ingresso selezionato → INPUT → compressore → divisione in due rami.

Ramo elaborato: BOOST → NAM pedal → NAM amp → EQ → cassa IR.
Ramo pulito: CLEAN ALIGN → CLEAN PHASE.

AMP / CLEAN unisce i due rami; seguono delay stereo → riverbero IR → OUTPUT → limiter finale.

Il ramo pulito parte DOPO il compressore: “pulito” significa senza booster, NAM, EQ e cassa, ma può essere compresso. Delay e riverbero agiscono sul mix dei due rami.

Input 1 / Left usa il primo canale; Input 2 / Right usa il secondo, se disponibile. Stereo > Mono fa la media dei due ingressi: non conserva una sorgente stereo. Il rig parte da uno strumento mono; gli effetti e le IR stereo possono creare ampiezza all'uscita.

Trascina una manopola per cambiarla; fai doppio clic per ripristinare il valore predefinito del parametro. Puoi modificare direttamente il numero sotto la manopola e premere Invio.

SALVA e APRI gestiscono i preset .lowline. Il menu in alto a destra contiene quattro configurazioni iniziali di fabbrica. IT / EN nella schermata principale cambia la lingua dell'interfaccia; i nomi tecnici dei controlli restano in inglese.

## 03 · Ingresso, uscita e indicatori
INPUT: da −36 a +24 dB. Regola il segnale prima del compressore e dei modelli. Aumentarlo può far saturare maggiormente una cattura NAM: non equivale ad alzare soltanto il volume finale.

OUTPUT: da −36 a +12 dB. Regola il livello dopo gli effetti e prima del limiter. Parti da −6 dB e confronta i suoni a volume simile.

Prima regola il gain fisico della Scarlett: suonando forte, l'ingresso non deve andare in clipping. Abbassare OUTPUT in Lowline non corregge una distorsione già prodotta dall'ingresso della scheda.

IN e OUT mostrano i picchi dei blocchi audio in dBFS, non una misura di volume percepito. IN si riferisce al segnale in ingresso prima del gain software. Valori vicini a −180 dB indicano silenzio numerico. GR mostra la riduzione di guadagno del compressore in dB.

LIMIT indica che il limiter sta riducendo il livello. Abbassa OUTPUT, MAKEUP o il livello a monte se interviene spesso. Il limiter è stereo-linked, aggiunge circa 1 ms di lookahead e mira a picchi di ±0,98. Non è un true-peak limiter e non sostituisce una buona regolazione dei livelli.

BYPASS salta la catena del rig, conservando selezione dell'ingresso, INPUT, OUTPUT e protezione finale. Non è un confronto a volume automaticamente pareggiato. Il bypass del plugin nel programma ospite conserva la latenza dichiarata.

## 04 · Compressore: rendere il basso uniforme
COMP ON attiva il compressore prima della divisione pulito/elaborato.

THRESHOLD (−60…0 dB): soglia oltre la quale aumenta la compressione. Abbassala per comprimere più note.
RATIO (1:1…12:1): intensità della compressione; 1:1 non riduce la dinamica, 4:1 è un punto di partenza evidente ma gestibile.
ATTACK (0,5…100 ms): reazione all'attacco della nota. Più breve controlla il transiente; più lungo lascia passare più pizzicato.
RELEASE (10…800 ms): ritorno dopo la nota. Troppo breve può rendere instabile il suono; troppo lungo può attenuare anche le note successive.
MAKEUP (0…24 dB): recupera volume dopo la compressione. Non è automatico e può far intervenire il limiter.

Prova RATIO 4:1, ATTACK 15 ms e RELEASE 120 ms. Suona normalmente e regola THRESHOLD finché GR indica qualche dB sulle note più forti. Confronta acceso/spento, correggendo MAKEUP per evitare di preferire soltanto il suono più forte.

## 05 · Booster, modelli NAM e cassa
BOOST ON attiva una saturazione semplice prima dei modelli. BOOST (0…24 dB) aumenta la spinta e la distorsione; non è un controllo trasparente di volume. Parti con poco drive, soprattutto con bassi attivi.

I due slot NAM sono in serie:
• PEDAL: una cattura di pedale, prima dell'amplificatore.
• AMP: una cattura di amplificatore.

Premi il pulsante .nam della riga desiderata e scegli il file. PEDAL ON / AMP ON attiva o esclude quel modello. La “x” rimuove il file dallo slot; il pulsante ON conserva il file e cambia soltanto lo stato dell'effetto. Un modello vuoto viene attraversato senza elaborazione NAM.

CAB ospita una IR di cassa mono o stereo in formato WAV o AIFF, fino a 2 secondi. Caricala con il pulsante IR e attiva CAB ON. Una IR riproduce un filtro acustico, non la saturazione dinamica di un amplificatore.

Una cattura può già includere cassa e microfono. In quel caso prova CAB spento per evitare di filtrare due volte il suono. Controlla sempre la descrizione fornita dall'autore della cattura.

Sono accettati modelli .nam mono con ingresso e uscita mono. Il core supporta A2, ma compatibilità e carico CPU dipendono dalla cattura. Se la frequenza richiesta non coincide con quella del dispositivo, segui il messaggio di stato, cambia frequenza e ricarica il modello. Lowline non normalizza automaticamente il volume delle catture.

TONE3000 apre il sito nel browser: scarica tu il file e poi caricalo nello slot corretto. Non c'è un downloader integrato. Modelli e IR non sono inclusi.

## 06 · EQ e suono pulito parallelo
BASS / 100 Hz, MID / 700 Hz e TREBLE / 3.5 kHz regolano tre filtri a campana da −12 a +12 dB. Le frequenze sono fisse. L'EQ agisce soltanto sul ramo elaborato, prima della cassa IR.

AMP / CLEAN indica la percentuale del ramo elaborato:
• 0% = solo ramo pulito.
• 100% = solo ramo elaborato.
• 70% = 70% elaborato e 30% pulito.

Il nome AMP / CLEAN non significa che agisca soltanto sul modello amp: il ramo elaborato comprende booster, pedal, amp, EQ e cassa. Anche senza una cattura amp, gli altri effetti di quel ramo possono funzionare.

CLEAN PHASE inverte la polarità del ramo pulito. Se miscelando i due rami il basso perde corpo, confronta le due posizioni e conserva quella più piena.

CLEAN ALIGN ritarda il pulito da 0 a 20 ms per compensare ritardi presenti nelle catture o nelle IR. Parti da 0, usa temporaneamente un blend vicino al 50% e aumenta di pochissimo ascoltando le note basse. Non è un allineamento automatico; non serve compensare qui l'intera latenza della scheda. Nessun crossover separa le basse dal resto del suono pulito.

## 07 · Delay stereo
DELAY ON rende udibili le ripetizioni e invia nuove note al delay.

TIME (30…1500 ms): distanza tra le ripetizioni. Un valore più basso dà eco breve; uno più alto separa chiaramente le note.
FEEDBACK (0…85%): quanta parte dell'eco torna nel delay. Più alto significa più ripetizioni; usa cautela con parti dense.
HIGH CUT (800…10000 Hz): scurisce il percorso di feedback e quindi le ripetizioni successive. Valori bassi rendono la coda più morbida.
DELAY LEVEL (0…60%): quantità di eco aggiunta al suono diretto; non è un crossfade che elimina il segnale originale.
PING PONG: alterna le ripetizioni tra sinistra e destra. Per sentirlo chiaramente usa cuffie o uscita stereo.

Punto di partenza: TIME 380 ms, FEEDBACK 27%, DELAY LEVEL 18%, HIGH CUT 3500 Hz.

Il delay non si sincronizza automaticamente al tempo della DAW e non ha tap tempo. Per una semiminima usa TIME = 60000 / BPM; a 120 BPM sono 500 ms. Per una croma dimezza il valore; per una croma puntata usa il 75% del valore della semiminima.

Spegnere il delay interrompe l'invio di nuove note e ne sfuma l'uscita. Non è un sistema di cambio preset con code garantite: una memoria precedente può essere ancora presente se lo riattivi presto.

## 08 · Riverbero a convoluzione
Carica una IR di ambiente nel riquadro CONVOLUTION ROOM, poi attiva REVERB ON. Sono accettati WAV o AIFF mono/stereo fino a 20 secondi.

REVERB MIX (0…70%) miscela il segnale dopo il delay con la risposta dell'ambiente. Aumentandolo cresce il contributo riverberato e diminuisce quello diretto. Parti da 8–15% per non coprire l'articolazione del basso.

Senza una IR caricata, attivare REVERB ON non crea un riverbero. Il file determina il carattere e la durata della coda: non ci sono controlli separati di decay o dimensione della stanza.

La normalizzazione delle IR è gestita dal motore di convoluzione; non equivale a pareggiare automaticamente il volume dei modelli NAM. Il timing iniziale della IR viene conservato. Una IR lunga può richiedere più risorse: se senti interruzioni, prova una IR più corta o aumenta il buffer.

## 09 · Preset, file e trascinamento
SALVA crea un preset .lowline; APRI lo richiama. I preset e le sessioni della DAW memorizzano controlli, lingua e percorsi assoluti ai file NAM/IR. Non incorporano i modelli né le IR.

Per conservare un suono: tieni gli asset in una cartella stabile, salva il preset e annota quali file usi. Per trasferirlo su un altro computer copia anche gli asset, ricaricali dagli slot sul nuovo sistema e salva di nuovo il preset. Lo spostamento non è automatico.

La “x” cancella un file dallo slot. Un messaggio di errore può indicare un file mancante, un formato incompatibile o una frequenza errata: controlla la barra di stato in basso. Durante Loading attendi il completamento prima di valutare il suono. Cambiare cattura può produrre una discontinuità: fallo tra una frase e l'altra.

Trascinamento:
• Un .nam sulla finestra va nello slot AMP; per PEDAL usa il suo pulsante.
• Una IR nella zona superiore va a CAB; nella riga inferiore degli effetti va a ROOM.
• Un .lowline apre il preset.

Le quattro configurazioni di fabbrica sono punti di partenza: Clean foundation, Warm edge, Wide ambience e Direct / DI. Non caricano asset e mantengono selezione dell'ingresso e calibrazione INPUT/OUTPUT. Direct / DI usa il ramo pulito ma conserva il compressore; Wide ambience richiede una IR per il riverbero. Richiamare una configurazione può sovrascrivere le regolazioni sonore: salva prima il tuo lavoro.

## 10 · MIDI e pedaliere
Nell'app autonoma abilita il controller nelle impostazioni Audio/MIDI. Nel plugin instrada il MIDI dalla DAW a Lowline; la procedura dipende dal programma ospite. Lowline accetta tutti i canali MIDI.

Program Change 0…3 richiama, nell'ordine, i quattro preset di fabbrica. Alcune pedaliere li mostrano come 1…4: controlla il messaggio realmente trasmesso. Non richiama i preset .lowline salvati e gli altri numeri sono ignorati.

Control Change (CC):
20 = compressore
21 = booster
22 = NAM pedal
23 = NAM amp
24 = cassa IR
25 = delay
26 = riverbero
27 = bypass
11 = blend: 0 tutto pulito, 127 tutto elaborato
7 = OUTPUT: 0 corrisponde a −36 dB, 127 a +6 dB

Per CC 20…27: valori 0…63 spengono, 64…127 accendono. Configura un interruttore che alterni 0 e 127. Un pulsante momentaneo può tenere acceso l'effetto soltanto finché è premuto, secondo la sua programmazione.

Un pedale d'espressione che invia CC 11 può regolare il blend, ma Lowline non offre MIDI learn, rimappatura, calibrazione del pedale o tap tempo. I messaggi sono applicati all'inizio del blocco audio, non al singolo campione.

## 11 · Uso come plugin su Mac, Windows e Linux
Standalone è l'app autonoma. VST3 è il plugin per un host compatibile; AU è il formato Mac usato, per esempio, da GarageBand e Logic. Installa soltanto i formati che ti servono.

Su Mac le cartelle utente abituali sono:
~/Library/Audio/Plug-Ins/Components/ per Lowline.component (AU)
~/Library/Audio/Plug-Ins/VST3/ per Lowline.vst3
La cartella Library è nella tua Home; nel Finder usa Vai → Vai alla cartella per raggiungerla.

Dopo aver copiato il bundle, riavvia il programma ospite o avvia una nuova scansione dei plugin. Inserisci Lowline su una traccia audio del basso, seleziona l'ingresso corretto della Scarlett e abilita il monitoraggio della traccia. Audio device e buffer si impostano nella DAW quando usi il plugin. Evita di ascoltare contemporaneamente l'app autonoma, il direct monitoring e il monitoraggio della DAW.

Per usare il ping-pong e le IR stereo scegli un'uscita stereo. Con uscita mono i canali vengono sommati e l'ampiezza stereo si perde.

Su Windows usa un host VST3 e il driver del produttore della scheda, se disponibile. Su Linux usa un host VST3 compatibile e configura il sistema audio nell'app o nell'host.

Il pacchetto Mac è universale Apple Silicon/Intel ma non è firmato Developer ID né notarizzato. Se macOS ne blocca l'apertura, controlla l'avviso e le opzioni di Privacy e sicurezza soltanto dopo aver verificato la provenienza del download.

## 12 · Problemi comuni e limiti attuali
Non sento nulla: verifica cavo e INST, ingresso selezionato, dispositivo in/out, volume cuffie, mute dell'app e monitoraggio della traccia. Disattiva BYPASS per ascoltare gli effetti. Per una prova semplice rimuovi i modelli, disattiva effetti ed EQ e usa il ramo pulito.

Sento due bassi o un effetto “vuoto”: disattiva il direct monitoring; verifica che non stiano suonando sia standalone sia DAW. Se il problema compare solo col blend, prova CLEAN PHASE e CLEAN ALIGN.

Suono distorto: controlla prima il clipping della Scarlett; poi riduci INPUT, BOOST, MAKEUP e OUTPUT secondo il punto che satura. Un modello NAM può distorcere per sua natura. LIMIT segnala il limiter di uscita, non il clipping fisico dell'ingresso.

Crepitii o ritardo eccessivo: aumenta il buffer per ridurre le interruzioni, diminuiscilo se vuoi meno latenza e il computer regge. Prova catture più leggere e IR più corte. Il limiter aggiunge circa 1 ms, oltre a dispositivo, buffer e altri plugin. Il carico CPU non è visualizzato in Lowline.

Preset diverso o file mancanti: conserva i percorsi originali oppure ricarica gli asset e salva nuovamente. Nessun modello è incluso nel preset.

NAM non caricato: leggi la barra di stato, verifica file mono compatibile e sample rate richiesto. Il pulsante AMP ON da solo non carica un amplificatore.

Funzioni non ancora presenti: accordatore, gate, normalizzazione dei modelli, EQ a cinque bande, crossover del pulito, sync del delay, MIDI learn, preset con asset incorporati e cambi cattura senza discontinuità. Usa l'accordatore e gli strumenti del tuo host quando servono. Le build automatiche verificano il software, non la latenza reale della Scarlett né il risultato all'ascolto.
