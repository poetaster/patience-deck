include(common.pri)

TEMPLATE = subdirs
SUBDIRS = src

CONFIG += sailfishapp

DISTFILES += common.pri \
    qml/*.qml \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/images/*.svg \
    rpm/$$NAME.spec \
    translations/*.ts \
    $$NAME.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

games.files = $$files(aisleriot/games/*.scm)
games.files -= aisleriot/games/api.scm
games.files -= aisleriot/games/card-monkey.scm
games.files -= aisleriot/games/template.scm
games.files -= aisleriot/games/test.scm
games.path = /usr/share/$$TARGET/games/

api.files = aisleriot/games/api.scm
api.path = /usr/share/$$TARGET/games/aisleriot/

data.files = aisleriot/AUTHORS \
    aisleriot/cards/anglo.svg
data.path = /usr/share/$$TARGET/data/

INSTALLS += games api data

#CONFIG += sailfishapp_i18n
#TRANSLATIONS += translations/$$NAME-de.ts
