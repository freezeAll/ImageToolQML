  //
// Created by lyric on 2021/4/12.
//

#include "ImageToolQML.hpp"
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QQmlExtensionPlugin>
#include <QWheelEvent>
#include <QBrush>
#include <QPen>
#include <QCursor>
#include <QJsonDocument>
#include <QRandomGenerator>
#include "PaintData.hpp"
#include <QQmlEngine>
#include "PaintData.hpp"

class ImageToolQMLPlugin : public QQmlExtensionPlugin     // 继承QQmlExtensionPlugin
{
Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)  // 为这个插件定义了一个唯一的接口，并注册至元对象系统

public:
    ImageToolQMLPlugin(QObject* parent = 0) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char* uri)
    {
        qmlRegisterType<ImageToolQML>(uri, 1, 0, "ImageToolQML");
        qmlRegisterType<ImageToolPaintData>(uri, 1, 0, "ImageToolPaintData");
        qmlRegisterType<ImageToolShape>(uri, 1, 0, "ImageToolShape");
        qmlRegisterType<RotatedRectShape>(uri, 1, 0, "RotatedRectShape");
        qmlRegisterType<PaintObject>(uri,1,0,"PaintObject");
        qmlRegisterType<PaintPath>(uri,1,0,"PaintPath");
        qmlRegisterType<PaintRect>(uri,1,0,"PaintRect");
        qmlRegisterType<PaintText>(uri,1,0,"PaintText");

    }
};

class ImageToolQMLPrivate : public QObject
{
Q_OBJECT
public:
    ImageToolQMLPrivate(ImageToolQML* pPtr) :
            parentPtr(pPtr),
            QObject(pPtr),
            backGroudColor(125, 125, 125),
            grabedShape(nullptr),
            paintData(new ImageToolPaintData(pPtr))
    {

    }
    ~ImageToolQMLPrivate() {}

    QColor backGroudColor;
    QPixmap imgSource;
    ImageToolPaintData* paintData;
    QRectF displayArea;
    std::list<ImageToolShape*> shapes;
    QPointF startPos;

    ImageToolShape* grabedShape;
    ImageToolQML* parentPtr;
};

ImageToolQML::ImageToolQML(QQuickItem* parent) :
        QQuickPaintedItem(parent),
        privatePtr(new ImageToolQMLPrivate(this))
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setKeepMouseGrab(true);
    setFlag(ItemAcceptsInputMethod, true);
    connect(this, &ImageToolQML::widthChanged, this, &ImageToolQML::resetScale);
    connect(this, &ImageToolQML::heightChanged, this, &ImageToolQML::resetScale);

    cv::Mat testImg;
    try
    {
        testImg = cv::imread("./test.png");
        if (testImg.empty())
            testImg = cv::imread("./test.jpg");
    }
    catch (...)
    {
    }
    displayCvMat(testImg);
}

ImageToolQML::~ImageToolQML()
{

}

void ImageToolQML::paint(QPainter* painter)
{
    painter->setBrush(QBrush(privatePtr->backGroudColor));
    painter->setPen(QPen(privatePtr->backGroudColor));
    painter->drawRect(QRect(0, 0, width(), height()));
    painter->drawPixmap(QRectF(0, 0, width(), height()), privatePtr->imgSource, privatePtr->displayArea);
    painter->setRenderHint(QPainter::Antialiasing);
    for (auto& shape : privatePtr->shapes)
    {
        shape->paint(painter, privatePtr->displayArea, this->size());
    }
    if(privatePtr->paintData)
    {
        auto pobjs = privatePtr->paintData->getPaintObjects();
        for(auto& pobj : pobjs)
        {
            pobj->paint(painter, privatePtr->displayArea, this->size());
        }
    }
}

void ImageToolQML::wheelEvent(QWheelEvent* event)
{
    auto itemWidth = this->width();
    auto itemHeight = this->height();
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    auto pos = event->position();
    /*if (!numPixels.isNull()) {
        scrollWithPixels(numPixels);

    } else */
    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        float power;
        if (privatePtr->displayArea.width() > privatePtr->displayArea.height())
        {
            power = privatePtr->displayArea.width() / itemWidth;
        }
        else
        {
            power = privatePtr->displayArea.height() / itemHeight;
        }
        pos.setX(pos.x() * power); pos.setY(pos.y() * power);
        if (numSteps.y() > 0)
        {
            pos *= 0.05;
            auto tl = privatePtr->displayArea.topLeft() + pos;
            auto sz = privatePtr->displayArea.size() * 0.95;
            privatePtr->displayArea.setRect(tl.x(), tl.y(), sz.width(), sz.height());
        }
        else
        {
            pos *= 0.05;
            auto tl = privatePtr->displayArea.topLeft() - pos;
            auto sz = privatePtr->displayArea.size() / 0.95;
            privatePtr->displayArea.setRect(tl.x(), tl.y(), sz.width(), sz.height());
        }
        update();
    }
    QQuickItem::wheelEvent(event);
}

void ImageToolQML::displayImage(QVariant img, ImageToolPaintData* paintData)
{
    if (img.canConvert<QImage>())
    {
        displayQImage(img.value<QImage>(), paintData);
    }
    if (img.canConvert<cv::Mat>())
    {
        displayCvMat(img.value<cv::Mat>(), paintData);
    }
}

void ImageToolQML::displayQImage(const QImage& img, ImageToolPaintData* paintData)
{
    privatePtr->imgSource = QPixmap::fromImage(img/*.copy(0,0,img.width(),img.height())*/);
    if (paintData)
    {
        if(privatePtr->paintData)
        {
            privatePtr->paintData->deleteLater();
            privatePtr->paintData = nullptr;
        }
        paintData->setParent(this);
        privatePtr->paintData = paintData;
    }
    if (privatePtr->imgSource.width() != privatePtr->displayArea.width() ||
        privatePtr->imgSource.height() != privatePtr->displayArea.height())
    {
        resetScale();
        return;
    }
    update();
}

void ImageToolQML::displayCvMat(const cv::Mat& img, ImageToolPaintData* paintData)
{
    QImage qimg;
    cv::Mat rgbimg;
    switch (img.channels())
    {
        case 1:
            qimg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format::Format_Grayscale8);
            break;
        case 3:
            cv::cvtColor(img, rgbimg, cv::COLOR_BGR2RGB);
            qimg = QImage(rgbimg.data, img.cols, img.rows, img.step, QImage::Format::Format_RGB888);
            break;
        case 4:
            cv::cvtColor(img, rgbimg, cv::COLOR_BGRA2RGBA);
            qimg = QImage(rgbimg.data, img.cols, img.rows, img.step, QImage::Format::Format_RGBA8888);
            break;
        default:
            return;
            break;
    }
    displayQImage(qimg, paintData);
}

void ImageToolQML::mousePressEvent(QMouseEvent* e)
{
    for (auto& shape : privatePtr->shapes)
    {
        if (shape->underMouse(e->localPos(), privatePtr->displayArea, this->size()))
        {
            privatePtr->grabedShape = shape;
        }
    }
    privatePtr->startPos = e->localPos();

    //QQuickItem::mousePressEvent(e);
}

void ImageToolQML::mouseMoveEvent(QMouseEvent* e)
{
    auto localPos = e->localPos();
    auto itemWidth = this->width();
    auto itemHeight = this->height();
    if (privatePtr->grabedShape)
    {
        privatePtr->grabedShape->mouseMove(localPos, privatePtr->displayArea, QSizeF(itemWidth, itemHeight));
    }
    else
    {
        auto vec = localPos - privatePtr->startPos;
        auto pos = privatePtr->displayArea.topLeft();

        vec.setX(vec.x() * (privatePtr->displayArea.width() / itemWidth));
        vec.setY(vec.y() * (privatePtr->displayArea.height() / itemHeight));

        pos.setX(pos.x() - vec.x());
        pos.setY(pos.y() - vec.y());
        privatePtr->displayArea.setRect(pos.x(), pos.y(), privatePtr->displayArea.width(), privatePtr->displayArea.height());
        privatePtr->startPos = localPos;
    }
    update();
    //QQuickItem::mouseMoveEvent(e);
}

void ImageToolQML::mouseDoubleClickEvent(QMouseEvent* event)
{
    resetScale();
}

void ImageToolQML::mouseReleaseEvent(QMouseEvent* e)
{
    privatePtr->grabedShape = nullptr;
    update();
}

void ImageToolShape::mouseMove(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize)
{
}

QPointF ImageToolShape::mapToPaint(const QPointF& source, const QRectF& imgRect, const QSizeF& itemSize)
{
    return PaintObject::mapToPaint(source,imgRect,itemSize);
}

void ImageToolShape::paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize)
{

}

struct ImageToolRotatedRect
{
    QPolygonF getPolygon()
    {
        double _angle = angle * CV_PI / 180.;
        float b = (float)cos(_angle) * 0.5f;
        float a = (float)sin(_angle) * 0.5f;
        QPolygonF out;
        QPointF pt1(center.x() - a * size.height() - b * size.width(), center.y() + b * size.height() - a * size.width());
        QPointF pt2(center.x() + a * size.height() - b * size.width(), center.y() - b * size.height() - a * size.width());
        out.push_back(pt1);
        out.push_back(pt2);
        out.push_back(QPointF(2 * center.x() - pt1.x(), 2 * center.y() - pt1.y()));
        out.push_back(QPointF(2 * center.x() - pt2.x(), 2 * center.y() - pt2.y()));
        return out;
    }

    QPointF center;
    double angle;
    QSizeF size;
};


class RotatedRectShapePrivate : public QObject
{
Q_OBJECT
public:
    RotatedRectShapePrivate(RotatedRectShape* pPtr) :
            parentPtr(pPtr),
            QObject(pPtr),
            brush(QColor(32, 159, 223, 100)),
            pen(QColor(32, 159, 223)),
            grabedType(GrabedType::None),
            grabThreshold(5.0),
            ctrlBrush(QColor(9, 45, 64,100)),
            ctrlPen(QColor(9, 45, 64,100))
    {
        rotateRect.center = QPointF(100,100);
        rotateRect.size = QSizeF(QRandomGenerator::global()->bounded(100,200),QRandomGenerator::global()->bounded(100,200));
        rotateRect.angle = 0.0;
    }
    enum class GrabedType
    {
        None,
        Center,
        TopEdge,
        BottomEdge,
        LeftEdge,
        RightEdge,
        ConerBottomLeft,
        ConerTopLeft,
        ConerTopRight,
        ConerBottomRight
    };
    ~RotatedRectShapePrivate() {}


    static double getAnglePI(const QPointF& v1, const QPointF& v2)
    {

        double a = QPointF::dotProduct(v1, v2) / sqrt(QPointF::dotProduct(v1, v1) * QPointF::dotProduct(v2, v2));
        a = std::min(1.0, std::max(-1.0, a));
        double t = std::acos(a);
        if (v1.x() * v2.y() - v1.y() * v2.x() < 0) t = -t;
        return t;
    }
    static double getDistance(const QPointF& pnt, const QLineF& line)
    {
        QPointF a = line.p1() - pnt;
        QPointF b = line.p2() - pnt;
        double r = (a.x() - b.x()) * (a.x()) + (a.y() - b.y()) * a.y();
        double d = (a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y());
        if (r <= 0) return a.x() * a.x() + a.y() * a.y();
        else if (r >= d)    return b.x() * b.x() + b.y() * b.y();
        r /= d;
        double x = a.x() + (b.x() - a.x()) * r, y = a.y() + (b.y() - a.y()) * r;
        return x * x + y * y;
    }
    static double getDistance(const QPointF& p1, const QPointF& p2)
    {
        auto vec = p1 - p2;
        return std::sqrt(vec.x() * vec.x() + vec.y() * vec.y());
    }

    GrabedType getGrabType(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize)
    {
        auto poly = rotateRect.getPolygon();
        for (int i = 0;i < poly.size();i++)
        {
            poly[i] = ImageToolShape::mapToPaint(poly[i], imgRect, itemSize);
            if (getDistance(poly[i], mouse) < grabThreshold)
            {
                switch (i)
                {
                    case 0:
                        return GrabedType::ConerBottomLeft;
                    case 1:
                        return GrabedType::ConerTopLeft;
                    case 2:
                        return GrabedType::ConerTopRight;
                    case 3:
                        return GrabedType::ConerBottomRight;
                }

            }
        }
        QLineF lline(poly[0], poly[1]), tline(poly[1], poly[2]), rline(poly[2], poly[3]), bline(poly[3], poly[0]);
        if (getDistance(mouse, lline) < grabThreshold)
        {
            return GrabedType::LeftEdge;
        }
        if (getDistance(mouse, tline) < grabThreshold)
        {
            return GrabedType::TopEdge;
        }
        if (getDistance(mouse, rline) < grabThreshold)
        {
            return GrabedType::RightEdge;
        }
        if (getDistance(mouse, bline) < grabThreshold)
        {
            return GrabedType::BottomEdge;
        }

        if (poly.containsPoint(mouse, Qt::OddEvenFill))
        {
            return GrabedType::Center;
        }
        return GrabedType::None;
    }

    ImageToolRotatedRect rotateRect;
    QBrush brush;
    QPen pen;
    QBrush ctrlBrush;
    QPen ctrlPen;
    GrabedType grabedType;
    QPointF mouseStart;
    double grabThreshold;
    RotatedRectShape* parentPtr;
};
RotatedRectShape::RotatedRectShape(QObject* parent) :
        ImageToolShape(parent),
        privatePtr(new RotatedRectShapePrivate(this))
{
    connect(this,&RotatedRectShape::centerChanged,this,[this](){
        auto str = serializationToJsonString();
        emit dataChanged(str);
    });
    connect(this,&RotatedRectShape::sizeChanged,this,[this](){
        auto str = serializationToJsonString();
        emit dataChanged(str);
    });
    connect(this,&RotatedRectShape::angleChanged,this,[this](){
        auto str = serializationToJsonString();
        emit dataChanged(str);
    });
}

bool RotatedRectShape::underMouse(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize)
{
    if (itemSize.width() < 1.0 || itemSize.width() < 1.0)
    {
        return false;
    }

    auto grabType = privatePtr->getGrabType(mouse, imgRect, itemSize);
    privatePtr->grabedType = grabType;
    privatePtr->mouseStart = mouse;
    if (grabType != RotatedRectShapePrivate::GrabedType::None)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void RotatedRectShape::mouseMove(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize)
{

    switch (privatePtr->grabedType)
    {
        case RotatedRectShapePrivate::GrabedType::Center:
        {
            auto sourceStart = ImageToolShape::mapToSource(privatePtr->mouseStart, imgRect, itemSize);
            auto sourceEnd = ImageToolShape::mapToSource(mouse, imgRect, itemSize);
            privatePtr->rotateRect.center += sourceEnd - sourceStart;
            privatePtr->mouseStart = mouse;
            emit centerChanged();
        }
            break;
        case RotatedRectShapePrivate::GrabedType::LeftEdge:
        {
            auto sourceStart = ImageToolShape::mapToSource(privatePtr->mouseStart, imgRect, itemSize);
            auto sourceEnd = ImageToolShape::mapToSource(mouse, imgRect, itemSize);
            auto poly = privatePtr->rotateRect.getPolygon();
            auto mouseVec = sourceEnd - sourceStart;
            QLineF lline(poly[0], poly[1]);
            auto moveVec = lline.center() - privatePtr->rotateRect.center;
            auto moveMod = std::sqrt(moveVec.x() * moveVec.x() + moveVec.y() * moveVec.y());
            double mouseMod = std::sqrt(mouseVec.x() * mouseVec.x() + mouseVec.y() * mouseVec.y());
            auto angle = std::abs(RotatedRectShapePrivate::getAnglePI(moveVec, mouseVec));

            auto moveDstMod = std::cos(angle) * mouseMod;
            //if(moveDstMod < 0)
            auto singleVec = moveVec / moveMod;
            auto leftMoveVec = singleVec * moveDstMod;

            if( (privatePtr->rotateRect.size.width() + moveDstMod) >= 1.0)
            {
                privatePtr->rotateRect.center += singleVec * (moveDstMod / 2.);

                privatePtr->rotateRect.size.setWidth(privatePtr->rotateRect.size.width() + moveDstMod);
            }
            else
            {
                privatePtr->grabedType = RotatedRectShapePrivate::GrabedType::None;
            }

            privatePtr->mouseStart = mouse;
            emit sizeChanged();
            emit centerChanged();
        }
            break;
        case RotatedRectShapePrivate::GrabedType::TopEdge:
        {
            auto sourceStart = ImageToolShape::mapToSource(privatePtr->mouseStart, imgRect, itemSize);
            auto sourceEnd = ImageToolShape::mapToSource(mouse, imgRect, itemSize);
            auto poly = privatePtr->rotateRect.getPolygon();
            auto mouseVec = sourceEnd - sourceStart;
            QLineF lline(poly[1], poly[2]);
            auto moveVec = lline.center() - privatePtr->rotateRect.center;
            auto moveMod = std::sqrt(moveVec.x() * moveVec.x() + moveVec.y() * moveVec.y());
            double mouseMod = std::sqrt(mouseVec.x() * mouseVec.x() + mouseVec.y() * mouseVec.y());
            auto angle = std::abs(RotatedRectShapePrivate::getAnglePI(moveVec, mouseVec));

            auto moveDstMod = std::cos(angle) * mouseMod;
            //if(moveDstMod < 0)
            auto singleVec = moveVec / moveMod;
            auto leftMoveVec = singleVec * moveDstMod;

            if( (privatePtr->rotateRect.size.height() + moveDstMod) >= 1.0)
            {
                privatePtr->rotateRect.center += singleVec * (moveDstMod / 2.);

                privatePtr->rotateRect.size.setHeight(privatePtr->rotateRect.size.height() + moveDstMod);
            }
            else
            {
                privatePtr->grabedType = RotatedRectShapePrivate::GrabedType::None;
            }

            privatePtr->mouseStart = mouse;
            emit sizeChanged();
            emit centerChanged();
        }
            break;
        case RotatedRectShapePrivate::GrabedType::RightEdge:
        {
            auto sourceStart = ImageToolShape::mapToSource(privatePtr->mouseStart, imgRect, itemSize);
            auto sourceEnd = ImageToolShape::mapToSource(mouse, imgRect, itemSize);
            auto poly = privatePtr->rotateRect.getPolygon();
            auto mouseVec = sourceEnd - sourceStart;
            QLineF lline(poly[2], poly[3]);
            auto moveVec = lline.center() - privatePtr->rotateRect.center;
            auto moveMod = std::sqrt(moveVec.x() * moveVec.x() + moveVec.y() * moveVec.y());
            double mouseMod = std::sqrt(mouseVec.x() * mouseVec.x() + mouseVec.y() * mouseVec.y());
            auto angle = std::abs(RotatedRectShapePrivate::getAnglePI(moveVec, mouseVec));

            auto moveDstMod = std::cos(angle) * mouseMod;
            //if(moveDstMod < 0)
            auto singleVec = moveVec / moveMod;
            auto leftMoveVec = singleVec * moveDstMod;

            if( (privatePtr->rotateRect.size.width() + moveDstMod) >= 1.0)
            {
                privatePtr->rotateRect.center += singleVec * (moveDstMod / 2.);

                privatePtr->rotateRect.size.setWidth(privatePtr->rotateRect.size.width() + moveDstMod);
            }
            else
            {
                privatePtr->grabedType = RotatedRectShapePrivate::GrabedType::None;
            }

            privatePtr->mouseStart = mouse;
            emit sizeChanged();
            emit centerChanged();
        }

            break;
        case RotatedRectShapePrivate::GrabedType::BottomEdge:
        {
            auto sourceStart = ImageToolShape::mapToSource(privatePtr->mouseStart, imgRect, itemSize);
            auto sourceEnd = ImageToolShape::mapToSource(mouse, imgRect, itemSize);
            auto poly = privatePtr->rotateRect.getPolygon();
            auto mouseVec = sourceEnd - sourceStart;
            QLineF lline(poly[3], poly[0]);
            auto moveVec = lline.center() - privatePtr->rotateRect.center;
            auto moveMod = std::sqrt(moveVec.x() * moveVec.x() + moveVec.y() * moveVec.y());
            double mouseMod = std::sqrt(mouseVec.x() * mouseVec.x() + mouseVec.y() * mouseVec.y());
            auto angle = std::abs(RotatedRectShapePrivate::getAnglePI(moveVec, mouseVec));

            auto moveDstMod = std::cos(angle) * mouseMod;
            //if(moveDstMod < 0)
            auto singleVec = moveVec / moveMod;
            auto leftMoveVec = singleVec * moveDstMod;

            if( (privatePtr->rotateRect.size.height() + moveDstMod) >= 1.0)
            {
                privatePtr->rotateRect.center += singleVec * (moveDstMod / 2.);

                privatePtr->rotateRect.size.setHeight(privatePtr->rotateRect.size.height() + moveDstMod);
            }
            else
            {
                privatePtr->grabedType = RotatedRectShapePrivate::GrabedType::None;
            }

            privatePtr->mouseStart = mouse;
            emit sizeChanged();
            emit centerChanged();
        }
            break;
        case RotatedRectShapePrivate::GrabedType::ConerTopLeft:
        case RotatedRectShapePrivate::GrabedType::ConerTopRight:
        case RotatedRectShapePrivate::GrabedType::ConerBottomRight:
        case RotatedRectShapePrivate::GrabedType::ConerBottomLeft:
        {
            auto sourceStart = ImageToolShape::mapToSource(privatePtr->mouseStart, imgRect, itemSize);
            auto sourceEnd = ImageToolShape::mapToSource(mouse, imgRect, itemSize);
            auto startVec = sourceStart - privatePtr->rotateRect.center;
            auto endVec = sourceEnd - privatePtr->rotateRect.center;

            double angle = 180. * (std::atan2(endVec.y(), endVec.x()) - std::atan2(startVec.y(), startVec.x())) / CV_PI;
            privatePtr->rotateRect.angle += angle;
            privatePtr->mouseStart = mouse;
            emit angleChanged();
        }
            break;
    }
}

void RotatedRectShape::paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize)
{
    auto poly = privatePtr->rotateRect.getPolygon();
    for (auto& polyPnt : poly)
    {
        polyPnt = ImageToolShape::mapToPaint(polyPnt, imgRect, itemSize);
    }
    painter->setBrush(privatePtr->brush);
    painter->setPen(privatePtr->pen);
    painter->drawPolygon(poly);
    painter->setBrush(privatePtr->ctrlBrush);
    painter->setPen(privatePtr->ctrlPen);
    auto d = privatePtr->grabThreshold;


    switch (privatePtr->grabedType)
    {
        case RotatedRectShapePrivate::GrabedType::None:

            break;
        case RotatedRectShapePrivate::GrabedType::LeftEdge:
        {
            painter->drawEllipse(QRectF(poly[0].x() - d, poly[0].y() - d, d * 2., d * 2.));
            painter->drawEllipse(QRectF(poly[1].x() - d, poly[1].y() - d, d * 2., d * 2.));
            auto linePen = privatePtr->ctrlPen;
            linePen.setWidth(d);
            painter->setPen(linePen);
            painter->drawLine(QLineF(poly[0], poly[1]));
        }
            break;
        case RotatedRectShapePrivate::GrabedType::TopEdge:
        {
            painter->drawEllipse(QRectF(poly[2].x() - d, poly[2].y() - d, d * 2., d * 2.));
            painter->drawEllipse(QRectF(poly[1].x() - d, poly[1].y() - d, d * 2., d * 2.));
            auto linePen = privatePtr->ctrlPen;
            linePen.setWidth(d);
            painter->setPen(linePen);
            painter->drawLine(QLineF(poly[1], poly[2]));
        }
            break;
        case RotatedRectShapePrivate::GrabedType::RightEdge:
        {
            painter->drawEllipse(QRectF(poly[2].x() - d, poly[2].y() - d, d * 2., d * 2.));
            painter->drawEllipse(QRectF(poly[3].x() - d, poly[3].y() - d, d * 2., d * 2.));
            auto linePen = privatePtr->ctrlPen;
            linePen.setWidth(d);
            painter->setPen(linePen);
            painter->drawLine(QLineF(poly[2], poly[3]));
        }
            break;
        case RotatedRectShapePrivate::GrabedType::BottomEdge:
        {
            painter->drawEllipse(QRectF(poly[0].x() - d, poly[0].y() - d, d * 2., d * 2.));
            painter->drawEllipse(QRectF(poly[3].x() - d, poly[3].y() - d, d * 2., d * 2.));
            auto linePen = privatePtr->ctrlPen;
            linePen.setWidth(d);
            painter->setPen(linePen);
            painter->drawLine(QLineF(poly[3], poly[0]));
        }
            break;
        case RotatedRectShapePrivate::GrabedType::ConerBottomLeft:
            painter->drawEllipse(QRectF(poly[0].x() - d, poly[0].y() - d, d * 2., d * 2.));
            break;
        case RotatedRectShapePrivate::GrabedType::ConerTopLeft:
            painter->drawEllipse(QRectF(poly[1].x() - d, poly[1].y() - d, d * 2., d * 2.));
            break;
        case RotatedRectShapePrivate::GrabedType::ConerTopRight:
            painter->drawEllipse(QRectF(poly[2].x() - d, poly[2].y() - d, d * 2., d * 2.));
            break;
        case RotatedRectShapePrivate::GrabedType::ConerBottomRight:
            painter->drawEllipse(QRectF(poly[3].x() - d, poly[3].y() - d, d * 2., d * 2.));
            break;
    }
    double power = 1.;
    if (imgRect.width() > imgRect.height())
    {
        power = itemSize.width() /imgRect.width();
    }
    else
    {
        power = itemSize.height() / imgRect.height();
    }
    QFont font;
    auto text_scale = 50.0 * power;
    font.setPixelSize(std::floor(text_scale < 1 ? 1 : text_scale));
    font.setFamily("Microsoft YaHei UI");
    painter->setFont(font);
    auto tl = poly[1];
    painter->save();
    painter->translate(tl);

    painter->rotate(privatePtr->rotateRect.angle);
    painter->setBrush(privatePtr->brush);
    painter->setPen(privatePtr->pen);
    painter->drawText(QPointF(0,font.pixelSize()), "Name:" + objectName() +
    QString(" (%1,%2,%3,%4,%5)").arg(
            QString::number(privatePtr->rotateRect.center.x(),'f',1),
            QString::number(privatePtr->rotateRect.center.y(),'f',1),
            QString::number(privatePtr->rotateRect.size.width(),'f',1),
            QString::number(privatePtr->rotateRect.size.height(),'f',1),
            QString::number(privatePtr->rotateRect.angle,'f',3)));
    painter->restore();
}

QVariant RotatedRectShape::getCenter() const
{
    return privatePtr->rotateRect.center;
}
void RotatedRectShape::setCenter(QVariant center)
{
    if (center.canConvert<QPointF>())
    {
        privatePtr->rotateRect.center = center.toPointF();
        auto str = serializationToJsonString();
        emit centerChanged();
        emit dataChanged(str);
    }
}
QVariant RotatedRectShape::getAngle() const
{
    return privatePtr->rotateRect.angle;
}

void RotatedRectShape::setAngle(QVariant angle)
{
    if (angle.canConvert<double>())
    {
        privatePtr->rotateRect.angle = angle.toDouble();
        auto str = serializationToJsonString();
        emit angleChanged();
        emit dataChanged(str);
    }
}

QVariant RotatedRectShape::getSize() const
{
    return privatePtr->rotateRect.size;
}

void RotatedRectShape::setSize(QVariant size)
{
    if (size.canConvert<QSizeF>())
    {
        privatePtr->rotateRect.size = size.toSizeF();
        auto str = serializationToJsonString();
        emit sizeChanged();
        emit dataChanged(str);
    }
}

RotatedRectShape::~RotatedRectShape()
{

}

void RotatedRectShape::hoverMove(const QPointF &mouse, const QRectF &imgRect, const QSizeF &itemSize)
{
    privatePtr->grabedType = privatePtr->getGrabType(mouse,imgRect,itemSize);
}

QVariant RotatedRectShape::serializationToJson() const
{
    QVariantMap varMap;
    for(int i = 0;i < metaObject()->propertyCount();i++)
    {
        auto metaProperty = metaObject()->property(i);
        auto readVar = metaProperty.read(this);
        if(readVar.canConvert<QSizeF>())
        {
            auto sz = readVar.toSizeF();
            QVariantMap varMapTmp;
            varMapTmp.insert("width",sz.width());
            varMapTmp.insert("height",sz.height());
            varMap.insert(metaProperty.name(),varMapTmp);
        }
        else if(readVar.canConvert<QPointF>())
        {
            auto pt = readVar.toPointF();
            QVariantMap varMapTmp;
            varMapTmp.insert("x",pt.x());
            varMapTmp.insert("y",pt.y());
            varMap.insert(metaProperty.name(),varMapTmp);
        }
        else
        {
            varMap.insert(metaProperty.name(),readVar);
        }

    }
    return varMap;
}

void RotatedRectShape::serializationFromJson(const QVariant &jsonObj)
{
    QVariantMap varMap = jsonObj.toMap();
    for(auto iter = varMap.keyValueBegin();iter != varMap.keyValueEnd();iter++)
    {
        auto propertyIdx = metaObject()->indexOfProperty(iter->first.toStdString().c_str());
        if(propertyIdx >= 0)
        {
            auto metaProperty = metaObject()->property(propertyIdx);
            metaProperty.write(this,iter->second);
        }
    }
}

QVariant ImageToolShape::serializationToJsonString() const
{
    auto varMap = serializationToJson();
    auto jsonObj = QJsonObject::fromVariantMap(varMap.toMap());
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return QString::fromUtf8(doc.toJson());
}

void ImageToolShape::serializationFromJsonString(const QVariant &jsonStr)
{
    auto obj = QJsonDocument::fromJson(jsonStr.toString().toUtf8()).object();
    serializationFromJson(obj.toVariantMap());
}

void ImageToolQML::resetScale()
{
    auto itemWidth = this->width();
    auto itemHeight = this->height();
    if (itemWidth >= 1.0 && itemHeight >= 1.0)
    {
        if (float(privatePtr->imgSource.width()) / float(privatePtr->imgSource.height()) > itemWidth / itemHeight)
        {
            float power = float(privatePtr->imgSource.width()) / itemWidth;
            auto w = itemHeight * power;
            privatePtr->displayArea.setY(-(w - float(privatePtr->imgSource.height())) / 2.);
            privatePtr->displayArea.setX(0);
            privatePtr->displayArea.setWidth(privatePtr->imgSource.width());
            privatePtr->displayArea.setHeight(w);
        }
        else
        {

            float power = float(privatePtr->imgSource.height()) / itemHeight;
            auto h = itemWidth * power;
            privatePtr->displayArea.setY(0);
            privatePtr->displayArea.setX(-(h - float(privatePtr->imgSource.width())) / 2.);
            privatePtr->displayArea.setWidth(h);
            privatePtr->displayArea.setHeight(privatePtr->imgSource.height());
        }
    }
    update();
}

void ImageToolQML::hoverMoveEvent(QHoverEvent* event)
{
    auto mouse = event->posF();
    for(auto & shape : privatePtr->shapes)
    {
        if(shape->underMouse(mouse,privatePtr->displayArea,this->size()))
        {
            shape->hoverMove(mouse,privatePtr->displayArea,this->size());
            update();
            break;
        }
        else
        {
            update();
        }
    }
}

void ImageToolQML::pushShape(ImageToolShape* shape)
{
    if(shape)
    {
        shape->setParent(this);
        connect(shape, &ImageToolShape::dataChanged, this, [this](QVariant jsonStr)
        {
            this->update();
        });
        privatePtr->shapes.push_front(shape);
        update();
    }
}

void ImageToolQML::clearShapes()
{
    for(auto& shapePtr : privatePtr->shapes)
    {
        shapePtr->deleteLater();
    }
    privatePtr->shapes.clear();
    update();
}

QJSValue ImageToolQML::createShape(const QVariant& str)
{
    auto engine = qmlEngine(this);
    if(str.toString().indexOf("rect",Qt::CaseInsensitive) >= 0)
    {
        return engine->newQObject(new RotatedRectShape());
    }
    return engine->newQObject(nullptr);
}

QJSValue ImageToolQML::createPaintObject(const QVariant& type)
{
    auto engine = qmlEngine(this);
    auto str = type.toString();
    if(str == "rect")
    {
        return engine->newQObject(new PaintRect());
    }
    if(str == "path")
    {
        return engine->newQObject(new PaintPath());
    }
    if(str == "text")
    {
        return engine->newQObject(new PaintText());
    }
    if(str == "cross")
    {
        return engine->newQObject(new PaintCrossLine());
    }
    return engine->newQObject(nullptr);
}

QJSValue ImageToolQML::createPaintData()
{
    auto engine = qmlEngine(this);
    return engine->newQObject(new ImageToolPaintData());
}

void ImageToolQML::setPaintData(ImageToolPaintData* paintData)
{
    if(!paintData)
    {
        return;
    }
    privatePtr->paintData->clearPaintObjects();
    for(auto& pobj : paintData->getPaintObjects())
    {
        privatePtr->paintData->pushPaintObject(pobj);
    }
    update();
}

  QVariantList ImageToolQML::findShapes(const QVariant& objectName)
  {
      QVariantList listShape;
      auto listChild = children();
      for(auto& child : listChild)
      {
          if(child->objectName() == objectName)
          {
              auto shapePtr = dynamic_cast<ImageToolShape*>(child);
              if(shapePtr)
              {
                  QVariant vr;
                  vr.setValue(shapePtr);
                  listShape.push_back(vr);
              }
          }
      }
      return listShape;
  }

void ImageToolQML::clearPaintData()
{
    privatePtr->paintData->clearPaintObjects();
    update();
}


  ImageToolShape::ImageToolShape(QObject* parent) :
        QObject(parent)
{
    connect(this,&ImageToolShape::objectNameChanged,this,[this](const QString& objStr){
        auto str = serializationToJsonString();
        emit dataChanged(str);
    });
}

ImageToolShape::~ImageToolShape()
{

}

bool ImageToolShape::underMouse(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize)
{
    return false;
}

QPointF ImageToolShape::mapToSource(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize)
{
    return PaintObject::mapToSource(mouse,imgRect,itemSize);
}

void ImageToolShape::hoverMove(const QPointF &mouse, const QRectF &imgRect, const QSizeF &itemSize)
{

}

QVariant ImageToolShape::serializationToJson() const
{
    return QVariant();
}

void ImageToolShape::serializationFromJson(const QVariant &jsonObj)
{

}

ImageToolShape::ImageToolShape(const ImageToolShape & other)
{
    operator=(other);
}
void ImageToolShape::operator=(const ImageToolShape & other)
{
    for (auto i = 0; i < other.metaObject()->propertyCount(); i++)
    {
        metaObject()->property(i).write(this, metaObject()->property(i).read(&other));
    }
}
#include "ImageToolQML.moc"


