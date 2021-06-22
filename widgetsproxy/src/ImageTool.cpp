#include "ImageTool.hpp"

ImageTool::ImageTool(QWidget *parent) :
    QQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSource(QUrl( "qrc:/ImageTool/plugin.qml"));
}

ImageTool::~ImageTool()
{

}

ImageToolQML *ImageTool::getImageToolQMLPointer()
{
    return dynamic_cast<ImageToolQML*>(rootObject());
}
