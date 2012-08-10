/*****************
 
 This port was based in a inital code from Jesus Gollonet, him port Penners easing equations to C/C++:
 
 http://www.jesusgollonet.com/blog/2007/09/24/penner-easing-cpp/
 http://robertpenner.com/easing/
 
 I´m just make a better wrapper a litlle more OOP e put some callbacks like the original Tweener
 (http://code.google.com/p/tweener/)
 
 
 wesley.marques@gmail.com  - Wesley Ferreira Marques
 http://codevein.com
 
 
 **********************/
#include "CppTweener.h"

#include <algorithm>

#define MIN(x, y) (x < y) ? x : y

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

namespace tween {
  Tweener* Tweener::singleton_ = NULL;

  /******************/
  /***** Tweener ****/
  /******************/
  Tweener::Tweener() {
    lastTime = -1;
    this->funcs[LINEAR] = &fLinear;
    this->funcs[SINE]  = &fSine;
    this->funcs[QUINT] = &fQuint;
    this->funcs[QUART] = &fQuart;
    this->funcs[QUAD] = &fQuad;
    this->funcs[EXPO] = &fExpo;
    this->funcs[ELASTIC] = &fElastic;
    this->funcs[CUBIC] = &fCubic;
    this->funcs[CIRC] =  &fCirc;
    this->funcs[BOUNCE] =  &fBounce;
    this->funcs[BACK] =  &fBack;
  }

  Tweener* Tweener::singleton() {
    if (singleton_ == NULL) {
      singleton_ = new Tweener();
    }
    return singleton_;
  }

  float Tweener::runEquation(int transition, int equation, float t, float b, float c, float d) {
    float result;
    if (equation == EASE_IN) {
      result = funcs[transition]->easeIn(t, b, c, d);
    } else if (equation == EASE_OUT) {
      result = funcs[transition]->easeOut(t, b, c, d);
    } else if (equation == EASE_IN_OUT) {
      result = funcs[transition]->easeInOut(t, b, c, d);
    }

    return result;
  }

  void Tweener::dispatchEvent(TweenerParam *param, short eventType) {
    TweenerListener *listener = param->listener;

    if (listener != NULL)  {
      switch (eventType) {
        case ON_START:
          listener->onStart(*param);
          break;
        case ON_STEP:
          listener->onStep(*param);
          break;
        case ON_COMPLETE:
          listener->onComplete(*param);
          break;
        default:
          break;
      }
    }
  }

  void Tweener::removeTween(TweenerParam  *param) {
    std::list<TweenerParam*>::iterator items_to_erase = std::remove(tweens.begin(), tweens.end(), param);
    tweens.erase(items_to_erase, tweens.end());
  }

  void Tweener::addTween(TweenerParam* param) {
    param->timeCount = 0;

    for (int i =0; i < param->total_properties; i++) {
      TweenerProperty prop = param->properties[i];
      param->properties[i].initialValue = *(prop.ptrValue);
    }
    // std::cout<<" \nParam: props"<< (param).total_properties  << " time" << (param).time;

    tweens.push_back(param);
    total_tweens = tweens.size();
  }

  void Tweener::setFunction(short funcEnum) {
    if (funcEnum > -1 && funcEnum <=11) {
      currentFunction = funcEnum;
    }
  }

  void Tweener::step(long currentMillis) {
    total_tweens = tweens.size();
    int t = 0;
    int d = 0;
    if (lastTime == -1) lastTime = currentMillis;
    int  dif = (currentMillis - lastTime);

    std::list<TweenerParam*> tweens_to_delete;
    
    for (tweensIT = tweens.begin(); tweensIT != tweens.end(); tweensIT++) {
      TweenerParam* tween = *tweensIT;
      if (!tween->started) {
        dispatchEvent(tween, ON_START);
        tween->started = true;
      }

      dispatchEvent(tween, ON_STEP);

      if (tween->useMilliSeconds == true) {
        // We have found that the message loop, for some reason, pauses ~280 ms after we invoke context menu
        // When that happens, an animation we start immediately before the pause just snaps to the end
        // Instead, as a special exception, if an animation has NOT started yet (ie timeCount == 0), then 
        // on its first step we only allow it to increment a max of 20ms. This effectively delays the start of the 
        // animation, but doesn't cause it to look strange; in fact, it looks nicer because it's no longer snapping
        if (tween->timeCount == 0)
          tween->timeCount += MIN(20, dif);
        else
          tween->timeCount += dif;
        t = tween->timeCount;
      } else {
        tween->timeCount++;
        t = tween->timeCount;
      }

      d = tween->time;
      if (t < d  && tween->total_properties > 0) {
        if (tween->timeCount < tween->time) {
          for (unsigned int i =0; i < tween->total_properties; i++) {
            TweenerProperty prop = tween->properties[i];
            if (prop.ptrValue != NULL) {
              float  res   = runEquation(tween->transition,
                                         tween->equation,
                                         t,
                                         prop.initialValue,
                                         (prop.finalValue - prop.initialValue),
                                         d);

              *(prop.ptrValue) = res;
            }
          }
        }
      } else {
        // When the tween is complete we snap all properties to their final values
        TweenerProperty prop;
        for (unsigned int i =0; i < tween->total_properties; i++) {
          prop = tween->properties[i];
          if (prop.ptrValue != NULL) {
            *(prop.ptrValue) = prop.finalValue;
          }
        }
        tweens_to_delete.push_back(tween);
      }
    }

    std::list<TweenerParam*>::iterator tweens_to_delete_iterator;
    for (tweens_to_delete_iterator = tweens_to_delete.begin();
         tweens_to_delete_iterator != tweens_to_delete.end();
         tweens_to_delete_iterator++) {
      removeTween(*tweens_to_delete_iterator);

      dispatchEvent(*tweens_to_delete_iterator, ON_COMPLETE);
}

    lastTime = currentMillis;
  }


  TweenerParam::TweenerParam() {
    useMilliSeconds = true;
    timeCount = 0;
    started = false;
    listener = NULL;
  }

  TweenerParam::~TweenerParam() {
    properties.clear();
  }

  TweenerParam::TweenerParam(float ptime, short ptransition, short pequation) {
    time = ptime;
    transition = ptransition;
    equation = pequation;
    useMilliSeconds = true;
    timeCount = 0;
    started = false;
  }

  void TweenerParam::addProperty(float *valor, float valorFinal) {
    TweenerProperty prop = {valor, valorFinal, *valor};
    properties.push_back(prop);
    total_properties = properties.size();
  }

  void TweenerParam::setListener(TweenerListener* theListener) {
    listener = theListener;
  }

  void TweenerParam::setUseMilliSeconds(bool use) {
    useMilliSeconds = use;
  }

  void TweenerParam::cleanProperties() {
    properties.clear();
  }

  bool TweenerParam::operator==(const TweenerParam& p) {
    bool equal = false;
    if ((time == p.time) && (transition == p.transition) && (equation == p.equation)) {
      equal = true;
    }
    if (equal) {
      for (unsigned int i =0; i < p.total_properties; i++) {
        if (properties[i].initialValue != p.properties[i].initialValue ||
           properties[i].finalValue != p.properties[i].finalValue) {
          equal = false;
          break;
        }
      }
    }
    return equal;
  }


  /************/

  /***** LINEAR ****/
  float Linear::easeNone(float t, float b, float c, float d) {
    return c*t/d + b;
  }
  float Linear::easeIn(float t, float b, float c, float d) {
    return c*t/d + b;
  }
  float Linear::easeOut(float t, float b, float c, float d) {
    return c*t/d + b;
  }

  float Linear::easeInOut(float t, float b, float c, float d) {
    return c*t/d + b;
  }

  /***** SINE ****/

  float Sine::easeIn(float t, float b, float c, float d) {
    return -c * cos(t/d * (PI/2)) + c + b;
  }
  float Sine::easeOut(float t, float b, float c, float d) {
    return c * sin(t/d * (PI/2)) + b;
  }

  float Sine::easeInOut(float t, float b, float c, float d) {
    return -c/2 * (cos(PI*t/d) - 1) + b;
  }

  /**** Quint ****/

  float Quint::easeIn(float t, float b, float c, float d) {
    return c*(t/=d)*t*t*t*t + b;
  }
  float Quint::easeOut(float t, float b, float c, float d) {
    return c*((t = t/d-1)*t*t*t*t + 1) + b;
  }

  float Quint::easeInOut(float t, float b, float c, float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t*t*t + b;
    return c/2*((t-=2)*t*t*t*t + 2) + b;
  }

  /**** Quart ****/
  float Quart::easeIn(float t, float b, float c, float d) {
    return c*(t/=d)*t*t*t + b;
  }

  float Quart::easeOut(float t, float b, float c, float d) {
    return -c * ((t = t/d-1)*t*t*t - 1) + b;
  }

  float Quart::easeInOut(float t, float b, float c, float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t*t + b;
    return -c/2 * ((t-=2)*t*t*t - 2) + b;
  }

  /**** Quad ****/
  float Quad::easeIn(float t, float b, float c, float d) {
    return c*(t/=d)*t + b;
  }

  float Quad::easeOut(float t, float b, float c, float d) {
    return -c *(t/=d)*(t-2) + b;
  }

  float Quad::easeInOut(float t, float b, float c, float d) {
    if ((t/=d/2) < 1) return ((c/2)*(t*t)) + b;
    return -c/2 * (((t-2)*(--t)) - 1) + b;
    /*
     originally return -c/2 * (((t-2)*(--t)) - 1) + b;
 
     I've had to swap (--t)*(t-2) due to diffence in behaviour in
     pre-increment operators between java and c++, after hours
     of joy
     */
  }

  /**** Expo ****/

  float Expo::easeIn(float t, float b, float c, float d) {
    return (t == 0) ? b : c * pow(2, 10 * (t/d - 1)) + b;
  }
  float Expo::easeOut(float t, float b, float c, float d) {
    return (t == d) ? b+c : c * (-pow(2, -10 * t/d) + 1) + b;
  }

  float Expo::easeInOut(float t, float b, float c, float d) {
    if (t == 0) return b;
    if (t == d) return b+c;
    if ((t/=d/2) < 1) return c/2 * pow(2, 10 * (t - 1)) + b;
    return c/2 * (-pow(2, -10 * --t) + 2) + b;
  }


  /****  Elastic ****/

  float Elastic::easeIn(float t, float b, float c, float d) {
    if (t == 0)
      return b;
    if ((t/=d) == 1)
      return b+c;
    float p = d*.3f;
    float a = c;
    float s = p/4;
    float postFix = a*pow(2, 10*(t-=1));  // this is a fix, again, with post-increment operators
    return -(postFix * sin((t*d-s)*(2*PI)/p)) + b;
  }

  float Elastic::easeOut(float t, float b, float c, float d) {
    if (t == 0)
      return b;
    if ((t/=d) == 1)
      return b+c;
    float p = d*.3f;
    float a = c;
    float s = p/4;
    return (a*pow(2, -10*t) * sin((t*d-s)*(2*PI)/p) + c + b);
  }

  float Elastic::easeInOut(float t, float b, float c, float d) {
    if (t == 0)
      return b;
    if ((t/=d/2) == 2)
      return b+c;
    float p = d*(.3f*1.5f);
    float a = c;
    float s = p/4;

    if (t < 1) {
      float postFix =a*pow(2, 10*(t-=1));  // postIncrement is evil
      return -.5f*(postFix* sin((t*d-s)*(2*PI)/p)) + b;
    }
    float postFix = a*pow(2, -10*(t-=1));  // postIncrement is evil
    return postFix * sin((t*d-s)*(2*PI)/p)*.5f + c + b;
  }

  /****  Cubic ****/
  float Cubic::easeIn(float t, float b, float c, float d) {
    return c*(t/=d)*t*t + b;
  }
  float Cubic::easeOut(float t, float b, float c, float d) {
    return c*((t = t/d-1)*t*t + 1) + b;
  }

  float Cubic::easeInOut(float t, float b, float c, float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t + b;
    return c/2*((t-=2)*t*t + 2) + b;
  }

  /*** Circ ***/

  float Circ::easeIn(float t, float b, float c, float d) {
    return -c * (sqrt(1 - (t/=d)*t) - 1) + b;
  }

  float Circ::easeOut(float t, float b, float c, float d) {
    return c * sqrt(1 - (t = t/d-1)*t) + b;
  }

  float Circ::easeInOut(float t, float b, float c, float d) {
    if ((t/=d/2) < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
    return c/2 * (sqrt(1 - t*(t-=2)) + 1) + b;
  }

  /****  Bounce ****/
  float Bounce::easeIn(float t, float b, float c, float d) {
    return c - easeOut (d-t, 0, c, d) + b;
  }
  float Bounce::easeOut(float t, float b, float c, float d) {
    if ((t/=d) < (1/2.75f)) {
      return c*(7.5625f*t*t) + b;
    } else if (t < (2/2.75f)) {
      float postFix = t-=(1.5f/2.75f);
      return c*(7.5625f*(postFix)*t + .75f) + b;
    } else if (t < (2.5/2.75)) {
      float postFix = t-=(2.25f/2.75f);
      return c*(7.5625f*(postFix)*t + .9375f) + b;
    } else {
      float postFix = t-=(2.625f/2.75f);
      return c*(7.5625f*(postFix)*t + .984375f) + b;
    }
  }

  float Bounce::easeInOut(float t, float b, float c, float d) {
    if (t < d/2)
      return easeIn(t*2, 0, c, d) * .5f + b;
    else
      return easeOut (t*2-d, 0, c, d) * .5f + c*.5f + b;
  }

  /**** Back *****/

  float Back::easeIn(float t, float b, float c, float d) {
    float s = 1.70158f;
    float postFix = t/=d;
    return c*(postFix)*t*((s+1)*t - s) + b;
  }
  float Back::easeOut(float t, float b, float c, float d) {
    float s = 1.70158f;
    return c*((t = t/d-1)*t*((s+1)*t + s) + 1) + b;
  }

  float Back::easeInOut(float t, float b, float c, float d) {
    float s = 1.70158f;
    if ((t/=d/2) < 1) return c/2*(t*t*(((s*=(1.525f))+1)*t - s)) + b;
    float postFix = t-=2;
    return c/2*((postFix)*t*(((s*=(1.525f))+1)*t + s) + 2) + b;
  }
}
