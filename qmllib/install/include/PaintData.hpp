//
// Created by lyric on 2021/4/22.
//

#ifndef IMAGETOOL_PAINTDATA_HPP
#define IMAGETOOL_PAINTDATA_HPP

#include <QPen>
#include <QBrush>
#include <QFont>
#include "opencv2/opencv.hpp"
#include "ImageToolQML_global.h"

class IMAGETOOLQML_EXPORT PaintObject : public QObject
{
Q_OBJECT
public:
    PaintObject(QObject* parent = nullptr);
    ~PaintObject();
    virtual void operator=(const PaintObject& other);
    PaintObject(const PaintObject & other);
    virtual void paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize);
    virtual void paintImage(cv::Mat* cvimg);
    static QPointF mapToPaint(const QPointF& source, const QRectF& imgRect, const QSizeF& itemSize);
    static QPointF mapToSource(const QPointF& mouse, const QRectF& imgRect, const QSizeF& itemSize);
    Q_PROPERTY(QVariant paintPen READ getPaintPen WRITE setPaintPen)
    Q_PROPERTY(QVariant paintBrush READ getPaintBrush WRITE setPaintBrush)
    Q_PROPERTY(QVariant paintPenColor READ getPaintPenColor WRITE setPaintPenColor)
    Q_PROPERTY(QVariant paintBrushColor READ getPaintBrushColor WRITE setPaintBrushColor)
public slots:
    QVariant getPaintPen() const;
    QVariant getPaintBrush() const;
    QVariant getPaintPenColor() const;
    QVariant getPaintBrushColor() const;
    void setPaintPen(QVariant var);
    void setPaintBrush(QVariant var);
    void setPaintPenColor(QVariant var);
    void setPaintBrushColor(QVariant var);
protected:
    QPen paintPen;
    QBrush paintBrush;
};

class IMAGETOOLQML_EXPORT PaintRect : public PaintObject
{
Q_OBJECT
public:
    PaintRect(QObject* parent = nullptr);
    ~PaintRect();
    PaintRect(const PaintRect & other);
    Q_PROPERTY(QVariant x READ getX WRITE setX)
    Q_PROPERTY(QVariant y READ getX WRITE setY)
    Q_PROPERTY(QVariant width READ getX WRITE setWidth)
    Q_PROPERTY(QVariant height READ getHeight WRITE setHeight)

    virtual void paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize) override;
    virtual void paintImage(cv::Mat*) override;
    QRectF getRect();
public slots:
    QVariant getX() const;
    QVariant getY() const;
    void setX(QVariant var);
    void setY(QVariant var);
    QVariant getWidth() const;
    QVariant getHeight() const;
    void setWidth(QVariant var);
    void setHeight(QVariant var);
private:
    double x;
    double y;
    double width;
    double height;
};Q_DECLARE_METATYPE(PaintRect);

class IMAGETOOLQML_EXPORT PaintPath : public PaintObject
{
Q_OBJECT
public:
    PaintPath(QObject* parent = nullptr);
    ~PaintPath();
    PaintPath(const PaintPath & other);
    virtual void paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize) override;
    virtual void paintImage(cv::Mat*) override;
    //QVector<QLineF> getLines();

    Q_PROPERTY(QVariant lines READ getLines WRITE setLines)
public slots:
    void addLineF(const QVariant& pt1,const QVariant& pt2);
    void addLineF(const QVariant& line);
    QVariant getLines() const;
    void setLines(QVariant var);
private:
    QVector<QLineF> lines;
};Q_DECLARE_METATYPE(PaintPath);

class IMAGETOOLQML_EXPORT PaintText : public PaintObject
{
Q_OBJECT
public:
    PaintText(QObject* parent = nullptr);
    ~PaintText();
    PaintText(const PaintText & other);
    virtual void paint(QPainter* painter, const QRectF& imgRect, const QSizeF& itemSize) override;
    virtual void paintImage(cv::Mat*) override;
    //QVector<QLineF> getLines();

    Q_PROPERTY(QVariant text READ getText WRITE setText)
    Q_PROPERTY(QVariant textWidth READ getTextWidth WRITE setTextWidth)
    Q_PROPERTY(QVariant textFamily READ getTextFamily WRITE setTextFamily)
    Q_PROPERTY(QVariant textPosition READ getTextPosition WRITE setTextPosition)
public slots:
    QVariant getText() const;
    void setText(QVariant var);
    QVariant getTextWidth() const;
    void setTextWidth(QVariant var);
    QVariant getTextFamily() const;
    void setTextFamily(QVariant var);
    QVariant getTextPosition() const;
    void setTextPosition(QVariant var);
private:
    QString text;
    QFont font;
    QPointF textPosition;
};Q_DECLARE_METATYPE(PaintText);

class IMAGETOOLQML_EXPORT ImageToolPaintData : public QObject
{
Q_OBJECT
public:
    ImageToolPaintData(QObject* parent = nullptr);
    ~ImageToolPaintData();
    ImageToolPaintData(const ImageToolPaintData & other);
    virtual void operator=(const ImageToolPaintData& other);
    void paint(QPainter*, const QRectF& imgRect, const QSizeF& itemSize);

public slots:
    void pushPaintObject(PaintObject* obj);
    QList<PaintObject*> getPaintObjects() const;
    PaintObject* createPaintObject(const QVariant& type);
private:
    QList<PaintObject*> paintObjects;
};Q_DECLARE_METATYPE(ImageToolPaintData);


#endif //IMAGETOOL_PAINTDATA_HPP
