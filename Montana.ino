// Game:      Montana
// Copyright: 2026 Frank van de Ven
// Licence:   MIT
 
 #include <Arduboy2.h>                                // installeer de Arduboy2 bibliotheek
 #include "data.h"
 Arduboy2 arduboy;                                    // maakt een Arduboy object aan
// ------------------------------------------------------------------------------------------

// variabelen
 #define wachttijd 2000                               // tijd dat startscherm zichtbaar 
 #define wachttijd2 1200                              // tijd voor verwijderen azen
 #define wachttijd3 1200                              // tijd voor wissen kaarten 
 #define wachttijd4 600                               // tijd voor verschijnen eind scherm

 unsigned long wacht;                                 // voor vertragingen 
 bool knipper = 0;                                    // laat kaart wel (1) / niet (0) zien
 byte snel = 10;                                      // snelheid knipperen kaart
 byte stok[52];                                       // de stok waar alle kaarten inzitten
 byte spel[13][4];                                    // alle kaarten in 4 rijen op tafel
 byte x,y;                                            // teller voor loeps
 byte Cx,Cy;                                          // locatie cursor
 byte game = 0;                                       // stage machine begin bij 0
 byte card;                                           // declareer backup variabele

// ------------------------------------------------------------------------------------------

void setup() {
 arduboy.begin();                                     // initialiseert Arduboy2 bibliotheek
 arduboy.setFrameRate(30);                            // framerate: 30
}
// ------------------------------------------------------------------------------------------

void loop() {
 if (!(arduboy.nextFrame())) {return;}                // wacht op volgende frame
 arduboy.pollButtons();                               // controleer of knop is ingedrukt
 if (arduboy.frameCount % snel == 0){                 // wacht 7 frames
  knipper = knipper ^ 1;                              // inverteer de knipper bit
 }
 arduboy.clear();                                     // wis het display (wordt zwart)
 switch (game) {                                      // stage machine
 case 0:                                              // opstart scherm
  intro();                                            // zet het startscherm op display
  game = 1;                                           // naar stap 1
 break;
 case 1:                                              // start een nieuw spel
  arduboy.initRandomSeed();                           // init random getallen
  beginWaarde();                                      // begin waardes array's en variabelen
  schudden();                                         // schud de kaarten in de stok
  stokNaarSpel();                                     // verdeel kaarten uit stok in spel[]
  wacht = millis();                                   // laad wachttijd met huidige tijd
  game = 2;                                           // naar de stap 2
 break;
 case 2:                                              // verwijder de azen
  if (millis() > wacht + wachttijd2) {                // wacht even
   zoekAzen();                                        // zoek azen en haal ze uit spel[] (98)
   stokNaarSpel();                                    // verdeel kaarten uit stok in spel[]
   while (spel[Cx][Cy] == 98) {Cx++;}                 // schuif cursor op als er een aas ligt
   game = 3;                                          // naar stap 3
  }
 break;
 case 3:                                              // lees toetsen
  toetsen();                                          // lees toetsen. pas cursor & game aan
  controle();                                         // controleer of spel uitgespeeld is
 break;
 case 4:                                              // opnieuw delen 1
  opnieuwSchudden1();                                 // wis de kaarten die fout liggen 
  WisKaartenUitStok();                                // wis kaarten die goed zijn uit stok
 break;
 case 5:                                              // opnieuw delen 2
  if (millis() > wacht + wachttijd3) {                // wacht even
  opnieuwSchudden2();                                 // schud kaarten en leg ze in open vak
  }
 break;
 case 6:                                              // verplaats kaart
  verplaats();                                        // verplaats kaart onder cursor
 break;
 }
 kaartenOpDisplay();                                  // zet kaarten uit spel[] op display
 arduboy.display();                                   // zet video geheugen op het display
}
// ------------------------------------------------------------------------------------------

// wis de kaarten die goed liggen uit de stok
 void WisKaartenUitStok() {
 for (byte tel = 0; tel < 52; tel++){                 // doorloop alle kaarten in de stok
  stok[tel] = tel;                                    // zet oplopende kaart nr in de stok
 }
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  card = spel[0][y];                                  // sla linker kaart op
  if (card==1 || card==14 || card==27 || card==40) {  // ligt er een 2?
   for (x = 0; x < 13; x++) {                         // doorloop alle kolommen met kaarten
    if (spel[x][y] != 99) {                           // ligt hier GEEN kaart goed?
     stok[card + x] = 99;                             // wis dan deze kaart uit de stok
    }
   }
  }
 }
}
// ------------------------------------------------------------------------------------------

// controleer of het spel is uitgespeeld
void controle() {
 byte aantal = 0;                                     // telt aantal kaarten dat goed ligt
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  card = spel[0][y];                                  // backup kaart onder cursor
  if (card==1 || card==14 || card==27 || card==40) {  // ligt er een 2?
   for (x = 0; x < 13; x++) {                         // doorloop alle kolommen met kaarten
   if (spel[x][y] == card) {aantal++;}                // ligt deze kaart goed
   card++;                                            // volgende kaart
   }
  }
 }
 if (aantal == 48) {gewonnen();}                      // liggen alle kaarten goed?
}
// ------------------------------------------------------------------------------------------

// print tekst gewonnen en start nieuw spel
void gewonnen() {
 knipper = 0;                                         // schakel knipperen uit
 kaartenOpDisplay();                                  // zet kaarten uit spel[] op display
 arduboy.display();                                   // zet video geheugen op het display
 wacht = millis();                                    // laad wachttijd met huidige tijd
 while (millis() < wacht + wachttijd4) {}             // wacht tot opgegeven tijd om is
 arduboy.fillRect (12,18,94,27, BLACK);               // maak onderste deel van scherm zwart
 arduboy.setCursor(14, 22);                           // verplaats tekst cursor
 arduboy.println("Congratulations");                  // zet tekst op het scherm
 arduboy.setCursor(32, 34);                           // verplaats tekst cursor
 arduboy.println("New game?");                        // zet tekst op het scherm
 arduboy.display();                                   // zet video geheugen op het display
 while (arduboy.pressed(A_BUTTON) == false && arduboy.pressed(B_BUTTON) == false) {} // knop
 game = 1;                                            // start nieuw spel
}
// ------------------------------------------------------------------------------------------

// wis alle fouten kaarten die fout liggen
void opnieuwSchudden1(){
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  card = spel[0][y];                                  // backup kaart onder cursor
  if (card==1 || card==14 || card==27 || card==40) {  // ligt er een 2?
   for (x = 1; x < 13; x++) {                         // doorloop alle kolommen met kaarten
    card++;                                           // volgende kaart
    if (spel[x][y] != card) {                         // is kaart op tafel != volgende kaart
     card = 100;                                      // schakel kaart teller uit
     spel[x][y] = 99;                                 // er liggen geen kaarten meer goed
    }
   }
  wacht = millis();                                   // laad wachttijd met huidige tijd 
  }
 else {                                               // ligt er geen 2 vooraan
  for (x = 0; x < 13; x++) {                          // doorloop rij met kaarten
   spel[x][y] = 99;                                   // er liggen geen kaarten meer goed
  }
 }
 }
 game = 5;                                            // ga naar kaarten schudden / plaatsen
}
// ------------------------------------------------------------------------------------------

// schud de kaarten die fout liggen opnieuw en plaats geschudde kaarten in open ruimtes
void opnieuwSchudden2() {
 schudden();                                          // schud de kaarten in de stok
 stokNaarSpel();                                      // verdeel kaarten uit stok in spel[] 
 Cx = 0; Cy = 0;                                      // verplaats cursor naar links boven
 while (spel[Cx][Cy] == 98) {Cx++;}                   // schuif cursor op als er een aas ligt
 wacht = millis();                                    // laad wachttijd met huidige tijd
 game = 2;                                            // naar de stap 2
}
// ------------------------------------------------------------------------------------------

// verplaats de kaart onder de cursor positie en leg hem op zijn plaats
void verplaats() {
 card = spel[Cx][Cy];                                 // backup kaart onder cursor
 if (card==1 || card==14 || card==27 || card==40) {   // ligt er een 2?
  for (y = 0; y < 4; y++){                            // doorloop alle kaarten 1e kolom
   if (spel[0][y] == 98) {                            // is dit een vrije plaats
    spel[0][y] = card;                                // verplaats kaart naar nieuwe positie
    spel[Cx][Cy] = 98;                                // wis kaart op oude positie
    y = 5;                                            // breek loep af
   }
  }
 }
 else {
  for (y = 0; y < 4; y++) {                           // doorloop alle rijen met kaarten
   for (x = 0; x < 12; x++) {                         // doorloop alle kolommen met kaarten
    if (spel[x][y] == card - 1) {                     // zoek de kaart die voor cursor komt
      if (spel[x + 1][y] == 98) {                     // is de plaats hierachter vrij?
        spel[x + 1][y] = card;                        // verplaats kaart naar nieuwe positie
        spel[Cx][Cy] = 98;                            // wis kaart op oude positie
        x = 13; y = 4;                                // breek loep af
      }
    }
   }
  }
 }
 game = 3;                                            // ga terug naar invoer toetsen
}
// ------------------------------------------------------------------------------------------

// lees de toetsen, verplaats cursor en pas game status aan
void toetsen() {
 if (arduboy.justPressed(A_BUTTON)) {game = 6;}        // knop A: verplaats kaart
 if (arduboy.justPressed(B_BUTTON)) {game = 4;}        // knop b: opnieuw schudden
 if (arduboy.justPressed(UP_BUTTON)) {                 // verplaats cursor omhoog
  if (Cy != 0) {Cy--;}                                 // sta je niet boven verlaag y met 1
 }
 if (arduboy.justPressed(DOWN_BUTTON)) {               // verplaats cursor omlaag
  if (Cy != 3) {Cy++;}                                 // sta je niet onder verhoog y met 1     
 }
 if (arduboy.justPressed(LEFT_BUTTON)) {               // verplaats cursor naar links
  if (Cx != 0) {Cx--;}                                 // sta je niet links verlaag x met 1   
  }
 if (arduboy.justPressed(RIGHT_BUTTON)) {              // verplaats cursor naar rechts
  if (Cx != 12) {Cx++;}                                // sta je niet rechts verhoog x met 1          
 }
}
// ------------------------------------------------------------------------------------------

// haal de kaaten uit de stok[] en verdeel ze over het spel[] array
void stokNaarSpel() {
 byte k3 = 0;                                         // declareer variabele
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  for (x = 0; x < 13; x++) {                          // doorloop alle kolommen met kaarten
   if (spel[x][y] == 99) {                            // kijk of op locatie geen kaart ligt
    while (stok[k3] == 99) {k3++;}                    // ligt kaart al op tafel. volgende
     spel[x][y] = stok[k3];                           // leg kaart uit stop in spel
     k3++;                                            // volgende kaart uit de stok
   }
  }
 }
}
// ------------------------------------------------------------------------------------------

// zet de kaarten uit "spel[]" op het display 
void kaartenOpDisplay() {
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  for (x = 0; x < 13; x++) {                          // doorloop alle kolommen met kaarten
   if (spel[x][y] != 98 && spel[x][y] != 99) {        // bij een leeg vak geen kaart printen
    if (!(x == Cx && y == Cy && knipper == 1)) {      // laat kaart onder cursor knipperen
     byte k1 = spel[x][y] % 13;                       // bereken kaart nr uit stok nr
     byte k2 = spel[x][y] / 13 + 13;                  // bereken kaart soort uit stok nr
     Sprites::drawOverwrite(x*10, y*16, kaart, k1);   // zet bovenste deel kaart op display
     Sprites::drawOverwrite(x*10, y*16+8, kaart, k2); // zet onderste deel kaart op display
    }
   }
   if (spel[Cx][Cy] == 98 && Cx == x && Cy == y && knipper ==1) { // cursor op leeg veld?
    Sprites::drawOverwrite(x*10, y*16, kaart, 17);   // zet bovenste deel kaart op display
    Sprites::drawOverwrite(x*10, y*16+8, kaart, 18); // zet onderste deel kaart op display 
   }
  }
 }
}
// ------------------------------------------------------------------------------------------

// zoek alle azen en haal ze uit de het spel[] (worden nr 98)
void zoekAzen() {
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  for (x = 0; x < 13; x++) {                          // doorloop alle kolommen met kaarten
   if (spel[x][y] == 0) {spel[x][y] = 98;}            // verander schoppen aas in nr 98
   if (spel[x][y] == 13) {spel[x][y] = 98;}           // verander klaveren aas in nr 98
   if (spel[x][y] == 26) {spel[x][y] = 98;}           // verander harten aas in nr 98
   if (spel[x][y] == 39) {spel[x][y] = 98;}           // verander ruiten aas in nr 98
  }
 }
}
// ------------------------------------------------------------------------------------------

// Zet bij starten spel de waardes van de array's en variabelen goed
void beginWaarde() {
 // laad de stok met alle kaarten op volgorde
 for (byte tel = 0; tel < 52; tel++){                 // doorloop alle kaarten in de stok
  stok[tel] = tel;                                    // zet oplopende kaart nr in de stok
 }
 for (y = 0; y < 4; y++) {                            // doorloop alle rijen met kaarten
  for (x = 0; x < 13; x++) {                          // doorloop alle kolommen met kaarten
   spel[x][y] = 99;                                   // maak deze locatie leeg
  }
 }
 Cx = 0; Cy = 0;                                      // zet cursor positie linksboven
}
// ------------------------------------------------------------------------------------------

// schud de kaarten in de stok door elkaar (aangepast Fiher Yates algoritme)
void schudden() {
 byte tijdelijk, arrayNr;                             // declareer variabelen
 for (int nr = 0; nr < 52; nr++){                     // doorloop alle kaarten in de stok
  arrayNr = random(nr,51);                            // kies een getal
  tijdelijk = stok[nr];                               // sla de kaart uit array tijdelijk op
  stok[nr] = stok[arrayNr];                           // wissel de kaart in de stok
  stok[arrayNr] = tijdelijk;                          // wissel de kaart in de stok
 }
}
// ------------------------------------------------------------------------------------------

// zet het startscherm op het display
void intro() {
 arduboy.clear();                                     // wis het display (wordt zwart)
 arduboy.drawCompressed(0, 0, plaatje, WHITE);        // zet de start afbeelding het display
 arduboy.display();                                   // zet video geheugen op het display
 wacht = millis();                                    // laad wachttijd met huidige tijd
 while (millis() < wacht + wachttijd) {}              // wacht tot opgegeven tijd om is
 arduboy.fillRect (11,49,106,12, BLACK);              // maak onderste deel van scherm zwart
 arduboy.setCursor(13, 51); arduboy.println("A:Play");// tekst op vaste plaats op scherm
 arduboy.setCursor(52, 51); arduboy.println("game");  // zet tekst op vaste plaats op scherm
 arduboy.setCursor(80, 51); arduboy.println("B:Code");// zet tekst op vaste plaats op scherm
 arduboy.display();                                   // zet video geheugen op het display
 while (arduboy.pressed(A_BUTTON) == false) {         // A knop: doorloop loep tot spel start
  if (arduboy.pressed(B_BUTTON) == true) {            // B knop: wordt er op B gedrukt?
   arduboy.fillRect (11,50,106,10, BLACK);            // maak onderste deel van scherm zwart
   arduboy.setCursor(22, 51); arduboy.println("Frank");// zet tekst op vaste plaats op scherm
   arduboy.setCursor(54, 51); arduboy.println("van"); // zet tekst op vaste plaats op scherm
   arduboy.setCursor(75, 51); arduboy.println("de");  // zet tekst op vaste plaats op scherm
   arduboy.setCursor(90, 51); arduboy.println("Ven"); // zet tekst op vaste plaats op scherm
   arduboy.display();                                 // zet video geheugen op het display
  }
 }
}
// ------------------------------------------------------------------------------------------
