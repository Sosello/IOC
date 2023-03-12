#include "esp32-hal-ledc.h"
#include "timer.h"
#include "flag.h"

#define PWM_CHANNEL 0
#define PWM_PIN 17
#define MAX_OCTAVE 6

//--------- définition de la tache Oled

struct Buzzer_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  note_t current_note    ;                                    // note courrante
  int current_octave ;                                     //octave courante
}; 

void setup_buzzer(Buzzer_s * ctx,int timer,int period)
{
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->current_note = NOTE_C;
  ctx->current_octave = 0;

  //configuration de la broche PWM pour le buzzer, on associe le channel au pin
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void loop_buzzer(Buzzer_s * ctx)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente
  Serial.printf("note %d %d\n",int( ctx->current_note),ctx->current_octave);
  ledcWriteNote(PWM_CHANNEL, ctx->current_note, ctx->current_octave);//on joue la note via PWM

  if((ctx->current_note = note_t(int(ctx->current_note) + 12)) == NOTE_MAX)//on incrémente la note joué à chaque periode
  {
    ctx->current_note = NOTE_C;
    if(ctx->current_octave++ > MAX_OCTAVE)
      ctx->current_octave = 0;
  }
}
