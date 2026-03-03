aiutami a scrivere un prompt per opus molto del codice è già scritto, lui deve fare il programma di inseguimento della linea per la robocup rescue line 2026. 



 se la linea è solo da seguire, deve solo chiamare la funzione pidLineFollowing, che legge la linea calcola il pid e muove il robot, mentre, in caso di bollini verdi, interruzioni, ostacoli, deve prendere il controllo.



HW:

Arduino mega 2560,

TOF VL53L0X,

scheda sotto (slave) array di 8 sensori, un sensore frontale, 2 sensori di colore, un pic e un micro



limitazioni: non ci possono essere funzioni bloccanti, la ram in questo momento è: used 5293 bytes from 8192 bytes



per fare ciò deve usare:

int16_t IR_board.line(): aggiorna i dati e ritorna quando il robot è scostato dalla linea come un numero che va da -1750 a +1750

bool IR_board.checkGreenSx() o bool IR_board.checkGreenDx(): ritorna un booleano che informa che è stato visto un verde in uno dei 2 lati

bool IR_board.checkLinea(): controlla un sensore posto davanti agli altri, per controllare se oltre per esempio una curva a 90° ce n'è una anche dritta

IR_board.Utils.val_sensor(): ritorna un array di 9 uint16_t, i primi 8 sono l'array dei sensori linea, il nono è un sensore posto davanti all'array.

uint16_t val_sensor(uint8_t n_sensore): come la funzione precedente ma ritorna solo un sensore di cui deve venir dato l'indice.



void motori.muovi(short vel, short ang): gestisce i 3 motori del robot, 2 frontali e uno centrale posteriore omnidirezionale. La velocità e l'angolo sono trattati come due vettori separati, calcolati e combinati per determinare il comportamento dei motori.

void motori.stop(): Ferma tutti i motori.



short pidLineFollowing(short base_vel, int16_t line_position): accetta una velocità a cui andare, e la posizione della linea come già data dalla scheda sotto cioè in un range da -1750 a +1750, questo per evitare di ritornare a comunicare con la scheda sotto che è abbastanza lenta, ritorna il valore di sterzo dato alla funzione muovi.

short pidLineFollowing(short base_vel): stessa cosa di prima ma si recupera da solo la posizione della linea.

void resetPID(): Chiama questa funzione quando desideri azzerare lo stato del controllore PID (utile quando il robot ricomincia a seguire la linea).



void tof_manager.refreshAll(): aggiorna i valori di ogni sensore

uint16_t* tofManager.getAllDistances(): ritorna un array di 5 distanze in mm, un valore per sensore, in questo ordine frontale, anteriore SX, anteriore DX, posteriore SX, posteriore DX

tof_manager.{nomesensore}: oggetto della classe tof, uno per sensore, i cui nomi sono: front, antSX, antDX, posSX, posDX

void tof.refresh(): aggiorna i valori del sensore

uint16_t tof.getDistance(): ritorna la distanza in mm letta dal sensore



debug: stampa su tutti i supporti dei messaggi di debug, per farlo usa le seguenti funzioni seguite da debug.:

void print(const String& message);

void print(const char* message);

void print(int value);

void print(float value);

void print(double value);

void println(const String& message);

void println(const char* message);

void println(int value);

void println(float value);

void println(double value);

void println();



behavour della scheda sotto:

ogni tanto da qualche val del verde errato.

se vede la linea su tutto l'array rende 0.

se perde la linea tiene il valore a -1750 o +1750 in base se l'ultima volta era scostata a sinistra o destra.

ogni tanto i doppi verdi vengono letti come singoli



cosa deve fare:

se è normalmente sulla linea seguirla con pidlinefollowing.

se c'è un interruzione, andare lentamente indietro leggendo tutti i sensori, per cercare una possibile curva persa (il 70% delle volte in cui perde una curva è perche essa è <90°), se no cerca di centrarti sulla linea e vai avanti fino a quando non la ritrova.

aggirare gli ostacoli permettendo di farlo sia a destra che a sinistra, per scegliere vai indietro di qualche cm per controllare che non ci siano ostacoli sui lati, l'ostacolo può essere sia su una dritta che su una curva di 90°, occhio, quando arrivi sulla linea devi stare attento a non girarti all'indietro.

gestire i verdi singoli, sinistra e destra che segnano una svolta obbligata, controllando che non siano da ignorare (sono dopo la svolta e non prima)

gestire gli incroci, se si arrivi in un incrocio, sia esso a T o a + e non ci sono verdi il robot è obbligato ad andare avanti.

gestire i doppi verdi, essi se sono prima della linea obbligano a una inversione a U

metti una funzione da chiamare quando il robot viene riposizionato, per resettare eventuali stati



tips:

se la linea è a 0 c'è la possibilita che io robot si trovi in un incrocio a T o a +



tutte le costanti devo essere messe in un file apposito