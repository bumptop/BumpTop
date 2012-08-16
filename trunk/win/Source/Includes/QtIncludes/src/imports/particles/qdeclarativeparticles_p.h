/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEPARTICLES_H
#define QDECLARATIVEPARTICLES_H

#include <QtDeclarative/qdeclarativeitem.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeParticle;
class QDeclarativeParticles;
class QDeclarativeParticleMotion : public QObject
{
    Q_OBJECT
public:
    QDeclarativeParticleMotion(QObject *parent=0);

    virtual void advance(QDeclarativeParticle &, int interval);
    virtual void created(QDeclarativeParticle &);
    virtual void destroy(QDeclarativeParticle &);
};

class QDeclarativeParticleMotionLinear : public QDeclarativeParticleMotion
{
    Q_OBJECT
public:
    QDeclarativeParticleMotionLinear(QObject *parent=0)
        : QDeclarativeParticleMotion(parent) {}

    virtual void advance(QDeclarativeParticle &, int interval);
};

class QDeclarativeParticleMotionGravity : public QDeclarativeParticleMotion
{
    Q_OBJECT

    Q_PROPERTY(qreal xattractor READ xAttractor WRITE setXAttractor NOTIFY xattractorChanged)
    Q_PROPERTY(qreal yattractor READ yAttractor WRITE setYAttractor NOTIFY yattractorChanged)
    Q_PROPERTY(qreal acceleration READ acceleration WRITE setAcceleration NOTIFY accelerationChanged)
public:
    QDeclarativeParticleMotionGravity(QObject *parent=0)
        : QDeclarativeParticleMotion(parent), _xAttr(0.0), _yAttr(0.0), _accel(0.00005) {}

    qreal xAttractor() const { return _xAttr; }
    void setXAttractor(qreal x);

    qreal yAttractor() const { return _yAttr; }
    void setYAttractor(qreal y);

    qreal acceleration() const { return _accel * 1000000; }
    void setAcceleration(qreal accel);

    virtual void advance(QDeclarativeParticle &, int interval);

Q_SIGNALS:
    void xattractorChanged();
    void yattractorChanged();
    void accelerationChanged();

private:
    qreal _xAttr;
    qreal _yAttr;
    qreal _accel;
};

class QDeclarativeParticleMotionWander : public QDeclarativeParticleMotion
{
    Q_OBJECT
public:
    QDeclarativeParticleMotionWander()
        : QDeclarativeParticleMotion(), particles(0), _xvariance(0), _yvariance(0), _pace(100) {}

    virtual void advance(QDeclarativeParticle &, int interval);
    virtual void created(QDeclarativeParticle &);
    virtual void destroy(QDeclarativeParticle &);

    struct Data {
        qreal x_targetV;
        qreal y_targetV;
        qreal x_peak;
        qreal y_peak;
        qreal x_var;
        qreal y_var;
    };

    Q_PROPERTY(qreal xvariance READ xVariance WRITE setXVariance NOTIFY xvarianceChanged)
    qreal xVariance() const { return _xvariance * 1000.0; }
    void setXVariance(qreal var);

    Q_PROPERTY(qreal yvariance READ yVariance WRITE setYVariance NOTIFY yvarianceChanged)
    qreal yVariance() const { return _yvariance * 1000.0; }
    void setYVariance(qreal var);

    Q_PROPERTY(qreal pace READ pace WRITE setPace NOTIFY paceChanged)
    qreal pace() const { return _pace * 1000.0; }
    void setPace(qreal pace);

Q_SIGNALS:
    void xvarianceChanged();
    void yvarianceChanged();
    void paceChanged();

private:
    QDeclarativeParticles *particles;
    qreal _xvariance;
    qreal _yvariance;
    qreal _pace;
};

class QDeclarativeParticlesPrivate;
class QDeclarativeParticles : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(int emissionRate READ emissionRate WRITE setEmissionRate NOTIFY emissionRateChanged)
    Q_PROPERTY(qreal emissionVariance READ emissionVariance WRITE setEmissionVariance NOTIFY emissionVarianceChanged)
    Q_PROPERTY(int lifeSpan READ lifeSpan WRITE setLifeSpan NOTIFY lifeSpanChanged)
    Q_PROPERTY(int lifeSpanDeviation READ lifeSpanDeviation WRITE setLifeSpanDeviation NOTIFY lifeSpanDeviationChanged)
    Q_PROPERTY(int fadeInDuration READ fadeInDuration WRITE setFadeInDuration NOTIFY fadeInDurationChanged)
    Q_PROPERTY(int fadeOutDuration READ fadeOutDuration WRITE setFadeOutDuration NOTIFY fadeOutDurationChanged)
    Q_PROPERTY(qreal angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(qreal angleDeviation READ angleDeviation WRITE setAngleDeviation NOTIFY angleDeviationChanged)
    Q_PROPERTY(qreal velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)
    Q_PROPERTY(qreal velocityDeviation READ velocityDeviation WRITE setVelocityDeviation NOTIFY velocityDeviationChanged)
    Q_PROPERTY(QDeclarativeParticleMotion *motion READ motion WRITE setMotion NOTIFY motionChanged)
    Q_CLASSINFO("DefaultProperty", "motion")

public:
    QDeclarativeParticles(QDeclarativeItem *parent=0);
    ~QDeclarativeParticles();

    QUrl source() const;
    void setSource(const QUrl &);

    int count() const;
    void setCount(int cnt);

    int emissionRate() const;
    void setEmissionRate(int);

    qreal emissionVariance() const;
    void setEmissionVariance(qreal);

    int lifeSpan() const;
    void setLifeSpan(int);

    int lifeSpanDeviation() const;
    void setLifeSpanDeviation(int);

    int fadeInDuration() const;
    void setFadeInDuration(int);

    int fadeOutDuration() const;
    void setFadeOutDuration(int);

    qreal angle() const;
    void setAngle(qreal);

    qreal angleDeviation() const;
    void setAngleDeviation(qreal);

    qreal velocity() const;
    void setVelocity(qreal);

    qreal velocityDeviation() const;
    void setVelocityDeviation(qreal);

    QDeclarativeParticleMotion *motion() const;
    void setMotion(QDeclarativeParticleMotion *);

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

public Q_SLOTS:
    void burst(int count, int emissionRate=-1);

protected:
    virtual void componentComplete();

Q_SIGNALS:
    void sourceChanged();
    void countChanged();
    void emissionRateChanged();
    void emissionVarianceChanged();
    void lifeSpanChanged();
    void lifeSpanDeviationChanged();
    void fadeInDurationChanged();
    void fadeOutDurationChanged();
    void angleChanged();
    void angleDeviationChanged();
    void velocityChanged();
    void velocityDeviationChanged();
    void emittingChanged();
    void motionChanged();

private Q_SLOTS:
    void imageLoaded();

private:
    Q_DISABLE_COPY(QDeclarativeParticles)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeParticles)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeParticleMotion)
QML_DECLARE_TYPE(QDeclarativeParticleMotionLinear)
QML_DECLARE_TYPE(QDeclarativeParticleMotionGravity)
QML_DECLARE_TYPE(QDeclarativeParticleMotionWander)
QML_DECLARE_TYPE(QDeclarativeParticles)

QT_END_HEADER

#endif
