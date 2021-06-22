//
// Created by lyric on 2021/4/12.
//

#pragma once
#include <QQuickPaintedItem>
#include <QImage>
#include "PaintData.hpp"
#include "opencv2/opencv.hpp"
#include "ImageToolQML_global.h"



class ImageToolQMLPrivate;
class QContextMenuEvent;


class ImageToolPaintData;

class IMAGETOOLQML_EXPORT ImageToolShape : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE ImageToolShape(QObject* parent = nullptr);
    Q_INVOKABLE ~ImageToolShape();
    ImageToolShape(const ImageToolShape & other);
    void operator=(const ImageToolShape & other);
    static QPointF mapToPaint(const QPointF& source, const QRectF& imgRect, const QSizeF& itemSize);
    static QPointF mapToSource(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize);
signals:
    void dataChanged(QVariant jsonStr);
public slots:
    virtual QVariant serializationToJson() const;
    virtual QVariant serializationToJsonString() const;
    virtual void serializationFromJson(const QVariant& jsonObj);
    virtual void serializationFromJsonString(const QVariant& jsonStr);
protected:
    friend class ImageToolQML;
    virtual bool underMouse(const QPointF& mouse,const QRectF& imgRect,const QSizeF& itemSize);
    virtual void paint(QPainter* painter,const QRectF& rect,const QSizeF& itemSize);
    virtual void mouseMove(const QPointF& mouse,const QRectF& imgRect,const QSizeF& itemSize);
    virtual void hoverMove(const QPointF& mouse,const QRectF& imgRect,const QSizeF& itemSize);

};Q_DECLARE_METATYPE(ImageToolShape)
class RotatedRectShapePrivate;
class IMAGETOOLQML_EXPORT RotatedRectShape : public ImageToolShape
{
Q_OBJECT
public:
    Q_INVOKABLE RotatedRectShape(QObject* parent = nullptr);
    Q_INVOKABLE ~RotatedRectShape();
signals:
    void centerChanged();
    void angleChanged();
    void sizeChanged();
public:
    Q_PROPERTY(QVariant center WRITE setCenter READ getCenter NOTIFY centerChanged)
    Q_PROPERTY(QVariant angle WRITE setAngle READ getAngle NOTIFY angleChanged)
    Q_PROPERTY(QVariant size WRITE setSize READ getSize NOTIFY sizeChanged)
public slots:
    QVariant getCenter() const;
    void setCenter(QVariant center);
    QVariant getAngle() const;
    void setAngle(QVariant angle);
    QVariant getSize() const;
    void setSize(QVariant size);
    virtual QVariant serializationToJson() const override;
    virtual void serializationFromJson(const QVariant& jsonObj) override;

protected:
    virtual bool underMouse(const QPointF& mouse,const QRectF& imgRect,const QSizeF& itemSize) override;
    virtual void mouseMove(const QPointF& mouse,const QRectF& imgRect,const QSizeF& itemSize) override;
    virtual void paint(QPainter* painter,const QRectF& imgRect,const QSizeF& itemSize) override;
    virtual void hoverMove(const QPointF& mouse,const QRectF& imgRect,const QSizeF& itemSize) override;
private:
    RotatedRectShapePrivate* privatePtr;
};Q_DECLARE_METATYPE(RotatedRectShape)

class IMAGETOOLQML_EXPORT ImageToolQML : public QQuickPaintedItem
{
    Q_OBJECT
public :
    Q_INVOKABLE ImageToolQML(QQuickItem *parent = nullptr);
    Q_INVOKABLE ~ImageToolQML();
public slots:
    void displayImage(QVariant img,ImageToolPaintData* paintData = nullptr);
    void displayQImage(const QImage& img,ImageToolPaintData* paintData = nullptr);
    void displayCvMat(const cv::Mat& img,ImageToolPaintData* paintData = nullptr);
    void resetScale();
    void pushShape(ImageToolShape* shape);
    void clearShapes();
    void clearPaintData();
    void setPaintData(ImageToolPaintData* paintData);
    QJSValue createShape(const QVariant& type);
    QJSValue createPaintData();
    QVariantList findShapes(const QVariant& objectName);
    QJSValue createPaintObject(const QVariant& type);
protected:
    virtual void paint(QPainter* painter) override;
    virtual void wheelEvent(QWheelEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void hoverMoveEvent(QHoverEvent* event) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
private:
    ImageToolQMLPrivate* privatePtr;
};



QML_DECLARE_TYPE(ImageToolQML)

#ifndef IMAGETOOLQML_IMAGETOOLQML_HPP
#define IMAGETOOLQML_IMAGETOOLQML_HPP

#endif //IMAGETOOLQML_IMAGETOOLQML_HPP
