QT += core gui sql printsupport charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    chatbot.cpp \
    connexion.cpp \
    gpersonnel.cpp \
    influenceur.cpp \
    main.cpp \
    materiel.cpp \
    ml_predictor.cpp \
    roi_analyzer.cpp \
    smart_creation.cpp \
    twillio.cpp \
    video.cpp

HEADERS += \
    chatbot.h \
    connexion.h \
    gpersonnel.h \
    influenceur.h \
    materiel.h \
    ml_predictor.h \
    roi_analyzer.h \
    smart_creation.h \
    twillio.h \
    video.h

FORMS += \
    smart_creation.ui

RESOURCES += \
    ressources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
