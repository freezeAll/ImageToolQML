#pragma once
#include <QQuickWidget>
#include "opencv2/opencv.hpp"
#include <QWidget>
#ifndef BUILD_STATIC
# if defined(IMAGETOOL_LIB)
#  define IMAGETOOL_EXPORT Q_DECL_EXPORT
# else
#  define IMAGETOOL_EXPORT Q_DECL_IMPORT
# endif
#else
# define IMAGETOOL_EXPORT
#endif
#include "ImageToolQML.hpp"
class IMAGETOOL_EXPORT ImageTool : public QQuickWidget
{
    Q_OBJECT
public:
    ImageTool(QWidget *parent = nullptr);
    ~ImageTool();
    ImageToolQML* getImageToolQMLPointer();
};
