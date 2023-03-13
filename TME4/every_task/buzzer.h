#include "esp32-hal-ledc.h"
#include "timer.h"
#include "flag.h"

#define PWM_CHANNEL 0
#define PWM_PIN 17
#define MIN_OCTAVE 3
#define MAX_OCTAVE 5

//--------- définition de la tache Oled

struct Buzzer_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  note_t current_note    ;                                    // note courrante
  int current_octave ;                                     //octave courante
  bool state;
}; 

void setup_buzzer(Buzzer_s * ctx,int timer,int period)
{
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->current_note = NOTE_C;
  ctx->current_octave = MIN_OCTAVE;
  ctx->state = true;

  //configuration de la broche PWM pour le buzzer, on associe le channel au pin
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void loop_buzzer(Buzzer_s * ctx,mailbox_s * mb_photo,mailbox_s * mb_button)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente
  // Serial.printf("note %d %d\n",int( ctx->current_note),ctx->current_octave);

  if (mb_button->state == FULL); // attend que la mailbox button soit vide pour gestion de l'appuie
  {
    if((mb_button->val & 0x10) == 0x10)
    {
      mb_button->val ^= 0x10;
      //sauvegarde des registres
      note_t note = ctx->current_note;
      int octave = ctx->current_octave;
      bool state = ctx->state;
      setup_buzzer(ctx, ctx->timer, ctx->period);
      ctx->state =  !state;
      ctx->current_note = note;
      ctx->current_octave = octave;
    }
    mb_photo->state = EMPTY;
  }

  if(ctx->state == false)  //on quitte s'il est éteint
  {
    ledcDetachPin(PWM_PIN);
    return;    
  }

  ledcWriteNote(PWM_CHANNEL, ctx->current_note, ctx->current_octave);//on joue la note via PWM

  if((ctx->current_note = note_t(int(ctx->current_note) + 1)) == NOTE_MAX)//on incrémente la note joué à chaque periode
  {
    ctx->current_note = NOTE_C;
    if(ctx->current_octave++ > MAX_OCTAVE)
      ctx->current_octave = MIN_OCTAVE;
  }
  //mailbox de la photo-résistance qui controle la vitesse du LFO (periode) 
  if (mb_photo->state == FULL); // attend que la mailbox led soit vide
  {
    ctx->period = mb_photo->val; // changement de la periode
    mb_photo->state = EMPTY;
  }
}
