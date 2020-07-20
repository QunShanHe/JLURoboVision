TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lgxiapi \
        -lpthread \
        -lX11

INCLUDEPATH += /usr/include/opencv4
LIBS += -L/usr/lib/aarch64-linux-gnu -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_ml -lopencv_video -lopencv_videoio  -lopencv_calib3d -lopencv_dnn -lopencv_features2d -lopencv_flann -lopencv_gapi -lopencv_photo -lopencv_stitching


SOURCES += \
        AngleSolver/AngleSolver.cpp \
        Armor/ArmorBox.cpp \
        Armor/ArmorDetector.cpp \
        Armor/ArmorNumClassifier.cpp \
        Armor/findLights.cpp \
        Armor/LightBar.cpp \
        Armor/matchArmors.cpp \
        GxCamera/GxCamera.cpp \
        main.cpp

SUBDIRS += \
    Armor/ARMOR.pro

DISTFILES += \
    123svm.xml \
    Armor/Armor.sln \
    Armor/Armor.vcxproj \
    Armor/Armor.vcxproj.filters \
    camera_params.xml

HEADERS += \
    AngleSolver/AngleSolver.h \
    Armor/Armor.h \
    GxCamera/include/DxImageProc.h \
    GxCamera/include/GxIAPI.h \
    GxCamera/GxCamera.h \
    General/General.h
