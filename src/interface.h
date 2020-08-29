#include <libguile.h>
#include <QString>
#include "engine_p.h"

namespace Interface {

// Functions that are part of Aisleriot's Guile interface
SCM setFeatureWord(SCM features);
SCM getFeatureWord(void);
SCM setStatusbarMessage(SCM message);
SCM resetSurface(void);
SCM addCardSlot(SCM slotData);
SCM getCardSlot(SCM slotId);
SCM setCards(SCM slotId, SCM newCards);
SCM setSlotYExpansion(SCM slotId, SCM newExpVal);
SCM setSlotXExpansion(SCM slotId, SCM newExpVal);
SCM setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
              SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
              SCM winningGameLambda, SCM hintLambda, SCM rest);
SCM setLambdaX(SCM symbol, SCM lambda);
SCM myrandom(SCM range);
SCM clickToMoveP(void);
SCM updateScore(SCM newScore);
SCM getTimeout(void);
SCM setTimeout(SCM newTimeout);
SCM delayedCall(SCM callback);
SCM undoSetSensitive(SCM state);
SCM redoSetSensitive(SCM state);
SCM dealableSetSensitive(SCM state);

// Initialization
void init_module(void *data);
void *init(void *data);

// Data
const int DelayedCallDelay = 50;

struct Call {
    SCM lambda;
    SCM *args;
    size_t n;
};

const char LambdaNames[] = {
  "new-game\0"
  "button-pressed\0"
  "button-released\0"
  "button-clicked\0"
  "button-double-clicked\0"
  "game-over\0"
  "winning-game\0"
  "hint\0"
  "get-options\0"
  "apply-options\0"
  "timeout\0"
  "droppable\0"
  "dealable\0"
};

}; // Interface

namespace Scheme {

// Unwind handlers
SCM preUnwindHandler(void *data, SCM tag, SCM throwArgs);
SCM catchHandler(void *data, SCM tag, SCM throwArgs);

// Helpers
inline QString getMessage(SCM message);
EnginePrivate::Card createCard(SCM data);
QList<EnginePrivate::Card> cardsFromSlot(SCM cards);
SCM cardToSCM(const EnginePrivate::Card &card);
SCM slotToSCM(const QList<EnginePrivate::Card> &slot);

// Calls from C to SCM
SCM startNewGame(void *data);
SCM loadGameFromFile(void *data);
SCM callLambda(void *data);

}; // Scheme
