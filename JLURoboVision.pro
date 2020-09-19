TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lgxiapi \
        -lpthread \
        -lX11

unix: PKGCONFIG += opencv
INCLUDEPATH += /usr/local/include \
               /usr/local/include/opencv \
               /usr/local/include/opencv2

# 编译生成的so文件（类似于windows下的dll文件）
LIBS += /usr/local/lib/libopencv_calib3d.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_imgcodecs.so\
        /usr/local/lib/libopencv_objdetect.so\
        /usr/local/lib/libopencv_photo.so \
        /usr/local/lib/libopencv_dnn.so \
        /usr/local/lib/libopencv_shape.so\
        /usr/local/lib/libopencv_features2d.so \
        /usr/local/lib/libopencv_stitching.so \
        /usr/local/lib/libopencv_flann.so\
        /usr/local/lib/libopencv_superres.so \
        /usr/local/lib/libopencv_videoio.so \
        /usr/local/lib/libopencv_video.so\
        /usr/local/lib/libopencv_videostab.so \
        /usr/local/lib/libopencv_ml.so

SOURCES += \
        AngleSolver/AngleSolver.cpp \
        Armor/ArmorBox.cpp \
        Armor/ArmorDetector.cpp \
        Armor/ArmorNumClassifier.cpp \
        Armor/findLights.cpp \
        Armor/LightBar.cpp \
        Armor/matchArmors.cpp \
        GxCamera/GxCamera.cpp \
        Main/ArmorDetecting.cpp \
        Main/ImageUpdating.cpp \
        Main/main.cpp \
        Serial/Serial.cpp

SUBDIRS += \
    Armor/ARMOR.pro

DISTFILES += \
    123svm.xml \
    camera_params.xml

HEADERS += \
    AngleSolver/AngleSolver.h \
    Armor/Armor.h \
    GxCamera/include/DxImageProc.h \
    GxCamera/include/GxIAPI.h \
    GxCamera/GxCamera.h \
    General/General.h \
    Serial/Serial.h
