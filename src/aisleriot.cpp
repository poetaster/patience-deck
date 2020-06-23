#include <QObject>
#include <QJSEngine>
#include <QQmlEngine>
#include "aisleriot.h"
#include "card.h"
#include "slot.h"

Aisleriot* Aisleriot::s_game = nullptr;

Aisleriot* Aisleriot::instance()
{
    if (!s_game)
        s_game = new Aisleriot();
    return s_game;
}

QObject* Aisleriot::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return instance();
}

Aisleriot::Aisleriot(QObject *parent)
    : QObject(parent)
    , m_canUndo(false)
    , m_canRedo(false)
    , m_canDeal(false)
    , m_state(Aisleriot::UninitializedState)
{
}

Aisleriot::~Aisleriot()
{
}

void Aisleriot::startNewGame()
{
    // TODO
}

void Aisleriot::restartGame()
{
    // TODO
}

bool Aisleriot::loadGame(QString gameFile)
{
    Q_UNUSED(gameFile) // TODO
    return false; // TODO
}

void Aisleriot::undoMove()
{
    // TODO
}

void Aisleriot::redoMove()
{
    // TODO
}

bool Aisleriot::canUndo() const
{
    return m_canUndo;
}

void Aisleriot::setCanUndo(bool canUndo)
{
    if (m_canUndo != canUndo) {
        m_canUndo = canUndo;
        emit canUndoChanged();
    }
}

bool Aisleriot::canRedo() const
{
    return m_canRedo;
}

void Aisleriot::setCanRedo(bool canUndo)
{
    if (m_canRedo != canUndo) {
        m_canRedo = canUndo;
        emit canRedoChanged();
    }
}

bool Aisleriot::canDeal() const
{
    return m_canDeal;
}

void Aisleriot::setCanDeal(bool canUndo)
{
    if (m_canDeal != canUndo) {
        m_canDeal = canUndo;
        emit canDealChanged();
    }
}

QString Aisleriot::gameFile() const
{
    return m_gameFile;
}

void Aisleriot::setGameFile(QString file)
{
    if (m_gameFile != file) {
        m_gameFile = file;
        emit gameFileChanged();
    }
}

int Aisleriot::score() const
{
    return m_score;
}

void Aisleriot::setScore(int score)
{
    if (m_score != score) {
        m_score = score;
        emit scoreChanged();
    }
}

Aisleriot::GameState Aisleriot::state() const
{
    return m_state;
}

void Aisleriot::setState(GameState state)
{
    if (m_state != state) {
        m_state = state;
        // TODO: Stop timer, record time
        emit stateChanged();
    }
}

QString Aisleriot::message() const
{
    return m_message;
}

void Aisleriot::setMessage(QString message)
{
    if (m_message != message) {
        m_message = message;
        emit messageChanged();
    }
}

void Aisleriot::addSlot(QSharedPointer<Slot> slot)
{
    m_cardSlots.append(slot);
}

QSharedPointer<Slot> Aisleriot::getSlot(int slot)
{
    Q_ASSERT_X(slot > 0 && slot < m_cardSlots.count(), "getSlot", "invalid slot index");
    return m_cardSlots[slot];
}

void Aisleriot::testGameOver()
{
    endMove();
    updateDealable();
    if (m_state < GameOverState) {
        if (isGameOver()) {
            setState(isWinningGame() ? WonState : GameOverState);
        }
    }
}

void Aisleriot::clearGame()
{
    m_cardSlots.clear();
    // TODO: Emit signal for clearing game
}