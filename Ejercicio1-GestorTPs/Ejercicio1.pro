QT += widgets



CONFIG += c++17



SOURCES += \

    actionlog.cpp \

    edittrabajodialog.cpp \

    loginwidget.cpp \

    main.cpp \

    mainwidget.cpp \

    notasdialog.cpp \

    sessionmanager.cpp \

    trabajostore.cpp \

    userrepository.cpp



HEADERS += \

    actionlog.h \

    edittrabajodialog.h \

    loginwidget.h \

    mainwidget.h \

    notasdialog.h \

    sessionmanager.h \

    trabajo.h \

    trabajostore.h \

    userrepository.h



# Default rules for deployment.

qnx: target.path = /tmp/$${TARGET}/bin

else: unix:!android: target.path = /opt/$${TARGET}/bin

!isEmpty(target.path): INSTALLS += target

