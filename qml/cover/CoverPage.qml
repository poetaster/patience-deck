/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021  Tomi Leppänen
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

import QtQuick 2.0
import Sailfish.Silica 1.0
import Patience 1.0

CoverBackground {
    Column {
        anchors.centerIn: parent
        width: parent.width

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: Patience.getIconPath(Theme.iconSizeLauncher)
            sourceSize.height: Theme.iconSizeLauncher
            sourceSize.width: Theme.iconSizeLauncher
        }

        Item {
            height: Theme.paddingMedium
            width: parent.width
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Patience.gameName
            horizontalAlignment: Text.AlignHCenter
            width: parent.width - 2 * Theme.paddingMedium
            wrapMode: Text.Wrap
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Patience.elapsedTime
            color: Theme.secondaryColor
        }

        Icon {
            anchors.horizontalCenter: parent.horizontalCenter
            source: Patience.state === Patience.WonState
                    ? "../../buttons/icon-m-fireworks.svg"
                    : "../../buttons/icon-m-skull.svg"
            sourceSize.height: Theme.iconSizeLarge
            sourceSize.width: Theme.iconSizeLarge
            visible: Patience.state >= Patience.GameOverState
        }
    }
}
