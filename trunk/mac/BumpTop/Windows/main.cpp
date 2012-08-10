/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

//  Adapted from code by Raymond Chen
// (http://blogs.msdn.com/oldnewthing/archive/2005/03/02/383562.aspx)
//

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>

#include "BumpTop/Windows/WindowsBumpTopApplication.h"
#include "BumpTop/BumpTopScene.h"

HINSTANCE g_hinst;

class Window {
 public:
  HWND GetHWND() {
    return m_hwnd;
  }
 protected:
  virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual void PaintContent(PAINTSTRUCT *pps) {
  }
  virtual LPCTSTR ClassName() = 0;
  virtual BOOL WinRegisterClass(WNDCLASS *pwc) {
    return RegisterClass(pwc);
  }
  virtual ~Window() {
  }

  HWND WinCreateWindow(DWORD dwExStyle, LPCTSTR pszName,
    DWORD dwStyle, int x, int y, int cx, int cy,
    HWND hwndParent, HMENU hmenu) {
      Register();
      return CreateWindowEx(dwExStyle, ClassName(), pszName, dwStyle,
        x, y, cx, cy, hwndParent, hmenu, g_hinst, this);
  }
 protected:
  void Register();
  virtual void OnPaint();
  void OnPrintClient(HDC hdc);
  static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  HWND m_hwnd;
};

void Window::Register() {
  WNDCLASS wc;
  wc.style         = 0;
  wc.lpfnWndProc   = Window::s_WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = g_hinst;
  wc.hIcon         = NULL;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = ClassName();

  WinRegisterClass(&wc);
}

LRESULT CALLBACK Window::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Window *self;
  if (uMsg == WM_NCCREATE) {
    LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
    self = reinterpret_cast<Window *>(lpcs->lpCreateParams);
    self->m_hwnd = hwnd;
    SetWindowLongPtr(hwnd, GWLP_USERDATA,
      reinterpret_cast<LPARAM>(self));
  } else {
    self = reinterpret_cast<Window *>
      (GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }
  if (self) {
    return self->HandleMessage(uMsg, wParam, lParam);
  } else {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT lres;

  switch (uMsg) {
    case WM_NCDESTROY:
      lres = DefWindowProc(m_hwnd, uMsg, wParam, lParam);
      SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
      delete this;
      return lres;

    case WM_PAINT:
      OnPaint();
      return 0;

    case WM_PRINTCLIENT:
      OnPrintClient(reinterpret_cast<HDC>(wParam));
      return 0;
  }

  return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void Window::OnPaint() {
  PAINTSTRUCT ps;
  BeginPaint(m_hwnd, &ps);
  PaintContent(&ps);
  EndPaint(m_hwnd, &ps);
}

void Window::OnPrintClient(HDC hdc) {
  PAINTSTRUCT ps;
  ps.hdc = hdc;
  GetClientRect(m_hwnd, &ps.rcPaint);
  PaintContent(&ps);
}

class RootWindow : public Window {
 public:
  RootWindow();
  virtual LPCTSTR ClassName() {
    return TEXT("Scratch");
  }
  static RootWindow *Create();
 protected:
  LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
  LRESULT OnCreate();
  virtual void OnPaint();

  HWND m_hwndChild;
  BumpTopApplication *bumptop_application_;
};

RootWindow::RootWindow()
: bumptop_application_(NULL) {
}

LRESULT RootWindow::OnCreate() {
  bumptop_application_ = new WindowsBumpTopApplication(GetHWND());
  bumptop_application_->init();
  BumpTopScene *scene = new BumpTopScene(bumptop_application_);
  scene->init();
  return 0;
}

LRESULT RootWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CREATE:
      return OnCreate();

    case WM_NCDESTROY:
      // Death of the root window ends the thread
      PostQuitMessage(0);
      break;

    case WM_SIZE:
      if (m_hwndChild) {
        SetWindowPos(m_hwndChild, NULL, 0, 0,
          GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
          SWP_NOZORDER | SWP_NOACTIVATE);
      }
      return 0;

    case WM_SETFOCUS:
      if (m_hwndChild) {
        SetFocus(m_hwndChild);
      }
      return 0;
  }

  return __super::HandleMessage(uMsg, wParam, lParam);
}

void RootWindow::OnPaint() {
  if (bumptop_application_) {
    bumptop_application_->render_window()->update();
    bumptop_application_->renderTick();
  }
}

RootWindow *RootWindow::Create() {
  RootWindow *self = new RootWindow();
  if (self && self->WinCreateWindow(0,
    TEXT("Scratch"), WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL)) {
      return self;
  }
  delete self;
  return NULL;
}

int PASCAL WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int nShowCmd) {
  g_hinst = hinst;

  if (SUCCEEDED(CoInitialize(NULL))) {
    // InitCommonControls();

    RootWindow *prw = RootWindow::Create();
    if (prw) {
      ShowWindow(prw->GetHWND(), nShowCmd);
      MSG msg;
      while (GetMessage(&msg, NULL, 0, 0)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }
    }
    CoUninitialize();
  }

  return 0;
}
