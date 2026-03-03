#ifndef LINELOGIC_CONFIG_H
#define LINELOGIC_CONFIG_H

// ═══════════════════════════════════════════════════════════════════
//  CONFIGURAZIONE RescueLineFollower — RoboCup Rescue Line 2026
//  Tutte le costanti numeriche in un unico punto.
// ═══════════════════════════════════════════════════════════════════

// ── Debug ─────────────────────────────────────────────────────────
// Attiva (1) o disattiva (0) il logging diagnostico dettagliato.
// Disattiva in gara per risparmiare CPU e banda seriale.
#define LL_DEBUG 1

#if LL_DEBUG
  #define LL_LOG(msg)         debug.println(msg)
  #define LL_LOG2(a, b)       do { debug.print(a); debug.println(b); } while(0)
  #define LL_LOG4(a,b,c,d)    do { debug.print(a); debug.print(b); debug.print(c); debug.println(d); } while(0)
  #define LL_LOG6(a,b,c,d,e,f) do { debug.print(a); debug.print(b); debug.print(c); debug.print(d); debug.print(e); debug.println(f); } while(0)
#else
  #define LL_LOG(msg)         ((void)0)
  #define LL_LOG2(a, b)       ((void)0)
  #define LL_LOG4(a,b,c,d)    ((void)0)
  #define LL_LOG6(a,b,c,d,e,f) ((void)0)
#endif

// ── Velocità (unità: -1023..+1023 per vel, -1750..+1750 per ang) ──

// Velocità base durante il following normale.
// Aumenta se il robot è troppo lento sulla linea, riduci se sbanda nelle curve.
#define BASE_VEL                ((short)800)

// Velocità di retromarcia durante il recovery della linea persa.
// Più negativo = retromarcia più veloce. Riduci (meno negativo) se il robot
// arretra troppo e perde la linea anche dietro.
#define REVERSE_VEL             ((short)-350)

// Velocità di avanzamento lento nella fase finale del recovery.
// Aumenta se il robot impiega troppo a ritrovare la linea dopo la sterzata.
#define SEARCH_VEL              ((short)250)

// Velocità durante l'attraversamento di un incrocio.
// Riduci se il robot perde la linea uscendo dall'incrocio.
#define CROSS_VEL               ((short)350)

// Velocità di retromarcia quando viene rilevato un ostacolo frontale.
// Più negativo = arretra più veloce. Riduci se va troppo lontano dall'ostacolo.
#define OBSTACLE_BACK_VEL       ((short)-300)

// Velocità di avanzamento durante l'aggiramento dell'ostacolo.
// Aumenta se l'aggiramento è troppo lento, riduci se il robot esce dal percorso.
#define OBSTACLE_BYPASS_VEL     ((short)400)

// Angolo di sterzata durante l'inversione a U (doppio verde).
// Aumenta per girare più stretto, riduci se il robot si destabilizza.
#define UTURN_ANG               ((short)1200)

// ── Soglie TOF (mm) ──────────────────────────────────────────────

// Distanza minima (mm) dal TOF frontale per rilevare un ostacolo.
// Aumenta se il robot reagisce troppo tardi, riduci se frena per oggetti lontani.
#define OBSTACLE_DETECT_MM      ((uint16_t)120)

// Numero di letture consecutive valide sotto OBSTACLE_DETECT_MM
// prima di dichiarare "ostacolo". Evita falsi positivi da spike del TOF.
#define OBSTACLE_CONFIRM_READS  ((uint8_t)3)

// Distanza minima laterale (mm) perché un lato sia considerato "libero"
// durante la scelta del lato di aggiramento.
// Aumenta se il robot sceglie un lato troppo stretto.
#define SIDE_CLEAR_MM           ((uint16_t)250)

// Distanza laterale (mm) usata nel riallineamento post-ostacolo.
// Aumenta se il robot ritorna sulla linea troppo presto e la manca.
#define REALIGN_LINE_MM         ((uint16_t)200)

// ── Timeout (ms) ─────────────────────────────────────────────────

// Tempo (ms) per cui la linea deve risultare persa (±1750) prima di
// entrare in recovery. Aumenta per evitare falsi recovery su piccoli gap.
#define LINE_LOST_CONFIRM_MS    ((uint16_t)250)

// Durata massima (ms) della retromarcia nel recovery.
// Aumenta se i gap sono lunghi e il robot non arretra abbastanza.
#define REVERSE_SEARCH_MS       ((uint16_t)600)

// Timeout totale (ms) per l'intera sequenza di recovery (REVERSE+CENTER+FORWARD).
// Se scade il robot si ferma. Aumenta su piste con gap molto lunghi.
#define SEARCH_TOTAL_MS         ((uint16_t)3000)

// Timeout (ms) per una svolta a 90° (verde singolo).
// Aumenta se il robot non completa la svolta in tempo.
#define TURN_TIMEOUT_MS         ((uint16_t)2000)

// Timeout (ms) per l'inversione a U (doppio verde).
// Aumenta se il robot non riesce a completare la rotazione a 180°.
#define UTURN_TIMEOUT_MS        ((uint16_t)3000)

// Tempo (ms) per cui line()==0 deve persistere per confermare un incrocio.
// Aumenta se il robot entra in modalità incrocio su semplici curve larghe.
#define CROSS_CONFIRM_MS        ((uint16_t)150)

// Durata (ms) dopo una svolta durante cui i verdi vengono ignorati.
// Aumenta se il robot rileva di nuovo il bollino verde appena uscito dalla curva.
#define POST_TURN_IGNORE_MS     ((uint16_t)500)

// Finestra (ms) di attesa dopo il primo verde confermato, per vedere se
// arriva anche il secondo lato (doppio verde).
// Aumenta se la scheda slave è molto lenta a riportare il secondo lato.
#define GREEN_DOUBLE_WINDOW_MS  ((uint16_t)120)

// Durata (ms) della retromarcia dopo aver rilevato un ostacolo.
// Aumenta se il robot non arretra abbastanza per avere spazio di manovra.
#define OBSTACLE_BACK_MS        ((uint16_t)500)

// Timeout massimo (ms) per la fase di aggiramento dell'ostacolo.
// Aumenta su ostacoli molto grandi; riduci se il robot si allontana troppo.
#define OBSTACLE_BYPASS_MS      ((uint16_t)4000)

// ── Soglie IR ────────────────────────────────────────────────────

// Soglia sulla posizione linea (abs) sotto la quale il robot è "centrato".
// Aumenta se il robot oscilla troppo intorno al centro, riduci per più precisione.
#define CENTER_THRESHOLD        ((int16_t)200)

// Soglia più larga usata per il completamento di svolte e U-turn.
// Aumenta se il robot non riesce a uscire dallo stato di svolta.
#define TURN_CENTER_THRESHOLD   ((int16_t)400)

// Numero minimo di sensori IR attivi per considerare la linea "trovata".
// Aumenta se ci sono falsi positivi con un solo sensore attivo.
#define CENTER_MIN_SENSORS      ((uint8_t)2)

// ── Debounce verde ───────────────────────────────────────────────

// Numero di cicli consecutivi con verde=true per confermare il rilevamento.
// Aumenta se ci sono troppi falsi positivi, riduci se il robot non rileva
// i bollini verdi abbastanza rapidamente.
#define GREEN_CONFIRM_READS     ((uint8_t)3)

// Cicli per confermare il secondo lato verde (nella finestra doppio-verde).
// Può essere minore di GREEN_CONFIRM_READS perché il secondo lato
// arriva spesso in ritardo dalla scheda slave.
#define GREEN_DOUBLE_CONFIRM_READS ((uint8_t)2)

// ── Direzione di default per aggiramento ostacolo ────────────────
// Lato preferito quando entrambi i lati sono liberi.
// true = aggira a sinistra, false = aggira a destra.
// Cambia in base al layout più frequente della pista di gara.
#define OBSTACLE_DEFAULT_LEFT   false

// ── Angolo sterzata per svolta 90° ──────────────────────────────

// Angolo di sterzata per la svolta a sinistra (negativo = sinistra).
// Aumenta il valore assoluto per girare più stretto.
#define TURN_LEFT_ANG           ((short)-1400)

// Angolo di sterzata per la svolta a destra (positivo = destra).
// Aumenta per girare più stretto, riduci se il robot si destabilizza.
#define TURN_RIGHT_ANG          ((short)1400)

// Velocità di avanzamento durante la fase di svolta.
// Usa la stessa velocità base per evitare che il robot si blocchi nella curva.
#define TURN_SLOW_VEL           BASE_VEL

// ── Angolo sterzata aggiramento ostacolo ─────────────────────────

// Angolo di sterzata laterale durante l'aggiramento.
// Aumenta per allontanarsi di più dall'ostacolo, riduci per traiettoria più stretta.
#define OBSTACLE_STEER_ANG      ((short)1000)

#endif
