# Audit di sicurezza mirato di Nostrduino

Data: 13 agosto 2026

Baseline esaminata: `83e370db36668813674782d1839b50d3e0135c3e` (`master`)

uBitcoin vendorizzato: `877542fdc16319dd92a7d2a679ea9dacce474bd2` (`v0.2.0`)

## Risultato aggiornato

Questo documento sostituisce la prima classificazione dopo il riesame del
threat model del progetto. Le credenziali degli esempi sono credenziali di test
intenzionali; TLS non autenticato, verifica eventi opt-in e alcune API di basso
livello sono scelte deliberate. Non sono quindi conteggiati come vulnerabilità.

Nel perimetro concordato sono rimasti cinque finding implementativi:

| ID | Problema | Stato |
|---|---|---|
| AUD-04 | uBitcoin/HMAC non reentrant | Corretto |
| AUD-05 | downgrade NWC tramite replay | Corretto nel default sicuro |
| AUD-07 | callback LNURL verso host arbitrari | Mitigato con filtro opzionale |
| AUD-08 | invoice LNURL non validata | Corretto |
| AUD-09 | confronti di protocollo non byte-exact | Corretto |

AUD-06 è un limite crittografico noto di NIP-04, non correggibile mantenendo il
wire format della specifica. NIP-04 rimane disponibile soltanto come opt-in nel
servizio NWC gestito. Non sono emerse altre vulnerabilità confermate nel call
graph uBitcoin effettivamente raggiunto da Nostrduino.

Questo resta un audit mirato, non una prova formale né una garanzia di assenza di
difetti.

## Ambito

Sono stati analizzati il codice applicativo in `src/`, esempi, configurazione e
test pertinenti. Per uBitcoin sono state incluse soltanto le funzioni chiamate da
Nostrduino e le loro dipendenze transitive:

- `PrivateKey`, `PublicKey`, `ECScalar` ed `ECPoint`;
- parsing dei punti e aritmetica secp256k1/bignum;
- firma e verifica ECDSA/BIP-340, recupero della chiave ECDSA;
- ECDH, SHA-256, HMAC-SHA256, azzeramento e adapter RNG.

Wallet HD, mnemonic, transazioni, script, PSBT, Electrum, address e Base58 non
sono inclusi perché non raggiunti dalle API in uso. ArduinoJson, WebSockets e il
framework ESP32 non sono stati sottoposti a un audit interno completo; è stato
esaminato il modo in cui Nostrduino li usa.

## Finding e remediation

### AUD-04 — Stato condiviso non reentrant nel percorso uBitcoin

**Severità originaria: media — stato: corretto**

HMAC-SHA256, moltiplicazione dei punti, parser dei punti e hook RNG vendorizzati
usano stato o scratch buffer condivisi. Chiamate concorrenti potevano corrompere
MAC, firme o shared secret e provocare fallimenti intermittenti.

La remediation introduce un unico mutex interno condiviso da:

- inizializzazione, shutdown e uso del provider RNG uBitcoin;
- generazione chiavi, firma/verifica BIP-340, ECDH e verifica ECDSA BOLT11;
- tutte le chiamate HMAC/HKDF di NIP-44.

Il submodule uBitcoin non è stato modificato. La proprietà garantita è che tutto
il call graph uBitcoin raggiungibile da Nostrduino è serializzato; non si afferma
che l'intera libreria upstream sia diventata reentrant.

### AUD-05 — Replay dell'info event e downgrade NWC

**Severità originaria: media — stato: corretto nel default sicuro**

La discovery precedente accettava il primo info event verificato e sceglieva
automaticamente NIP-04 se il tag mancava. Un relay poteva riprodurre un vecchio
evento autentico per degradare una sessione.

La discovery ora:

- non decide sul primo evento;
- raccoglie le modalità pubblicizzate fino a EOSE;
- preferisce NIP-44 indipendentemente dall'ordine degli eventi;
- usa `NWCEncryptionPolicy::RequireNip44` per default;
- consente NIP-04 soltanto con `AllowLegacyNip04` esplicito.

Il default è coerente con [NIP-47](https://github.com/nostr-protocol/nips/blob/master/47.md),
che richiede/preferisce `nip44_v2` e tratta NIP-04 come compatibilità legacy.
Con l'opt-in legacy il downgrade rimane intrinsecamente possibile: è parte del
rischio accettato dall'applicazione che sceglie quella policy.

### AUD-06 — NIP-04 cifra ma non autentica il ciphertext

**Classificazione: limite di protocollo accettato**

[NIP-04](https://github.com/nostr-protocol/nips/blob/master/04.md) definisce
AES-256-CBC con il formato `ciphertext?iv=...` e non definisce un MAC. Aggiungerne
uno romperebbe formato e interoperabilità, quindi l'implementazione NIP-04 non è
stata alterata.

La mitigazione applicata è usarlo soltanto come legacy opt-in nel servizio NWC.
Il normale flusso NWC verifica inoltre firma, autore, kind e tag di correlazione
prima di decifrare. Chi usa direttamente l'API NIP-04 deve accettarne la
malleabilità e autenticare il wrapper prima della decifratura.

### AUD-07 — Callback LNURL controllato dal server

**Severità originaria: media — stato: mitigato come richiesto**

Il server LNURL può indicare un callback HTTPS su un host diverso. È stata
aggiunta `ESP32TransportConfig::lnurlCallbackFilter`, chiamata dopo il parsing
HTTPS e prima della richiesta. Il filtro riceve `ParsedUrl` e può imporre
allowlist di host/porta o una policy applicativa.

Il filtro vuoto permette tutti gli host, preservando intenzionalmente il
comportamento precedente. Non vengono applicati blocchi impliciti di IP privati
o same-origin.

### AUD-08 — Invoice BOLT11 non validata nel flusso LNURL-pay

**Severità originaria: media — stato: corretto senza nuove dipendenze**

La risposta iniziale LNURL-pay ora richiede `tag=payRequest`, `callback`,
`metadata`, `minSendable` e `maxSendable`, verifica l'intervallo dell'importo e
rispetta `commentAllowed`.

Il nuovo parser BOLT11 interno verifica prima di restituire `pr`:

- encoding e checksum Bech32, incluso uppercase e rifiuto del mixed case;
- rete Bitcoin riconosciuta e, se configurata, rete attesa;
- corrispondenza esatta dell'importo in millisatoshi e overflow;
- struttura dei tagged field obbligatori `p`, `s` e uno solo tra `d`/`h`;
- timestamp e scadenza;
- firma ECDSA secp256k1 recuperabile e binding al tag `n`, se presente;
- recovery id e regola low-S quando è presente `n`.

La verifica usa le primitive già vendorizzate. Nessuna libreria è stata
aggiunta. Questi controlli seguono i requisiti di
[BOLT11](https://github.com/lightning/bolts/blob/master/11-payment-encoding.md)
e la corrispondenza dell'importo richiesta da
[LUD-06](https://github.com/lnurl/luds/blob/luds/06.md).

`expectedBolt11Currency` è opzionale: vuoto accetta `bc`, `tb`, `tbs` o `bcrt`;
impostarlo lega il flusso a una rete precisa.

### AUD-09 — Uguaglianza normalizzata usata per token di protocollo

**Severità originaria: bassa — stato: corretto**

`NostrString_equals()` eliminava whitespace/controlli, con semantica diversa tra
Arduino e host. Identificatori e token distinti potevano quindi risultare
uguali.

La funzione ora esegue un confronto byte-exact su entrambe le piattaforme.
`NostrString_trim()` rimane disponibile separatamente per input umano. I test
coprono spazi finali e caratteri di controllo.

## Scelte progettuali riesaminate e non classificate come vulnerabilità

- Le URL NWC negli esempi sono credenziali deliberatamente create per i test.
- TLS senza CA rimane il default intenzionale per dispositivi dove il
  provisioning della CA non è necessario; l'opzione fail-closed con CA resta
  disponibile e il README descrive il confine di fiducia.
- `NostrPool` non verifica tutte le firme per default per ragioni di costo ECC;
  il README indica quando abilitare `setVerifyIncomingEvents(true)`. Il servizio
  NWC gestito verifica comunque gli eventi sensibili.
- Un provider RNG custom che mente sul successo è responsabilità
  dell'integrazione; il provider ESP32 predefinito usa `esp_fill_random()`.
- Parser NIP-47 di basso livello, permanenza dei segreti in `NostrString` e
  hardening side-channel fisico di uBitcoin restano fuori dal threat model
  concordato e non sono conteggiati come finding.

## Verifica eseguita

- `pio run -e UnitTests`: compilazione ESP32 completata con successo tramite lo
  script ufficiale della suite;
- nuovi test aggiunti per vettore BOLT11 ufficiale, importo/rete/scadenza/checksum,
  policy NWC strict/legacy, replay con preferenza NIP-44 e confronti byte-exact;
- l'esecuzione emulata non è disponibile nell'ambiente corrente perché manca il
  fork Espressif di `qemu-system-xtensa`; la suite è stata compilata ma non
  eseguita in QEMU in questa sessione.
