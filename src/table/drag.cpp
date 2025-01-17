/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QScreen>
#include <QStyleHints>
#include "card.h"
#include "constants.h"
#include "drag.h"
#include "feedbackevent.h"
#include "logging.h"
#include "selection.h"
#include "slot.h"
#include "table.h"

QElapsedTimer Drag::s_doubleClickTimer;

const Card *Drag::s_lastCard = nullptr;

Drag::Drag(QMouseEvent *event, Table *table, Card *card, Selection *selection)
    : QQuickItem(card)
    , m_state(selection && selection->state() < Selection::Finished ? WaitingForSelection : NoDrag)
    , m_mayBeADoubleClick(couldBeDoubleClick(card))
    , m_table(table)
    , m_card(card)
    , m_source(card->slot())
    , m_selection(m_state == WaitingForSelection ? selection : nullptr)
    , m_target(-1)
{
    setParentItem(table);
    setX(m_source->x());
    setY(m_source->y());

    auto engine = Engine::instance();
    connect(this, &Drag::doDrag, engine, &Engine::drag);
    connect(this, &Drag::doCancelDrag, engine, &Engine::cancelDrag);
    connect(this, &Drag::doCheckDrop, engine, &Engine::checkDrop);
    connect(this, &Drag::doDrop, engine, &Engine::drop);
    connect(this, &Drag::doClick, engine, &Engine::click);
    connect(this, &Drag::doDoubleClick, engine, &Engine::doubleClick);
    connect(engine, &Engine::couldDrag, this, &Drag::handleCouldDrag);
    connect(engine, &Engine::couldDrop, this, &Drag::handleCouldDrop);
    connect(engine, &Engine::dropped, this, &Drag::handleDropped);
    connect(engine, &Engine::clicked, this, &Drag::handleClicked);
    connect(engine, &Engine::doubleClicked, this, &Drag::handleDoubleClicked);

    m_startPoint = m_lastPoint = card->mapToItem(m_table, event->pos());
    m_timer.start();

    if (m_state == WaitingForSelection) {
        connect(selection, &Selection::finished, this, [this, selection] {
            if (m_state == Canceled) {
                // User raised their finger before selection finished
                qCDebug(lcDrag) << "Drag of" << m_card << "was canceled before" << selection << "finished";
                done();
                return;
            }

            switch (selection->state()) {
            case Selection::Finished:
                // Selection did the work for us
                m_state = Finished;
                qCDebug(lcDrag) << "Drag of" << m_card << "ended by" << selection;
                done();
                break;
            case Selection::Canceled:
                // No selection, let's drag the cards
                qCDebug(lcDrag) << "Continuing drag of" << m_card << "after" << selection << "ended";
                m_state = NoDrag;
                break;
            default:
                qCCritical(lcDrag) << selection << "finished in unexpected state";
                break;
            }
            m_selection = selection->clone();
        });
        selection->blockVibration();
        if (selection->contains(card))
            selection->finish(m_source);
        else
            selection->cancel();
        qCDebug(lcDrag) << "Drag of" << m_card << "for" << m_source << "waiting for" << selection;
    } else {
        qCDebug(lcDrag) << "Started drag of" << m_card << "for" << m_source;
    }
}

Drag::~Drag()
{
    if (m_state < Finished)
        qCWarning(lcDrag) << "Drag was not finished or canceled when it was destroyed";
    if (m_selection)
        m_selection->deleteLater();
}

Card *Drag::card() const
{
    return m_card;
}

Card *Drag::firstCard() const
{
    return m_selection ? m_selection->card() : m_card;
}

void Drag::update(QMouseEvent *event)
{
    if (mayBeAClick(event))
        return;

    switch (m_state) {
    case AboutToDrag:
        m_state = StartingDrag;
        emit doDrag(CountableId::id(), m_source->id(), m_source->asCardData(firstCard()));
        break;
    case Dragging: {
        move(m_card->mapToItem(m_table, event->pos()));
        checkTargets();
        break;
    }
    default:
        qCCritical(lcDrag) << "Invalid state in update:" << m_state;
        [[fallthrough]];
    case WaitingForSelection:
    case StartingDrag:
    case Canceled:
        break;
    }
}

void Drag::finish(QMouseEvent *event)
{
    if (mayBeAClick(event)) {
        m_state = Clicked;
        if (m_mayBeADoubleClick) {
            qCDebug(lcDrag) << "Detected double click on" << m_card;
            emit doDoubleClick(id(), m_source->id());
        } else {
            qCDebug(lcDrag) << "Detected click on" << m_card;
            emit doClick(id(), m_source->id());
        }
        return;
    }

    switch (m_state) {
    case Dragging:
        move(m_card->mapToItem(m_table, event->pos()));
        m_state = Dropping;
        checkTargets(true);
        break;
    case Canceled:
        done();
        break;
    default:
        cancel();
        break;
    }
}

void Drag::move(const QPointF point)
{
    QPointF move = point - m_lastPoint;
    setX(x() + move.x());
    setY(y() + move.y());
    m_lastPoint = point;
}

void Drag::checkTargets(bool force)
{
    auto targets = m_table->getSlotsFor(m_card, m_cards, m_source);
    if (force || m_targets != targets) {
        m_target = -1;
        m_targets = targets;
        highlightOrDrop();
    }
}

void Drag::highlightOrDrop()
{
    if (m_state > Dropping)
        return;

    m_target++;
    if (m_target < m_targets.count()) {
        Slot *target = m_targets.at(m_target);
        switch (m_couldDrop.value(target->id(), Unknown)) {
        case Unknown:
            // Cache miss, check
            m_couldDrop.insert(target->id(), Checking);
            qCDebug(lcDrag) << "Testing move from" << m_source << "to" << target;
            emit doCheckDrop(id(), m_source->id(), target->id(), toCardData(m_cards, m_state));
            [[fallthrough]];
        case Checking: // Signal for the same slot received twice, the correct signal should still arrive
            m_target--; // Check the slot again later
            break;
        case CanDrop:
            // Found target to highlight or drop to
            if (m_state < Dropping) {
                qCDebug(lcDrag) << "Highlighting" << target;
                m_table->highlight(target);
            } else {
                m_table->highlight(nullptr);
                drop(target);
            }
            break;
        case CantDrop:
            // Recurse to next iteration
            highlightOrDrop();
            break;
        }
    } else {
        // No suitable target, remove highlights or cancel
        if (m_state < Dropping) {
            m_table->highlight(nullptr);
        } else {
            emit m_table->feedback()->dropFailed();
            cancel();
        }
    }
}

void Drag::drop(Slot *slot)
{
    qCDebug(lcDrag) << "Moving from" << m_source << "to" << slot;
    m_state = Dropped;
    emit doDrop(id(), m_source->id(), slot->id(), toCardData(m_cards, m_state));
}

void Drag::cancel()
{
    qCDebug(lcDrag) << "Canceling drag of" << m_card << "at state" << m_state;

    if (m_state < Dropped) {
        if (m_state >= Dragging) { 
            emit doCancelDrag(id(), m_source->id(), toCardData(m_cards, Canceled));
            setParentItem(nullptr);
            m_source->put(m_cards);
            m_cards.clear();
        }

        if (m_state == Dropping || m_state == AboutToDrag)
            done();
        m_state = Canceled;
    }
}

void Drag::handleCouldDrag(quint32 id, int slotId, bool could)
{
    if (id != CountableId::id() && slotId != m_source->id())
        return;

    if (m_state == StartingDrag && could) {
        m_state = Dragging;
        m_cards = m_source->take(firstCard());
        if (m_cards.empty())
            qCCritical(lcDrag) << "Tried to drag empty stack of cards from slot" << m_source
                               << "starting from" << m_card;
        for (Card *card : m_cards) {
            if (card)
                card->moveTo(this);
            else
                qCCritical(lcDrag) << "Tried to drag non-existing card";
        }

        checkTargets();
    } else if (m_state == Canceled) {
        if (could)
            emit doCancelDrag(CountableId::id(), m_source->id(), m_source->asCardData(firstCard()));
        done();
    } else {
        cancel();
    }
}

void Drag::handleCouldDrop(quint32 id, int slotId, bool could)
{
    if (id != CountableId::id())
        return;

    m_couldDrop.insert(slotId, could ? CanDrop : CantDrop);
    highlightOrDrop();
}

void Drag::handleDropped(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != CountableId::id())
        return;

    if (m_state >= Finished) {
        qCWarning(lcDrag) << this << "has already handled dropping";
        return;
    }

    if (could) {
        emit m_table->feedback()->dropSucceeded();
        m_state = Finished;
        m_table->store(m_cards);
    } else {
        // Unlikely
        emit m_table->feedback()->dropFailed();
        m_state = Canceled;
        m_source->put(m_cards);
    }
    m_cards.clear();
    done();
}

void Drag::handleClicked(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != CountableId::id())
        return;

    if (could) {
        emit m_table->feedback()->clicked();
        s_doubleClickTimer.invalidate();
    } else {
        if (!m_selection || m_selection->card() != m_card)
            m_table->select(m_card);
        emit m_table->feedback()->selectionChanged();
    }

    done();
}

void Drag::handleDoubleClicked(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != CountableId::id())
        return;

    if (could)
        emit m_table->feedback()->clicked();

    done();
}

void Drag::done()
{
    emit finished();
    deleteLater();
}

bool Drag::mayBeAClick(QMouseEvent *event)
{
    if (m_state != NoDrag)
        return false;

    auto styleHints = QGuiApplication::styleHints();
    if (m_timer.hasExpired(styleHints->startDragTime()))
        m_state = AboutToDrag;

    qreal distance = (m_startPoint - m_card->mapToItem(m_table, event->pos())).manhattanLength();
    if (distance >= styleHints->startDragDistance())
        m_state = AboutToDrag;

    return m_state == NoDrag;
}

bool Drag::couldBeDoubleClick(const Card *card)
{
    qint64 time = (card == s_lastCard && s_doubleClickTimer.isValid()) ?
        s_doubleClickTimer.elapsed() : std::numeric_limits<qint64>::max();
    s_lastCard = card;

    if (time <= QGuiApplication::styleHints()->mouseDoubleClickInterval()) {
        s_doubleClickTimer.invalidate();
        return true;
    } else {
        s_doubleClickTimer.start();
        return false;
    }
}

CardList Drag::toCardData(const QList<Card *> &cards, DragState state)
{
    CardList list;
    for (const Card *card : cards) {
        if (card)
            list << card->data();
        else
            qCCritical(lcDrag) << "Tried to convert non-existing card to data";
    }
    if (list.isEmpty())
        qCCritical(lcDrag) << "Returning an empty list of CardData in state" << state;
    return list;
}

QDebug operator<<(QDebug debug, const Drag *drag)
{
    if (drag) {
        debug.nospace() << "Drag(";
        switch (drag->m_state) {
        case Drag::WaitingForSelection:
            debug.nospace() << "WaitingForSelection";
            break;
        case Drag::NoDrag:
            debug.nospace() << "NoDrag";
            break;
        case Drag::AboutToDrag:
            debug.nospace() << "AboutToDrag";
            break;
        case Drag::StartingDrag:
            debug.nospace() << "StartingDrag";
            break;
        case Drag::Dragging:
            debug.nospace() << "Dragging";
            break;
        case Drag::Dropping:
            debug.nospace() << "Dropping";
            break;
        case Drag::Dropped:
            debug.nospace() << "Dropped";
            break;
        case Drag::Finished:
            debug.nospace() << "Finished";
            break;
        case Drag::Canceled:
            debug.nospace() << "Canceled";
            break;
        case Drag::Clicked:
            debug.nospace() << "Clicked";
            break;
        }
        debug.nospace() << ")";
    } else {
        debug.nospace() << "invalid drag";
    }
    return debug.space();
}
