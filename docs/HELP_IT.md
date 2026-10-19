# Lowline 0.3 — Guida utente

## 01 · Inizia da qui
Lowline è un rig per basso: combina gate, compressore, booster, due modelli NAM, cassa a risposta impulsiva (IR), equalizzatore, suono pulito parallelo, delay stereo e riverbero IR. Funziona come app autonoma oppure come plugin VST3; su Mac è disponibile anche AU.

Per una prima prova:
1. Collega il basso alla Scarlett, attiva INST sull'ingresso usato e abbassa il volume delle cuffie.
2. Nell'app autonoma apri Options / Audio settings e scegli Scarlett come ingresso e uscita. Se compare “Mute audio input”, disattivalo per sentire lo strumento.
3. Scegli Input 1 / Left oppure Input 2 / Right in Lowline, in base alla presa utilizzata.
4. Parti da 48 kHz e buffer di 128 o 256 campioni. Se il modello NAM richiede un'altra frequenza, imposta quella e ricaricalo.
5. Disattiva il direct monitoring della Scarlett per ascoltare soltanto il segnale elaborato.
6. Scegli Clean foundation. Puoi già suonare senza modelli: gli slot vuoti lasciano passare il segnale.
7. Carica un modello amp .nam e, se serve, una cassa IR. Attiva AMP ON e CAB ON, quindi regola AMP / CLEAN e OUTPUT.

Il verde indica un pulsante attivo; un pulsante scuro è spento. Le parole “ON” sono etichette fisse: guarda il colore per conoscere lo stato. Una spia attiva non significa che nello slot sia stato caricato un file.

Le schede RIG, STRUMENTI e MIDI separano catena audio, strumenti di regolazione e pedaliere. In STRUMENTI trovi accordatore, gate, EQ, crossover, normalizzazione e tempo del delay.

Apri GUIDA dalla schermata principale. Scegli un argomento dall'indice; ENGLISH / ITALIANO cambia la lingua di questa guida. La guida è inclusa nell'app e funziona senza Internet.

## 02 · Percorso del segnale e interfaccia
Il percorso è:
Ingresso selezionato → INPUT → gate → compressore → divisione in due rami.

Ramo elaborato: BOOST → NAM pedal → NAM amp → EQ → cassa IR.
Ramo pulito: filtro CLEAN LP opzionale → CLEAN ALIGN → CLEAN PHASE.

AMP / CLEAN unisce i due rami; seguono delay stereo → riverbero IR → OUTPUT → compensazione A/B temporanea → base e metronomo → limiter finale → mute accordatore. La registrazione FX è prelevata prima di base, metronomo e limiter; la DI prima di INPUT.

Il ramo pulito parte DOPO gate e compressore: “pulito” significa senza booster, NAM, EQ e cassa, ma può essere compresso. Delay e riverbero agiscono sul mix dei due rami.

Input 1 / Left usa il primo canale; Input 2 / Right usa il secondo, se disponibile. Stereo > Mono fa la media dei due ingressi: non conserva una sorgente stereo. Il rig parte da uno strumento mono; gli effetti e le IR stereo possono creare ampiezza all'uscita.

Trascina una manopola per cambiarla; fai doppio clic per ripristinare il valore predefinito del parametro. Puoi modificare direttamente il numero sotto la manopola e premere Invio.

SALVA e APRI gestiscono i preset .lowline. ESPORTA crea un preset portabile con modelli e IR incorporati. Il menu in alto a destra contiene quattro configurazioni iniziali di fabbrica. IT / EN nella schermata principale cambia la lingua dell'interfaccia; i nomi tecnici dei controlli restano in inglese.

## 03 · Livelli, CPU e accordatore
INPUT: da −36 a +24 dB. Regola il segnale prima del compressore e dei modelli. Aumentarlo può far saturare maggiormente una cattura NAM: non equivale ad alzare soltanto il volume finale.

OUTPUT: da −36 a +12 dB. Regola il livello dopo gli effetti e prima del limiter. Parti da −6 dB e confronta i suoni a volume simile.

Prima regola il gain fisico della Scarlett: suonando forte, l'ingresso non deve andare in clipping. Abbassare OUTPUT in Lowline non corregge una distorsione già prodotta dall'ingresso della scheda.

IN e OUT mostrano i picchi dei blocchi audio in dBFS, non una misura di volume percepito. IN si riferisce al segnale in ingresso prima del gain software. Valori vicini a −180 dB indicano silenzio numerico. GR mostra la riduzione di guadagno del compressore in dB.

LIMIT indica che il limiter sta riducendo il livello. Abbassa OUTPUT, MAKEUP o il livello a monte se interviene spesso. Il limiter è stereo-linked, aggiunge circa 1 ms di lookahead e mira a picchi di ±0,98. Non è un true-peak limiter e non sostituisce una buona regolazione dei livelli.

BYPASS salta la catena del rig, conservando selezione dell'ingresso, INPUT, OUTPUT e protezione finale. Non è un confronto a volume automaticamente pareggiato. Il bypass del plugin nel programma ospite conserva la latenza dichiarata.

CPU mostra il tempo impiegato dal callback audio come percentuale del tempo disponibile per ogni blocco, con una media smussata. Non è l'utilizzo totale del processore: un picco vicino o oltre 100% può causare interruzioni. Durante un cambio cattura vengono elaborati temporaneamente due motori.

ACCORDATORE (scheda STRUMENTI): attiva TUNER e suona una corda alla volta. Mostra nota, ottava, frequenza e scostamento in cent; ±3 cent è evidenziato in verde. A4 REFERENCE regola il riferimento da 430 a 450 Hz (default 440). Sono ricercate fondamentali da 25 a 500 Hz, compreso il Si basso di un cinque corde. Il rilevamento è monofonico: armonici molto forti, rumore e accordi possono confonderlo. Lascia stabilizzare la nota per una frazione di secondo.

MUTE silenzia gradualmente l'uscita soltanto quando TUNER è attivo. Spegnere TUNER ripristina il suono anche se MUTE resta selezionato. L'accordatore legge l'ingresso prima di gate ed effetti; il suo lavoro di analisi avviene in un thread separato.

## 04 · Gate e compressore: rendere il basso uniforme
GATE ON, in STRUMENTI, attenua il rumore nelle pause prima del compressore. GATE THRESHOLD (−80…−10 dB) è la soglia d'apertura; GATE RELEASE (20…1000 ms) determina la chiusura graduale. L'apertura è di circa 2 ms, con mantenimento di 40 ms e isteresi di 3 dB per evitare oscillazioni vicino alla soglia.

Parti da −55 dB e 150 ms. Smetti di suonare e alza lentamente la soglia fino a ridurre il rumore; poi verifica che le note deboli e il sustain non vengano tagliati. Il gate è inizialmente spento per mantenere compatibilità con i vecchi preset.

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

Sono accettati modelli .nam mono con ingresso e uscita mono. Il core supporta A2, ma compatibilità e carico CPU dipendono dalla cattura. Se la frequenza richiesta non coincide con quella del dispositivo, segui il messaggio di stato, cambia frequenza e ricarica il modello. NORMALISE, nella scheda STRUMENTI, abilita una compensazione del volume basata sul campo loudness del modello NAM. NAM TARGET imposta il riferimento da −30 a −6 dB; la correzione per ciascun modello è limitata a ±18 dB e viene applicata gradualmente. Il valore di default è −18 dB.

PEDAL e AMP mostrano il loudness dichiarato; “nessun dato” significa che la cattura è vuota o non include un dato valido. In quel caso Lowline lascia il guadagno del modello invariato: non inventa una misura e non fa un'analisi automatica del volume percepito. La compensazione del PEDAL modifica anche il livello che entra nell'AMP, quindi può cambiarne la saturazione. La calibrazione fisica d'ingresso in dBu resta manuale.

Il cambio NAM/IR usa una dissolvenza di 50 ms fra il motore precedente e quello nuovo, preparato fuori dal callback audio. Se arrivano molte richieste, viene conservata quella più recente in attesa. Il carico aumenta temporaneamente; la dissolvenza non garantisce che ogni combinazione di catture, fase e carico CPU risulti inudibile. Una coda del riverbero precedente può continuare; vedi PALCO per limiti e comportamento.

TONE3000 apre il sito nel browser: scarica tu il file e poi caricalo nello slot corretto. Non c'è un downloader integrato. Modelli e IR non sono inclusi.

## 06 · EQ e suono pulito parallelo
L'EQ è nella scheda STRUMENTI. Cinque filtri a campana regolano il guadagno da −12 a +12 dB: BASS (100 Hz), LOW MID (default 250 Hz), MID (default 700 Hz), HIGH MID (default 1500 Hz) e TREBLE (3500 Hz).

LOW MID FREQ varia da 80 a 800 Hz; MID FREQ da 150 a 2500 Hz; HIGH MID FREQ da 500 a 6000 Hz. Le bande possono sovrapporsi: usa piccoli cambiamenti e confronta il volume. Le due bande aggiuntive partono da 0 dB, e i vecchi preset conservano le regolazioni originali. L'EQ agisce soltanto sul ramo elaborato, prima della cassa IR.

AMP / CLEAN indica la percentuale del ramo elaborato:
• 0% = solo ramo pulito.
• 100% = solo ramo elaborato.
• 70% = 70% elaborato e 30% pulito.

Il nome AMP / CLEAN non significa che agisca soltanto sul modello amp: il ramo elaborato comprende booster, pedal, amp, EQ e cassa. Anche senza una cattura amp, gli altri effetti di quel ramo possono funzionare.

CLEAN PHASE inverte la polarità del ramo pulito. Se miscelando i due rami il basso perde corpo, confronta le due posizioni e conserva quella più piena.

CLEAN ALIGN ritarda il pulito da 0 a 20 ms per compensare ritardi presenti nelle catture o nelle IR. Parti da 0, usa temporaneamente un blend vicino al 50% e aumenta di pochissimo ascoltando le note basse. Non è un allineamento automatico; non serve compensare qui l'intera latenza della scheda. CLEAN LP attiva un passa-basso a 12 dB/ottava sul solo ramo pulito. CLEAN LOW PASS sceglie il taglio fra 50 e 1000 Hz (default 180). Serve a conservare il fondo pulito mentre il ramo elaborato fornisce carattere e armonici. Il ramo elaborato resta a banda intera: non è una separazione complementare delle frequenze. A 0% di blend e CLEAN LP attivo sentirai soltanto il pulito filtrato.

## 07 · Delay stereo
DELAY ON rende udibili le ripetizioni e invia nuove note al delay.

TIME (30…1500 ms): distanza tra le ripetizioni. Un valore più basso dà eco breve; uno più alto separa chiaramente le note.
FEEDBACK (0…85%): quanta parte dell'eco torna nel delay. Più alto significa più ripetizioni; usa cautela con parti dense.
HIGH CUT (800…10000 Hz): scurisce il percorso di feedback e quindi le ripetizioni successive. Valori bassi rendono la coda più morbida.
DELAY LEVEL (0…60%): quantità di eco aggiunta al suono diretto; non è un crossfade che elimina il segnale originale.
PING PONG: alterna le ripetizioni tra sinistra e destra. Per sentirlo chiaramente usa cuffie o uscita stereo.

Punto di partenza: TIME 380 ms, FEEDBACK 27%, DELAY LEVEL 18%, HIGH CUT 3500 Hz.

SYNC, in STRUMENTI, calcola il ritardo da TEMPO e dalla suddivisione scelta: 1/16, 1/8, 1/8 puntata, 1/4, 1/4 puntata o 1/2. A 120 BPM, 1/4 vale 500 ms e 1/8 puntata 375 ms. Con SYNC attivo TIME viene ignorato; la durata effettiva è mostrata sotto il selettore.

HOST BPM usa il tempo comunicato dalla DAW, se disponibile e compreso fra 30 e 300 BPM. In standalone o senza un tempo valido si usa TEMPO (30…240 BPM). Il testo DAW/Manual indica quale sorgente è in uso. TAP ricava il tempo dall'intervallo fra due pressioni, fra 250 e 2000 ms: premi regolarmente almeno due volte. Se HOST BPM sta fornendo un tempo valido, disattivalo per usare quello battuto. TAP è assegnabile anche via MIDI learn.

DELAY LOW CUT (0…1000 Hz) riduce le basse delle ripetizioni e del feedback; 0 lo disattiva. Prova 100–200 Hz se gli echi impastano il basso. HIGH CUT rimane nella scheda RIG. Il ritardo sincronizzato può superare i 1500 ms del controllo manuale.

Spegnere il delay interrompe l'invio di nuove note e ne sfuma l'uscita. Non è un sistema di cambio preset con code garantite: una memoria precedente può essere ancora presente se lo riattivi presto.

## 08 · Riverbero a convoluzione
Carica una IR di ambiente nel riquadro CONVOLUTION ROOM, poi attiva REVERB ON. Sono accettati WAV o AIFF mono/stereo fino a 20 secondi.

REVERB MIX (0…70%) miscela il segnale dopo il delay con la risposta dell'ambiente. Aumentandolo cresce il contributo riverberato e diminuisce quello diretto. Parti da 8–15% per non coprire l'articolazione del basso.

Senza una IR caricata, attivare REVERB ON non crea un riverbero. Il file determina il carattere e la durata della coda: non ci sono controlli separati di decay o dimensione della stanza.

La normalizzazione delle IR è gestita dal motore di convoluzione; non equivale a pareggiare automaticamente il volume dei modelli NAM. Il timing iniziale della IR viene conservato. Una IR lunga può richiedere più risorse: se senti interruzioni, prova una IR più corta o aumenta il buffer.

## 09 · Preset, file e trascinamento
SALVA crea un preset .lowline leggero con controlli, lingua, mappature MIDI, abbinamenti Program Change e percorsi assoluti ai file NAM/IR. Le sessioni della DAW conservano lo stesso stato. APRI richiama sia preset normali sia portabili.

ESPORTA crea un .lowline portabile che include copie dei modelli e delle IR caricate nei quattro slot. Il limite è 64 MB per asset e 192 MB totali; se manca un file o supera il limite, l'esportazione viene annullata. Attendi che Loading sia terminato prima di salvare. Incorpora e condividi soltanto asset che hai diritto di ridistribuire.

All'apertura gli asset incorporati vengono estratti in una cache persistente Lowline/Assets nella cartella dati dell'utente. Non servono più i percorsi originali. I successivi salvataggi normali fanno riferimento alla cache: usa nuovamente ESPORTA per trasferirli. I percorsi verso altri preset assegnati ai Program Change non sono incorporati ricorsivamente; trasferisci anche quei preset e riassegnali se necessario.

Per conservare un suono: tieni gli asset in una cartella stabile, salva il preset e annota quali file usi. Per trasferire un preset normale copia anche gli asset e ricaricali; in alternativa usa ESPORTA per ottenere un unico file portabile.

La “x” cancella un file dallo slot. Un messaggio di errore può indicare un file mancante, un formato incompatibile o una frequenza errata: controlla la barra di stato in basso. Durante Loading attendi il completamento prima di valutare il suono. Il cambio cattura usa una dissolvenza di 50 ms. Un preset completo applica i controlli e prepara le catture come gruppo: non è un cambio scena atomico con tutte le code preservate.

Trascinamento:
• Un .nam sulla finestra va nello slot AMP; per PEDAL usa il suo pulsante.
• Una IR nella zona superiore va a CAB; nella riga inferiore degli effetti va a ROOM.
• Un .lowline apre il preset.

Le quattro configurazioni di fabbrica sono punti di partenza: Clean foundation, Warm edge, Wide ambience e Direct / DI. Non caricano asset e mantengono selezione dell'ingresso e calibrazione INPUT/OUTPUT. Direct / DI usa il ramo pulito ma conserva il compressore; Wide ambience richiede una IR per il riverbero. Richiamare una configurazione può sovrascrivere le regolazioni sonore: salva prima il tuo lavoro.

## 10 · MIDI e pedaliere
Nell'app autonoma abilita il controller nelle impostazioni Audio/MIDI. Nel plugin instrada il MIDI dalla DAW a Lowline; la procedura dipende dal programma ospite. Lowline accetta tutti i canali MIDI.

Program Change 0…3 richiama, nell'ordine, i quattro preset di fabbrica. Alcune pedaliere li mostrano come 1…4: controlla il messaggio realmente trasmesso. Nella scheda MIDI puoi assegnare un file .lowline a qualsiasi PC 0…127: scegli il numero, premi ASSEGNA PRESET e seleziona il file. CLEAR PC rimuove l'abbinamento. Un abbinamento personale prevale sul preset di fabbrica; i numeri senza abbinamento oltre 3 vengono ignorati. Il caricamento avviene nel worker, quindi include il tempo necessario a leggere e preparare i file.

SALVA conserva la banca nel preset/sessione corrente. Un richiamo tramite PC mantiene la banca e il MIDI learn correnti, così il preset caricato non cancella i comandi della pedaliera. APRI manualmente ripristina invece le mappature salvate nel file.

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

MIDI LEARN: scegli il parametro nel menu, premi LEARN e muovi il controllo della pedaliera. Il prossimo CC ricevuto viene associato al parametro; la precedente associazione di quel parametro viene rimossa. CANCEL annulla l'attesa, CLEAR rimuove l'associazione del parametro selezionato e RESET ripristina la mappa iniziale riportata sopra. La lista mostra i CC assegnati.

I controlli continui usano l'intervallo completo del parametro con valori 0…127; quindi OUTPUT appreso via learn raggiunge +12 dB, mentre il CC 7 predefinito mantiene il limite storico di +6 dB. I pulsanti usano 0…63 spento e 64…127 acceso. Tap tempo reagisce al passaggio dal valore basso a quello alto: la pedaliera deve inviare anche il rilascio.

Un pedale d'espressione può controllare blend, output o altri parametri. Calibra il suo intervallo sulla pedaliera: Lowline non offre ancora calibrazione min/max o curve personalizzate. Le associazioni accettano tutti i canali; i messaggi si applicano all'inizio del blocco audio, non al singolo campione.

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

Crepitii o ritardo eccessivo: aumenta il buffer per ridurre le interruzioni, diminuiscilo se vuoi meno latenza e il computer regge. Prova catture più leggere e IR più corte. Il limiter aggiunge circa 1 ms, oltre a dispositivo, buffer e altri plugin. Controlla anche il valore CPU, che misura il tempo di elaborazione audio rispetto al budget del blocco.

Preset diverso o file mancanti: conserva i percorsi originali oppure ricarica gli asset e salva nuovamente. SALVA usa percorsi; ESPORTA include i modelli e le IR.

NAM non caricato: leggi la barra di stato, verifica file mono compatibile e sample rate richiesto. Il pulsante AMP ON da solo non carica un amplificatore.

Limiti attuali: normalizzazione NAM solo quando è presente un loudness valido; calibrazione d'ingresso e dei pedali d'espressione manuale; nessuna garanzia di cambio scena atomico o mantenimento completo delle code. La dissolvenza di 50 ms evita il cambio secco del motore ma aumenta temporaneamente il costo CPU. L'accordatore è monofonico. Nessun modello o IR è fornito di serie.

Le build automatiche verificano il software con segnali sintetici e modelli di prova. Latenza, drop-out, accuratezza su bassi reali e risultato all'ascolto devono essere provati sulla Scarlett e nel tuo host. Inizia con buffer 256 e volume basso; prova poi 128 se il sistema rimane stabile.

## 13 · Libreria, preferiti e preset
Apri AREA STUDIO → LIBRERIA. AGGIUNGI FILE inserisce catture NAM, IR o preset .lowline; SCANSIONA CARTELLA cerca anche nelle sottocartelle, senza seguire collegamenti simbolici, fino a 10.000 file per scansione. La scansione avviene in background. I file non vengono copiati: non spostarli dopo averli indicizzati. Una voce rossa [MISSING] segnala un percorso non più disponibile; rimuovila dall'elenco e aggiungi il nuovo percorso.

Scegli NAM, IR o Preset e cerca nel nome, nel percorso o nei tag. PREFERITO +/- modifica la stella; Solo preferiti filtra l'elenco e Ordina per recenti porta in alto gli ultimi elementi caricati. Doppio clic o CARICA apre la selezione; le frecce percorrono l'ordine corrente. Il selettore Pedal/Amp/Cab/Room sceglie la destinazione: un NAM viene sempre indirizzato a uno slot NAM e un IR a uno slot IR. MOSTRA FILE apre la cartella nel sistema operativo. RIMUOVI DALLA LISTA non cancella il file.

Il campo accanto a TAG VOCE assegna tag alla voce selezionata. I tag del catalogo sono locali; quelli del preset sono separati. Caricando un preset si importano nel catalogo il suo nome e i suoi tag. Il catalogo viene conservato nei dati utente di Lowline; preferiti e recenti non vengono incorporati nei preset portabili. Evita modifiche simultanee del catalogo in più istanze: viene mantenuto l'ultimo salvataggio.

A destra puoi modificare nome, tag e descrizione del preset CORRENTE. Premi APPLICA DATI PRESET, poi SALVA o ESPORTA nel rig. Questi dati accompagnano il file .lowline. ANNULLA/RIPRISTINA agisce sui controlli e sui dati editoriali; non annulla il caricamento dei file, la registrazione o le operazioni del catalogo. Ogni trascinamento di una manopola apre una nuova transazione. La cronologia è temporanea. L'asterisco nella pagina PALCO indica modifiche rispetto all'ultimo salvataggio/apertura; i controlli dello studio e la compensazione A/B sono temporanei e non fanno parte di quel preset.

## 14 · Confronto A/B e volume misurato
AREA STUDIO → CONFRONTO A/B conserva due istantanee temporanee di controlli, mappa MIDI e percorsi delle quattro catture. MEMORIZZA A/B sostituisce la rispettiva istantanea; ASCOLTA A/B la richiama. Attendi il caricamento dei modelli. Le istantanee non contengono i file e non sopravvivono alla chiusura dell'istanza: usa SALVA per conservarle. Non sono due rig elaborati continuamente in parallelo.

Per confrontare il volume: imposta il suono A, premi MISURA 5 SECONDI e suona una frase rappresentativa. Alla fine premi MEMORIZZA A. Ripeti con B. Dopo ogni richiamo misura nuovamente la stessa frase e scegli ALLINEA AD A/B per compensare il livello rispetto al riferimento memorizzato. Usa lo stesso basso, gain e frase: una diversa esecuzione altera il risultato. Il numero RMS è misurato sul canale sinistro del basso elaborato, prima della compensazione, della base, del metronomo e del limiter finale.

La compensazione è limitata a ±12 dB e rifiuta misure inferiori a −60 dB RMS. Non è una misura LUFS né un'analisi percettiva. La manopola consente un'aggiustatura manuale; AZZERA COMPENSAZIONE ritorna a 0 dB. Il richiamo A/B azzera il trim. Il trim influenza ascolto e registrazione FX, resta fuori dai preset e non modifica la spinta all'ingresso dei modelli. La misura non sostituisce la normalizzazione NAM basata sui metadati.

## 15 · Basi, loop e metronomo
AREA STUDIO → STUDIO carica una base WAV/AIFF mono o stereo. Limiti: dieci minuti e 16 milioni di campioni per canale; a 48 kHz il secondo limite equivale a circa 5 minuti e 33 secondi. Non tutti i file di dieci minuti rientrano quindi nel limite. Il file è decodificato in memoria fuori dal callback audio. Base e metronomo vengono aggiunti dopo la catena del basso e passano nel limiter finale e nel mute dell'accordatore. I rispettivi volumi sono indipendenti da OUTPUT del basso.

PLAY/PAUSA controlla la riproduzione. STOP/INIZIO ferma e torna all'inizio. POSIZIONE usa sempre i secondi del file originale. INIZIO LOOP = ORA e FINE LOOP = ORA fissano i due punti; attiva LOOP ON/OFF per ripetere un intervallo di almeno circa 50 ms. Il loop ha brevi rampe ai bordi, ma non riconosce automaticamente battute o transienti. La fine deve seguire l'inizio. Prima di cambiare intervallo, metti in pausa e posizionati nel punto desiderato.

VELOCITÀ varia fra 0,50 e 1,50. Premi APPLICA VELOCITÀ: il file viene rielaborato in background con WSOLA, mantenendo approssimativamente la tonalità anziché cambiare semplicemente frequenza di riproduzione. Al completamento playback, posizione e punti loop vengono azzerati. Sui transienti o su materiale complesso possono comparire artefatti; non è uno stretching di qualità da mastering. Durante la preparazione può continuare la vecchia base. Velocità e loop non sono salvati nei preset.

Il metronomo ha tempo 30–300 BPM, volume, misura da 2 a 7 pulsazioni e tre scelte di accento: nessuno, primo battito, oppure 2 e 4. Per il walking bass scegli misura 4 e accenti 2 e 4. Parte dal primo battito quando viene attivato ed è indipendente da base, tempo host e TAP del delay: non sincronizza automaticamente una canzone. Il numero Beat mostra la pulsazione corrente. In un plugin, base e click suonano soltanto quando l'host continua a elaborare audio.

## 16 · Registrazione rapida DI + FX
REGISTRA DI + FX apre la scelta di nome e cartella. Dal nome scelto vengono creati due file: nome-DI.wav mono e nome-FX.wav stereo. Entrambi sono WAV a 32 bit float alla frequenza corrente dell'interfaccia/host. Il float conserva anche valori oltre 0 dBFS: abbassa il volume quando li importi in un altro programma. Lowline rifiuta di sovrascrivere una coppia già esistente.

DI contiene l'ingresso selezionato prima di INPUT, gate, compressore ed effetti. FX contiene il basso elaborato e l'eventuale trim A/B, prima del limiter finale e del mute dell'accordatore. Base e metronomo non sono inclusi: sono strumenti di ascolto durante la registrazione. La coppia non include la latenza hardware; alcuni modelli/IR possono introdurre propri ritardi. Usa DI per rielaborare il suono in seguito nella tua DAW, con Lowline come plugin.

FERMA REGISTRAZIONE svuota le code di scrittura e finalizza i WAV. Anche un cambio di configurazione audio o la chiusura dell'istanza interrompe la registrazione. La scrittura su disco è delegata a un thread; il contatore dropped blocks segnala eventuali blocchi persi per sovraccarico. Se è diverso da zero, la take potrebbe avere interruzioni o disallineamenti: ripeti con un buffer maggiore e un disco disponibile. Registra una breve prova prima di una take importante. Il bypass dell'host non alimenta queste funzioni; evita di attivarlo durante una take. Il pulsante BYPASS interno lascia invece attivo il percorso di registrazione.

## 17 · Palco e cambi di suono
PALCO presenta nome del preset, asterisco delle modifiche, accordatore grande, frecce preset, TAP e bypass. Entrando nella pagina viene selezionato il tipo Preset nella Libreria; le frecce seguono l'elenco filtrato e ordinato della libreria. Prepara quindi la selezione di preferiti e la ricerca prima di suonare. Non è ancora un editor di scalette con un ordine manuale.

Con il focus nella pagina, freccia sinistra/destra richiama il preset precedente/successivo, T attiva o spegne accordatore e mute, spazio aziona TAP del delay. Il MIDI configurato nella scheda MIDI continua a funzionare e permette di richiamare i file assegnati a PC 0–127. ACCORDA/MUTE silenzia l'uscita ascoltata, incluse base e click; la registrazione FX è presa prima del mute.

Per un preset le quattro catture vengono preparate come gruppo prima della pubblicazione; non si inserisce più un motore vuoto prima di ogni file. I controlli del preset vengono comunque applicati prima che la preparazione finisca: il richiamo NON è una transazione atomica dell'intero suono. Le transizioni già in corso possono ritardare l'adozione di uno slot. Attendi il messaggio di caricamento e controlla il risultato.

Il riverbero precedente può continuare a elaborare il silenzio per la durata della sua IR, con rampa finale di 100 ms. Si conserva al massimo una coda precedente; un altro cambio può troncarla. Disattivare il riverbero, rimuovere lo slot, cambiare mix o usare il bypass può mascherare la coda. Non si clonano le impostazioni precedenti del delay o dell'intera catena. La sovrapposizione aumenta il costo CPU e il livello: prova i cambi reali prima di un'esibizione.

## 18 · Audio, installazione e aggiornamenti
AUDIO / APP mostra versione, sistema operativo, sample rate, massimo blocco preparato, durata teorica del buffer, latenza dichiarata del plugin e livelli di ingresso/uscita. La diagnostica distingue segnale assente, presente e picco prossimo al clipping. Un segnale assente può anche significare che non stai suonando: non è una diagnosi automatica del cavo. COPIA DIAGNOSTICA copia questi dati negli appunti, senza indirizzi di file o credenziali.

La configurazione guidata ricorda di scegliere ingresso e uscita della stessa interfaccia, il canale corretto, INST e direct monitoring. Le vere impostazioni del dispositivo restano nel menu Options dello standalone o nella DAW. Il valore di un buffer non è la latenza di andata e ritorno: servono anche i buffer di ingresso/uscita, i driver e i convertitori. Per misurarla occorre una prova fisica di loopback.

I download Mac sono disponibili come ZIP e DMG. Il pacchetto include README-INSTALL.txt e Install-Lowline.command: chiudi app e DAW, poi usa il comando per copiare app e plugin nelle cartelle del tuo utente senza sudo. Le copie precedenti vengono rinominate con un suffisso di backup. È possibile anche copiare manualmente i bundle. Firma Developer ID e notarizzazione NON sono state effettuate: non disattivare Gatekeeper; consulta le istruzioni del pacchetto e le opzioni di Privacy e sicurezza di macOS.

CONTROLLA AGGIORNAMENTI apre nel browser la pagina della Release più recente. Confronta il numero con 0.3.0 visualizzato nell'app; non vengono scaricati o installati aggiornamenti automaticamente e non viene eseguito alcun controllo periodico. Per gli sviluppatori scripts/sign-mac.sh prepara il percorso di firma e notarizzazione con un'identità e un profilo notarytool già configurati nel Portachiavi: richiede un account Apple Developer e una verifica su Mac, non è parte della build pubblica non firmata.
