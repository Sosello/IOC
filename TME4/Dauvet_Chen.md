# Programmation ESP32 / Arduino

## Installation ESP32 / Arduino

Après installer les librairies ESP32, on commence de lancer notre premier programme en Arduino: Blink

Ce programme permet de faire une clignotement infinie dans LED sur ESP32.

Le code est dans le fichier **Blink**

```c
sudo chmod -R 777/dev/ttyUSB0
```
Il faut exécuter ce commande pour permet d'utiliser Arduino dans la suite de TP.

À l'aide de comentaire dans ce code, on peut savoir que la fonction **setup()** sert à initialise la sortie, et la fonction **loop()** permet d'exécuter le programme.

## Exécution multi-tâches

### Questions

* Que contient le tableau waitForTimer[] et à quoi sert-il ?

    Ce tableau contient l'horodatage du dernière tâche exécuté. C'est à dire la période entre ce moment et le dernière moment que la tâche dans l'indice timer était exécuté.

* Si on a deux tâches indépendantes avec la même période, pourquoi ne peut-on pas utiliser le même timer dans waitFor() ?

    Parce que normalement les tâches indépendantes ont des périodes exécution différentes, donc si on veut savoir l'horodatage d'une tâche, ça peut être causer un problème puisque nous ne savons pas quelle tâche est enregistrée dans cet horodatage.

* Dans quel cas la fonction waitFor() peut rendre 2 ?

    Dans le cas qu'on doit attendre une période pour exécuter la tâche courant ou le moment a dépassé la borne de l'horodatage.

## Utilisation de l'écran OLED

On va utiliser la fonction testdrawchar qui est déjà fournis dans le template.
```c
void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(30, 20);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(1000);
}
```