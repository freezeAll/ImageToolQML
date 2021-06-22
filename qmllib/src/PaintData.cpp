//
// Created by lyric on 2021/4/22.
//

#include "PaintData.hpp"
//#include "ImageToolQML.hpp"
#include <QPainter>
#include <QVariant>
#include <QMetaObject>
#include <QMetaProperty>
ImageToolPaintData::ImageToolPaintData(QObject* parent) :
        QObject(parent)
{

}

ImageToolPaintData::~ImageToolPaintData()
{
}

ImageToolPaintData::ImageToolPaintData(const ImageToolPaintData& other)
{
    operator=(other);
}

void ImageToolPaintData::operator=(const ImageToolPaintData& other)
{
    for(auto& obj : other.paintObjects)
    {
        auto newObj = qobject_cast<PaintObject*>(obj->metaObject()->newInstance());
        if(newObj)
            *newObj = *obj;
    }
    std::copy(other.paintObjects.begin(),other.paintObjects.end(),std::back_inserter(paintObjects));
}

void ImageToolPaintData::paint(QPainter* painter,const QRectF& imgRect, const QSizeF& itemSize)
{
    for(auto& obj : paintObjects)
    {
        obj->paint(painter,imgRect,itemSize);
    }
}

PaintObject::PaintObject(QObject *parent) : QObject(parent)
{
}

QVariant PaintObject::getPaintPen() const
{
    return paintPen;
}

QVariant PaintObject::getPaintBrush() const
{
    return paintBrush;
}

void PaintObject::setPaintPen(QVariant var)
{
    if(var.canConvert<QPen>())
        paintPen = var.value<QPen>();
}

void PaintObject::setPaintBrush(QVariant var)
{
    if(var.canConvert<QBrush>())
        paintBrush = var.value<QBrush>();
}


PaintObject::~PaintObject()
{

}

void PaintObject::paint(QPainter *, const QRectF& imgRect, const QSizeF& itemSize)
{

}

QPointF PaintObject::mapToPaint(const QPointF &source, const QRectF &imgRect, const QSizeF &itemSize)
{
    double power(1.);
    if (imgRect.width() > imgRect.height())
    {
        power = itemSize.width() / imgRect.width();
    }
    else
    {
        power = itemSize.height() / imgRect.height();
    }
    return QPointF((source.x() - imgRect.x()) * power, (source.y() - imgRect.y()) * power);
}

QPointF PaintObject::mapToSource(const QPointF &mouse, const QRectF &imgRect, const QSizeF &itemSize)
{
    double power(1.);
    if (imgRect.width() > imgRect.height())
    {
        power = imgRect.width() / itemSize.width();
    }
    else
    {
        power = imgRect.height() / itemSize.height();
    }
    return QPointF(mouse.x() * power + imgRect.x(), mouse.y() * power + imgRect.y());
}

void PaintObject::operator=(const PaintObject &other)
{
    for (auto i = 0; i < other.metaObject()->propertyCount(); i++)
    {
        metaObject()->property(i).write(this, metaObject()->property(i).read(&other));
    }
}

PaintObject::PaintObject(const PaintObject & other)
{
    operator=(other);
}

QRectF PaintRect::getRect()
{
    return QRectF(x,y,width,height);
}

PaintRect::PaintRect(QObject *parent) : PaintObject(parent)
{

}

QVariant PaintRect::getX() const
{
    return x;
}
QVariant PaintRect::getY() const
{
    return y;
}

void PaintRect::setX(QVariant var)
{
    x = var.toDouble();
}
void PaintRect::setY(QVariant var)
{
    y = var.toDouble();
}

QVariant PaintRect::getWidth() const
{
    return width;
}

QVariant PaintRect::getHeight() const
{
    return height;
}

void PaintRect::setWidth(QVariant var)
{
    width = var.toDouble();
}

void PaintRect::setHeight(QVariant var)
{
    height = var.toDouble();
}

PaintRect::~PaintRect()
{

}

PaintRect::PaintRect(const PaintRect &other)
{
    operator=(other);
}

void PaintPath::paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize)
{
    QVector<QLineF> plines;
    std::copy(lines.begin(),lines.end(),std::back_inserter(plines));
    for(auto& ln : plines)
    {
        auto p1 = mapToPaint(ln.p1(),imgRect,itemSize);
        auto p2 = mapToPaint(ln.p2(),imgRect,itemSize);
        ln = QLineF(p1,p2);
    }
    painter->setPen(paintPen);
    painter->setBrush(paintBrush);
    painter->drawLines(plines);
}

void PaintPath::paintImage(cv::Mat* img)
{
    try
    {
        for(auto& ln : lines)
        {
            auto p1 = ln.p1();
            auto p2 = ln.p2();
            auto cl = paintPen.color();
            cv::line(*img,cv::Point2d(p1.x(),p1.y()),cv::Point2d(p2.x(),p2.y()),cv::Scalar(cl.blue(),cl.green(),cl.red()),paintPen.width());
        }
    }
    catch(const cv::Exception& e)
    {

    }
}

QVariant PaintPath::getLines() const
{
    QVector<QLineF> out;
    std::copy(lines.begin(),lines.end(),std::back_inserter(out));
    QVariant var;
    var.setValue(out);
    return var;
}

void PaintPath::setLines(QVariant var)
{
    if(var.canConvert<QVector<QLineF>>())
    {
        auto input = var.value<QVector<QLineF>>();
        lines.clear();
        std::copy(input.begin(),input.end(),std::back_inserter(lines));
    }
}

void PaintPath::addLineF(const QVariant& pt1,const QVariant& pt2)
{
    if(pt1.canConvert<QPointF>() && pt2.canConvert<QPointF>())
    {
        lines.push_back(QLineF(pt1.toPointF(), pt2.toPointF()));
    }
}

void PaintPath::addLineF(const QVariant& line)
{
    if(line.canConvert<QLineF>())
    {
        lines.push_back(line.toLineF());
    }

}

PaintPath::PaintPath(QObject *parent) : PaintObject(parent)
{

}

PaintPath::~PaintPath()
{

}

PaintPath::PaintPath(const PaintPath &other)
{
    operator=(other);
}

void ImageToolPaintData::pushPaintObject(PaintObject* obj)
{
    if(obj)
    {
        auto newObj = dynamic_cast<PaintObject*>( obj->metaObject()->newInstance());
        *newObj = *obj;
        newObj->setParent(this);
        paintObjects.push_back(newObj);
    }
}

QList<PaintObject*> ImageToolPaintData::getPaintObjects() const
{
    return paintObjects;
}

PaintObject *ImageToolPaintData::createPaintObject(const QVariant& type)
{
    auto str = type.toString();
    if(str == "rect")
    {
        return new PaintRect(this);
    }
    if(str == "path")
    {
        return new PaintPath(this);
    }
    if(str == "text")
    {
        return new PaintText(this);
    }
    if(str == "cross")
    {
        return new PaintCrossLine(this);
    }
    return nullptr;

}

void ImageToolPaintData::clearPaintObjects()
{
    for(auto &pobj : paintObjects)
    {
        pobj->deleteLater();

    }
    paintObjects.clear();
}


void PaintObject::paintImage(cv::Mat* cvimg)
{

}

void PaintObject::setPaintBrushColor(QVariant var)
{
    if(var.canConvert<QColor>())
        paintBrush.setColor(var.value<QColor>());
}

QVariant PaintObject::getPaintPenColor() const
{
    return paintPen.color();
}

QVariant PaintObject::getPaintBrushColor() const
{
    return paintBrush.color();
}

void PaintObject::setPaintPenColor(QVariant var)
{
    if(var.canConvert<QColor>())
        paintPen.setColor(var.value<QColor>());
}

void PaintRect::paint(QPainter *painter, const QRectF& imgRect, const QSizeF& itemSize)
{
    painter->setPen(paintPen);
    painter->setBrush(paintBrush);
    auto rect = getRect();
    auto paintTl = mapToPaint( rect.topLeft(),imgRect,itemSize);
    auto paintBr = mapToPaint( rect.bottomRight(),imgRect,itemSize);
    painter->drawRect(QRectF(paintTl,paintBr));
}

void PaintRect::paintImage(cv::Mat* cvimg)
{
    try
    {
        if(cvimg)
        {
            auto penColor = paintPen.color();
            auto brushColor = paintBrush.color();
            if(paintBrush.style() != Qt::NoBrush)
                cv::rectangle(*cvimg,cv::Rect(x,y,width,height),cv::Scalar(brushColor.blue(),brushColor.green(),brushColor.red()),-1);
            cv::rectangle(*cvimg,cv::Rect(x,y,width,height),cv::Scalar(penColor.blue(),penColor.green(),penColor.red()),paintPen.width());
        }
    }
    catch(const cv::Exception& e)
    {

    }
}

PaintText::PaintText(QObject* parent) :
        PaintObject(parent)
{
    font.setFamily("Microsoft YaHei UI");
    font.setPixelSize(16);
}

QVariant PaintText::getTextPosition() const
{
    return textPosition;
}

void PaintText::setTextPosition(QVariant var)
{
    textPosition = var.toPointF();
}

PaintText::~PaintText()
{

}

PaintText::PaintText(const PaintText & other)
{
    operator=(other);
}
void PaintText::paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize)
{
    double power = 1.;
    if (imgRect.width() > imgRect.height())
    {
        power = itemSize.width() /imgRect.width();
    }
    else
    {
        power = itemSize.height() / imgRect.height();
    }
    auto paintFont = font;
    auto text_scale = double(paintFont.pixelSize()) * power;
    paintFont.setPixelSize(std::floor(text_scale < 1 ? 1 : text_scale));
    painter->setPen(paintPen);
    painter->setBrush(paintBrush);
    painter->setFont(paintFont);
    auto paintTextPosition = mapToPaint(textPosition,imgRect,itemSize);
    painter->drawText(paintTextPosition,text);
}
void PaintText::paintImage(cv::Mat* img)
{
    try
    {

        auto ppC = paintPen.color();
        cv::putText(
            *img,
            text.toStdString(),
            cv::Point2d(textPosition.x(),textPosition.y()) ,
            cv::FONT_HERSHEY_COMPLEX,
            cv::getFontScaleFromHeight(cv::FONT_HERSHEY_COMPLEX,font.pixelSize()),
            cv::Scalar(ppC.blue(),ppC.green(),ppC.red()));

    }
    catch(const cv::Exception& e)
    {

    }
}

QVariant PaintText::getText() const
{
    return text;
}

void PaintText::setText(QVariant var)
{
    text = var.toString();
}

QVariant PaintText::getTextWidth() const
{
    return font.pixelSize();
}
void PaintText::setTextWidth(QVariant var)
{
    font.setPixelSize(var.toInt());
}

QVariant PaintText::getTextFamily() const
{
    return font.family();
}
void PaintText::setTextFamily(QVariant var)
{
    font.setFamily(var.toString());
}

PaintCrossLine::PaintCrossLine(QObject *parent) :
    PaintObject(parent),
    length(3),
    width(1)
{

}

PaintCrossLine::~PaintCrossLine()
{

}

PaintCrossLine::PaintCrossLine(const PaintCrossLine &other)
{
    operator=(other);
}

void PaintCrossLine::paint(QPainter *painter, const QRectF &imgRect, const QSizeF &itemSize)
{
    QLineF horLine(mapToPaint( QPointF(x + double(length) / 2.,y),imgRect,itemSize),mapToPaint(QPointF(x - double(length) / 2.,y),imgRect,itemSize));
    QLineF verLine(mapToPaint( QPointF(x ,y + double(length) / 2.),imgRect,itemSize),mapToPaint( QPointF(x ,y- double(length) / 2.),imgRect,itemSize));
    paintPen.setWidth(width);
    painter->setPen(paintPen);
    painter->drawLine(horLine);
    painter->drawLine(verLine);
}

void PaintCrossLine::paintImage(cv::Mat * paintImg)
{
    auto pColor = paintPen.color();
    auto cl = cv::Scalar (pColor.blue(),pColor.green(),pColor.red());
    cv::line(*paintImg,cv::Point2d(x + double(length) / 2.,y),cv::Point2d(x - double(length) / 2.,y),cl,width);
    cv::line(*paintImg,cv::Point2d(x,y + double(length) / 2.),cv::Point2d(x ,y- double(length) / 2.),cl,width);
}

QVariant PaintCrossLine::getX() const
{
    return x;
}
QVariant PaintCrossLine::getY() const
{
    return y;
}

void PaintCrossLine::setX(QVariant var)
{
    x = var.toDouble();
}

void PaintCrossLine::setY(QVariant var)
{
    y = var.toDouble();
}

QVariant PaintCrossLine::getWidth() const
{
    return width;
}

void PaintCrossLine::setWidth(QVariant var)
{
    width = var.toInt();
}

QVariant PaintCrossLine::getLength() const
{
    return length;
}

void PaintCrossLine::setLength(QVariant var)
{
    length = var.toInt();
}

QVariant PaintCrossLine::getCenter() const
{
    return QPointF(x,y);
}

void PaintCrossLine::setCenter(QVariant var)
{
    auto pnt = var.toPointF();
    x = pnt.x();
    y = pnt.y();
}