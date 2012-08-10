/*****************

 This port was based in a inital code from Jesus Gollonet, him port Penners easing equations to C/C++:

 http://www.jesusgollonet.com/blog/2007/09/24/penner-easing-cpp/
 http://robertpenner.com/easing/

 I¥m just make a better wrapper a litlle more OOP e put some callbacks like the original Tweener
 (http://code.google.com/p/tweener/)


 wesley.marques@gmail.com  - Wesley Ferreira Marques
 http://codevein.com


 **********************/

#ifndef _CPP_TWEEEN_
#define _CPP_TWEEEN_

#include <math.h>
#include <list>
#include <vector>

#if defined(OS_WIN)
#ifdef WIN32DLL_EXPORTS
#define WIN32DLL_EXPORT __declspec(dllexport)
#else
#define WIN32DLL_EXPORT __declspec(dllimport)
#endif
#else
#define WIN32DLL_EXPORT
#endif

namespace tween {
class WIN32DLL_EXPORT Easing {
 public:
  Easing() {}
  virtual ~Easing() {}

  // pure virtual
  virtual float easeIn(float t, float b, float c, float d)=0;
  virtual float easeOut(float t, float b, float c, float d)=0;
  virtual float easeInOut(float t, float b, float c, float d)=0;
};

class WIN32DLL_EXPORT Back : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Bounce : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Circ : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Cubic : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Elastic : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Expo : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Quad : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};


class WIN32DLL_EXPORT Quart : public Easing {
 public:
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Quint : public Easing {
 public :
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Sine : public Easing {
 public :
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};

class WIN32DLL_EXPORT Linear : public Easing {
 public :
  float easeNone(float t, float b, float c, float d);
  float easeIn(float t, float b, float c, float d);
  float easeOut(float t, float b, float c, float d);
  float easeInOut(float t, float b, float c, float d);
};


enum {
  LINEAR,
  SINE,
  QUINT,
  QUART,
  QUAD,
  EXPO,
  ELASTIC,
  CUBIC,
  CIRC,
  BOUNCE,
  BACK
};

enum {
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT
};

static Linear fLinear;
static Sine fSine;
static Quint fQuint;
static Quart fQuart;
static Quad  fQuad;
static Expo fExpo;
static Elastic fElastic;
static Cubic fCubic;
static Circ fCirc;
static Bounce fBounce;
static Back fBack;

struct TweenerProperty {
  float *ptrValue;
  float finalValue;
  float initialValue;
};


class TweenerListener;  // Forward declaration
class WIN32DLL_EXPORT TweenerParam {
 public:
  std::vector<TweenerProperty>  properties;
  float time;
  short transition;
  short equation;
  float delay;
  float timeCount;
  int total_properties;
  bool useMilliSeconds;
  bool started;
  TweenerListener* listener;

  TweenerParam();
  ~TweenerParam();

  TweenerParam(float ptime, short ptransition = EXPO, short pequation = EASE_OUT);

  void addProperty(float *valor, float valorFinal);
  void setListener(TweenerListener* theListener);
  void setUseMilliSeconds(bool use);
  void cleanProperties();
  bool operator==(const TweenerParam& p);
};


class WIN32DLL_EXPORT TweenerListener {
 public:
  TweenerListener() {}
  virtual ~TweenerListener() {}
  virtual void onStart(const TweenerParam& param) = 0;
  virtual void onStep(const TweenerParam& param) = 0;
  virtual void onComplete(const TweenerParam& param) = 0;
  bool operator==(const TweenerListener& l) {
    return (this == &l);
  };
};


class WIN32DLL_EXPORT Tweener {
 public:
  static Tweener* singleton();

  void addTween(TweenerParam* param);
  void removeTween(TweenerParam  *param);
  void setFunction(short funcEnum);
  void step(long currentMillis);

 protected :
  explicit Tweener();
  static Tweener* singleton_;

  enum {ON_START, ON_STEP, ON_COMPLETE};
  short currentFunction;
  Easing *funcs[11];
  long lastTime;

  std::list<TweenerParam*> tweens;
  std::list<TweenerParam*>::iterator tweensIT;
  int total_tweens;

  float runEquation(int transition, int equation, float t, float b, float c, float d);
  void dispatchEvent(TweenerParam *param, short eventType);
};

}  // namespace Tween
#endif

