#include "timer.h"
#include "flag.h"
#include "oled.h"
#include "lum.h"
#include "isr.h"
#include "buzzer.h"
#include "button.h"

//--------- définition de la tache Led

struct Led_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
}; 

void setup_Led( struct Led_s * ctx, int timer, unsigned long period, byte pin) {
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void loop_Led( struct Led_s * ctx,mailbox_s * mb_photo,mailbox_s * mb_isr) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  
  if(mb_isr->state == FULL)
  {
    ctx->etat = 0;
    digitalWrite(ctx->pin,ctx->etat);
    return;
  }                           //si interruption la led ne clignotte plus

  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;                              // changement d'état
  // Serial.println(ctx->period);            

  if (mb_photo->state == FULL)                       // pour ne pas être bridé à la periode de la tache lum 
  {
      ctx->period = mb_photo->val;                   // changement de la periode
      mb_photo->state = EMPTY;
  }
}

//--------- definition de la tache Mess

struct Mess_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
  char mess[20];
} ; 

void setup_Mess( struct Mess_s * ctx, int timer, unsigned long period, const char * mess) {
  ctx->timer = timer;
  ctx->period = period;
  strcpy(ctx->mess, mess);
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
  while(!Serial);
}

void loop_Mess(struct Mess_s *ctx) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  Serial.println(ctx->mess);                              // affichage du message
}

//--------- Déclaration des tâches

struct Led_s Led1;
struct Mess_s Mess1;
struct Oled_s Oled1;
struct Lum_s Lum1;
struct Isr_s Isr1;
struct Buzzer_s Buzz1;
struct Button_s Button1;

//---------- Déclaration des mailboxs

struct mailbox_s mb_photo_oled = {.state = EMPTY};
struct mailbox_s mb_photo_led = {.state = EMPTY};
struct mailbox_s mb_isr_led = {.state = EMPTY,.val = 0};
struct mailbox_s mb_photo_buzz = {.state = EMPTY};
struct mailbox_s mb_button_buzz = {.state = EMPTY,.val = 0};

//--------- Setup et Loop

void setup() {
  setup_Led(&Led1, 0, 1, LED_BUILTIN);                        // Led est exécutée toutes les 100ms 
  setup_Mess(&Mess1, 1, 5000000, "bonjour");              // Mess est exécutée toutes les secondes 
  setup_oled(&Oled1, 2, 500000);
  setup_lum(&Lum1,3,500000);
  setup_isr(&Isr1, 4, 500000);
  setup_buzzer(&Buzz1, 5, 100000);
  setup_button(&Button1,6,20000);
}

void loop() {
  loop_Led(&Led1,&mb_photo_led,&mb_isr_led);                                      
  loop_Mess(&Mess1);
  loop_oled(&Oled1,&mb_photo_oled); 
  loop_lum(&Lum1,&mb_photo_oled,&mb_photo_led,&mb_photo_buzz);
  loop_isr(&Isr1,&mb_isr_led);
  loop_buzzer(&Buzz1,&mb_photo_buzz,&mb_button_buzz);
  loop_button(&Button1,&mb_button_buzz);
}
 