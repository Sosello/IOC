# TP4 DAUVET CHEN

## Installation ESP32 / Arduino

Après avoir installé les librairies ESP32, on sélectionne dans l'IDE arduino notre carte **Tools -> Boards -> TTGO-Lora-OLED V1** avec le port USB où on a connecté notre ESP32 (en ayant pris soin d'accordé les droits total sur ce port: `sudo chmod -R 777/dev/ttyUSB0`), puis lançons notre premier programme Arduino: **File -> Exemples -> 0.1 Basics -> Blink**

Ce programme permet de faire clignotter la LED intégré à l'ESP32 à une fréquence d'in clignottement par seconde.

* La fonction `setup()` est éxécuté lorsqu'on branche l'ESP cela correspond à l'initialisation. Dans cette fonction on indique à l'ESP que la LED intégré va être utiliser en mode sortie.
* La fonction `loop()` est a boucle pricipale de notre programme. On appelle la fonction `digitalWrite(pin, val)` pour allumer ou éteindre la LED et `delay(ms)` pour attendre. 

## Exécution multi-tâches

### Tâches standards

Une tache est une structure de données représentant des actions à effectué dans notre application, ses actions sont initialisées par la fonction `setup_Tache()` appelé dans `setup()` avec certains paramètre `params`, et leurs code est éxécutées dans la fonction `loop_Tache()` appelé dans la fontion `loop()` avec des pointeurs `connectors` utilisées pour la communications inter-tâches.

```C
void setup_Tache(struct Tache_s *ctx, params...) {
   // Initialisation du contexte}
   ctx->etat = etat_initial;  //  reçu dans les paramètres
   ...
}
```C
void loop_Tache(tache_t *ctx, connectors...) {   
   // test de la condition d'exécution, si absent on SORT
   if (evement_attendu_absent) return;
   // code de la tache
   ...
}
```

Pour éxécuté une tâche, on a besoin de sauvegarder son contexte d'éxécution (où on en est dans l'action), dans une variable globale, la structure d'une tâche contient donc son `etat` et la valeurs de ses *variables* :

```C
struct Tache_s {
  unsigned int etat;  
  int config;        
}; 
```

En effet, si on déclarait ces *variables* dans `loop_Tache()` ou `setup_Tache()`, ces variables ne serait pas conservé entre deux appelles de ces fonctions. On aurait aussi pu déclaré les *variables* comme **static** dans ces fonctions (valeurs conservé entre deux appelles), mais cela pause deux problèmes :

1. On ne peut pas déclaré de variable **static** dans setup, cela produit une erreur de compilation.
2. Une variable static est locales à une fonction, donc l'éxécutions de la même fontion par plusieurs tâches n'est pas possible.

Donc la strcture pour sauvegarder le contexte d'une tâche reste la meilleur option.

### Questions

* Que contient le tableau waitForTimer[] et à quoi sert-il ?

    Ce tableau contient pour chaque timer, le temps restant à attendre, il est mis à jour a chaque fois que l'on dépasse la periode.

* Si on a deux tâches indépendantes avec la même période, pourquoi ne peut-on pas utiliser le même timer dans `waitFor()` ?

    Normalement les tâches indépendantes ont des périodes d'exécution différentes, dans ce cas, on ne pourrait pas savoir laquelle des tâches qui a le même timer à été éxécutées.

* Dans quel cas la fonction `waitFor()` peut rendre 2 ?

    2 serait le nombre de periode depuis le dernier appelle de `waiFor()` sur ce timer.

## Utilisation de l'écran OLED

On va utiliser la fonction `display.sprintln(F())` qui est déjà fournis dans le template dans la fonction `testScrollText()` .On aurai aussi pu utiliser la suite de fonction décrites dans `testDrawChar()`

### Questions

On déplace la fonction `waitFor()` dans un header **timer.h**, on créé un header **oled.h** qui contient les fonctions de l'éran ainsi que la structure de la tâche Oled (et ses fonctions setup et loop), et on modifie notre programme principale.
La nouvelle tâche affiche et enlève le mot "Test" de l'écran.

## Communications inter-tâches

### Questions

## Gestions des interruptions

### Questions

## Tout ensemble